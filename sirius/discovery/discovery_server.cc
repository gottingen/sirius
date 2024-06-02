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


#include <sirius/discovery/discovery_server.h>
#include <sirius/discovery/auto_incr_state_machine.h>
#include <sirius/discovery/tso_state_machine.h>
#include <sirius/discovery/discovery_state_machine.h>
#include <sirius/discovery/privilege_manager.h>
#include <sirius/discovery/schema_manager.h>
#include <sirius/discovery/config_manager.h>
#include <sirius/discovery/query_config_manager.h>
#include <sirius/discovery/query_privilege_manager.h>
#include <sirius/discovery/query_app_manager.h>
#include <sirius/discovery/query_zone_manager.h>
#include <sirius/discovery/query_servlet_manager.h>
#include <sirius/discovery/discovery_rocksdb.h>

namespace sirius::discovery {

    DiscoveryServer::~DiscoveryServer() {}

    int DiscoveryServer::init(const std::vector<melon::raft::PeerId> &peers) {
        auto ret = DiscoveryRocksdb::get_instance()->init();
        if (ret < 0) {
            LOG(ERROR) << "rocksdb init fail";
            return -1;
        }
        mutil::EndPoint addr;
        mutil::str2endpoint(FLAGS_sirius_listen.c_str(), &addr);
        //addr.ip = mutil::my_ip();
        //addr.port = FLAGS_discovery_port;
        melon::raft::PeerId peer_id(addr, 0);
        _discovery_state_machine = new(std::nothrow)DiscoveryStateMachine(peer_id);
        if (_discovery_state_machine == nullptr) {
            LOG(ERROR) << "new discovery_state_machine fail";
            return -1;
        }
        //state_machine初始化
        ret = _discovery_state_machine->init(peers);
        if (ret != 0) {
            LOG(ERROR) << "discovery state machine init fail";
            return -1;
        }
        LOG(WARNING) << "discovery state machine init success";

        _auto_incr_state_machine = new(std::nothrow)AutoIncrStateMachine(peer_id);
        if (_auto_incr_state_machine == nullptr) {
            LOG(ERROR) << "new auot_incr_state_machine fail";
            return -1;
        }
        ret = _auto_incr_state_machine->init(peers);
        if (ret != 0) {
            LOG(ERROR) << "auot_incr_state_machine init fail";
            return -1;
        }
        LOG(WARNING) << "auot_incr_state_machine init success";

        _tso_state_machine = new(std::nothrow)TSOStateMachine(peer_id);
        if (_tso_state_machine == nullptr) {
            LOG(ERROR) << "new _tso_state_machine fail";
            return -1;
        }
        ret = _tso_state_machine->init(peers);
        if (ret != 0) {
            LOG(ERROR) << " _tso_state_machine init fail";
            return -1;
        }
        LOG(INFO) << " tso state machine init success";

        SchemaManager::get_instance()->set_discovery_state_machine(_discovery_state_machine);
        ConfigManager::get_instance()->set_discovery_state_machine(_discovery_state_machine);
        PrivilegeManager::get_instance()->set_discovery_state_machine(_discovery_state_machine);
        _flush_bth.run([this]() { flush_memtable_thread(); });
        _init_success = true;
        return 0;
    }

    void DiscoveryServer::flush_memtable_thread() {
        while (!_shutdown) {
            fiber_usleep_fast_shutdown(FLAGS_flush_memtable_interval_us, _shutdown);
            if (_shutdown) {
                return;
            }
            auto rocksdb = RocksStorage::get_instance();
            rocksdb::FlushOptions flush_options;
            auto status = rocksdb->flush(flush_options, rocksdb->get_meta_info_handle());
            if (!status.ok()) {
                LOG(WARNING) << "flush discovery info to rocksdb fail, err_msg:" << status.ToString();
            }
            status = rocksdb->flush(flush_options, rocksdb->get_raft_log_handle());
            if (!status.ok()) {
                LOG(WARNING) << "flush log_cf to rocksdb fail, err_msg:" << status.ToString();
            }
        }
    }


