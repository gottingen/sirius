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
// Created by jeff on 24-7-4.
//

#include <sirius/service/restful_servlet.h>
#include <sirius/service/api_proxy.h>
#include <collie/nlohmann/json.hpp>
#include <sirius/service/utility.h>
#include <turbo/strings/str_split.h>

namespace sirius::restful {

    static void servlet_to_json(const eapi::sirius::ServletInfo &servlet, nlohmann::json &j) {
        j["app_id"] = servlet.app_id();
        j["app_name"] = servlet.app_name();
        j["zone_id"] = servlet.zone_id();
        j["zone_name"] = servlet.zone();
        j["servlet_id"] = servlet.servlet_id();
        j["servlet_name"] = servlet.servlet_name();
        j["color"] = servlet.color();
        j["status"] = servlet.status();
        j["ctime"] = servlet.ctime();
        j["mtime"] = servlet.mtime();
        j["env"] = servlet.env();
        j["address"] = servlet.address();
        j["manager_status"] = servlet.manager_status();
    }

    void ListServletProcessor::process(const melon::RestfulRequest *request, melon::RestfulResponse *response) {
        response->set_content_json();
        response->set_access_control_all_allow();
        response->set_status_code(200);
        eapi::sirius::SnsQueryRequest req;
        eapi::sirius::ServletListResponse resp;
        auto rs = ApiProxy::instance()->list_servlet(&req, &resp);
        nlohmann::json j;
        if (!rs.ok()) {
            j["code"] = rs.code();
            j["message"] = rs.message();
        } else {

            j["code"] = resp.code();
            j["message"] = resp.message();
        }

        for (int i = 0; i < resp.servlets_size(); i++) {
            auto &servlet = resp.servlets(i);
            nlohmann::json item;
            servlet_to_json(servlet, item);
            j["servlets"].push_back(item);
        }
        response->set_body(j.dump());
    }

    void GetServletProcessor::process(const melon::RestfulRequest *request, melon::RestfulResponse *response) {
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
        if (it == input.end()) {
            nlohmann::json j;
            j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
            j["message"] = "app_name is required";
            response->set_body(j.dump());
            return;
        }
        app_name = it->get<std::string>();

        std::string zone_name;
        it = input.find("zone_name");
        if (it == input.end()) {
            nlohmann::json j;
            j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
            j["message"] = "zone_name is required";
            response->set_body(j.dump());
            return;
        }

        zone_name = it->get<std::string>();

        std::string servlet_name;
        it = input.find("servlet_name");
        if (it == input.end()) {
            nlohmann::json j;
            j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
            j["message"] = "servlet_name is required";
            response->set_body(j.dump());
            return;
        }
        servlet_name = it->get<std::string>();
        eapi::sirius::SnsQueryRequest req;
        eapi::sirius::ServletListResponse resp;
        req.set_app_name(app_name);
        req.set_zone_name(zone_name);
        req.set_servlet(servlet_name);
        auto rs = ApiProxy::instance()->get_servlet(&req, &resp);
        nlohmann::json j;
        if (!rs.ok()) {
            j["code"] = rs.code();
            j["message"] = rs.message();
        } else {
            j["code"] = resp.code();
            j["message"] = resp.message();
        }

        if (resp.servlets_size() != 1) {
            j["code"] = static_cast<int>(turbo::StatusCode::kNotFound);
            j["message"] = "servlet not found";
        } else {
            nlohmann::json item;
            servlet_to_json(resp.servlets(0), item);
            j["servlets"].push_back(item);
        }

        response->set_body(j.dump());
    }

    void OnlineServletProcessor::process(const melon::RestfulRequest *request, melon::RestfulResponse *response) {
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
        if (it == input.end()) {
            nlohmann::json j;
            j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
            j["message"] = "app_name is required";
            response->set_body(j.dump());
            return;
        }
        app_name = it->get<std::string>();

        std::string zone_name;
        it = input.find("zone_name");
        if (it == input.end()) {
            nlohmann::json j;
            j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
            j["message"] = "zone_name is required";
            response->set_body(j.dump());
            return;
        }

        zone_name = it->get<std::string>();

        std::string servlet_name;
        it = input.find("servlet_name");
        if (it == input.end()) {
            nlohmann::json j;
            j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
            j["message"] = "servlet_name is required";
            response->set_body(j.dump());
            return;
        }
        servlet_name = it->get<std::string>();
        eapi::sirius::SnsManageRequest req;
        eapi::CommonResponse resp;
        req.set_app_name(app_name);
        req.set_zone_name(zone_name);
        req.set_servlet_name(servlet_name);
        auto rs = ApiProxy::instance()->online_servlet(&req, &resp);
        nlohmann::json j;
        if (!rs.ok()) {
            j["code"] = rs.code();
            j["message"] = rs.message();
        } else {
            j["code"] = resp.code();
            j["message"] = resp.message();
        }


        response->set_body(j.dump());
    }

