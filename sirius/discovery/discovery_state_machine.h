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

#include <rocksdb/db.h>
#include <sirius/discovery/base_state_machine.h>
#include <sirius/proto/discovery.interface.pb.h>
#include <sirius/flags/sirius.h>
#include <sirius/discovery/discovery_constants.h>

namespace sirius::discovery {


    class DiscoveryStateMachine : public BaseStateMachine {
    public:
        DiscoveryStateMachine(const melon::raft::PeerId &peerId) :
                BaseStateMachine(DiscoveryConstants::DiscoveryMachineRegion, FLAGS_sirius_raft_group, "/discovery_server", peerId) {
        }

        ~DiscoveryStateMachine() override = default;

        // state machine method
        void on_apply(melon::raft::Iterator &iter) override;

        void on_snapshot_save(melon::raft::SnapshotWriter *writer, melon::raft::Closure *done) override;

        int on_snapshot_load(melon::raft::SnapshotReader *reader) override;

        void on_leader_start() override;

        void on_leader_stop() override;

        int64_t applied_index() { return _applied_index; }

    private:
        void save_snapshot(melon::raft::Closure *done,
                           rocksdb::Iterator *iter,
                           melon::raft::SnapshotWriter *writer);

        int64_t _applied_index = 0;
    };

}  // namespace sirius::discovery

