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


#pragma once

#include <melon/raft/raft.h>
#include <melon/rpc/server.h>
#include <sirius/discovery/raft_control.h>
#include <sirius/proto/discovery.interface.pb.h>
#include <sirius/base/fiber.h>
#include <sirius/base/time_cast.h>

namespace sirius::discovery {
    class BaseStateMachine;

struct DiscoveryServerClosure : public melon::raft::Closure {
        void Run() override;

        melon::Controller *cntl;
        BaseStateMachine *common_state_machine;
        google::protobuf::Closure *done;
        sirius::proto::DiscoveryManagerResponse *response;
        std::string request;
        int64_t raft_time_cost;
        int64_t total_time_cost;
        TimeCost time_cost;
    };

    struct TsoClosure : public melon::raft::Closure {
        TsoClosure() : sync_cond(nullptr) {};

        TsoClosure(FiberCond *cond) : sync_cond(cond) {};

        virtual void Run();

        melon::Controller *cntl;
        BaseStateMachine *common_state_machine;
        google::protobuf::Closure *done;
        sirius::proto::TsoResponse *response;
        int64_t raft_time_cost;
        int64_t total_time_cost;
        TimeCost time_cost;
        bool is_sync = false;
        FiberCond *sync_cond;
    };

    struct ApplyraftClosure : public google::protobuf::Closure {
        virtual void Run() {
            cond.decrease_signal();
            delete this;
        }

        ApplyraftClosure(FiberCond &cond) : cond(cond) {}

        FiberCond &cond;
    };

    class BaseStateMachine : public melon::raft::StateMachine {
    public:

        BaseStateMachine(int64_t dummy_region_id,
                         const std::string &identify,
                         const std::string &file_path,
                         const melon::raft::PeerId &peerId) :
                _node(identify, peerId),
                _is_leader(false),
                _dummy_region_id(dummy_region_id),
                _file_path(file_path) {}

        virtual ~BaseStateMachine() {}

        virtual int init(const std::vector<melon::raft::PeerId> &peers);

        virtual void raft_control(google::protobuf::RpcController *controller,
                                  const sirius::proto::RaftControlRequest *request,
                                  sirius::proto::RaftControlResponse *response,
                                  google::protobuf::Closure *done) {
            melon::ClosureGuard done_guard(done);
            if (!is_leader() && !request->force()) {
                SS_LOG(INFO) << "node is not leader when raft control, region_id: " << request->region_id();
                response->set_errcode(sirius::proto::NOT_LEADER);
                response->set_region_id(request->region_id());
                response->set_leader(mutil::endpoint2str(_node.leader_id().addr).c_str());
                response->set_errmsg("not leader");
                return;
            }
            common_raft_control(controller, request, response, done_guard.release(), &_node);
        }

        virtual void process(google::protobuf::RpcController *controller,
                             const sirius::proto::DiscoveryManagerRequest *request,
                             sirius::proto::DiscoveryManagerResponse *response,
                             google::protobuf::Closure *done);

        // state machine method
        virtual void on_apply(melon::raft::Iterator &iter) = 0;

        virtual void on_shutdown() {
            SS_LOG(INFO) << "raft is shut down";
        };

        virtual void on_snapshot_save(melon::raft::SnapshotWriter *writer, melon::raft::Closure *done) = 0;

        virtual int on_snapshot_load(melon::raft::SnapshotReader *reader) = 0;

        virtual void on_leader_start();

        virtual void on_leader_start(int64_t term);

        virtual void on_leader_stop();

        virtual void on_leader_stop(const mutil::Status &status);

        virtual void on_error(const ::melon::raft::Error &e);

        virtual void on_configuration_committed(const ::melon::raft::Configuration &conf);

        virtual mutil::EndPoint get_leader() {
            return _node.leader_id().addr;
        }

        virtual void shutdown_raft() {
            _node.shutdown(nullptr);
            SS_LOG(INFO) << "raft node was shutdown";
            _node.join();
            SS_LOG(INFO) << "raft node join completely";
        }

        virtual bool is_leader() const{
            return _is_leader;
        }

        bool have_data() {
            return _have_data;
        }

        void set_have_data(bool flag) {
            _have_data = flag;
        }

    protected:
        melon::raft::Node _node;
        std::atomic<bool> _is_leader;
        int64_t _dummy_region_id;
        std::string _file_path;
    private:
        bool _have_data = false;
    };

#define ERROR_SET_RESPONSE(response, errcode, err_message, op_type, log_id) \
    do {\
        SS_LOG(ERROR)<<"request op_type: "<<op_type<<", "<<err_message<<",log_id:"<<log_id;\
        if (response != nullptr) {\
            response->set_errcode(errcode);\
            response->set_errmsg(err_message);\
            response->set_op_type(op_type);\
        }\
    }while (0);

#define ERROR_SET_RESPONSE_WARN(response, errcode, err_message, op_type, log_id) \
    do {\
        SS_LOG(WARN)<<"request op_type: "<<op_type<<", "<<err_message<<",log_id:"<<log_id;\
        if (response != nullptr) {\
            response->set_errcode(errcode);\
            response->set_errmsg(err_message);\
            response->set_op_type(op_type);\
        }\
    }while (0);

#define IF_DONE_SET_RESPONSE(done, errcode, err_message) \
    do {\
        if (done && ((DiscoveryServerClosure*)done)->response) {\
            ((DiscoveryServerClosure*)done)->response->set_errcode(errcode);\
            ((DiscoveryServerClosure*)done)->response->set_errmsg(err_message);\
        }\
    }while (0);

#define SET_RESPONSE(response, errcode, err_message) \
    do {\
        if (response) {\
            response->set_errcode(errcode);\
            response->set_errmsg(err_message);\
        }\
    }while (0);

#define RETURN_IF_NOT_INIT(init, response, log_id) \
    do {\
        if (!init) {\
            SS_LOG(WARN)<<"have not init, log_id: "<<  log_id;\
            response->set_errcode(sirius::proto::HAVE_NOT_INIT);\
            response->set_errmsg("have not init");\
            return;\
        }\
    } while (0);

}  // namespace sirius::discovery

