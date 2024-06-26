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
// Created by jeff on 24-6-24.
//
#include <sirius/restful/discovery.h>
#include <sirius/restful/client.h>
#include <collie/nlohmann/json.hpp>
#include <sirius/client/utility.h>

namespace sirius::restful {

    void AppCreateProcessor::process(const melon::RestfulRequest *request, melon::RestfulResponse *response) {
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

        std::string app_name;
        auto it = input.find("app_name");
        if (it != input.end()) {
            app_name = it->get<std::string>();
        } else {
            nlohmann::json j;
            j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
            j["message"] = "app_name is required";
            response->set_body(j.dump());
            return;
        }

        int quota = 0;
        it = input.find("quota");
        if (it != input.end()) {
            quota = it->get<int>();
        }

        auto rs = Client::instance().discovery().create_app(app_name, quota);
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

    void AppListProcessor::process(const melon::RestfulRequest *request, melon::RestfulResponse *response) {
        response->set_content_type("application/json");
        response->set_header("Access-Control-Allow-Origin", "*");
        response->set_header("Access-Control-Allow-Method","*");
        response->set_header("Access-Control-Allow-Headers","*");
        response->set_header("Access-Control-Allow-Credentials","true");
        response->set_header("Access-Control-Expose-Headers","*");
        response->set_status_code(200);

        std::vector<sirius::proto::AppInfo> apps;

        auto rs = Client::instance().discovery().list_namespace(apps);
        nlohmann::json j;
        if (!rs.ok()) {
            j["code"] = rs.code();
            j["message"] = rs.message();
            response->set_body(j.dump());
            return;
        } else {
            j["code"] = 0;
            j["message"] = "ok";
        }
        for(auto &app : apps) {
            nlohmann::json app_j;
            app_j["app_name"] = app.app_name();
            app_j["app_id"] = app.app_id();
            app_j["quota"] = app.quota();
            j["apps"].push_back(app_j);
        }
        response->set_body(j.dump());
    }
    void AppQueryProcessor::process(const melon::RestfulRequest *request, melon::RestfulResponse *response) {
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

        sirius::proto::AppInfo app;
        std::string app_name;
        auto it = input.find("app_name");
        if (it != input.end()) {
            app_name = it->get<std::string>();
        } else {
            nlohmann::json j;
            j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
            j["message"] = "app_name is required";
            response->set_body(j.dump());
            return;
        }

        auto rs = Client::instance().discovery().get_namespace(app_name, app);
        nlohmann::json j;
        if (!rs.ok()) {
            j["code"] = rs.code();
            j["message"] = rs.message();
            response->set_body(j.dump());
            return;
        } else {
            j["code"] = 0;
            j["message"] = "ok";
        }
        j["app_name"] = app.app_name();
        j["app_id"] = app.app_id();
        j["quota"] = app.quota();
        response->set_body(j.dump());
    }

    void AppRemoveProcessor::process(const melon::RestfulRequest *request, melon::RestfulResponse *response) {
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

        std::string app_name;
        auto it = input.find("app_name");
        if (it != input.end()) {
            app_name = it->get<std::string>();
        } else {
            nlohmann::json j;
            j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
            j["message"] = "app_name is required";
            response->set_body(j.dump());
            return;
        }

        auto rs = Client::instance().discovery().remove_namespace(app_name);
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

    void ZoneCreateProcessor::process(const melon::RestfulRequest *request, melon::RestfulResponse *response) {
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

        std::string app_name;
        auto it = input.find("app_name");
        if (it != input.end()) {
            app_name = it->get<std::string>();
        } else {
            nlohmann::json j;
            j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
            j["message"] = "app_name is required";
            response->set_body(j.dump());
            return;
        }

        std::string zone_name;
        it = input.find("zone_name");
        if (it != input.end()) {
            zone_name = it->get<std::string>();
        } else {
            nlohmann::json j;
            j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
            j["message"] = "zone_name is required";
            response->set_body(j.dump());
            return;
        }
        int64_t quota = 0;
        it = input.find("quota");
        if (it != input.end()) {
            quota = it->get<int>();
        }

        auto rs = Client::instance().discovery().create_zone(app_name, zone_name, quota);
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

    void ZoneListProcessor::process(const melon::RestfulRequest *request, melon::RestfulResponse *response) {
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

        std::string app_name;
        auto it = input.find("app_name");
        if (it != input.end()) {
            app_name = it->get<std::string>();
        }

        std::vector<sirius::proto::ZoneInfo> zones;
        turbo::Status rs;
        if(app_name.empty()) {
            rs = Client::instance().discovery().list_zone(zones);
        } else {
            rs = Client::instance().discovery().list_zone(app_name, zones);
        }
        nlohmann::json j;
        if (!rs.ok()) {
            j["code"] = rs.code();
            j["message"] = rs.message();
            response->set_body(j.dump());
            return;
        } else {
            j["code"] = 0;
            j["message"] = "ok";
        }
        for(auto &zone : zones) {
            nlohmann::json app_j;
            app_j["app_name"] = zone.app_name();
            app_j["app_id"] = zone.app_id();
            app_j["quota"] = zone.quota();
            app_j["zone_name"] = zone.zone();
            app_j["zone_id"] = zone.zone_id();
            j["zones"].push_back(app_j);
        }
        response->set_body(j.dump());
    }

    void ZoneQueryProcessor::process(const melon::RestfulRequest *request, melon::RestfulResponse *response) {
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

        std::string app_name;
        auto it = input.find("app_name");
        if (it != input.end()) {
            app_name = it->get<std::string>();
        } else {
            nlohmann::json j;
            j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
            j["message"] = "app_name is required";
            response->set_body(j.dump());
            return;
        }

        std::string zone_name;
        it = input.find("zone_name");
        if (it != input.end()) {
            zone_name = it->get<std::string>();
        } else {
            nlohmann::json j;
            j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
            j["message"] = "zone_name is required";
            response->set_body(j.dump());
            return;
        }

        sirius::proto::ZoneInfo zone;
        turbo::Status rs;
        if(app_name.empty()) {
            rs = Client::instance().discovery().get_zone(app_name, zone_name, zone);
        }
        nlohmann::json j;
        if (!rs.ok()) {
            j["code"] = rs.code();
            j["message"] = rs.message();
            response->set_body(j.dump());
            return;
        } else {
            j["code"] = 0;
            j["message"] = "ok";
        }
        j["app_name"] = zone.app_name();
        j["app_id"] = zone.app_id();
        j["quota"] = zone.quota();
        j["zone_name"] = zone.zone();
        j["zone_id"] = zone.zone_id();
        response->set_body(j.dump());
    }

    void ZoneRemoveProcessor::process(const melon::RestfulRequest *request, melon::RestfulResponse *response) {
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

        std::string app_name;
        auto it = input.find("app_name");
        if (it != input.end()) {
            app_name = it->get<std::string>();
        } else {
            nlohmann::json j;
            j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
            j["message"] = "app_name is required";
            response->set_body(j.dump());
            return;
        }

        std::string zone_name;
        it = input.find("zone_name");
        if (it != input.end()) {
            zone_name = it->get<std::string>();
        } else {
            nlohmann::json j;
            j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
            j["message"] = "zone_name is required";
            response->set_body(j.dump());
            return;
        }

        turbo::Status rs;
        if(app_name.empty()) {
            rs = Client::instance().discovery().remove_zone(app_name, zone_name);
        }
        nlohmann::json j;
        if (!rs.ok()) {
            j["code"] = rs.code();
            j["message"] = rs.message();
            response->set_body(j.dump());
            return;
        } else {
            j["code"] = 0;
            j["message"] = "ok";
        }
        response->set_body(j.dump());
    }
}  // namespace sirius::restful