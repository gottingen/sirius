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

#include <sirius/discovery/config_manager.h>
#include <sirius/discovery/base_state_machine.h>
#include <sirius/discovery/sirius_db.h>
#include <sirius/discovery/sirius_constants.h>
#include <sirius/base/scope_exit.h>

namespace sirius::discovery {

    collie::ModuleVersion ConfigManager::kDefaultVersion(0,0,1);

    void ConfigManager::process_schema_info(google::protobuf::RpcController *controller,
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
            LOG(WARNING) << "discovery state machine is not leader, request: " << request->ShortDebugString();
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
                LOG(WARNING) << "response error, remote_side:" << remote_side << ", log_id:" << log_id;
            }
        }));

        switch (request->op_type()) {
            case sirius::proto::OP_CREATE_CONFIG:
            case sirius::proto::OP_REMOVE_CONFIG:
                if(!request->has_config_info()) {
                    ERROR_SET_RESPONSE(response, sirius::proto::INPUT_PARAM_ERROR,
                                       "no config_info", request->op_type(), log_id);
                    return;
                }
                _discovery_state_machine->process(controller, request, response, done_guard.release());
                return;
            default:
                ERROR_SET_RESPONSE(response, sirius::proto::INPUT_PARAM_ERROR,
                                   "invalid op_type", request->op_type(), log_id);
                return;
        }

    }
    void ConfigManager::create_config(const ::sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done) {
        auto &create_request = request.config_info();
        auto &name = create_request.name();
        collie::ModuleVersion version = kDefaultVersion;

        if(create_request.has_version()) {
            version = collie::ModuleVersion(create_request.version().major(), create_request.version().minor(),
                    create_request.version().patch());
        }


        MELON_SCOPED_LOCK(_config_mutex);
        if (_configs.find(name) == _configs.end()) {
            _configs[name] = std::map<collie::ModuleVersion, sirius::proto::ConfigInfo>();
        }
        auto it = _configs.find(name);
        // do not rewrite.
        if (it->second.find(version) != it->second.end()) {
            /// already exists
            LOG(INFO) << "config : " << name << " version: " << version.to_string() << " exist";
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "config already exist");
            return;
        }
        if(!it->second.empty() && it->second.rbegin()->first >= version) {
            /// Version numbers must increase monotonically
            LOG(INFO) << "config : " << name << " version: " << version.to_string() << " must be larger than current: " << it->second.rbegin()->first.to_string();
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "Version numbers must increase monotonically");
            return;
        }
        std::string rocks_key = make_config_key(name, version);
        std::string rocks_value;
        auto tmp_request = create_request;
        tmp_request.set_time(time(nullptr));
        auto tmp_id = _max_config_id + 1;
        tmp_request.set_id(tmp_id);
        std::string max_config_id_value;
        max_config_id_value.append((char *) &tmp_id, sizeof(int64_t));
        auto max_config_id_key = construct_max_config_id_key();
        if (!tmp_request.SerializeToString(&rocks_value)) {
            IF_DONE_SET_RESPONSE(done, sirius::proto::PARSE_TO_PB_FAIL, "serializeToArray fail");
            return;
        }

        int ret = DiscoveryRocksdb::get_instance()->put_discovery_info({rocks_key,max_config_id_key}, {rocks_value, max_config_id_value});
        if (ret < 0) {
            IF_DONE_SET_RESPONSE(done, sirius::proto::INTERNAL_ERROR, "write db fail");
            return;
        }
        it->second[version] = tmp_request;
        _max_config_id = tmp_id;
        LOG(INFO) << "config : " << name << " version: " << version.to_string() << " create";
        IF_DONE_SET_RESPONSE(done, sirius::proto::SUCCESS, "success");
    }


    void ConfigManager::remove_config(const ::sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done) {
        auto &remove_request = request.config_info();
        auto &name = remove_request.name();
        bool remove_signal = remove_request.has_version();
        MELON_SCOPED_LOCK(_config_mutex);
        if (!remove_signal) {
            remove_config_all(request, done);
            return;
        }
        auto it = _configs.find(name);
        if (it == _configs.end()) {
            IF_DONE_SET_RESPONSE(done, sirius::proto::PARSE_TO_PB_FAIL, "config not exist");
            return;
        }
        collie::ModuleVersion version(remove_request.version().major(), remove_request.version().minor(),
                                     remove_request.version().patch());

        if (it->second.find(version) == it->second.end()) {
            /// not exists
            LOG(INFO) << "config : " << name << " version: " << version.to_string() << " not exist";
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "config not exist");
        }

        std::string rocks_key = make_config_key(name, version);
        int ret = DiscoveryRocksdb::get_instance()->remove_discovery_info(std::vector{rocks_key});
        if (ret < 0) {
            IF_DONE_SET_RESPONSE(done, sirius::proto::INTERNAL_ERROR, "delete from db fail");
            return;
        }
        it->second.erase(version);
        if(it->second.empty()) {
            _configs.erase(name);
        }
        IF_DONE_SET_RESPONSE(done, sirius::proto::SUCCESS, "success");
    }

    void ConfigManager::remove_config_all(const ::sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done) {
        auto &remove_request = request.config_info();
        auto &name = remove_request.name();
        auto it = _configs.find(name);
        if (it == _configs.end()) {
            IF_DONE_SET_RESPONSE(done, sirius::proto::PARSE_TO_PB_FAIL, "config not exist");
            return;
        }
        std::vector<std::string> del_keys;

        for(auto vit = it->second.begin(); vit != it->second.end(); ++vit) {
            std::string key = make_config_key(name, vit->first);
            del_keys.push_back(key);
        }

        int ret = DiscoveryRocksdb::get_instance()->remove_discovery_info(del_keys);
        if (ret < 0) {
            IF_DONE_SET_RESPONSE(done, sirius::proto::INTERNAL_ERROR, "delete from db fail");
            return;
        }
        _configs.erase(name);
        IF_DONE_SET_RESPONSE(done, sirius::proto::SUCCESS, "success");
    }

    int ConfigManager::load_snapshot() {
        MELON_SCOPED_LOCK( ConfigManager::get_instance()->_config_mutex);
        LOG(INFO) << "start to load config snapshot";
        _configs.clear();
        std::string config_prefix = DiscoveryConstants::CONFIG_IDENTIFY;
        mizar::ReadOptions read_options;
        read_options.prefix_same_as_start = true;
        read_options.total_order_seek = false;
        RocksStorage *db = RocksStorage::get_instance();
        std::unique_ptr<mizar::Iterator> iter(
                db->new_iterator(read_options, db->get_meta_info_handle()));
        iter->Seek(config_prefix);
        std::string config_content_prefix = DiscoveryConstants::CONFIG_IDENTIFY + DiscoveryConstants::CONFIG_CONTENT_IDENTIFY;
        std::string config_max_id_prefix = DiscoveryConstants::CONFIG_IDENTIFY + DiscoveryConstants::MAX_ID_SCHEMA_IDENTIFY;
        for (; iter->Valid(); iter->Next()) {
            if(iter->key().starts_with(config_content_prefix)) {
                if (load_config_snapshot(iter->value().ToString()) != 0) {
                    return -1;

                }
            } else {
                if (iter->key().starts_with(config_max_id_prefix)) {
                    if (iter->value().size() != sizeof(int64_t)) {
                        LOG(ERROR) << "config max id value size error";
                        return -1;
                    }
                    int64_t max_id = 0;
                    memcpy(&max_id, iter->value().data(), sizeof(int64_t));
                    _max_config_id = max_id;
                }

            }
        }
        LOG(INFO) << "load config snapshot done";
        return 0;
    }

    int ConfigManager::load_config_snapshot(const std::string &value) {
        sirius::proto::ConfigInfo config_pb;
        if (!config_pb.ParseFromString(value)) {
            LOG(ERROR) << "parse from pb fail when load config snapshot, key:" << value;
            return -1;
        }
        ///TLOG_INFO("load config:{}", config_pb.name());
        if(_configs.find(config_pb.name()) == _configs.end()) {
            _configs[config_pb.name()] = std::map<collie::ModuleVersion, sirius::proto::ConfigInfo>();
        }
        auto it = _configs.find(config_pb.name());
        collie::ModuleVersion version(config_pb.version().major(), config_pb.version().minor(),
                                     config_pb.version().patch());
        it->second[version] = config_pb;
        return 0;
    }

    std::string ConfigManager::make_config_key(const std::string &name, const collie::ModuleVersion &version) {
        return DiscoveryConstants::CONFIG_IDENTIFY + DiscoveryConstants::CONFIG_CONTENT_IDENTIFY + name + version.to_string();
    }

}  // namespace sirius::discovery