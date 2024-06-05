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
#include <sirius/proto/discovery.interface.pb.h>
#include <sirius/base/fiber.h>

namespace sirius::discovery {

    class DiscoveryStateMachine;

    class AutoIncrStateMachine;

    class TSOStateMachine;

    class DiscoveryServer : public sirius::proto::DiscoveryService {
    public:
        ~DiscoveryServer() override;

        static DiscoveryServer *get_instance() {
            static DiscoveryServer _instance;
            return &_instance;
        }

        int init(const std::vector<melon::raft::PeerId> &peers);

        //schema control method
        void discovery_manager(google::protobuf::RpcController *controller,
                                  const sirius::proto::DiscoveryManagerRequest *request,
                                  sirius::proto::DiscoveryManagerResponse *response,
                                  google::protobuf::Closure *done) override;

        void discovery_query(google::protobuf::RpcController *controller,
                           const sirius::proto::DiscoveryQueryRequest *request,
                           sirius::proto::DiscoveryQueryResponse *response,
                           google::protobuf::Closure *done) override;

        void naming(google::protobuf::RpcController *controller,
                             const sirius::proto::ServletNamingRequest *request,
                             sirius::proto::ServletNamingResponse *response,
                             google::protobuf::Closure *done) override;

        //raft control method
        void raft_control(google::protobuf::RpcController *controller,
                                  const sirius::proto::RaftControlRequest *request,
                                  sirius::proto::RaftControlResponse *response,
                                  google::protobuf::Closure *done) override;


        void tso_service(google::protobuf::RpcController *controller,
                                 const sirius::proto::TsoRequest *request,
                                 sirius::proto::TsoResponse *response,
                                 google::protobuf::Closure *done) override;


        void flush_memtable_thread();

        void shutdown_raft();

        bool have_data();

        void close();

    private:
        DiscoveryServer() {}

        fiber::Mutex discovery_nteract_mutex;
        DiscoveryStateMachine *_discovery_state_machine = nullptr;
        AutoIncrStateMachine *_auto_incr_state_machine = nullptr;
        TSOStateMachine *_tso_state_machine = nullptr;
        Fiber _flush_bth;
        bool _init_success = false;
        bool _shutdown = false;
    }; //class

}  // namespace sirius::discovery
