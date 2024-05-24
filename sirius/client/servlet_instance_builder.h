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

#ifndef EA_CLIENT_SERVLET_INSTANCE_BUILDER_H_
#define EA_CLIENT_SERVLET_INSTANCE_BUILDER_H_

#include <sirius/proto/discovery.struct.pb.h>
#include <collie/utility/status.h>

namespace sirius::client {

    /**
     * @ingroup ea_proto
     * @brief ConfigInfoBuilder is helper class for build ConfigInfo object,
     *        it does not hold the ConfigInfo object memory, and not thread safe. user should guard
     *        that the object is usable and make sure it is thread safe call. eg.
     * @code
     *        sirius::proto::ConfigInfo info;
     *        ServletInstanceBuilder builder(&info);
     *        std::string json_str = "{}"
     *        auto status = builder.build_from_json(json_str);
     *        if(!status.ok) {
     *          handler_error();
     *         }
     *         ...
     *         handler_success();
     * @endcode
     */
    class ServletInstanceBuilder {
    public:
        ServletInstanceBuilder() = default;

        explicit ServletInstanceBuilder(sirius::proto::ServletInfo *ins);

        ~ServletInstanceBuilder() = default;

    public:

        /**
         *
         * @param ins the ConfigInfo object to build
         */
        void set(sirius::proto::ServletInfo *ins);

        /**
         *
         * @param json_str [input] the json string to build ConfigInfo object.
         * @return Status::OK if the ConfigInfo object was built successfully. Otherwise, an error status is returned.
         */
        collie::Status build_from_json(const std::string &json_str);

        /**
         *
         * @param json_path [input] the json file path to build ConfigInfo object.
         * @return Status::OK if the ConfigInfo object was built successfully. Otherwise, an error status is returned.
         */
        collie::Status build_from_json_file(const std::string &json_path);

        /**
         *
         * @param namespace_name [input] the namespace name to set.
         * @return the ServletInstanceBuilder.
         */
        ServletInstanceBuilder &set_namespace(const std::string &namespace_name);

        /**
         *
         * @param zone [input] the zone to set.
         * @return the ServletInstanceBuilder.
         */
        ServletInstanceBuilder &set_zone(const std::string &zone);

        /**
         *
         * @param servlet [input] the servlet to set.
         * @return the ServletInstanceBuilder.
         */
        ServletInstanceBuilder &set_servlet(const std::string &servlet);


        /**
         *
         * @param color [input] the color to set.
         * @return the ServletInstanceBuilder.
         */
        ServletInstanceBuilder &set_color(const std::string &color);

        /**
         *
         * @param env [input] the env to set.
         * @return the ServletInstanceBuilder.
         */
        ServletInstanceBuilder &set_env(const std::string &env);


        /**
         *
         * @param passwd [input] the passwd to set.
         * @return the ServletInstanceBuilder.
         */
        ServletInstanceBuilder &set_passwd(const std::string &passwd);

        /**
         *
         * @param status [input] the status to set.
         * @return the ServletInstanceBuilder.
         */
        ServletInstanceBuilder &set_status(const std::string &status);


        /**
         *
         * @param address [input] the address to set.
         * @return the ServletInstanceBuilder.
         */
        ServletInstanceBuilder &set_address(const std::string &address);

        /**
         *
         * @param status [input] the status to set.
         * @return the ServletInstanceBuilder.
         */        
        ServletInstanceBuilder &set_status(const sirius::proto::Status &status);


    private:
        sirius::proto::ServletInfo *_instance;
    };
}  // namespace sirius::client

#endif  // EA_CLIENT_SERVLET_INSTANCE_BUILDER_H_
