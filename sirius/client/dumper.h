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

#ifndef EA_CLIENT_DUMPER_H_
#define EA_CLIENT_DUMPER_H_

#include <turbo/utility/status.h>
#include <string>
#include <google/protobuf/descriptor.h>
#include <sirius/proto/discovery.interface.pb.h>

namespace sirius::client {

    /**
     * @ingroup ea_proto
     * @details Dumper is a helper class for protobuf object convert to json string.
     *        do not ignore the result of function, it is recommend use like:
     * @code
     *        const std::string json_content = "R{
     *             "name": "example",
     *             "version": {
     *               "major": 1,
     *               "minor": 2,
     *               "patch": 3
     *             },
     *             "content": "{\"servlet\":\"sug\",\"zone\":{\"instance\":"
     *                        "[\"192.168.1.2\",\"192.168.1.3\",\"192.168.1.3\"]"
     *                        ",\"name\":\"ea_search\",\"user\":\"jeff\"}}",
     *             "type": "CF_JSON",
     *             "time": 1701477509
     *           }"
     *
     *        sirius::proto::ConfigInfo info;
     *        info.set_name("example");
     *        info.mutable_version()->set_major(1);
     *        info.mutable_version()->set_minor(2);
     *        info.mutable_version()->set_path(3);
     *        info.set_content("{\"servlet\":\"sug\",\"zone\":{\"instance\":[\"192.168.1.2\","
     *                          "\"192.168.1.3\",\"192.168.1.3\"],\"name\":\"ea_search\",\"user\":\"jeff\"}}");
     *        info.set_type(sirius::proto::CF_JSON);
     *        info.set_time(1701477509);
     *        std::string parsed_string;
     *        auto status = Loader::dump_proto(info, parsed_string);
     *        if(!status.ok) {
     *          handler_error();
     *         }
     *         ...
     *         handler_success();
     * @endcode
     */
    class Dumper {
    public:

        /**
         *
         * @param path
         * @param message
         * @return status.ok() if success else return the reason of format fail.
         */
        static turbo::Status dump_proto_to_file(const std::string &path, const google::protobuf::Message &message);

        /**
         * @brief dump a pb object message to json format string
         * @param message [input] a pb object message
         * @param content [output] string to store json format result
         * @return status.ok() if success else return the reason of format fail.
         */
        static turbo::Status dump_proto(const google::protobuf::Message &message, std::string &content);
    };
}  // namespace sirius::client

#endif  // EA_CLIENT_DUMPER_H_
