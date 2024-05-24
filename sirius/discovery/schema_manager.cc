//
// Copyright (C) 2024 EA group inc.
// Author: Jeff.li lijippy@163.com
// All rights reserved.
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//


#include <sirius/discovery/schema_manager.h>
#include <sirius/discovery/zone_manager.h>
#include <sirius/discovery/servlet_manager.h>
#include <sirius/discovery/app_manager.h>
#include <sirius/storage/rocks_storage.h>
#include <sirius/base/scope_exit.h>
#include <melon/rpc/server.h>
#include <melon/raft/raft.h>

namespace sirius::discovery {

    void SchemaManager::process_schema_info(google::protobuf::RpcController *controller,
                                            const sirius::proto::DiscoveryManagerRequest *request,
                                            sirius::proto::DiscoveryManagerResponse *response,
                                            google::protobuf::Closure *done) {
        melon::ClosureGuard done_guard(done);
        if (!_discovery_state_machine->is_leader()) {
            if (response) {
                response->set_errcode(sirius::proto::NOT_LEADER);
                response->set_errmsg("not leader");
                response->set_leader(mutil::endpoint2str(_discovery_state_machine->get_leader()).c_str());
            }
            SS_LOG(WARN) << "discovery state machine is not leader, request: " << request->ShortDebugString();
            return;
        }
        uint64_t log_id = 0;
        melon::Controller *cntl = nullptr;
        if (controller != nullptr) {
            cntl = static_cast<melon::Controller *>(controller);
            if (cntl->has_log_id()) {
                log_id = cntl->log_id();
            }
        }
        ON_SCOPE_EXIT(([cntl, log_id, response]() {
            if (response != nullptr && response->errcode() != sirius::proto::SUCCESS) {
                const auto &remote_side_tmp = mutil::endpoint2str(cntl->remote_side());
                const char *remote_side = remote_side_tmp.c_str();
                SS_LOG(WARN) << "response error, remote_side:" << remote_side << ", log_id:" << log_id;
            }
        }));
        switch (request->op_type()) {
            case sirius::proto::OP_CREATE_NAMESPACE:
            case sirius::proto::OP_MODIFY_NAMESPACE:
            case sirius::proto::OP_DROP_NAMESPACE: {
                if (!request->has_app_info()) {
                    ERROR_SET_RESPONSE(response, sirius::proto::INPUT_PARAM_ERROR,
                                       "no namespace_info", request->op_type(), log_id);
                    return;

                }
                _discovery_state_machine->process(controller, request, response, done_guard.release());
                return;
                case sirius::proto::OP_CREATE_ZONE:
                case sirius::proto::OP_MODIFY_ZONE:
                case sirius::proto::OP_DROP_ZONE: {
                    if (!request->has_zone_info()) {
                        ERROR_SET_RESPONSE(response, sirius::proto::INPUT_PARAM_ERROR,
                                           "no zone_info", request->op_type(), log_id);
                        return;
                    }
                    _discovery_state_machine->process(controller, request, response, done_guard.release());
                    return;
                }
                case sirius::proto::OP_CREATE_SERVLET:
                case sirius::proto::OP_MODIFY_SERVLET:
                case sirius::proto::OP_DROP_SERVLET: {
                    if (!request->has_servlet_info()) {
                        ERROR_SET_RESPONSE(response, sirius::proto::INPUT_PARAM_ERROR,
                                           "no servlet info", request->op_type(), log_id);
                        return;
                    }
                    _discovery_state_machine->process(controller, request, response, done_guard.release());
                    return;
                }
            }

            default:
                ERROR_SET_RESPONSE(response, sirius::proto::INPUT_PARAM_ERROR,
                                   "invalid op_type", request->op_type(), log_id);
                return;
        }
    }


