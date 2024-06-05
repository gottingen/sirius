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
// Created by jeff on 23-11-29.
//

#pragma once

#include <collie/cli/cli.h>
#include <sirius/proto/discovery.interface.pb.h>
#include <collie/table/table.h>
#include <turbo/status/status.h>
#include <string>

namespace sirius::cli {

    struct AtomicOptionContext {
        static AtomicOptionContext *get_instance() {
            static AtomicOptionContext ins;
            return &ins;
        }

        // for config
        int64_t servlet_id{0};
        int64_t start_id{0};
        int64_t count{0};
        int64_t increment{0};
        int64_t force{false};
    };

    struct AtomicCmd {
        static void setup_atomic_cmd(collie::App &app);

        static void run_atomic_cmd(collie::App *app);

        static void run_atomic_create_cmd();

        static void run_atomic_remove_cmd();

        static void run_atomic_gen_cmd();

        static void run_atomic_update_cmd();
    };

}  // namespace sirius::cli
