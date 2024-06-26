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
// Created by jeff on 24-6-23.
//

#include <sirius/restful/config.h>
#include <sirius/restful/client.h>
#include <collie/nlohmann/json.hpp>
#include <sirius/client/utility.h>

namespace sirius::restful {


    void ListConfigProcessor::process(const melon::RestfulRequest *request, melon::RestfulResponse *response) {
        std::vector<sirius::proto::ConfigInfo> config;
        response->set_content_type("application/json");
        response->set_header("Access-Control-Allow-Origin", "*");
        response->set_header("Access-Control-Allow-Method","*");
        response->set_header("Access-Control-Allow-Headers","*");
        response->set_header("Access-Control-Allow-Credentials","true");
        response->set_header("Access-Control-Expose-Headers","*");
        response->set_status_code(200);
        auto rs = Client::instance().discovery().list_config(config);
        nlohmann::json j;
        if (!rs.ok()) {
            j["code"] = rs.code();
            j["message"] = rs.message();
        } else {
            j["code"] = 0;
            j["message"] = "ok";
        };
        for (const auto &c : config) {
            nlohmann::json item;
            item["name"] = c.name();
            item["version"] = sirius::client::version_to_string(c.version());
            item["content"] = c.content();
            item["type"] = sirius::client::config_type_to_string(c.type());
            item["createtime"] = c.time();
            item["id"] = c.id();
            LOG(INFO)<< "version major: " << c.version().major() << " minor: " << c.version().minor();
            LOG(INFO) << "config name: " << c.name() << " version: " << sirius::client::version_to_string(c.version());
            j["configs"].push_back(item);
        }
        response->set_body(j.dump());
    }

    void CreateConfigProcessor::process(const melon::RestfulRequest *request, melon::RestfulResponse *response) {
        response->set_content_type("application/json");
        response->set_status_code(200);
        response->set_header("Access-Control-Allow-Origin", "*");
        response->set_header("Access-Control-Allow-Method","*");
        response->set_header("Access-Control-Allow-Headers","*");
        response->set_header("Access-Control-Allow-Credentials","true");
        response->set_header("Access-Control-Expose-Headers","*");
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
        auto rs = Client::instance().discovery().create_config(name, content, version, type);
        if (!rs.ok()) {
            j["code"] = rs.code();
            j["message"] = rs.message();
        } else {
            j["code"] = 0;
            j["message"] = "ok";
        }
        response->set_body(j.dump());
    }

    void RemoveConfigProcessor::process(const melon::RestfulRequest *request, melon::RestfulResponse *response) {
        response->set_content_type("application/json");
        response->set_header("Access-Control-Allow-Origin", "*");
        response->set_header("Access-Control-Allow-Method","*");
        response->set_header("Access-Control-Allow-Headers","*");
        response->set_header("Access-Control-Allow-Credentials","true");
        response->set_header("Access-Control-Expose-Headers","*");
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
        if(version.empty()) {
            rs = Client::instance().discovery().remove_config_all_version(name);
        } else {
            rs = Client::instance().discovery().remove_config(name, version);
        }

        nlohmann::json j;
        if (!rs.ok()) {
            j["code"] = rs.code();
            j["message"] = rs.message();
        } else {
            j["code"] = 0;
            j["message"] = "ok";
        }
        response->set_body(j.dump());
    }

    void ListConfigVersionProcessor::process(const melon::RestfulRequest *request, melon::RestfulResponse *response) {
        std::vector<sirius::proto::ConfigInfo> versions;
        response->set_content_type("application/json");
        response->set_header("Access-Control-Allow-Origin", "*");
        response->set_header("Access-Control-Allow-Method","*");
        response->set_header("Access-Control-Allow-Headers","*");
        response->set_header("Access-Control-Allow-Credentials","true");
        response->set_header("Access-Control-Expose-Headers","*");
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
        auto rs = Client::instance().discovery().list_config_version(name, versions);
        nlohmann::json j;
        if (!rs.ok()) {
            j["code"] = rs.code();
            j["message"] = rs.message();
        } else {
            j["code"] = 0;
            j["message"] = "ok";
        }
        for (const auto &c : versions) {
            nlohmann::json item;
            item["name"] = c.name();
            item["version"] = sirius::client::version_to_string(c.version());
            item["content"] = c.content();
            item["type"] = sirius::client::config_type_to_string(c.type());
            item["createtime"] = c.time();
            item["id"] = c.id();
            j["configs"].push_back(item);
        }
        response->set_body(j.dump());
    }

    void ConfigQueryProcessor::process(const melon::RestfulRequest *request, melon::RestfulResponse *response) {
        std::vector<std::string> versions;
        response->set_content_type("application/json");
        response->set_header("Access-Control-Allow-Origin", "*");
        response->set_header("Access-Control-Allow-Method","*");
        response->set_header("Access-Control-Allow-Headers","*");
        response->set_header("Access-Control-Allow-Credentials","true");
        response->set_header("Access-Control-Expose-Headers","*");
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
        sirius::proto::ConfigInfo config;
        if(version.empty()) {
            rs = Client::instance().discovery().get_config_latest(name, config);
        } else {
            rs = Client::instance().discovery().get_config(name, version, config);
        }
        nlohmann::json j;
        if (!rs.ok()) {
            j["code"] = rs.code();
            j["message"] = rs.message();
        } else {
            j["code"] = 0;
            j["message"] = "ok";
        }
        nlohmann::json item;
        item["name"] = config.name();
        item["version"] = sirius::client::version_to_string(config.version());
        item["content"] = config.content();
        item["type"] = sirius::client::config_type_to_string(config.type());
        item["createtime"] = config.time();
        item["id"] = config.id();
        j["configs"].push_back(item);
        response->set_body(j.dump());
    }


}  // namespace sirius::restful