    void DiscoveryServer::discovery_manager(google::protobuf::RpcController *controller,
                                  const sirius::proto::DiscoveryManagerRequest *request,
                                  sirius::proto::DiscoveryManagerResponse *response,
                                  google::protobuf::Closure *done) {
        melon::ClosureGuard done_guard(done);
        melon::Controller *cntl =
                static_cast<melon::Controller *>(controller);
        uint64_t log_id = 0;
        if (cntl->has_log_id()) {
            log_id = cntl->log_id();
        }
        RETURN_IF_NOT_INIT(_init_success, response, log_id);
        if (request->op_type() == sirius::proto::OP_CREATE_USER
            || request->op_type() == sirius::proto::OP_DROP_USER
            || request->op_type() == sirius::proto::OP_ADD_PRIVILEGE
            || request->op_type() == sirius::proto::OP_DROP_PRIVILEGE) {
            PrivilegeManager::get_instance()->process_user_privilege(controller,
                                                                     request,
                                                                     response,
                                                                     done_guard.release());
            return;
        }
        if (request->op_type() == sirius::proto::OP_CREATE_NAMESPACE
            || request->op_type() == sirius::proto::OP_DROP_NAMESPACE
            || request->op_type() == sirius::proto::OP_MODIFY_NAMESPACE
            || request->op_type() == sirius::proto::OP_CREATE_ZONE
            || request->op_type() == sirius::proto::OP_DROP_ZONE
            || request->op_type() == sirius::proto::OP_MODIFY_ZONE
            || request->op_type() == sirius::proto::OP_CREATE_SERVLET
            || request->op_type() == sirius::proto::OP_DROP_SERVLET
            || request->op_type() == sirius::proto::OP_MODIFY_SERVLET
            || request->op_type() == sirius::proto::OP_MODIFY_RESOURCE_TAG
            || request->op_type() == sirius::proto::OP_UPDATE_MAIN_LOGICAL_ROOM) {
            SchemaManager::get_instance()->process_schema_info(controller,
                                                               request,
                                                               response,
                                                               done_guard.release());
            return;
        }
        if(request->op_type() == sirius::proto::OP_CREATE_CONFIG
            ||request->op_type() == sirius::proto::OP_REMOVE_CONFIG) {
            ConfigManager::get_instance()->process_schema_info(controller,
                                                               request,
                                                               response,
                                                               done_guard.release());
            return;
        }
        if (request->op_type() == sirius::proto::OP_GEN_ID_FOR_AUTO_INCREMENT
            || request->op_type() == sirius::proto::OP_UPDATE_FOR_AUTO_INCREMENT
            || request->op_type() == sirius::proto::OP_ADD_ID_FOR_AUTO_INCREMENT
            || request->op_type() == sirius::proto::OP_DROP_ID_FOR_AUTO_INCREMENT) {
            _auto_incr_state_machine->process(controller,
                                              request,
                                              response,
                                              done_guard.release());
            return;
        }


        LOG(ERROR) << "request has wrong op_type:" << request->op_type() << ", log_id:" << log_id;
        response->set_errcode(sirius::proto::INPUT_PARAM_ERROR);
        response->set_errmsg("invalid op_type");
        response->set_op_type(request->op_type());
    }

    void DiscoveryServer::discovery_query(google::protobuf::RpcController *controller,
                           const sirius::proto::DiscoveryQueryRequest *request,
                           sirius::proto::DiscoveryQueryResponse *response,
                           google::protobuf::Closure *done) {
        melon::ClosureGuard done_guard(done);
        melon::Controller *cntl =
                static_cast<melon::Controller *>(controller);
        const auto &remote_side_tmp = mutil::endpoint2str(cntl->remote_side());
        const char *remote_side = remote_side_tmp.c_str();
        uint64_t log_id = 0;
        if (cntl->has_log_id()) {
            log_id = cntl->log_id();
        }
        RETURN_IF_NOT_INIT(_init_success, response, log_id);
        TimeCost time_cost;
        response->set_errcode(sirius::proto::SUCCESS);
        response->set_errmsg("success");
        switch (request->op_type()) {
            case sirius::proto::QUERY_USER_PRIVILEGE: {
                QueryPrivilegeManager::get_instance()->get_user_info(request, response);
                break;
            }
            case sirius::proto::QUERY_APP: {
                QueryAppManager::get_instance()->get_app_info(request, response);
                break;
            }
            case sirius::proto::QUERY_ZONE: {
                QueryZoneManager::get_instance()->get_zone_info(request, response);
                break;
            }
            case sirius::proto::QUERY_SERVLET: {
                QueryServletManager::get_instance()->get_servlet_info(request, response);
                break;
            }
            case sirius::proto::QUERY_GET_CONFIG: {
                QueryConfigManager::get_instance()->get_config(request, response);
                break;
            }
            case sirius::proto::QUERY_LIST_CONFIG: {
                QueryConfigManager::get_instance()->list_config(request, response);
                break;
            }
            case sirius::proto::QUERY_LIST_CONFIG_VERSION: {
                QueryConfigManager::get_instance()->list_config_version(request, response);
                break;
            }

            case sirius::proto::QUERY_PRIVILEGE_FLATTEN: {
                QueryPrivilegeManager::get_instance()->get_flatten_servlet_privilege(request, response);
                break;
            }

            default: {
                LOG(WARNING) << "invalid op_type, request: " << request->ShortDebugString() << ", log_id: " << log_id;
                response->set_errcode(sirius::proto::INPUT_PARAM_ERROR);
                response->set_errmsg("invalid op_type");
            }
        }
        LOG(INFO) << "query op_type_name:" << sirius::proto::QueryOpType_Name(request->op_type())
                     << ", time_cost:" << time_cost.get_time() << ", log_id:" << log_id
                     << ", ip:" << remote_side << ", request: " << request->ShortDebugString();
    }

