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
#include <sirius/service/restful_sns.h>
#include <sirius/service/api_proxy.h>
#include <collie/nlohmann/json.hpp>
#include <sirius/service/utility.h>

namespace sirius::restful {

    void AppCreateProcessor::process(const melon::RestfulRequest *request, melon::RestfulResponse *response) {
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
        eapi::sirius::AppInfo app;
        app.set_app_name(app_name);
        app.set_quota(quota);
        eapi::CommonResponse res;
        auto rs = ApiProxy::instance()->create_app(&app, &res);
        nlohmann::json j;
        if (!rs.ok()) {
            j["code"] = rs.code();
            j["message"] = rs.message();
        } else {
            j["code"] = res.code();
            j["message"] = res.message();
        }
        response->set_body(j.dump());
    }

    void AppListProcessor::process(const melon::RestfulRequest *request, melon::RestfulResponse *response) {
        response->set_content_json();
        response->set_access_control_all_allow();
        response->set_status_code(200);

        eapi::sirius::SnsQueryRequest req;
        eapi::sirius::AppListResponse res;
        auto rs = ApiProxy::instance()->list_app(&req, &res);
        nlohmann::json j;
        if (!rs.ok()) {
            j["code"] = rs.code();
            j["message"] = rs.message();
            response->set_body(j.dump());
            return;
        }
        j["code"] = res.code();
        j["message"] = res.message();
        for (auto &app: res.app_info()) {
            nlohmann::json app_j;
            app_j["app_name"] = app.app_name();
            app_j["app_id"] = app.app_id();
            app_j["quota"] = app.quota();
            j["apps"].push_back(app_j);
        }
        response->set_body(j.dump());
    }

    void AppQueryProcessor::process(const melon::RestfulRequest *request, melon::RestfulResponse *response) {
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
        eapi::sirius::SnsQueryRequest req;
        req.set_app_name(app_name);
        eapi::sirius::AppListResponse res;
        auto rs = ApiProxy::instance()->get_app(&req, &res);
        nlohmann::json j;
        if (!rs.ok()) {
            j["code"] = rs.code();
            j["message"] = rs.message();
            response->set_body(j.dump());
            return;
        }
        if(res.app_info_size() != 1) {
            j["code"] = static_cast<int>(turbo::StatusCode::kNotFound);
            j["message"] = "app not found";
            response->set_body(j.dump());
            return;
        }
        j["code"] = res.code();
        j["message"] = res.message();
        if(res.app_info_size() == 1) {
            auto &app = res.app_info(0);
            j["app_name"] = app.app_name();
            j["app_id"] = app.app_id();
            j["quota"] = app.quota();
            j["code"] = res.code();
            j["message"] = res.message();
        }
        response->set_body(j.dump());
    }

    void AppRemoveProcessor::process(const melon::RestfulRequest *request, melon::RestfulResponse *response) {
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
        eapi::sirius::AppInfo app;
        app.set_app_name(app_name);
        eapi::CommonResponse res;
        auto rs = ApiProxy::instance()->delete_app(&app, &res);
        nlohmann::json j;
        if (!rs.ok()) {
            j["code"] = rs.code();
            j["message"] = rs.message();
        } else {
            j["code"] = res.code();
            j["message"] = res.message();
        }
        response->set_body(j.dump());
    }

    void ZoneCreateProcessor::process(const melon::RestfulRequest *request, melon::RestfulResponse *response) {
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
        eapi::sirius::ZoneInfo zone;
        zone.set_app_name(app_name);
        zone.set_zone(zone_name);
        zone.set_quota(quota);
        eapi::CommonResponse res;
        auto rs = ApiProxy::instance()->create_zone(&zone, &res);
        nlohmann::json j;
        if (!rs.ok()) {
            j["code"] = rs.code();
            j["message"] = rs.message();
        } else {
            j["code"] = res.code();
            j["message"] = res.message();
        }
        response->set_body(j.dump());
    }

    void ZoneListProcessor::process(const melon::RestfulRequest *request, melon::RestfulResponse *response) {
        response->set_content_json();
        response->set_access_control_all_allow();
        response->set_status_code(200);

        std::string input_str = request->body().to_string();

        eapi::sirius::SnsQueryRequest req;
        eapi::sirius::ZoneListResponse res;
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
            req.set_app_name(app_name);
        }

        turbo::Status rs = ApiProxy::instance()->list_zone(&req, &res);
        nlohmann::json j;
        if (!rs.ok()) {
            j["code"] = rs.code();
            j["message"] = rs.message();
            response->set_body(j.dump());
            return;
        } else {
            j["code"] = res.code();
            j["message"] = res.message();
        }
        for (auto &zone: res.zone_info()) {
            nlohmann::json app_j;
            if(!app_name.empty() && app_name != zone.app_name()) {
                continue;
            }
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

        eapi::sirius::SnsQueryRequest req;
        req.set_app_name(app_name);
        req.set_zone_name(zone_name);
        eapi::sirius::ZoneListResponse res;
        auto rs = ApiProxy::instance()->get_zone(&req, &res);
        nlohmann::json j;
        if (!rs.ok()) {
            j["code"] = rs.code();
            j["message"] = rs.message();
            response->set_body(j.dump());
            return;
        } else {
            j["code"] = res.code();
            j["message"] = res.message();
        }
        if(res.zone_info_size() == 1) {
            auto &zone = res.zone_info(0);
            j["app_name"] = zone.app_name();
            j["app_id"] = zone.app_id();
            j["quota"] = zone.quota();
            j["zone_name"] = zone.zone();
            j["zone_id"] = zone.zone_id();
            response->set_body(j.dump());
        }
    }

    void ZoneRemoveProcessor::process(const melon::RestfulRequest *request, melon::RestfulResponse *response) {
        response->set_content_type("application/json");
        response->set_header("Access-Control-Allow-Origin", "*");
        response->set_header("Access-Control-Allow-Method", "*");
        response->set_header("Access-Control-Allow-Headers", "*");
        response->set_header("Access-Control-Allow-Credentials", "true");
        response->set_header("Access-Control-Expose-Headers", "*");
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
        if (app_name.empty()) {
            nlohmann::json j;
            j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
            j["message"] = "app_name is required, but is empty";
            response->set_body(j.dump());
            return;
        }
        if (zone_name.empty()) {
            nlohmann::json j;
            j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
            j["message"] = "zone_name is required, but is empty";
            response->set_body(j.dump());
            return;
        }
        eapi::sirius::ZoneInfo zone;
        zone.set_app_name(app_name);
        zone.set_zone(zone_name);
        eapi::CommonResponse res;
        rs = ApiProxy::instance()->delete_zone(&zone, &res);
        nlohmann::json j;
        if (!rs.ok()) {
            j["code"] = rs.code();
            j["message"] = rs.message();
        } else {
            j["code"] = res.code();
            j["message"] = res.message();
        }
        response->set_body(j.dump());
    }
}  // namespace sirius::restful