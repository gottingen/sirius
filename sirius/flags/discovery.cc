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

#include <sirius/flags/discovery.h>
#include <gflags/gflags.h>

namespace sirius {

    /// for discovery
    DEFINE_string(discovery_server_peers, "127.0.0.1:8010", "discovery server peers");
    DEFINE_int32(discovery_replica_number, 3, "Meta replica num");
    DEFINE_int32(discovery_snapshot_interval_s, 600, "raft snapshot interval(s)");
    DEFINE_int32(discovery_election_timeout_ms, 1000, "raft election timeout(ms)");
    DEFINE_string(discovery_raft_group, "discovery_raft", "discovery raft group");
    DEFINE_string(discovery_log_uri, "local://./discovery/raft_log/", "raft log uri");
    DEFINE_string(discovery_stable_uri, "local://./discovery/raft_data/stable", "raft stable path");
    DEFINE_string(discovery_snapshot_uri, "local://./discovery/raft_data/snapshot", "raft snapshot path");
    DEFINE_int64(discovery_check_migrate_interval_us, 60 * 1000 * 1000LL, "check discovery server migrate interval (60s)");
    DEFINE_int32(discovery_tso_snapshot_interval_s, 60, "tso raft snapshot interval(s)");
    DEFINE_string(discovery_db_path, "./discovery/rocks_db", "rocks db path");
    DEFINE_string(discovery_listen,"127.0.0.1:8010", "discovery listen addr");
    DEFINE_int32(discovery_request_timeout, 30000,
                 "discovery as server request timeout, default:30000ms");
    DEFINE_int32(discovery_connect_timeout, 5000,
                 "discovery as server connect timeout, default:5000ms");

    DEFINE_string(backup_discovery_server_peers, "", "backup_discovery_server_peers");
    DEFINE_int64(time_between_discovery_connect_error_ms, 0, "time_between_discovery_connect_error_ms. default(0ms)");


}  // namespace sirius