    int SchemaManager::check_and_get_for_privilege(sirius::proto::UserPrivilege &user_privilege) {
        std::string app_name = user_privilege.app_name();
        int64_t app_id = AppManager::get_instance()->get_app_id(app_name);
        if (app_id == 0) {
            SS_LOG(ERROR) << "namespace not exist, namespace:" << app_name
                          << ", request:" << user_privilege.ShortDebugString();
            return -1;
        }
        user_privilege.set_app_id(app_id);

        for (auto &pri_zone: *user_privilege.mutable_privilege_zone()) {
            std::string base_name = ZoneManager::make_zone_key(app_name, pri_zone.zone());
            int64_t zone_id = ZoneManager::get_instance()->get_zone_id(base_name);
            if (zone_id == 0) {
                SS_LOG(ERROR) << "zone not exist, zone:" << base_name
                              << ", namespace:" << app_name
                              << ", request:" << user_privilege.ShortDebugString();
                return -1;
            }
            pri_zone.set_zone_id(zone_id);
        }
        for (auto &pri_servlet: *user_privilege.mutable_privilege_servlet()) {
            std::string base_name = ZoneManager::make_zone_key(app_name, pri_servlet.zone());
            std::string servlet_name = ServletManager::make_servlet_key(base_name, pri_servlet.servlet_name());
            int64_t zone_id = ZoneManager::get_instance()->get_zone_id(base_name);
            if (zone_id == 0) {
                SS_LOG(ERROR) << "zone: " << base_name << " not exist, namespace: " << app_name
                              << ", request: " << user_privilege.ShortDebugString();
                return -1;
            }
            int64_t servlet_id = ServletManager::get_instance()->get_servlet_id(servlet_name);
            if (servlet_id == 0) {
                SS_LOG(ERROR) << "table_name: " << servlet_name << " not exist, zone: " << base_name
                              << ", namespace: " << app_name
                              << ", request: " << user_privilege.ShortDebugString();
                return -1;
            }
            pri_servlet.set_zone_id(zone_id);
            pri_servlet.set_servlet_id(servlet_id);
        }
        return 0;
    }

    int SchemaManager::load_snapshot() {
        SS_LOG(INFO) << "SchemaManager start load_snapshot";
        AppManager::get_instance()->clear();
        ZoneManager::get_instance()->clear();
        ServletManager::get_instance()->clear();
        //创建一个snapshot
        rocksdb::ReadOptions read_options;
        read_options.prefix_same_as_start = true;
        read_options.total_order_seek = false;
        RocksStorage *db = RocksStorage::get_instance();
        std::unique_ptr<rocksdb::Iterator> iter(
                RocksStorage::get_instance()->new_iterator(read_options, db->get_meta_info_handle()));
        iter->Seek(DiscoveryConstants::SCHEMA_IDENTIFY);
        std::string max_id_prefix = DiscoveryConstants::SCHEMA_IDENTIFY;
        max_id_prefix += DiscoveryConstants::MAX_ID_SCHEMA_IDENTIFY;

        std::string namespace_prefix = DiscoveryConstants::SCHEMA_IDENTIFY;
        namespace_prefix += DiscoveryConstants::APP_SCHEMA_IDENTIFY;


        std::string zone_prefix = DiscoveryConstants::SCHEMA_IDENTIFY;
        zone_prefix += DiscoveryConstants::ZONE_SCHEMA_IDENTIFY;

        std::string servlet_prefix = DiscoveryConstants::SCHEMA_IDENTIFY;
        servlet_prefix += DiscoveryConstants::SERVLET_SCHEMA_IDENTIFY;


        for (; iter->Valid(); iter->Next()) {
            int ret = 0;
            if (iter->key().starts_with(zone_prefix)) {
                ret = ZoneManager::get_instance()->load_zone_snapshot(iter->value().ToString());
            } else if (iter->key().starts_with(servlet_prefix)) {
                ret = ServletManager::get_instance()->load_servlet_snapshot(iter->value().ToString());
            } else if (iter->key().starts_with(namespace_prefix)) {
                ret = AppManager::get_instance()->load_app_snapshot(iter->value().ToString());
            } else if (iter->key().starts_with(max_id_prefix)) {
                ret = load_max_id_snapshot(max_id_prefix, iter->key().ToString(), iter->value().ToString());
            } else {
                SS_LOG(ERROR) << "unknown schema info when load snapshot, key:" << iter->key().data();
            }
            if (ret != 0) {
                SS_LOG(ERROR) << "load snapshot fail, key:" << iter->key().data()
                              << ", value:" << iter->value().data();
                return -1;
            }
        }
        SS_LOG(INFO) << "SchemaManager load_snapshot done...";
        return 0;
    }


    int SchemaManager::load_max_id_snapshot(const std::string &max_id_prefix,
                                            const std::string &key,
                                            const std::string &value) {
        std::string max_key(key, max_id_prefix.size());
        int64_t *max_id = (int64_t *) (value.c_str());
        if (max_key == DiscoveryConstants::MAX_APP_ID_KEY) {
            AppManager::get_instance()->set_max_app_id(*max_id);
            SS_LOG(WARN) << "max_app_id:" << *max_id;
            return 0;
        }
        if (max_key == DiscoveryConstants::MAX_ZONE_ID_KEY) {
            ZoneManager::get_instance()->set_max_zone_id(*max_id);;
            SS_LOG(WARN) << "max_zone_id:" << *max_id;
            return 0;
        }
        if (max_key == DiscoveryConstants::MAX_SERVLET_ID_KEY) {
            ServletManager::get_instance()->set_max_servlet_id(*max_id);
            SS_LOG(WARN) << "max_servlet_id:" << *max_id;
            return 0;
        }
        return 0;
    }

}  // namespace sirius::discovery
