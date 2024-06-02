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


#include <sirius/discovery/base_state_machine.h>
#include <sirius/flags/sirius.h>

namespace sirius::discovery {

    void DiscoveryServerClosure::Run() {
        if (!status().ok()) {
            if (response) {
                response->set_errcode(sirius::proto::NOT_LEADER);
                response->set_leader(mutil::endpoint2str(common_state_machine->get_leader()).c_str());
            }
            LOG(ERROR) << "discovery server closure fail, error_code:" << status().error_code() << ", error_mas:"
                          << status().error_cstr();
        }
        total_time_cost = time_cost.get_time();
        std::string remote_side;
        if (cntl != nullptr) {
            remote_side = mutil::endpoint2str(cntl->remote_side()).c_str();
        }

        if (response != nullptr && response->op_type() != sirius::proto::OP_GEN_ID_FOR_AUTO_INCREMENT) {
            LOG(INFO)
            << "request:" << request << ", response:" << response->ShortDebugString() << ", raft_time_cost:["
            << raft_time_cost << "], total_time_cost:[" << total_time_cost << "], remote_side:[" << remote_side << "]";
        }
        if (done != nullptr) {
            done->Run();
        }
        delete this;
    }

    void TsoClosure::Run() {
        if (!status().ok()) {
            if (response) {
                response->set_errcode(sirius::proto::NOT_LEADER);
                response->set_leader(mutil::endpoint2str(common_state_machine->get_leader()).c_str());
            }
            LOG(ERROR) << "discovery server closure fail, error_code:" << status().error_code() << ", error_mas:"
                          << status().error_cstr();
        }
        if (sync_cond) {
            sync_cond->decrease_signal();
        }
        if (done != nullptr) {
            done->Run();
        }
        delete this;
    }

    int BaseStateMachine::init(const std::vector<melon::raft::PeerId> &peers) {
        melon::raft::NodeOptions options;
        options.election_timeout_ms = FLAGS_sirius_election_timeout_ms;
        options.fsm = this;
        options.initial_conf = melon::raft::Configuration(peers);
        options.snapshot_interval_s = FLAGS_sirius_snapshot_interval_s;
        options.log_uri = FLAGS_sirius_log_uri + std::to_string(_dummy_region_id);
        options.raft_meta_uri = FLAGS_sirius_stable_uri + _file_path;
        options.snapshot_uri = FLAGS_sirius_snapshot_uri + _file_path;
        int ret = _node.init(options);
        if (ret < 0) {
            LOG(ERROR) << "raft node init fail";
            return ret;
        }
        LOG(INFO) << "raft init success, meat state machine init success";
        return 0;
    }

    void BaseStateMachine::process(google::protobuf::RpcController *controller,
                                   const sirius::proto::DiscoveryManagerRequest *request,
                                   sirius::proto::DiscoveryManagerResponse *response,
                                   google::protobuf::Closure *done) {
        melon::ClosureGuard done_guard(done);
        if (!_is_leader) {
            if (response) {
                response->set_errcode(sirius::proto::NOT_LEADER);
                response->set_errmsg("not leader");
                response->set_leader(mutil::endpoint2str(_node.leader_id().addr).c_str());
            }
            LOG(WARNING) << "state machine not leader, request: " << request->ShortDebugString();
            return;
        }
        melon::Controller *cntl =
                static_cast<melon::Controller *>(controller);
        mutil::IOBuf data;
        mutil::IOBufAsZeroCopyOutputStream wrapper(&data);
        if (!request->SerializeToZeroCopyStream(&wrapper) && cntl) {
            cntl->SetFailed(melon::EREQUEST, "Fail to serialize request");
            return;
        }
        DiscoveryServerClosure *closure = new DiscoveryServerClosure;
        closure->request = request->ShortDebugString();
        closure->cntl = cntl;
        closure->response = response;
        closure->done = done_guard.release();
        closure->common_state_machine = this;
        melon::raft::Task task;
        task.data = &data;
        task.done = closure;
        _node.apply(task);
    }

    void BaseStateMachine::on_leader_start() {
        _is_leader.store(true);
    }

    void BaseStateMachine::on_leader_start(int64_t term) {
        LOG(INFO) << "leader start at term: " << term;
        on_leader_start();
    }

    void BaseStateMachine::on_leader_stop() {
        _is_leader.store(false);
        LOG(INFO) << "leader stop";
    }

    void BaseStateMachine::on_leader_stop(const mutil::Status &status) {
        LOG(INFO) << "leader stop, error_code:" << status.error_code() << ", error_des:" << status.error_cstr();
        on_leader_stop();
    }

    void BaseStateMachine::on_error(const ::melon::raft::Error &e) {
        LOG(ERROR) << "discovery state machine error, error_type:" << static_cast<int>(e.type())
                      << ", error_code:" << e.status().error_code() << ", error_des:" << e.status().error_cstr();
    }

    void BaseStateMachine::on_configuration_committed(const ::melon::raft::Configuration &conf) {
        std::string new_peer;
        for (auto iter = conf.begin(); iter != conf.end(); ++iter) {
            new_peer += iter->to_string() + ",";
        }
        LOG(INFO) << "new conf committed, new peer: " << new_peer;
    }

}  // namespace sirius::discovery
