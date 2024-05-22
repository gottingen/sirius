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

#include <collie/utility/result.h>
#include <string>
#include <sirius/proto/discovery.interface.pb.h>
#include <collie/module/semver.h>

namespace sirius::client {

    /**
     * @ingroup ea_proto
     * @brief config_type_to_string is used to convert a ConfigType to a string.
     * @param type [input] is the ConfigType to convert.
     * @return a string representation of the ConfigType.
     */
    std::string config_type_to_string(sirius::proto::ConfigType type);

    /**
     * @ingroup ea_proto
     * @brief string_to_config_type is used to convert a string to a ConfigType.
     * @param str [input] is the string to convert.
     * @return a ConfigType if the string is valid. Otherwise, an error status is returned.
     */
    collie::Result<sirius::proto::ConfigType> string_to_config_type(const std::string &str);

    /**
     * @ingroup ea_proto
     * @brief string_to_version is used to convert a string to a Version.
     * @param str [input] is the string to convert.
     * @param v [output] is the Version received from the string.
     * @return Status::OK if the string was converted successfully. Otherwise, an error status is returned.
     */
    collie::Status string_to_version(const std::string &str, sirius::proto::Version *v);

    /**
     * @ingroup ea_proto
     * @brief string_to_module_version is used to convert a string to a ModuleVersion.
     * @param str [input] is the string to convert.
     * @param v [output] is the ModuleVersion received from the string.
     * @return Status::OK if the string was converted successfully. Otherwise, an error status is returned.
     */
    collie::Status string_to_module_version(const std::string &str, collie::ModuleVersion *v);

    /**
     * @ingroup ea_proto
     * @brief version_to_string is used to convert a Version to a string.
     * @param v [input] is the Version to convert.
     * @return a string representation of the Version.
     */
    std::string version_to_string(const sirius::proto::Version &v);

    /**
     * @ingroup ea_proto
     * @brief module_version_to_string is used to convert a ModuleVersion to a string.
     * @param v [input] is the ModuleVersion to convert.
     * @return a string representation of the ModuleVersion.
     */
    std::string module_version_to_string(const collie::ModuleVersion &v);


    /**
     * @ingroup ea_proto
     * @brief string_to_config_info is used to convert a string to a ConfigInfo.
     * @param name [input] is the name of the ConfigInfo to convert.
     * @return a ConfigInfo if the string is valid. Otherwise, an error status is returned.
     */
    [[nodiscard]] collie::Status check_valid_name_type(std::string_view name);

}  // namespace sirius::client
