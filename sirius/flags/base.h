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

    /// for common
    DECLARE_int64(memory_gc_interval_s);
    DECLARE_int64(memory_stats_interval_s);
    DECLARE_int64(min_memory_use_size);
    DECLARE_int64(min_memory_free_size_to_release);
    DECLARE_int64(mem_tracker_gc_interval_s);
    DECLARE_int64(process_memory_limit_bytes);
    DECLARE_int64(query_memory_limit_ratio);

}  // namespace sirius

namespace fiber {
    DECLARE_int32(fiber_concurrency); //bthread.cpp
}
namespace melon::raft {
    DECLARE_int32(raft_election_heartbeat_factor);
    DECLARE_bool(raft_enable_leader_lease);
}  // namespace melon::raft