    void DiscoveryServer::naming(google::protobuf::RpcController *controller,
                const sirius::proto::ServletNamingRequest *request,
                sirius::proto::ServletNamingResponse *response,
                google::protobuf::Closure *done) {
        melon::ClosureGuard done_guard(done);
        melon::Controller *cntl =
                static_cast<melon::Controller *>(controller);
        const auto &remote_side_tmp = mutil::endpoint2str(cntl->remote_side());
        //const char *remote_side = remote_side_tmp.c_str();
        uint64_t log_id = 0;
        if (cntl->has_log_id()) {
            log_id = cntl->log_id();
        }
        RETURN_IF_NOT_INIT(_init_success, response, log_id);
        auto * query_app_manager = QueryAppManager::get_instance();
        query_app_manager->naming(request, response);
    }

    void DiscoveryServer::raft_control(google::protobuf::RpcController *controller,
                                  const sirius::proto::RaftControlRequest *request,
                                  sirius::proto::RaftControlResponse *response,
                                  google::protobuf::Closure *done) {
        melon::ClosureGuard done_guard(done);
        if (request->region_id() == 0) {
            _discovery_state_machine->raft_control(controller, request, response, done_guard.release());
            return;
        }
        if (request->region_id() == 1) {
            _auto_incr_state_machine->raft_control(controller, request, response, done_guard.release());
            return;
        }
        if (request->region_id() == 2) {
            _tso_state_machine->raft_control(controller, request, response, done_guard.release());
            return;
        }
        response->set_region_id(request->region_id());
        response->set_errcode(sirius::proto::INPUT_PARAM_ERROR);
        response->set_errmsg("unmatch region id");
        LOG(ERROR) << "unmatch region_id in discovery server, request: " << request->ShortDebugString();
    }


    void DiscoveryServer::tso_service(google::protobuf::RpcController *controller,
                                 const sirius::proto::TsoRequest *request,
                                 sirius::proto::TsoResponse *response,
                                 google::protobuf::Closure *done) {
        melon::ClosureGuard done_guard(done);
        melon::Controller *cntl =
                static_cast<melon::Controller *>(controller);
        uint64_t log_id = 0;
        if (cntl->has_log_id()) {
            log_id = cntl->log_id();
        }
        RETURN_IF_NOT_INIT(_init_success, response, log_id);
        if (_tso_state_machine != nullptr) {
            _tso_state_machine->process(controller, request, response, done_guard.release());
        }
    }

    void DiscoveryServer::shutdown_raft() {
        _shutdown = true;
        if (_discovery_state_machine != nullptr) {
            _discovery_state_machine->shutdown_raft();
        }
        if (_auto_incr_state_machine != nullptr) {
            _auto_incr_state_machine->shutdown_raft();
        }
        if (_tso_state_machine != nullptr) {
            _tso_state_machine->shutdown_raft();
        }
    }

    bool DiscoveryServer::have_data() {
        return _discovery_state_machine->have_data()
               && _auto_incr_state_machine->have_data()
               && _tso_state_machine->have_data();
    }

    void DiscoveryServer::close() {
        _flush_bth.join();
        LOG(INFO) << "DiscoveryServer flush joined";
    }

}  // namespace sirius::discovery
