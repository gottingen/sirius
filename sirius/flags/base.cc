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


#include <sirius/flags/base.h>
#include <gflags/gflags.h>

namespace sirius {
    /// common
    DEFINE_int64(memory_gc_interval_s, 10, "mempry GC interval , default: 10s");
    DEFINE_int64(memory_stats_interval_s, 60, "mempry GC interval , default: 60s");
    DEFINE_int64(min_memory_use_size, 8589934592, "minimum memory use size , default: 8G");
    DEFINE_int64(min_memory_free_size_to_release, 2147483648, "minimum memory free size to release, default: 2G");
    DEFINE_int64(mem_tracker_gc_interval_s, 60, "do memory limit when row number more than #, default: 60");
    DEFINE_int64(process_memory_limit_bytes, -1, "all memory use size, default: -1");
    DEFINE_int64(query_memory_limit_ratio, 90, "query memory use ratio , default: 90%");

}  // namespace
