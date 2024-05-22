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


#include <sirius/flags/client.h>
#include <gflags/gflags.h>

namespace sirius {
    DEFINE_string(config_cache_dir, "./config_cache", "config cache dir");
    DEFINE_int32(config_watch_interval_ms, 1, "config watch sleep between two watch config");
    DEFINE_int32(config_watch_interval_round_s, 30, "every x(s) to fetch and get config for a round");
}  // namespace sirius