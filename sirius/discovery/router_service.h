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
// Created by jeff on 23-11-29.
//

#pragma once

#include <sirius/client/discovery_sender.h>

namespace sirius::discovery {

    class RouterServiceImpl : public sirius::proto::DiscoveryRouterService {
    public:

        static RouterServiceImpl* get_instance() {
            static RouterServiceImpl ins;
            return &ins;
        }

        collie::Status init(const std::string &discovery_peers);

        ~RouterServiceImpl()  = default;

        void discovery_manager(::google::protobuf::RpcController *controller,
                          const ::sirius::proto::DiscoveryManagerRequest *request,
                          ::sirius::proto::DiscoveryManagerResponse *response,
                          ::google::protobuf::Closure *done) override;

        void discovery_query(::google::protobuf::RpcController *controller,
                        const ::sirius::proto::DiscoveryQueryRequest *request,
                        ::sirius::proto::DiscoveryQueryResponse *response,
                        ::google::protobuf::Closure *done) override;

        void registry(::google::protobuf::RpcController *controller,
                   const ::sirius::proto::ServletInfo *request,
                     ::sirius::proto::DiscoveryRegisterResponse *response,
                        ::google::protobuf::Closure *done) override;

        void update(::google::protobuf::RpcController *controller,
                      const ::sirius::proto::ServletInfo *request,
                      ::sirius::proto::DiscoveryRegisterResponse *response,
                      ::google::protobuf::Closure *done) override;

        void cancel(::google::protobuf::RpcController *controller,
                    const ::sirius::proto::ServletInfo *request,
                    ::sirius::proto::DiscoveryRegisterResponse *response,
                    ::google::protobuf::Closure *done) override;

        void naming(google::protobuf::RpcController *controller,
                    const sirius::proto::ServletNamingRequest *request,
                    sirius::proto::ServletNamingResponse *response,
                    google::protobuf::Closure *done) override;

    private:
        bool _is_init;
        client::DiscoverySender _manager_sender;
        client::DiscoverySender _query_sender;
    };

}  // namespace sirius::discovery
