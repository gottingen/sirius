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

#include <turbo/utility/status.h>
#include <string>
#include <google/protobuf/descriptor.h>
#include <sirius/proto/discovery.interface.pb.h>

/**
 * @defgroup ea_proto proto  proto operators
 * @defgroup discovery_client discovery_client discovery_client operators
 * @defgroup ea_rpc rpc rpc operators
 * @defgroup config_client config_client config_client operators
 */

namespace sirius::client {

    /**
     * @ingroup ea_proto
     * @brief Loader is a helper class for proto convert from json to protobuf.
     *        do not ignore the result of function, it is recommend use like:
     * @code
     *        auto json_config = "R{
     *             "name": "example",
     *             "version": {
     *               "major": 1,
     *               "minor": 2,
     *               "patch": 3
     *             },
     *             "content": "{\"servlet\":\"sug\",\"zone\":{\"instance\":[\"192.168.1.2\",\"192.168.1.3\","
     *                          "\"192.168.1.3\"],\"name\":\"ea_search\",\"user\":\"jeff\"}}",
     *             "type": "CF_JSON",
     *             "time": 1701477509
     *           }"
     *        sirius::proto::ConfigInfo info;
     *        auto status = Loader::load_proto(json_config, info);
     *        if(!status.ok) {
     *          handler_error();
     *         }
     *         ...
     *         handler_success();
     * @endcode
     */
    class Loader {
    public:

        /**
         * @brief load a json format string to the message
         * @param content [input] json format string.
         * @param message [output] result of parse the json format string.
         * @return status.ok() if success else return the reason of parse fail.
         */
        static turbo::Status load_proto(const std::string &content, google::protobuf::Message &message);

        /**
         *
         * @param path [input]file path, the file content must be json format.
         * @param message [output] result of parse the json format string.
         * @return status.ok() if success else return the reason of parse fail.
         */
        static turbo::Status load_proto_from_file(const std::string &path, google::protobuf::Message &message);
    };
}  // namespace sirius::client
