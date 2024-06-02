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

#include <sirius/flags/sirius.h>
#include <gflags/gflags.h>

namespace sirius {

    /// for sirius
    DEFINE_string(sirius_server_peers, "127.0.0.1:8010", "sirius server peers");
    DEFINE_int32(sirius_snapshot_interval_s, 600, "raft snapshot interval(s)");
    DEFINE_int32(sirius_election_timeout_ms, 1000, "raft election timeout(ms)");
    DEFINE_string(sirius_raft_group, "sirius_raft", "sirius raft group");
    DEFINE_string(sirius_log_uri, "local://./sirius_data/raft_log/", "raft log uri");
    DEFINE_string(sirius_stable_uri, "local://./sirius_data/raft_data/stable", "raft stable path");
    DEFINE_string(sirius_snapshot_uri, "local://./sirius_data/raft_data/snapshot", "raft snapshot path");
    DEFINE_int32(sirius_tso_snapshot_interval_s, 60, "tso raft snapshot interval(s)");
    DEFINE_string(sirius_db_path, "./sirius_data/rocks_db", "rocks db path");
    DEFINE_string(sirius_listen,"127.0.0.1:8010", "sirius listen addr");
    DEFINE_int32(sirius_request_timeout, 30000,
                 "sirius as server request timeout, default:30000ms");
    DEFINE_int32(sirius_connect_timeout, 5000,
                 "sirius as server connect timeout, default:5000ms");

    DEFINE_int64(time_between_sirius_connect_error_ms, 0, "time between sirius connect error(ms)");


}  // namespace sirius
