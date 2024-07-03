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

#include <turbo/flags/declare.h>

TURBO_DECLARE_FLAG(int64_t, memory_gc_interval_s);
TURBO_DECLARE_FLAG(int64_t, memory_stats_interval_s);
TURBO_DECLARE_FLAG(int64_t, min_memory_use_size);
TURBO_DECLARE_FLAG(int64_t, min_memory_free_size_to_release);
TURBO_DECLARE_FLAG(int64_t, mem_tracker_gc_interval_s);
TURBO_DECLARE_FLAG(int64_t, process_memory_limit_bytes);
TURBO_DECLARE_FLAG(int64_t, query_memory_limit_ratio);
TURBO_DECLARE_FLAG(int32_t, raft_election_heartbeat_factor);
TURBO_DECLARE_FLAG(bool, raft_enable_leader_lease);
