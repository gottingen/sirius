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
#include <melon/proto/rpc/sns.pb.h>
#include <turbo/utility/status.h>
#include <melon/proto/rpc/sns.pb.h>

namespace sirius {

    class SiriusServiceImpl : public eapi::sirius::SiriusService {
    public:
        SiriusServiceImpl() = default;

        static SiriusServiceImpl *instance() {
            static SiriusServiceImpl ins;
            return &ins;
        }


        turbo::Status initialize(const std::vector<std::string> &discovery_addr);

        void create_config(::google::protobuf::RpcController *controller,
                           const ::eapi::sirius::ConfigInfo *request,
                           ::eapi::CommonResponse *response,
                           ::google::protobuf::Closure *done) override;

        void delete_config(::google::protobuf::RpcController *controller,
                           const ::eapi::sirius::ConfigInfo *request,
                           ::eapi::CommonResponse *response,
                           ::google::protobuf::Closure *done) override;

        void get_config(::google::protobuf::RpcController *controller,
                        const ::eapi::sirius::ConfigInfo *request,
                        ::eapi::sirius::ConfigListResponse *response,
                        ::google::protobuf::Closure *done) override;

        void list_config_versions(::google::protobuf::RpcController *controller,
                                  const ::eapi::sirius::ConfigQueryRequest *request,
                                  ::eapi::sirius::ConfigListResponse *response,
                                  ::google::protobuf::Closure *done) override;

        void list_config(::google::protobuf::RpcController *controller,
                         const ::eapi::sirius::ConfigQueryRequest *request,
                         ::eapi::sirius::ConfigListResponse *response,
                         ::google::protobuf::Closure *done) override;

        void create_app(::google::protobuf::RpcController *controller,
                        const ::eapi::sirius::AppInfo *request,
                        ::eapi::CommonResponse *response,
                        ::google::protobuf::Closure *done) override;

        void update_app(::google::protobuf::RpcController *controller,
                        const ::eapi::sirius::AppInfo *request,
                        ::eapi::CommonResponse *response,
                        ::google::protobuf::Closure *done) override;

        void delete_app(::google::protobuf::RpcController *controller,
                        const ::eapi::sirius::AppInfo *request,
                        ::eapi::CommonResponse *response,
                        ::google::protobuf::Closure *done) override;

        void get_app(::google::protobuf::RpcController *controller,
                     const ::eapi::sirius::SnsQueryRequest *request,
                     ::eapi::sirius::AppListResponse *response,
                     ::google::protobuf::Closure *done) override;

        void list_app(::google::protobuf::RpcController *controller,
                      const ::eapi::sirius::SnsQueryRequest *request,
                      ::eapi::sirius::AppListResponse *response,
                      ::google::protobuf::Closure *done) override;

        void create_zone(::google::protobuf::RpcController *controller,
                         const ::eapi::sirius::ZoneInfo *request,
                         ::eapi::CommonResponse *response,
                         ::google::protobuf::Closure *done) override;

        void update_zone(::google::protobuf::RpcController *controller,
                         const ::eapi::sirius::ZoneInfo *request,
                         ::eapi::CommonResponse *response,
                         ::google::protobuf::Closure *done) override;

        void delete_zone(::google::protobuf::RpcController *controller,
                         const ::eapi::sirius::ZoneInfo *request,
                         ::eapi::CommonResponse *response,
                         ::google::protobuf::Closure *done) override;

        void get_zone(::google::protobuf::RpcController *controller,
                      const ::eapi::sirius::SnsQueryRequest *request,
                      ::eapi::sirius::ZoneListResponse *response,
                      ::google::protobuf::Closure *done) override;

        void list_zone(::google::protobuf::RpcController *controller,
                       const ::eapi::sirius::SnsQueryRequest *request,
                       ::eapi::sirius::ZoneListResponse *response,
                       ::google::protobuf::Closure *done) override;

        void offline_zone(::google::protobuf::RpcController* controller,
                                  const ::eapi::sirius::SnsManageRequest* request,
                                  ::eapi::CommonResponse* response,
                                  ::google::protobuf::Closure* done) override;
        void online_zone(::google::protobuf::RpcController* controller,
                                 const ::eapi::sirius::SnsManageRequest* request,
                                 ::eapi::CommonResponse* response,
                                 ::google::protobuf::Closure* done) override;
        void offline_servlet(::google::protobuf::RpcController* controller,
                                     const ::eapi::sirius::SnsManageRequest* request,
                                     ::eapi::CommonResponse* response,
                                     ::google::protobuf::Closure* done) override;

        void online_servlet(::google::protobuf::RpcController* controller,
                                    const ::eapi::sirius::SnsManageRequest* request,
                                    ::eapi::CommonResponse* response,
                                    ::google::protobuf::Closure* done) override;
        void tombstone_servlet(::google::protobuf::RpcController* controller,
                                       const ::eapi::sirius::SnsManageRequest* request,
                                       ::eapi::CommonResponse* response,
                                       ::google::protobuf::Closure* done) override;

        void list_servlet(::google::protobuf::RpcController* controller,
                                  const ::eapi::sirius::SnsQueryRequest* request,
                                  ::eapi::sirius::ServletListResponse* response,
                                  ::google::protobuf::Closure* done) override;
        void get_servlet(::google::protobuf::RpcController* controller,
                                 const ::eapi::sirius::SnsQueryRequest* request,
                                 ::eapi::sirius::ServletListResponse* response,
                                 ::google::protobuf::Closure* done) override;

    };

    class SnsServiceImpl : public melon::SnsService {
    public:

        static SnsServiceImpl* instance() {
            static SnsServiceImpl ins;
            return &ins;
        }

        void registry(::google::protobuf::RpcController* controller,
                      const ::melon::SnsPeer* request,
                      ::melon::SnsResponse* response,
                      ::google::protobuf::Closure* done) override;
        void update(::google::protobuf::RpcController* controller,
                    const ::melon::SnsPeer* request,
                    ::melon::SnsResponse* response,
                    ::google::protobuf::Closure* done) override;
        void cancel(::google::protobuf::RpcController* controller,
                    const ::melon::SnsPeer* request,
                    ::melon::SnsResponse* response,
                    ::google::protobuf::Closure* done) override;
        void naming(::google::protobuf::RpcController* controller,
                    const ::melon::SnsRequest* request,
                    ::melon::SnsResponse* response,
                    ::google::protobuf::Closure* done) override;
    };

}  // namespace sirius
