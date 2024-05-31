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

#include <sirius/proto/discovery.struct.pb.h>
#include <turbo/status/status.h>

namespace sirius::client {

    /**
     * @ingroup ea_proto
     * @brief ConfigInfoBuilder is helper class for build ConfigInfo object,
     *        it does not hold the ConfigInfo object memory, and not thread safe. user should guard
     *        that the object is usable and make sure it is thread safe call. eg.
     * @code
     *        sirius::proto::ConfigInfo info;
     *        ConfigInfoBuilder builder(&info);
     *        std::string content = "listen_port=8010;raft_group=meta_raft";
     *        auto status = builder.build_from_content("meta_config", content, "1.2.3", "json");
     *        if(!status.ok) {
     *          handler_error();
     *         }
     *         ...
     *         handler_success();
     * @endcode
     */
    class ConfigInfoBuilder {
    public:
        ConfigInfoBuilder() = default;

        ~ConfigInfoBuilder() = default;

        explicit ConfigInfoBuilder(sirius::proto::ConfigInfo *info);

        /**
         * @brief set up ConfigInfo pointer for building
         * @param info
         */
        void set_info(sirius::proto::ConfigInfo *info);

        /**
         * @brief load ConfigInfo from json format string.
         * @param json_str json format string
         * @return
         */
        turbo::Status build_from_json(const std::string &json_str);

        /**
         * @brief load ConfigInfo from json format string that read from file.
         * @param json_path file path of json string
         * @return status.ok() if success else return the reason of parse fail.
         */
        turbo::Status build_from_json_file(const std::string &json_path);

        /**
         * @brief build ConfigInfo by parameters, the config store in the file
         * @param name [input] config name
         * @param file [input] config content data file.
         * @param version [input] config version, format is major.minor.patch eg "1.2.3"
         * @param type [input] config type, can be any one of this [CF_JSON|CF_GFLAGS|CF_TEXT|CF_TOML|CF_XML|CF_YAML|CF_INI].
         * @return status.ok() if success else return the reason of parse fail.
         */
        turbo::Status build_from_file(const std::string &name, const std::string &file, const sirius::proto::Version &version,
                                      const sirius::proto::ConfigType &type = sirius::proto::CF_JSON);

        /**
         * @brief build ConfigInfo by parameters, the config store in content
         * @param name [input] config name
         * @param file [input] config content data file.
         * @param version [input] config version, format is major.minor.patch eg "1.2.3"
         * @param type [input] config type, can be any one of this [json|toml|yaml|xml|gflags|text|ini].
         * @return status.ok() if success else return the reason of build fail.
         */
        turbo::Status build_from_file(const std::string &name, const std::string &file, const sirius::proto::Version &version,
                                      const std::string &type = "json");

        /**
         *  @brief build ConfigInfo by parameters, the config store in content
         * @param name [input] config name
         * @param file [input] config content data file.
         * @param version [input] config version, format is major.minor.patch eg "1.2.3"
         * @param type [input] config type, can be any one of this [CF_JSON|CF_GFLAGS|CF_TEXT|CF_TOML|CF_XML|CF_YAML|CF_INI].
         * @return status.ok() if success else return the reason of build fail.
         */
        turbo::Status build_from_file(const std::string &name, const std::string &file, const std::string &version,
                                      const sirius::proto::ConfigType &type = sirius::proto::CF_JSON);

        /**
         * @brief build ConfigInfo by parameters, the config store in content
         * @param name [input] config name
         * @param file [input] config content data file.
         * @param version [input] config version, format is major.minor.patch eg "1.2.3"
         * @param type [input] config type, can be any one of this [json|toml|yaml|xml|gflags|text|ini].
         * @return status.ok() if success else return the reason of build fail.
         */
        turbo::Status build_from_file(const std::string &name, const std::string &file, const std::string &version,
                                      const std::string &type = "json");

        /**
         * @brief build ConfigInfo by parameters, the config store in content
         * @param name [input] config name
         * @param content [input] config content.
         * @param version [input] config version, format is major.minor.patch eg "1.2.3"
         * @param type [input] config type, can be any one of this [CF_JSON|CF_GFLAGS|CF_TEXT|CF_TOML|CF_XML|CF_YAML|CF_INI].
         * @return status.ok() if success else return the reason of build fail.
         */
        turbo::Status build_from_content(const std::string &name, const std::string &content, const sirius::proto::Version &version,
                                         const sirius::proto::ConfigType &type = sirius::proto::CF_JSON);
        /**
         * @brief build ConfigInfo by parameters, the config store in content
         * @param name [input] config name
         * @param content [input] config content.
         * @param version [input] config version, format is major.minor.patch eg "1.2.3"
         * @param type [input] config type, can be any one of this [json|toml|yaml|xml|gflags|text|ini].
         * @return status.ok() if success else return the reason of build fail.
         */
        turbo::Status build_from_content(const std::string &name, const std::string &content, const sirius::proto::Version &version,
                                         const std::string &type = "json");
        /**
         * @brief build ConfigInfo by parameters, the config store in content
         * @param name
         * @param content [input] config content.
         * @param version [input] config version, format is major.minor.patch eg "1.2.3"
         * @param type [input] config type, can be any one of this [CF_JSON|CF_GFLAGS|CF_TEXT|CF_TOML|CF_XML|CF_YAML|CF_INI].
         * @return status.ok() if success else return the reason of build fail.
         */
        turbo::Status build_from_content(const std::string &name, const std::string &content, const std::string &version,
                                         const sirius::proto::ConfigType &type = sirius::proto::CF_JSON);
        /**
         * @brief build ConfigInfo by parameters, the config store in content
         * @param name [input] config name
         * @param content [input] config content.
         * @param version [input] config version, format is major.minor.patch eg "1.2.3"
         * @param type [input] config type, can be any one of this [json|toml|yaml|xml|gflags|text|ini].
         * @return status.ok() if success else return the reason of build fail.
         */
        turbo::Status
        build_from_content(const std::string &name, const std::string &content, const std::string &version,
                           const std::string &type = "json");

    private:
        sirius::proto::ConfigInfo *_info{nullptr};
    };
}  // namespace client
