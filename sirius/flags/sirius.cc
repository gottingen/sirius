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
#include <turbo/flags/flag.h>

TURBO_FLAG(std::vector<std::string>, sirius_server_peers, std::vector<std::string>{"127.0.0.1:8010"}, "sirius server peers");
TURBO_FLAG(int32_t, sirius_snapshot_interval_s, 600, "raft snapshot interval(s)");
TURBO_FLAG(int32_t, sirius_election_timeout_ms, 1000, "raft election timeout(ms)");
TURBO_FLAG(std::string, sirius_raft_group, "sirius_raft", "sirius raft group");
TURBO_FLAG(std::string, sirius_log_uri, "local://./sirius_data/raft_log/", "raft log uri");
TURBO_FLAG(std::string, sirius_stable_uri, "local://./sirius_data/raft_data/stable", "raft stable path");
TURBO_FLAG(std::string, sirius_snapshot_uri, "local://./sirius_data/raft_data/snapshot", "raft snapshot path");
TURBO_FLAG(int32_t, sirius_tso_snapshot_interval_s, 60, "tso raft snapshot interval(s)");
TURBO_FLAG(std::string, sirius_db_path, "./sirius_data/rocks_db", "rocks db path");
TURBO_FLAG(std::string, sirius_listen,"127.0.0.1:8010", "sirius listen addr");
TURBO_FLAG(int32_t, sirius_request_timeout, 30000,
           "sirius as server request timeout, default:30000ms");
TURBO_FLAG(int32_t, sirius_connect_timeout, 5000,
             "sirius as server connect timeout, default:5000ms");

TURBO_FLAG(int64_t, time_between_sirius_connect_error_ms, 0, "time between sirius connect error(ms)");

