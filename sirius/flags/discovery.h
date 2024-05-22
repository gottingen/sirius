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

#include <gflags/gflags_declare.h>

namespace sirius {

    /// for discovery
    DECLARE_string(discovery_server_peers);
    DECLARE_int32(discovery_replica_number);
    DECLARE_int32(discovery_snapshot_interval_s);
    DECLARE_int32(discovery_election_timeout_ms);
    DECLARE_string(discovery_raft_group);
    DECLARE_string(discovery_log_uri);
    DECLARE_string(discovery_stable_uri);
    DECLARE_string(discovery_snapshot_uri);
    DECLARE_int64(discovery_check_migrate_interval_us);
    DECLARE_int32(discovery_tso_snapshot_interval_s);
    DECLARE_string(discovery_db_path);
    DECLARE_string(discovery_listen);
    DECLARE_int32(discovery_request_timeout);
    DECLARE_int32(discovery_connect_timeout);
    DECLARE_string(backup_discovery_server_peers);
    DECLARE_int64(time_between_discovery_connect_error_ms);

}  // namespace sirius
