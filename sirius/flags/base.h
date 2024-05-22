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
    DECLARE_bool(need_health_check);
    DECLARE_int32(raft_write_concurrency);
    DECLARE_int32(service_write_concurrency);
    DECLARE_int32(snapshot_load_num);
    DECLARE_int32(baikal_heartbeat_concurrency);
    DECLARE_int64(incremental_info_gc_time);
    DECLARE_bool(enable_self_trace);
    DECLARE_bool(servitysinglelog);
    DECLARE_bool(open_service_write_concurrency);
    DECLARE_bool(schema_ignore_case);
    DECLARE_bool(disambiguate_select_name);
    DECLARE_int32(new_sign_read_concurrency);
    DECLARE_bool(open_new_sign_read_concurrency);
    DECLARE_bool(need_verify_ddl_permission);
    DECLARE_int32(histogram_split_threshold_percent);
    DECLARE_int32(limit_slow_sql_size);
    DECLARE_bool(like_predicate_use_re2);
    DECLARE_bool(transfor_hll_raw_to_sparse);

}  // namespace sirius

namespace fiber {
    DECLARE_int32(fiber_concurrency); //bthread.cpp
}
namespace melon::raft {
    DECLARE_int32(raft_election_heartbeat_factor);
    DECLARE_bool(raft_enable_leader_lease);
}  // namespace melon::raft
