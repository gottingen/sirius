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
//

#include <sirius/base/log.h>
#include <sirius/flags/log.h>
#include <thread>
#include <mutex>

namespace sirius {

    std::once_flag log_init_flag;

    bool init_result = false;

    static void log_init_func() {
        if (init_result) {
            return;
        }
        if (FLAGS_enable_console_log) {
            turbo::setup_color_stderr_sink();
        } else {
            std::string base_log_name = FLAGS_log_root + "/" + FLAGS_log_base_name;
            turbo::setup_daily_file_sink(base_log_name, FLAGS_log_rotation_hour, FLAGS_log_rotation_minute,
                                          FLAGS_log_save_days);
        }
        LOG(INFO)<< "log init success, "<<"console log: "<<FLAGS_enable_console_log;
        init_result = true;
    }

    bool initialize_log() {
        std::call_once(log_init_flag, log_init_func);
        return init_result;
    }
}  // namespace sirius
