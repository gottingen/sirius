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


#pragma once

#include <collie/cli/cli.h>
#include <sirius/proto/discovery.interface.pb.h>
#include <turbo/status/status.h>
#include <collie/table/table.h>
#include <string>
#include <sirius/client/config_client.h>

namespace sirius::cli {

    struct ConfigOptionContext {
        static ConfigOptionContext *get_instance() {
            static ConfigOptionContext ins;
            return &ins;
        }
        // for config
        std::string config_name;
        std::string config_data;
        std::string config_file;
        std::string config_version;
        std::string config_type;
        std::string config_json;
        std::string config_example;
        std::vector<std::string> config_watch_list;
        std::string config_watch_dir;
        bool clean_local;
        sirius::proto::ConfigInfo config_request;
    };

    struct ConfigCmd {
        static void setup_config_cmd(collie::App &app);

        static void run_config_cmd(collie::App *app);

        static void run_config_create_cmd();

        static void run_config_list_cmd();

        static void run_config_dump_cmd();

        static void run_config_test_cmd();

        static void run_config_version_list_cmd();

        static void run_config_get_cmd();

        static void run_config_remove_cmd();

        static void run_config_watch_cmd();

        [[nodiscard]] static turbo::Status
        make_example_config_dump(sirius::proto::ConfigInfo *req);

        [[nodiscard]] static turbo::Status
        make_config_list(sirius::proto::DiscoveryQueryRequest *req);

        [[nodiscard]] static turbo::Status
        make_config_list_version(sirius::proto::DiscoveryQueryRequest *req);

        [[nodiscard]] static turbo::Status
        make_config_get(sirius::proto::DiscoveryQueryRequest *req);

        [[nodiscard]] static turbo::Status
        make_config_remove(sirius::proto::DiscoveryManagerRequest *req);


        static collie::table::Table show_query_ops_config_list_response(const sirius::proto::DiscoveryQueryResponse &res);

        static collie::table::Table show_query_ops_config_list_version_response(const sirius::proto::DiscoveryQueryResponse &res);

        static collie::table::Table
        show_query_ops_config_get_response(const sirius::proto::DiscoveryQueryResponse &res, const turbo::Status &save_status);

        static turbo::Status save_config_to_file(const std::string &path, const sirius::proto::DiscoveryQueryResponse &res);

        static turbo::Status save_config_to_file(const std::string &basedir, const sirius::client::ConfigCallbackData &data);
    };

}  // namespace sirius::cli
