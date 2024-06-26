// Copyright 2023 The Turbo Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#pragma once

#include <string>
#include <cstdint>
#include <vector>

namespace sirius::cli {

    struct OptionContext {
        static OptionContext *get_instance() {
            static OptionContext ins;
            return &ins;
        }
        // for global
        std::string router_server;
        std::string discovery_server;
        std::string discovery_leader;
        std::string load_balancer;
        int32_t  timeout_ms{2000};
        int32_t  connect_timeout_ms{100};
        int32_t  max_retry{3};
        int32_t time_between_discovery_connect_error_ms{1000};
        bool verbose;
        bool router{false};
    };

}  // namespace sirius::cli
