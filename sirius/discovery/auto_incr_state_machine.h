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

#include <unordered_map>
#include <sirius/discovery/base_state_machine.h>
#include <sirius/discovery/sirius_constants.h>

namespace sirius::discovery {
    class AutoIncrStateMachine : public BaseStateMachine {
    public:

        explicit AutoIncrStateMachine(const melon::raft::PeerId &peerId) :
                BaseStateMachine(DiscoveryConstants::AutoIDMachineRegion, "auto_incr_raft", "/auto_incr", peerId) {}

        ~AutoIncrStateMachine() override = default;

        /// state machine method override
        void on_apply(melon::raft::Iterator &iter) override;

        ///
        /// \brief servlet inc id initialize
        /// \param request [in]
        /// \param done [out]
        void add_servlet_id(const sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done);

        ///
        /// \brief servlet inc id removing
        /// \param request [in]
        /// \param done [out]
        void drop_servlet_id(const sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done);

        ///
        /// \brief gen a servlet inc id by given count in request
        /// \param request [in]
        /// \param done [out]
        void gen_id(const sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done);

        ///
        /// \brief reset a servlet inc by start_id or increment_id, if backwards,
        ///        increment_info.force() should be enabled.
        /// \param request [in]
        /// \param done [out]
        void update(const sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done);

        ///
        /// \brief override BaseStateMachine::on_snapshot_save
        /// \param writer   braft snapshot writer.
        /// \param done
        void on_snapshot_save(melon::raft::SnapshotWriter *writer, melon::raft::Closure *done) override;

        ///
        /// \brief override BaseStateMachine::on_snapshot_load
        /// \param writer   braft snapshot writer.
        /// \param done
        int on_snapshot_load(melon::raft::SnapshotReader *reader)  override;

    private:
        void save_auto_increment(std::string &max_id_string);

        void save_snapshot(melon::raft::Closure *done,
                           melon::raft::SnapshotWriter *writer,
                           std::string max_id_string);
        ///
        /// \brief load json servlet_id --> max_id from json file
        /// \param max_id_file
        /// \return

        int load_auto_increment(const std::string &max_id_file);

        ///
        /// \param json_string
        /// \return
        int parse_json_string(const std::string &json_string);

        std::unordered_map<int64_t, uint64_t> _auto_increment_map;
    };

} //namespace sirius::discovery

