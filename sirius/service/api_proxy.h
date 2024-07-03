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

#pragma once

#include <eapi/sirius/sirius.interface.pb.h>
#include <turbo/utility/status.h>
#include <sirius/discovery/discovery_sender.h>
#include <melon/proto/rpc/sns.pb.h>

namespace sirius {

    class ApiProxy {
    public:
        ApiProxy() = default;

        static ApiProxy *instance() {
            static ApiProxy ins;
            return &ins;
        }

        turbo::Status initialize(const std::vector<std::string> &discovery_addr);

        turbo::Status create_config(const ::eapi::sirius::ConfigInfo *request,
                                    ::eapi::CommonResponse *response, int retry = 3);

        turbo::Status delete_config(const ::eapi::sirius::ConfigInfo *request,
                                    ::eapi::CommonResponse *responsem, int retry = 3);

        turbo::Status get_config(const ::eapi::sirius::ConfigInfo *request,
                                 ::eapi::sirius::ConfigListResponse *response, int retry = 3);

        turbo::Status list_config_versions(const ::eapi::sirius::ConfigQueryRequest *request,
                                           ::eapi::sirius::ConfigListResponse *response,
                                           int retry = 3);

        turbo::Status list_config(const ::eapi::sirius::ConfigQueryRequest *request,
                                  ::eapi::sirius::ConfigListResponse *response, int retry = 3);

        turbo::Status create_app(const ::eapi::sirius::AppInfo *request,
                                 ::eapi::CommonResponse *response,
                                 int retry = 3);

        turbo::Status update_app(const ::eapi::sirius::AppInfo *request,
                                 ::eapi::CommonResponse *response, int retry = 3);

        turbo::Status delete_app(const ::eapi::sirius::AppInfo *request,
                                 ::eapi::CommonResponse *response, int retry = 3);

        turbo::Status get_app(const ::eapi::sirius::SnsQueryRequest *request,
                              ::eapi::sirius::AppListResponse *response,
                              int retry = 3);

        turbo::Status list_app(const ::eapi::sirius::SnsQueryRequest *request,
                               ::eapi::sirius::AppListResponse *response, int retry = 3);

        turbo::Status create_zone(const ::eapi::sirius::ZoneInfo *request,
                                  ::eapi::CommonResponse *response, int retry = 3);

        turbo::Status update_zone(const ::eapi::sirius::ZoneInfo *request,
                                  ::eapi::CommonResponse *response, int retry = 3);

        turbo::Status delete_zone(const ::eapi::sirius::ZoneInfo *request,
                                  ::eapi::CommonResponse *response, int retry = 3);

        turbo::Status get_zone(const ::eapi::sirius::SnsQueryRequest *request,
                               ::eapi::sirius::ZoneListResponse *response, int retry = 3);

        turbo::Status list_zone(const ::eapi::sirius::SnsQueryRequest *request,
                                ::eapi::sirius::ZoneListResponse *response,
                                int retry = 3);

        turbo::Status create_servlet(const ::eapi::sirius::ServletInfo *request,
                                     ::eapi::CommonResponse *response,
                                     int retry = 3);

        turbo::Status delete_servlet(const ::eapi::sirius::ServletInfo *request,
                                     ::eapi::CommonResponse *response,
                                     int retry = 3);

        turbo::Status tombstone_servlet(const ::eapi::sirius::SnsManageRequest *request,
                                        ::eapi::CommonResponse *response, int retry = 3);

        turbo::Status update_servlet(const ::eapi::sirius::ServletInfo *request,
                                     ::eapi::CommonResponse *response,
                                     int retry = 3);

        turbo::Status online_zone(const ::eapi::sirius::SnsManageRequest* request,
                         ::eapi::CommonResponse* response,
                         int retry = 3);

        turbo::Status offline_zone(const ::eapi::sirius::SnsManageRequest* request,
                         ::eapi::CommonResponse* response,
                         int retry = 3);

        turbo::Status offline_servlet(const ::eapi::sirius::SnsManageRequest *request,
                                      ::eapi::CommonResponse *response, int retry = 3);

        turbo::Status online_servlet(const ::eapi::sirius::SnsManageRequest *request,
                                     ::eapi::CommonResponse *response, int retry = 3);

        turbo::Status list_servlet(const ::eapi::sirius::SnsQueryRequest *request,
                                   ::eapi::sirius::ServletListResponse *response, int retry = 3);

        turbo::Status get_servlet(const ::eapi::sirius::SnsQueryRequest *request,
                                  ::eapi::sirius::ServletListResponse *response, int retry = 3);

        turbo::Status naming(const eapi::sirius::ServletNamingRequest *request, eapi::sirius::ServletNamingResponse *response);

    private:

        void abort_not_init();

        sirius::DiscoverySender sender_;
        std::atomic<bool> is_init_{false};
        std::mutex mutex_;
    };
}  // namespace sirius

