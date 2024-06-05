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

#include <sirius/discovery/base_state_machine.h>
#include <melon/raft/repeated_timer_task.h>
#include <time.h>
#include <sirius/discovery/sirius_constants.h>

namespace sirius::discovery {

    class TSOStateMachine;

    class TsoTimer : public melon::raft::RepeatedTimerTask {
    public:
        TsoTimer() : _node(nullptr) {}

        virtual ~TsoTimer() {}

        int init(TSOStateMachine *node, int timeout_ms);

        virtual void run();

    protected:
        virtual void on_destroy() {}

        TSOStateMachine *_node;
    };

    struct TsoObj {
        sirius::proto::TsoTimestamp current_timestamp;
        int64_t last_save_physical;
    };

    class TSOStateMachine : public BaseStateMachine {
    public:
        TSOStateMachine(const melon::raft::PeerId &peerId) :
                BaseStateMachine(DiscoveryConstants::TsoMachineRegion, "tso_raft", "/tso", peerId) {
            fiber_mutex_init(&_tso_mutex, nullptr);
        }

        virtual ~TSOStateMachine() {
            _tso_update_timer.stop();
            _tso_update_timer.destroy();
            fiber_mutex_destroy(&_tso_mutex);
        }

        virtual int init(const std::vector<melon::raft::PeerId> &peers);

        // state machine method
        virtual void on_apply(melon::raft::Iterator &iter);

        void process(google::protobuf::RpcController *controller,
                     const sirius::proto::TsoRequest *request,
                     sirius::proto::TsoResponse *response,
                     google::protobuf::Closure *done);

        void gen_tso(const sirius::proto::TsoRequest *request, sirius::proto::TsoResponse *response);

        void reset_tso(const sirius::proto::TsoRequest &request, melon::raft::Closure *done);

        void update_tso(const sirius::proto::TsoRequest &request, melon::raft::Closure *done);

        int load_tso(const std::string &tso_file);

        int sync_timestamp(const sirius::proto::TsoTimestamp &current_timestamp, int64_t save_physical);

        void update_timestamp();

        virtual void on_snapshot_save(melon::raft::SnapshotWriter *writer, melon::raft::Closure *done);

        void save_snapshot(melon::raft::Closure *done,
                           melon::raft::SnapshotWriter *writer,
                           std::string sto_str);

        virtual int on_snapshot_load(melon::raft::SnapshotReader *reader);

        virtual void on_leader_start();

        virtual void on_leader_stop();

        static const std::string SNAPSHOT_TSO_FILE;;
        static const std::string SNAPSHOT_TSO_FILE_WITH_SLASH;

    private:
        TsoTimer _tso_update_timer;
        TsoObj _tso_obj;
        fiber_mutex_t _tso_mutex;  // 保护_tso_obj，C++20 atomic<std::shared_ptr<U>>
        bool _is_healty = true;
    };

}  // namespace sirius::discovery
