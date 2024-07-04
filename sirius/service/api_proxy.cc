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
// Created by jeff on 24-7-3.
//

#include <sirius/service/api_proxy.h>
#include <sirius/discovery/discovery_sender.h>

namespace sirius {

    void ApiProxy::abort_not_init() {
        if (!is_init_)
            LOG(FATAL) << "ApiProxy not initialized";
    }

    turbo::Status ApiProxy::initialize(const std::vector<std::string> &discovery_addr) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (is_init_) {
            return turbo::OkStatus();
        }
        auto rs = sender_.init(discovery_addr);
        if (!rs.ok()) {
            return rs;
        }
        is_init_ = true;
        return turbo::OkStatus();
    }

    turbo::Status ApiProxy::create_config(const ::eapi::sirius::ConfigInfo *request,
                                          ::eapi::CommonResponse *response, int retry) {
        abort_not_init();
        ::sirius::proto::DiscoveryManagerRequest req;
        ::sirius::proto::DiscoveryManagerResponse res;
        req.set_op_type(::sirius::proto::OP_CREATE_CONFIG);
        *req.mutable_config_info() = *request;
        auto rs = sender_.discovery_manager(req, res, retry);
        if (!rs.ok()) {
            LOG(ERROR)<<rs.message();
            return rs;
        }
        response->set_code(res.errcode());
        if (res.has_errmsg()) {
            response->set_message(res.errmsg());
        }
        return turbo::OkStatus();
    }

    turbo::Status ApiProxy::delete_config(const ::eapi::sirius::ConfigInfo *request,
                                          ::eapi::CommonResponse *response, int retry_time) {
        abort_not_init();
        ::sirius::proto::DiscoveryManagerRequest req;
        ::sirius::proto::DiscoveryManagerResponse res;
        *req.mutable_config_info() = *request;
        req.set_op_type(::sirius::proto::OP_REMOVE_CONFIG);
        auto rs = sender_.discovery_manager(req, res, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        response->set_code(res.errcode());
        if (res.has_errmsg()) {
            response->set_message(res.errmsg());
        }
        return turbo::OkStatus();
    }

    turbo::Status ApiProxy::get_config(const ::eapi::sirius::ConfigInfo *request,
                                       ::eapi::sirius::ConfigListResponse *response, int retry_time) {
        abort_not_init();
        ::sirius::proto::DiscoveryQueryRequest req;
        ::sirius::proto::DiscoveryQueryResponse res;
        if (request->name().empty()) {
            return turbo::invalid_argument_error("config name is empty");
        }
        req.set_config_name(request->name());
        *req.mutable_config_version() = request->version();
        req.set_op_type(::sirius::proto::QUERY_GET_CONFIG);
        auto rs = sender_.discovery_query(req, res, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        response->set_code(res.errcode());
        if (res.has_errmsg()) {
            response->set_message(res.errmsg());
        }
        if (res.config_infos_size() > 0) {
            *response->add_configs() = res.config_infos(0);
        }
        return turbo::OkStatus();
    }

    turbo::Status ApiProxy::list_config_versions(const ::eapi::sirius::ConfigQueryRequest *request,
                                                 ::eapi::sirius::ConfigListResponse *response,
                                                 int retry) {
        abort_not_init();
        sirius::proto::DiscoveryQueryRequest req;
        sirius::proto::DiscoveryQueryResponse res;
        req.set_op_type(sirius::proto::QUERY_LIST_CONFIG_VERSION);
        if (!request->has_name()) {
            return turbo::invalid_argument_error("config name is empty");
        }
        req.set_config_name(request->name());
        auto rs = sender_.discovery_query(req, res, retry);
        if (!rs.ok()) {
            return rs;
        }
        response->set_code(res.errcode());
        if (res.has_errmsg()) {
            response->set_message(res.errmsg());
        }

        response->mutable_configs()->Reserve(res.config_infos_size());
        for (int i = 0; i < res.config_infos_size(); ++i) {
            *response->add_configs() = res.config_infos(i);
        }
        return turbo::OkStatus();
    }

    turbo::Status ApiProxy::list_config(const ::eapi::sirius::ConfigQueryRequest *request,
                                        ::eapi::sirius::ConfigListResponse *response, int retry_time) {
        abort_not_init();
        ::sirius::proto::DiscoveryQueryRequest req;
        ::sirius::proto::DiscoveryQueryResponse res;
        req.set_op_type(sirius::proto::QUERY_LIST_CONFIG);
        auto rs = sender_.discovery_query(req, res, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        response->set_code(res.errcode());
        if (res.has_errmsg()) {
            response->set_message(res.errmsg());
        }
        response->mutable_configs()->Reserve(res.config_infos_size());
        for (int i = 0; i < res.config_infos_size(); ++i) {
            *response->add_configs() = res.config_infos(i);
        }
        return turbo::OkStatus();
    }

    turbo::Status ApiProxy::create_app(const ::eapi::sirius::AppInfo *request,
                                       ::eapi::CommonResponse *response,
                                       int retry) {
        abort_not_init();
        ::sirius::proto::DiscoveryManagerRequest req;
        ::sirius::proto::DiscoveryManagerResponse res;
        req.set_op_type(::sirius::proto::OP_CREATE_APP);
        *req.mutable_app_info() = *request;
        auto rs = sender_.discovery_manager(req, res, retry);
        if (!rs.ok()) {
            return rs;
        }
        response->set_code(res.errcode());
        if (res.has_errmsg()) {
            response->set_message(res.errmsg());
        }
        return turbo::OkStatus();
    }

    turbo::Status ApiProxy::update_app(const ::eapi::sirius::AppInfo *request,
                                       ::eapi::CommonResponse *response, int retry) {
        abort_not_init();
        sirius::proto::DiscoveryManagerRequest req;
        sirius::proto::DiscoveryManagerResponse res;
        req.set_op_type(sirius::proto::OP_MODIFY_APP);
        *req.mutable_app_info() = *request;
        auto rs = sender_.discovery_manager(req, res, retry);
        if (!rs.ok()) {
            return rs;
        }
        response->set_code(res.errcode());
        if (res.has_errmsg()) {
            response->set_message(res.errmsg());
        }
        return turbo::OkStatus();
    }

    turbo::Status ApiProxy::delete_app(const ::eapi::sirius::AppInfo *request,
                                       ::eapi::CommonResponse *response, int retry) {
        abort_not_init();
        sirius::proto::DiscoveryManagerRequest req;
        sirius::proto::DiscoveryManagerResponse res;
        req.set_op_type(sirius::proto::OP_REMOVE_APP);
        *req.mutable_app_info() = *request;
        auto rs = sender_.discovery_manager(req, res, retry);
        if (!rs.ok()) {
            return rs;
        }
        response->set_code(res.errcode());
        if (res.has_errmsg()) {
            response->set_message(res.errmsg());
        }
        return turbo::OkStatus();
    }

    turbo::Status ApiProxy::get_app(const ::eapi::sirius::SnsQueryRequest *request,
                                    ::eapi::sirius::AppListResponse *response,
                                    int retry) {
        abort_not_init();
        sirius::proto::DiscoveryQueryRequest req;
        sirius::proto::DiscoveryQueryResponse res;
        req.set_op_type(sirius::proto::QUERY_APP);
        if (request->app_name().empty()) {
            return turbo::invalid_argument_error("app name is empty");
        }
        req.set_app_name(request->app_name());
        auto rs = sender_.discovery_query(req, res, retry);
        if (!rs.ok()) {
            return rs;
        }
        response->set_code(res.errcode());
        if (res.has_errmsg()) {
            response->set_message(res.errmsg());
        }
        if (res.app_infos_size() > 0) {
            *response->add_app_info() = res.app_infos(0);
        }
        return turbo::OkStatus();
    }

    turbo::Status ApiProxy::list_app(const ::eapi::sirius::SnsQueryRequest *request,
                                     ::eapi::sirius::AppListResponse *response, int retry) {
        abort_not_init();
        sirius::proto::DiscoveryQueryRequest req;
        sirius::proto::DiscoveryQueryResponse res;
        req.set_op_type(sirius::proto::QUERY_APP);
        auto rs = sender_.discovery_query(req, res, retry);
        if (!rs.ok()) {
            return rs;
        }
        response->set_code(res.errcode());
        if (res.has_errmsg()) {
            response->set_message(res.errmsg());
        }
        response->mutable_app_info()->Reserve(res.app_infos_size());
        for (int i = 0; i < res.app_infos_size(); ++i) {
            *response->add_app_info() = res.app_infos(i);
        }
        return turbo::OkStatus();
    }

    turbo::Status ApiProxy::create_zone(const ::eapi::sirius::ZoneInfo *request,
                                        ::eapi::CommonResponse *response, int retry) {
        abort_not_init();
        sirius::proto::DiscoveryManagerRequest req;
        sirius::proto::DiscoveryManagerResponse res;
        req.set_op_type(sirius::proto::OP_CREATE_ZONE);
        *req.mutable_zone_info() = *request;
        auto rs = sender_.discovery_manager(req, res, retry);
        if (!rs.ok()) {
            return rs;
        }

        response->set_code(res.errcode());
        if (res.has_errmsg()) {
            response->set_message(res.errmsg());
        }
        return turbo::OkStatus();
    }

    turbo::Status ApiProxy::update_zone(const ::eapi::sirius::ZoneInfo *request,
                                        ::eapi::CommonResponse *response, int retry) {
        abort_not_init();
        sirius::proto::DiscoveryManagerRequest req;
        sirius::proto::DiscoveryManagerResponse res;
        req.set_op_type(sirius::proto::OP_MODIFY_ZONE);
        *req.mutable_zone_info() = *request;
        auto rs = sender_.discovery_manager(req, res, retry);
        if (!rs.ok()) {
            return rs;
        }
        response->set_code(res.errcode());
        if (res.has_errmsg()) {
            response->set_message(res.errmsg());
        }
        return turbo::OkStatus();
    }

    turbo::Status ApiProxy::delete_zone(const ::eapi::sirius::ZoneInfo *request,
                                        ::eapi::CommonResponse *response, int retry) {
        abort_not_init();
        sirius::proto::DiscoveryManagerRequest req;
        sirius::proto::DiscoveryManagerResponse res;
        req.set_op_type(sirius::proto::OP_DROP_ZONE);
        *req.mutable_zone_info() = *request;
        auto rs = sender_.discovery_manager(req, res, retry);
        if (!rs.ok()) {
            return rs;
        }
        response->set_code(res.errcode());
        if (res.has_errmsg()) {
            response->set_message(res.errmsg());
        }
        return turbo::OkStatus();
    }

    turbo::Status ApiProxy::get_zone(const ::eapi::sirius::SnsQueryRequest *request,
                                     ::eapi::sirius::ZoneListResponse *response, int retry) {
        abort_not_init();
        sirius::proto::DiscoveryQueryRequest req;
        sirius::proto::DiscoveryQueryResponse res;
        req.set_op_type(sirius::proto::QUERY_ZONE);
        if (!request->has_zone_name() || request->zone_name().empty()) {
            return turbo::invalid_argument_error("zone name is empty");
        }
        if (!request->has_app_name() || request->app_name().empty()) {
            return turbo::invalid_argument_error("app name is empty");
        }
        req.set_zone(request->zone_name());
        req.set_app_name(request->app_name());
        auto rs = sender_.discovery_query(req, res, retry);
        if (!rs.ok()) {
            return rs;
        }
        response->set_code(res.errcode());
        if (res.has_errmsg()) {
            response->set_message(res.errmsg());
        }
        if (res.zone_infos_size() > 0) {
            *response->add_zone_info() = res.zone_infos(0);
        }
        return turbo::OkStatus();
    }

    turbo::Status ApiProxy::list_zone(const ::eapi::sirius::SnsQueryRequest *request,
                                      ::eapi::sirius::ZoneListResponse *response,
                                      int retry) {
        abort_not_init();
        sirius::proto::DiscoveryQueryRequest req;
        sirius::proto::DiscoveryQueryResponse res;
        req.set_op_type(sirius::proto::QUERY_ZONE);
        auto rs = sender_.discovery_query(req, res, retry);
        if (!rs.ok()) {
            return rs;
        }
        response->set_code(res.errcode());
        if (res.has_errmsg()) {
            response->set_message(res.errmsg());
        }
        response->mutable_zone_info()->Reserve(res.zone_infos_size());
        for (int i = 0; i < res.zone_infos_size(); ++i) {
            *response->add_zone_info() = res.zone_infos(i);
        }
        return turbo::OkStatus();
    }

    turbo::Status ApiProxy::create_servlet(const ::eapi::sirius::ServletInfo *request,
                                           ::eapi::CommonResponse *response,
                                           int retry) {
        abort_not_init();
        sirius::proto::DiscoveryManagerRequest req;
        sirius::proto::DiscoveryManagerResponse res;
        req.set_op_type(sirius::proto::OP_CREATE_SERVLET);
        *req.mutable_servlet_info() = *request;
        auto rs = sender_.discovery_manager(req, res, retry);
        if (!rs.ok()) {
            return rs;
        }
        response->set_code(res.errcode());
        response->set_message(res.errmsg());

        return turbo::OkStatus();

    }

    turbo::Status ApiProxy::delete_servlet(const ::eapi::sirius::ServletInfo *request,
                                 ::eapi::CommonResponse *response,
                                 int retry) {
        abort_not_init();
        sirius::proto::DiscoveryManagerRequest req;
        sirius::proto::DiscoveryManagerResponse res;
        req.set_op_type(sirius::proto::OP_DROP_SERVLET);
        *req.mutable_servlet_info() = *request;
        auto rs = sender_.discovery_manager(req, res, retry);
        if (!rs.ok()) {
            return rs;
        }
        response->set_code(res.errcode());
        response->set_message(res.errmsg());
        return turbo::OkStatus();
    }

    turbo::Status ApiProxy::tombstone_servlet(const ::eapi::sirius::SnsManageRequest *request,
                                              ::eapi::CommonResponse *response, int retry) {
        abort_not_init();
        if (!request->has_servlet_name() || request->servlet_name().empty()) {
            return turbo::invalid_argument_error("servlet name is empty");
        }
        if (!request->has_zone_name() || request->zone_name().empty()) {
            return turbo::invalid_argument_error("zone name is empty");
        }
        if (!request->has_app_name() || request->app_name().empty()) {
            return turbo::invalid_argument_error("app name is empty");
        }
        sirius::proto::DiscoveryManagerRequest req;
        sirius::proto::DiscoveryManagerResponse res;
        req.set_op_type(sirius::proto::OP_TOMBSTONE_SERVLET);
        auto *sinfo = req.mutable_servlet_info();
        sinfo->set_app_name(request->app_name());
        sinfo->set_zone(request->zone_name());
        sinfo->set_servlet_name(request->servlet_name());
        sinfo->set_address("any");
        sinfo->set_env("any");
        auto rs = sender_.discovery_manager(req, res, retry);
        if (!rs.ok()) {
            return rs;
        }
        response->set_code(res.errcode());
        response->set_message(res.errmsg());
        return turbo::OkStatus();
    }

    turbo::Status ApiProxy::update_servlet(const ::eapi::sirius::ServletInfo *request,
                                           ::eapi::CommonResponse *response,
                                           int retry) {
        abort_not_init();
        sirius::proto::DiscoveryManagerRequest req;
        sirius::proto::DiscoveryManagerResponse res;
        req.set_op_type(sirius::proto::OP_MODIFY_SERVLET);
        *req.mutable_servlet_info() = *request;
        auto rs = sender_.discovery_manager(req, res, retry);
        if (!rs.ok()) {
            return rs;
        }
        response->set_code(res.errcode());
        response->set_message(res.errmsg());
        return turbo::OkStatus();
    }

    turbo::Status ApiProxy::online_zone(const ::eapi::sirius::SnsManageRequest* request,
                     ::eapi::CommonResponse* response,
                     int retry) {
        abort_not_init();
        if(!request->has_zone_name() || request->zone_name().empty()){
            return turbo::invalid_argument_error("zone name is empty");
        }
        if(!request->has_app_name() || request->app_name().empty()){
            return turbo::invalid_argument_error("app name is empty");
        }
        sirius::proto::DiscoveryManagerRequest req;
        sirius::proto::DiscoveryManagerResponse res;
        req.set_op_type(sirius::proto::OP_MODIFY_ZONE);
        auto *zone = req.mutable_zone_info();
        zone->set_app_name(request->app_name());
        zone->set_zone(request->zone_name());
        zone->set_status(eapi::sirius::NORMAL);
        auto rs = sender_.discovery_manager(req, res, retry);
        if (!rs.ok()) {
            return rs;
        }
        response->set_code(res.errcode());
        if (res.has_errmsg()) {
            response->set_message(res.errmsg());
        }
        return turbo::OkStatus();
    }

    turbo::Status ApiProxy::offline_zone(const ::eapi::sirius::SnsManageRequest* request,
                      ::eapi::CommonResponse* response,
                      int retry) {
        abort_not_init();
        if(!request->has_zone_name() || request->zone_name().empty()){
            return turbo::invalid_argument_error("zone name is empty");
        }
        if(!request->has_app_name() || request->app_name().empty()){
            return turbo::invalid_argument_error("app name is empty");
        }
        sirius::proto::DiscoveryManagerRequest req;
        sirius::proto::DiscoveryManagerResponse res;
        req.set_op_type(sirius::proto::OP_MODIFY_ZONE);
        auto *zone = req.mutable_zone_info();
        zone->set_app_name(request->app_name());
        zone->set_zone(request->zone_name());
        zone->set_status(eapi::sirius::OFFLINE);
        auto rs = sender_.discovery_manager(req, res, retry);
        if (!rs.ok()) {
            return rs;
        }
        response->set_code(res.errcode());
        if (res.has_errmsg()) {
            response->set_message(res.errmsg());
        }
        return turbo::OkStatus();
    }

    turbo::Status ApiProxy::offline_servlet(const ::eapi::sirius::SnsManageRequest *request,
                                            ::eapi::CommonResponse *response, int retry) {
        abort_not_init();
        if (!request->has_servlet_name() || request->servlet_name().empty()) {
            return turbo::invalid_argument_error("servlet name is empty");
        }
        if (!request->has_zone_name() || request->zone_name().empty()) {
            return turbo::invalid_argument_error("zone name is empty");
        }
        if (!request->has_app_name() || request->app_name().empty()) {
            return turbo::invalid_argument_error("app name is empty");
        }
        sirius::proto::DiscoveryManagerRequest req;
        sirius::proto::DiscoveryManagerResponse res;
        req.set_op_type(sirius::proto::OP_MGR_SERVLET);
        auto *sinfo = req.mutable_servlet_info();
        sinfo->set_app_name(request->app_name());
        sinfo->set_zone(request->zone_name());
        sinfo->set_servlet_name(request->servlet_name());
        sinfo->set_address("any");
        sinfo->set_env("any");
        sinfo->set_manager_status(eapi::sirius::OFFLINE);
        auto rs = sender_.discovery_manager(req, res, retry);
        if (!rs.ok()) {
            return rs;
        }
        response->set_code(res.errcode());
        response->set_message(res.errmsg());
        return turbo::OkStatus();
    }

    turbo::Status ApiProxy::online_servlet(const ::eapi::sirius::SnsManageRequest *request,
                                           ::eapi::CommonResponse *response, int retry) {
        abort_not_init();
        if (!request->has_servlet_name() || request->servlet_name().empty()) {
            return turbo::invalid_argument_error("servlet name is empty");
        }
        if (!request->has_zone_name() || request->zone_name().empty()) {
            return turbo::invalid_argument_error("zone name is empty");
        }
        if (!request->has_app_name() || request->app_name().empty()) {
            return turbo::invalid_argument_error("app name is empty");
        }
        sirius::proto::DiscoveryManagerRequest req;
        sirius::proto::DiscoveryManagerResponse res;
        req.set_op_type(sirius::proto::OP_MGR_SERVLET);
        auto *sinfo = req.mutable_servlet_info();
        sinfo->set_app_name(request->app_name());
        sinfo->set_zone(request->zone_name());
        sinfo->set_servlet_name(request->servlet_name());
        sinfo->set_address("any");
        sinfo->set_env("any");
        sinfo->set_manager_status(eapi::sirius::NORMAL);
        auto rs = sender_.discovery_manager(req, res, retry);
        if (!rs.ok()) {
            return rs;
        }
        response->set_code(res.errcode());
        response->set_message(res.errmsg());
        return turbo::OkStatus();
    }

    turbo::Status
    ApiProxy::list_servlet(const ::eapi::sirius::SnsQueryRequest *request, ::eapi::sirius::ServletListResponse *response,
                 int retry) {
        abort_not_init();
        sirius::proto::DiscoveryQueryRequest req;
        sirius::proto::DiscoveryQueryResponse res;
        req.set_op_type(sirius::proto::QUERY_SERVLET);
        auto rs = sender_.discovery_query(req, res, retry);
        if (!rs.ok()) {
            return rs;
        }
        response->set_code(res.errcode());
        if (res.has_errmsg()) {
            response->set_message(res.errmsg());
        }
        response->mutable_servlets()->Reserve(res.servlet_infos_size());
        for (int i = 0; i < res.servlet_infos_size(); ++i) {
            *response->add_servlets() = res.servlet_infos(i);
        }
        return turbo::OkStatus();
    }

    turbo::Status ApiProxy::get_servlet(const ::eapi::sirius::SnsQueryRequest *request,
                              ::eapi::sirius::ServletListResponse *response, int retry) {
        abort_not_init();
        sirius::proto::DiscoveryQueryRequest req;
        sirius::proto::DiscoveryQueryResponse res;
        req.set_op_type(sirius::proto::QUERY_SERVLET);
        if (!request->has_servlet() || request->servlet().empty()) {
            return turbo::invalid_argument_error("servlet name is empty");
        }
        if (!request->has_zone_name() || request->zone_name().empty()) {
            return turbo::invalid_argument_error("zone name is empty");
        }

        if (!request->has_app_name() || request->app_name().empty()) {
            return turbo::invalid_argument_error("app name is empty");
        }
        req.set_servlet(request->servlet());
        req.set_zone(request->zone_name());
        req.set_app_name(request->app_name());
        auto rs = sender_.discovery_query(req, res, retry);
        if (!rs.ok()) {
            return rs;
        }
        response->set_code(res.errcode());
        if (res.has_errmsg()) {
            response->set_message(res.errmsg());
        }
        if (res.servlet_infos_size() > 0) {
            *response->add_servlets() = res.servlet_infos(0);
        }
        return turbo::OkStatus();
    }

    turbo::Status ApiProxy::naming(const eapi::sirius::ServletNamingRequest *request, eapi::sirius::ServletNamingResponse *response) {
        abort_not_init();
        if (!request->has_app_name() || request->app_name().empty()) {
            return turbo::invalid_argument_error("app name is empty");
        }
        sirius::proto::DiscoveryQueryRequest req;
        sirius::proto::DiscoveryQueryResponse res;
        req.set_op_type(sirius::proto::QUERY_NAMING_SERVLET);
        *req.mutable_servlet_naming() = *request;
        auto rs = sender_.discovery_query(req, res);
        if (!rs.ok()) {
            return rs;
        }
        *response = res.servlet_naming();
        return turbo::OkStatus();
    }
}  // namespace sirius