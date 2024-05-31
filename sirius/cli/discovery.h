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
// Created by jeff on 23-11-30.
//

#pragma once

#include <collie/cli/cli.h>
#include <sirius/proto/discovery.interface.pb.h>
#include <turbo/status/result.h>
#include <collie/table/table.h>
#include <string>
#include <sirius/client/config_client.h>
#include <melon/naming/sns_naming_service.h>

namespace sirius::cli {


    struct DiscoveryOptionContext {
        static DiscoveryOptionContext *get_instance() {
            static DiscoveryOptionContext ins;
            return &ins;
        }

        // for config
        std::string app_name;
        std::string servlet_name;
        std::string zone_name;
        std::string env;
        std::string color;
        std::string status;
        std::string address;
        std::string dump_file;
        std::string json_file;
        std::vector<std::string> zones;
        std::vector<std::string> colors;
        std::vector<std::string> envs;
        uint32_t    fibers{10};
        bool quiet{false};
    };

    struct DiscoveryCmd {
        static void setup_discovery_cmd(collie::App &app);

        static void run_discovery_cmd(collie::App *app);

        static void run_discovery_add_instance_cmd();

        static void run_mock_instance_cmd();

        static void run_discovery_remove_instance_cmd();

        static void run_discovery_update_instance_cmd();

        static void run_discovery_list_instance_cmd();

        static void run_discovery_info_instance_cmd();

        static void run_discovery_dump_cmd();

        [[nodiscard]] static turbo::Status
        make_discovery_add_instance(sirius::proto::DiscoveryManagerRequest *req);

        [[nodiscard]] static turbo::Status
        make_discovery_remove_instance(sirius::proto::DiscoveryManagerRequest *req);

        [[nodiscard]] static turbo::Status
        make_discovery_update_instance(sirius::proto::DiscoveryManagerRequest *req);

        [[nodiscard]] static turbo::Status
        make_discovery_list_instance(sirius::proto::DiscoveryQueryRequest *req);

        [[nodiscard]] static turbo::Status
        make_discovery_info_instance(melon::SnsRequest *req);

        [[nodiscard]] static turbo::Result<int> string_to_status(const std::string &status);

        static collie::table::Table show_query_instance_list_response(const sirius::proto::DiscoveryQueryResponse &res);
        static collie::table::Table show_query_instance_info_response(const sirius::proto::DiscoveryQueryResponse &res);
    };
}  // namespace sirius::cli
