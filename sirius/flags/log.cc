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


#include <sirius/flags/log.h>
#include <gflags/gflags.h>

namespace sirius {
    /// for log
    DEFINE_bool(enable_console_log, true, "console or file log");
    DEFINE_string(log_root, "./logs", "ea flags log root");
    DEFINE_int32(log_rotation_hour, 2, "rotation hour");
    DEFINE_int32(log_rotation_minute, 30, "rotation minutes");
    DEFINE_string(log_base_name, "ea_log.txt", "base name for EA");
    DEFINE_int32(log_save_days, 7, "ea log save days");

}  // namespace sirius