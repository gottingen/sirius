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

#include <sirius/service/restful_config.h>
#include <sirius/service/api_proxy.h>
#include <collie/nlohmann/json.hpp>
#include <sirius/service/utility.h>
#include <turbo/strings/str_split.h>

namespace sirius::restful {

    void ListConfigProcessor::process(const melon::RestfulRequest *request, melon::RestfulResponse *response) {
        response->set_content_json();
        response->set_access_control_all_allow();
        response->set_status_code(200);
        ::eapi::sirius::ConfigQueryRequest req;
        ::eapi::sirius::ConfigListResponse resp;
        auto rs = ApiProxy::instance()->list_config(&req, &resp);
        nlohmann::json j;
        if (!rs.ok()) {
            j["code"] = rs.code();
            j["message"] = rs.message();
        } else {
            j["code"] = 0;
            j["message"] = "ok";
        };
        for (const auto &c : resp.configs()) {
            nlohmann::json item;
            item["name"] = c.name();
            item["version"] = sirius::version_to_string(c.version());
            item["content"] = c.content();
            item["type"] = sirius::config_type_to_string(c.type());
            item["createtime"] = c.time();
            item["id"] = c.id();
            j["configs"].push_back(item);
        }
        response->set_body(j.dump());
    }

    void CreateConfigProcessor::process(const melon::RestfulRequest *request, melon::RestfulResponse *response) {
        response->set_content_json();
        response->set_access_control_all_allow();
        response->set_status_code(200);
        std::string input_str = request->body().to_string();
        nlohmann::json input;
        try {
            input = nlohmann::json::parse(input_str);
        } catch (const std::exception &e) {
            nlohmann::json j;
            j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
            j["message"] = "invalid json";
            response->set_body(j.dump());
            return;
        }

        std::string name;
        if (input.find("name") == input.end()) {
            nlohmann::json j;
            j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
            j["message"] = "name is required";
            response->set_body(j.dump());
            return;
        } else {
            name = input["name"].get<std::string>();
        }

        std::string version;
        if (input.find("version") == input.end()) {
            nlohmann::json j;
            j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
            j["message"] = "version is required";
            response->set_body(j.dump());
            return;
        } else {
            version = input["version"].get<std::string>();
        }

        std::string content;
        if (input.find("content") == input.end()) {
            nlohmann::json j;
            j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
            j["message"] = "content is required";
            response->set_body(j.dump());
            return;
        } else {
            content = input["content"].get<std::string>();
        }

        std::string type;
        if (input.find("type") == input.end()) {
           type = "json";
        } else {
            type = input["type"].get<std::string>();
        }

        nlohmann::json j;
        eapi::sirius::ConfigInfo config;
        eapi::CommonResponse resp;
        auto rs = string_to_version(version, config.mutable_version());
        if (!rs.ok()) {
            j["code"] = rs.code();
            j["message"] = rs.message();
            response->set_body(j.dump());
            return;
        }
        config.set_name(name);
        config.set_type(sirius::string_to_config_type(type).value_or(eapi::sirius::CF_JSON));
        config.set_content(content);
        rs = ApiProxy::instance()->create_config(&config, &resp);
        if (!rs.ok()) {
            j["code"] = rs.code();
            j["message"] = rs.message();
        } else {
            j["code"] = resp.code();
            j["message"] = resp.message();
        }
        response->set_body(j.dump());
    }

    void RemoveConfigProcessor::process(const melon::RestfulRequest *request, melon::RestfulResponse *response) {
        response->set_content_json();
        response->set_access_control_all_allow();
        response->set_status_code(200);
        std::string input_str = request->body().to_string();
        nlohmann::json input;
        try {
            input = nlohmann::json::parse(input_str);
        } catch (const std::exception &e) {
            nlohmann::json j;
            j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
            j["message"] = "invalid json";
            response->set_body(j.dump());
            return;
        }
        if(input.find("name") == input.end()) {
            nlohmann::json j;
            j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
            j["message"] = "name is required";
            response->set_body(j.dump());
            return;
        }
        auto name = input["name"].get<std::string>();
        std::string version;
        turbo::Status rs;
        if (input.find("version") == input.end()) {
            version = "";
        } else {
            version = input["version"].get<std::string>();
        }
        eapi::sirius::ConfigInfo config;
        eapi::CommonResponse resp;
        config.set_name(name);
        if(!version.empty()) {
            rs = string_to_version(version, config.mutable_version());
        }
        if(!rs.ok()) {
            nlohmann::json j;
            j["code"] = rs.code();
            j["message"] = rs.message();
            response->set_body(j.dump());
            return;
        }
        rs = ApiProxy::instance()->delete_config(&config, &resp);

        nlohmann::json j;
        if (!rs.ok()) {
            j["code"] = rs.code();
            j["message"] = rs.message();
        } else {
            j["code"] = resp.code();
            j["message"] = resp.has_message() ? resp.message() : "ok";
        }
        response->set_body(j.dump());
    }

