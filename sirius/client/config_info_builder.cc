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

#include <sirius/client/config_info_builder.h>
#include <sirius/client/utility.h>
#include <alkaid/files/localfs.h>
#include <sirius/client/loader.h>

namespace sirius::client {

    ConfigInfoBuilder::ConfigInfoBuilder(sirius::proto::ConfigInfo *info) : _info(info) {
        _info->Clear();
    }


    void ConfigInfoBuilder::set_info(sirius::proto::ConfigInfo *info) {
        _info = info;
        _info->Clear();
    }

    turbo::Status ConfigInfoBuilder::build_from_json(const std::string &json_str) {
        auto rs = Loader::load_proto(json_str, *_info);
        if (!rs.ok()) {
            return rs;
        }
        /// check field
        if (!_info->has_name() || _info->name().empty()) {
            return turbo::data_loss_error("miss required field name");
        }
        if (!_info->has_version() ||
            (_info->version().major() == 0 && _info->version().minor() == 0 && _info->version().patch() == 0)) {
            return turbo::data_loss_error("miss field version");
        }

        if (!_info->has_content() || _info->content().empty()) {
            return turbo::data_loss_error("miss required field name");
        }
        return turbo::OkStatus();
    }

    turbo::Status ConfigInfoBuilder::build_from_json_file(const std::string &json_path) {
        auto lfs = alkaid::Filesystem::localfs();
        std::string content;
        STATUS_RETURN_IF_ERROR(lfs->read_file(json_path, &content));
        return build_from_json(content);
    }

    turbo::Status ConfigInfoBuilder::build_from_file(const std::string &name, const std::string &file_path,
                                                     const sirius::proto::Version &version,
                                                     const sirius::proto::ConfigType &type) {
        auto lfs = alkaid::Filesystem::localfs();
        std::string content;
        STATUS_RETURN_IF_ERROR(lfs->read_file(file_path, &content));
        return build_from_content(name, content, version, type);
    }

    turbo::Status ConfigInfoBuilder::build_from_file(const std::string &name, const std::string &file_path,
                                                     const sirius::proto::Version &version,
                                                     const std::string &type) {
        auto lfs = alkaid::Filesystem::localfs();
        std::string content;
        STATUS_RETURN_IF_ERROR(lfs->read_file(file_path, &content));
        auto rt = string_to_config_type(type);
        if (!rt.ok()) {
            return rt.status();
        }

        return build_from_content(name, content, version, rt.value());
    }

    turbo::Status ConfigInfoBuilder::build_from_file(const std::string &name, const std::string &file_path,
                                                     const std::string &version,
                                                     const sirius::proto::ConfigType &type) {
        auto lfs = alkaid::Filesystem::localfs();
        std::string content;
        STATUS_RETURN_IF_ERROR(lfs->read_file(file_path, &content));
        sirius::proto::Version tmp_version;
        STATUS_RETURN_IF_ERROR(string_to_version(version, &tmp_version));
        return build_from_content(name, content, tmp_version, type);
    }

    turbo::Status ConfigInfoBuilder::build_from_file(const std::string &name, const std::string &file_path,
                                                     const std::string &version,
                                                     const std::string &type) {
        auto lfs = alkaid::Filesystem::localfs();
        std::string content;
        STATUS_RETURN_IF_ERROR(lfs->read_file(file_path, &content));
        RESULT_ASSIGN_OR_RETURN(auto r, string_to_config_type(type));
        sirius::proto::Version tmp_version;
        STATUS_RETURN_IF_ERROR(string_to_version(version, &tmp_version));
        return build_from_content(name, content, tmp_version, r);
    }

    turbo::Status ConfigInfoBuilder::build_from_content(const std::string &name, const std::string &content,
                                                        const sirius::proto::Version &version,
                                                        const sirius::proto::ConfigType &type) {
        _info->set_name(name);
        _info->set_content(content);
        *_info->mutable_version() = version;
        _info->set_type(type);
        return turbo::OkStatus();
    }

    turbo::Status ConfigInfoBuilder::build_from_content(const std::string &name, const std::string &content,
                                                        const sirius::proto::Version &version,
                                                        const std::string &type) {
        auto rt = string_to_config_type(type);
        if (!rt.ok()) {
            return rt.status();
        }
        return build_from_content(name, content, version, rt.value());
    }

    turbo::Status ConfigInfoBuilder::build_from_content(const std::string &name, const std::string &content,
                                                        const std::string &version,
                                                        const sirius::proto::ConfigType &type) {
        sirius::proto::Version tmp_version;
        auto rs = string_to_version(version, &tmp_version);
        if (!rs.ok()) {
            return rs;
        }
        return build_from_content(name, content, tmp_version, type);
    }

    turbo::Status
    ConfigInfoBuilder::build_from_content(const std::string &name, const std::string &content,
                                          const std::string &version,
                                          const std::string &type) {
        auto rt = string_to_config_type(type);
        if (!rt.ok()) {
            return rt.status();
        }
        sirius::proto::Version tmp_version;
        auto rs = string_to_version(version, &tmp_version);
        if (!rs.ok()) {
            return rs;
        }
        return build_from_content(name, content, tmp_version, rt.value());

    }

}  // namespace sirius::client
