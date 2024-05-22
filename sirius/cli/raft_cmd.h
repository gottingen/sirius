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

//
// Created by jeff on 23-11-29.
//

#pragma once

#include <collie/cli/cli.h>
#include <sirius/proto/discovery.interface.pb.h>
#include <collie/table/table.h>
#include <collie/utility/result.h>
#include <sirius/client/discovery_sender.h>
#include <string>

namespace sirius::cli {

    struct RaftOptionContext {
        static RaftOptionContext *get_instance() {
            static RaftOptionContext ins;
            return &ins;
        }

        // for config
        std::string raft_group;
        std::string opt_peer;
        std::string cluster;
        std::string new_leader;
        int64_t vote_time_ms;
        std::vector<std::string> old_peers;
        std::vector<std::string> new_peers;
        sirius::client::DiscoverySender sender;
        std::string discovery_server;
        bool force{false};
    };

    struct RaftCmd {

        static void setup_raft_cmd(collie::App &app);

        static void run_raft_cmd(collie::App *app);

        static void run_status_cmd();

        static void run_snapshot_cmd();

        static void run_vote_cmd();

        static void run_shutdown_cmd();

        static void run_set_cmd();

        static void run_trans_cmd();

        static collie::Result<int> to_region_id();

        static collie::table::Table show_raft_result(sirius::proto::RaftControlResponse &res);
    };

}  // namespace sirius::cli
