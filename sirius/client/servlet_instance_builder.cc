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
#include <sirius/client/servlet_instance_builder.h>
#include <sirius/client/utility.h>
#include <alkaid/files/filesystem.h>
#include <melon/utility/endpoint.h>
#include <turbo/strings/numbers.h>
#include <sirius/client/loader.h>

namespace sirius::client {

    ServletInstanceBuilder::ServletInstanceBuilder(sirius::proto::ServletInfo *ins) : _instance(ins) {
        _instance->Clear();
    }

    void ServletInstanceBuilder::set(sirius::proto::ServletInfo *ins) {
        _instance = ins;
        _instance->Clear();
    }

    turbo::Status ServletInstanceBuilder::build_from_json(const std::string &json_str) {
        auto rs = Loader::load_proto(json_str, *_instance);
        if (!rs.ok()) {
            return rs;
        }
        /// check field
        if (!_instance->has_app_name() || _instance->app_name().empty()) {
            return turbo::data_loss_error("miss required field app_name");
        }

        if (!_instance->has_zone() || _instance->zone().empty()) {
            return turbo::data_loss_error("miss required field zone_name");
        }

        if (!_instance->has_servlet_name() || _instance->servlet_name().empty()) {
            return turbo::data_loss_error("miss required field servlet_name");
        }

        if (!_instance->has_address() || _instance->address().empty()) {
            return turbo::data_loss_error("miss required field address");
        }

        if (!_instance->has_env() || _instance->env().empty()) {
            return turbo::data_loss_error("miss required field address");
        }

        mutil::EndPoint peer;
        if (mutil::str2endpoint(_instance->address().c_str(), &peer) != 0) {
            return turbo::invalid_argument_error("bad address");
        }

        return turbo::OkStatus();
    }

    turbo::Status ServletInstanceBuilder::build_from_json_file(const std::string &json_path) {
        std::string content;
        auto lfs = alkaid::Filesystem::localfs();
        STATUS_RETURN_IF_ERROR(lfs->read_file(json_path, &content));
        return build_from_json(content);
    }

    ServletInstanceBuilder &ServletInstanceBuilder::set_namespace(const std::string &app_name) {
        _instance->set_app_name(app_name);
        return *this;
    }

    ServletInstanceBuilder &ServletInstanceBuilder::set_zone(const std::string &zone) {
        _instance->set_zone(zone);
        return *this;
    }

    ServletInstanceBuilder &ServletInstanceBuilder::set_servlet(const std::string &servlet) {
        _instance->set_servlet_name(servlet);
        return *this;
    }

    ServletInstanceBuilder &ServletInstanceBuilder::set_color(const std::string &color) {
        _instance->set_color(color);
        return *this;
    }

    ServletInstanceBuilder &ServletInstanceBuilder::set_env(const std::string &env) {
        _instance->set_env(env);
        return *this;
    }

    ServletInstanceBuilder &ServletInstanceBuilder::set_passwd(const std::string &passwd) {
        return *this;
    }

    ServletInstanceBuilder &ServletInstanceBuilder::set_status(const std::string &s) {
        int st;
        turbo::simple_atoi(s, &st);
        _instance->set_status(st);
        return *this;
    }

    ServletInstanceBuilder &ServletInstanceBuilder::set_address(const std::string &address) {
        _instance->set_address(address);
        return *this;
    }

    ServletInstanceBuilder &ServletInstanceBuilder::set_status(int s) {
        _instance->set_status(s);
        return *this;
    }


}  // namespace sirius::client
