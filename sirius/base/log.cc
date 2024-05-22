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

namespace sirius {

    std::shared_ptr<collie::log::logger> ss_logger = collie::log::default_logger();

    std::once_flag log_init_flag;

    bool init_result = false;

    static void log_init_func() {
        if (init_result) {
            return;
        }
        if (!collie::filesystem::exists(FLAGS_log_root)) {
            if (!collie::filesystem::create_directories(FLAGS_log_root)) {
                init_result =  false;
            }
        }
        collie::filesystem::path lpath(FLAGS_log_root);
        lpath /= FLAGS_log_base_name;
        collie::log::sink_ptr file_sink = std::make_shared<collie::log::sinks::daily_file_sink_mt>(lpath.string(),
                                                                                                   FLAGS_log_rotation_hour,
                                                                                                   FLAGS_log_rotation_minute,
                                                                                                   false,
                                                                                                   FLAGS_log_save_days);
        file_sink->set_level(collie::log::level::info);


        if (!FLAGS_enable_console_log) {
            ss_logger = std::make_shared<collie::log::logger>("sirius-logger", file_sink);
            ss_logger->set_level(collie::log::level::info);
        } else {
            collie::log::sink_ptr console_sink = std::make_shared<collie::log::sinks::stdout_color_sink_mt>();
            ss_logger = std::make_shared<collie::log::logger>("sirius-logger",
                                                              collie::log::sinks_init_list{file_sink,
                                                                                           console_sink});
            ss_logger->set_level(collie::log::level::info);
        }
        init_result = true;
    }
    bool initialize_log() {
        std::call_once(log_init_flag, log_init_func);
        return init_result;
    }

    std::once_flag warn_once_flag;
    void warn_once() {
        if(ss_logger == collie::log::default_logger()) {
            SS_LOG(WARN) << "Please call init_tlog() before using SS_LOG";
        }
    }

    collie::log::logger *get_logger() {
        std::call_once(warn_once_flag, warn_once);
        return ss_logger.get();
    }
}  // namespace sirius
