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
#include <turbo/status/status.h>
#include <string>

namespace sirius::cli {

    struct ServletOptionContext {
        static ServletOptionContext *get_instance() {
            static ServletOptionContext ins;
            return &ins;
        }
        // for namespace
        std::string app_name;
        int64_t     app_quota;
        std::string servlet_name;
        std::string zone_name;
    };

    // We could manually make a few variables and use shared pointers for each; this
    // is just done this way to be nicely organized

    // Function declarations.
    void setup_servlet_cmd(collie::App &app);

    void run_servlet_cmd(collie::App *app);

    void run_servlet_create_cmd();

    void run_servlet_remove_cmd();

    void run_servlet_modify_cmd();

    void run_servlet_list_cmd();

    void run_servlet_info_cmd();

    [[nodiscard]] turbo::Status
    make_servlet_create(sirius::proto::DiscoveryManagerRequest *req);

    [[nodiscard]] turbo::Status
    make_servlet_remove(sirius::proto::DiscoveryManagerRequest *req);

    [[nodiscard]] turbo::Status
    make_servlet_modify(sirius::proto::DiscoveryManagerRequest *req);

    [[nodiscard]] turbo::Status
    make_servlet_list(sirius::proto::DiscoveryQueryRequest *req);

    [[nodiscard]] turbo::Status
    make_servlet_info(sirius::proto::DiscoveryQueryRequest *req);

    collie::table::Table show_discovery_query_servlet_response(const sirius::proto::DiscoveryQueryResponse &res);

}  // namespace sirius::cli
