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

#include <collie/cli/cli.h>
#include <sirius/proto/discovery.interface.pb.h>
#include <collie/table/table.h>
#include <turbo/utility/status.h>
#include <string>

namespace sirius::cli {

    // We could manually make a few variables and use shared pointers for each; this
    // is just done this way to be nicely organized

    struct AppOptionContext {
        static AppOptionContext *get_instance() {
            static AppOptionContext ins;
            return &ins;
        }
        // for namespace
        std::string app_name;
        int64_t     app_quota;
    };

    // Function declarations.
    void setup_app_cmd(collie::App &app);

    void run_namespace_cmd(collie::App *app);

    void run_ns_create_cmd();

    void run_ns_remove_cmd();

    void run_ns_modify_cmd();

    void run_ns_list_cmd();

    void run_ns_info_cmd();

    [[nodiscard]] turbo::Status
    make_namespace_create(sirius::proto::DiscoveryManagerRequest *req);

    [[nodiscard]] turbo::Status
    make_namespace_remove(sirius::proto::DiscoveryManagerRequest *req);

    [[nodiscard]] turbo::Status
    make_namespace_modify(sirius::proto::DiscoveryManagerRequest *req);

    [[nodiscard]] turbo::Status
    make_namespace_query(sirius::proto::DiscoveryQueryRequest *req);

    collie::table::Table show_discovery_query_ns_response(const sirius::proto::DiscoveryQueryResponse &res);
}  // namespace sirius::cli