    void OfflineServletProcessor::process(const melon::RestfulRequest *request, melon::RestfulResponse *response) {
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
        if (it == input.end()) {
            nlohmann::json j;
            j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
            j["message"] = "app_name is required";
            response->set_body(j.dump());
            return;
        }
        app_name = it->get<std::string>();

        std::string zone_name;
        it = input.find("zone_name");
        if (it == input.end()) {
            nlohmann::json j;
            j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
            j["message"] = "zone_name is required";
            response->set_body(j.dump());
            return;
        }

        zone_name = it->get<std::string>();

        std::string servlet_name;
        it = input.find("servlet_name");
        if (it == input.end()) {
            nlohmann::json j;
            j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
            j["message"] = "servlet_name is required";
            response->set_body(j.dump());
            return;
        }
        servlet_name = it->get<std::string>();
        eapi::sirius::SnsManageRequest req;
        eapi::CommonResponse resp;
        req.set_app_name(app_name);
        req.set_zone_name(zone_name);
        req.set_servlet_name(servlet_name);
        auto rs = ApiProxy::instance()->online_servlet(&req, &resp);
        nlohmann::json j;
        if (!rs.ok()) {
            j["code"] = rs.code();
            j["message"] = rs.message();
        } else {
            j["code"] = resp.code();
            j["message"] = resp.message();
        }

        response->set_body(j.dump());
    }

    void TombstoneServletProcessor::process(const melon::RestfulRequest *request, melon::RestfulResponse *response) {
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
        if (it == input.end()) {
            nlohmann::json j;
            j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
            j["message"] = "app_name is required";
            response->set_body(j.dump());
            return;
        }
        app_name = it->get<std::string>();

        std::string zone_name;
        it = input.find("zone_name");
        if (it == input.end()) {
            nlohmann::json j;
            j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
            j["message"] = "zone_name is required";
            response->set_body(j.dump());
            return;
        }

        zone_name = it->get<std::string>();

        std::string servlet_name;
        it = input.find("servlet_name");
        if (it == input.end()) {
            nlohmann::json j;
            j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
            j["message"] = "servlet_name is required";
            response->set_body(j.dump());
            return;
        }
        servlet_name = it->get<std::string>();
        eapi::sirius::SnsManageRequest req;
        eapi::CommonResponse resp;
        req.set_app_name(app_name);
        req.set_zone_name(zone_name);
        req.set_servlet_name(servlet_name);
        auto rs = ApiProxy::instance()->tombstone_servlet(&req, &resp);
        nlohmann::json j;
        if (!rs.ok()) {
            j["code"] = rs.code();
            j["message"] = rs.message();
        } else {
            j["code"] = resp.code();
            j["message"] = resp.message();
        }

        response->set_body(j.dump());
    }

    void NamingServletProcessor::process(const melon::RestfulRequest *request, melon::RestfulResponse *response) {
        response->set_content_json();
        response->set_access_control_all_allow();
        response->set_status_code(200);
        std::string input_str = request->body().to_string();
        nlohmann::json input;
        std::string app_name;
        std::vector<std::string> zone_name;
        std::vector<std::string> envs;
        std::vector<std::string> colors;
        try {
            input = nlohmann::json::parse(input_str);

            auto it = input.find("app_name");
            if (it == input.end()) {
                nlohmann::json j;
                j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
                j["message"] = "app_name is required";
                response->set_body(j.dump());
                return;
            }
            app_name = it->get<std::string>();
            it = input.find("zone_name");
            if (it == input.end()) {
                nlohmann::json j;
                j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
                j["message"] = "zone_name is required";
                response->set_body(j.dump());
                return;
            }
            zone_name = it->get<std::vector<std::string>>();

            it = input.find("envs");
            if (it == input.end()) {
                nlohmann::json j;
                j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
                j["message"] = "envs is required";
                response->set_body(j.dump());
                return;
            }
            envs = it->get<std::vector<std::string>>();

            it = input.find("colors");
            if (it == input.end()) {
                nlohmann::json j;
                j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
                j["message"] = "colors is required";
                response->set_body(j.dump());
                return;
            }
            colors = it->get<std::vector<std::string>>();
        } catch (const std::exception &e) {
            nlohmann::json j;
            j["code"] = static_cast<int>(turbo::StatusCode::kInvalidArgument);
            j["message"] = "invalid json";
            response->set_body(j.dump());
            return;
        }

        eapi::sirius::ServletNamingRequest req;
        eapi::sirius::ServletNamingResponse resp;
        req.set_app_name(app_name);
        req.mutable_zones()->Assign(zone_name.begin(), zone_name.end());
        req.mutable_env()->Assign(envs.begin(), envs.end());
        req.mutable_color()->Assign(colors.begin(), colors.end());

        auto rs = ApiProxy::instance()->naming(&req, &resp);
        nlohmann::json j;
        if (!rs.ok()) {
            j["code"] = rs.code();
            j["message"] = rs.message();
        } else {
            j["code"] = resp.code();
            j["message"] = resp.message();
        }

        for(int i = 0; i < resp.servlets_size(); i++) {
            nlohmann::json item;
            servlet_to_json(resp.servlets(i), item);
            j["servlets"].push_back(item);
        }
        response->set_body(j.dump());
    }
}  // namespace sirius::restful