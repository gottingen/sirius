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
#include <collie/utility/status.h>
#include <string>

namespace sirius::cli {

    struct ZoneOptionContext {
        static ZoneOptionContext *get_instance() {
            static ZoneOptionContext ins;
            return &ins;
        }
        // for namespace
        std::string app_name;
        int64_t     namespace_quota;
        std::string zone_name;
    };

    // We could manually make a few variables and use shared pointers for each; this
    // is just done this way to be nicely organized

    // Function declarations.
    void setup_zone_cmd(collie::App &app);

    void run_zone_cmd(collie::App *app);

    void run_zone_create_cmd();

    void run_zone_remove_cmd();

    void run_zone_modify_cmd();

    void run_zone_list_cmd();

    void run_zone_info_cmd();

    [[nodiscard]] collie::Status
    make_zone_create(sirius::proto::DiscoveryManagerRequest *req);

    [[nodiscard]] collie::Status
    make_zone_remove(sirius::proto::DiscoveryManagerRequest *req);

    [[nodiscard]] collie::Status
    make_zone_modify(sirius::proto::DiscoveryManagerRequest *req);

    [[nodiscard]] collie::Status
    make_zone_list(sirius::proto::DiscoveryQueryRequest *req);

    [[nodiscard]] collie::Status
    make_zone_info(sirius::proto::DiscoveryQueryRequest *req);

    collie::table::Table show_discovery_query_zone_response(const sirius::proto::DiscoveryQueryResponse &res);

}  // namespace sirius::cli