    void ListConfigVersionProcessor::process(const melon::RestfulRequest *request, melon::RestfulResponse *response) {
        response->set_content_json();
        response->set_access_control_all_allow();
        response->set_status_code(200);
        std::string input_str = request->body().to_string();
        nlohmann::json input;
        try {
            input = nlohmann::json::parse(input_str);
        } catch (const std::exception &e) {
            nlohmann::json j;
            j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
            j["message"] = "invalid json";
            response->set_body(j.dump());
            return;
        }
        std::string name;
        if(input.find("name") == input.end()) {
            nlohmann::json j;
            j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
            j["message"] = "name is required";
            response->set_body(j.dump());
            return;
        }
        name = input["name"].get<std::string>();
        ::eapi::sirius::ConfigQueryRequest req;
        ::eapi::sirius::ConfigListResponse resp;
        req.set_name(name);
        auto rs = ApiProxy::instance()->list_config_versions(&req, &resp);
        nlohmann::json j;
        if (!rs.ok()) {
            j["code"] = rs.code();
            j["message"] = rs.message();
        } else {
            j["code"] = resp.code();
            j["message"] = resp.has_message();
        }
        for (const auto &c : resp.configs()) {
            nlohmann::json item;
            item["name"] = c.name();
            item["version"] = sirius::version_to_string(c.version());
            item["content"] = c.content();
            item["type"] = sirius::config_type_to_string(c.type());
            item["createtime"] = c.time();
            item["id"] = c.id();
            j["configs"].push_back(item);
        }
        response->set_body(j.dump());
    }

    void ConfigQueryProcessor::process(const melon::RestfulRequest *request, melon::RestfulResponse *response) {
        response->set_content_json();
        response->set_access_control_all_allow();
        response->set_status_code(200);
        std::string input_str = request->body().to_string();
        nlohmann::json input;
        try {
            input = nlohmann::json::parse(input_str);
        } catch (const std::exception &e) {
            nlohmann::json j;
            j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
            j["message"] = "invalid json";
            response->set_body(j.dump());
            return;
        }
        std::string name;
        if(input.find("name") == input.end()) {
            nlohmann::json j;
            j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
            j["message"] = "name is required";
            response->set_body(j.dump());
            return;
        }
        name = input["name"].get<std::string>();
        std::string version;
        if(input.find("version") == input.end()) {
            version = "";
        } else {
            version = input["version"].get<std::string>();
        }
        turbo::Status rs;
        eapi::sirius::ConfigInfo config;
        eapi::sirius::ConfigListResponse resp;
        config.set_name(name);
        if(!version.empty()) {
            rs = string_to_version(version, config.mutable_version());
        }
        if(!rs.ok()) {
            nlohmann::json j;
            j["code"] = rs.code();
            j["message"] = rs.message();
            response->set_body(j.dump());
            return;
        }
        rs =ApiProxy::instance()->get_config(&config, &resp);
        nlohmann::json j;
        if (!rs.ok()) {
            j["code"] = rs.code();
            j["message"] = rs.message();
            response->set_body(j.dump());
            return;
        }
        j["code"] = resp.code();
        j["message"] = resp.has_message();
        if(resp.configs_size() > 0) {
            auto &conf = resp.configs(0);
            nlohmann::json item;
            item["name"] = conf.name();
            item["version"] = sirius::version_to_string(conf.version());
            item["content"] = conf.content();
            item["type"] = sirius::config_type_to_string(conf.type());
            item["createtime"] = conf.time();
            item["id"] = conf.id();
            j["configs"].push_back(item);
        }

        response->set_body(j.dump());
    }


}  // namespace sirius::restful