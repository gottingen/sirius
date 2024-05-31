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


#ifndef EA_CLIENT_BASE_MESSAGE_SENDER_H_
#define EA_CLIENT_BASE_MESSAGE_SENDER_H_

#include <turbo/status/status.h>
#include <sirius/proto/discovery.interface.pb.h>

namespace sirius::client {

    /**
     * @ingroup ea_rpc
     * @brief BaseMessageSender is the interface for sending messages to the meta server.
     *        It is used by the DiscoveryClient to send messages to the meta server. The DiscoveryClient will
     *        implement this interface. 
     */
    class BaseMessageSender {
    public:
        virtual ~BaseMessageSender() = default;

        /**
         * @brief discovery_manager is used to send a DiscoveryManagerRequest to the meta server.
         * @param request [input] is the DiscoveryManagerRequest to send.
         * @param response [output] is the MetaManagerResponse received from the meta server.
         * @param retry_times [input] is the number of times to retry sending the request.
         * @return Status::OK if the request was sent successfully. Otherwise, an error status is returned. 
         */
        virtual turbo::Status discovery_manager(const sirius::proto::DiscoveryManagerRequest &request,
                                                                     sirius::proto::DiscoveryManagerResponse &response, int retry_times) = 0;
        /**
         * @brief discovery_manager is used to send a DiscoveryManagerRequest to the meta server.
         * @param request [input] is the DiscoveryManagerRequest to send.
         * @param response [output] is the MetaManagerResponse received from the meta server.
         * @return Status::OK if the request was sent successfully. Otherwise, an error status is returned. 
         */        
        virtual turbo::Status discovery_manager(const sirius::proto::DiscoveryManagerRequest &request,
                                           sirius::proto::DiscoveryManagerResponse &response) = 0;

        /**
         * @brief discovery_query is used to send a DiscoveryQueryRequest to the meta server.
         * @param request [input] is the DiscoveryQueryRequest to send.
         * @param response [output] is the DiscoveryQueryResponse received from the meta server.
         * @param retry_times [input] is the number of times to retry sending the request.
         * @return Status::OK if the request was sent successfully. Otherwise, an error status is returned.
         */
        virtual turbo::Status discovery_query(const sirius::proto::DiscoveryQueryRequest &request,
                                                    sirius::proto::DiscoveryQueryResponse &response, int retry_times) = 0;
        /**
         * @brief discovery_query is used to send a DiscoveryQueryRequest to the meta server.
         * @param request [input] is the DiscoveryQueryRequest to send.
         * @param response [output] is the DiscoveryQueryResponse received from the meta server.
         * @return Status::OK if the request was sent successfully. Otherwise, an error status is returned. 
         */
        virtual turbo::Status discovery_query(const sirius::proto::DiscoveryQueryRequest &request,
                                         sirius::proto::DiscoveryQueryResponse &response) = 0;

        virtual turbo::Status discovery_naming(const sirius::proto::ServletNamingRequest &request,
                                        sirius::proto::ServletNamingResponse &response, int retry_time)  = 0;

        virtual turbo::Status discovery_naming(const sirius::proto::ServletNamingRequest &request,
                                                sirius::proto::ServletNamingResponse &response)  = 0;
    };
}  // namespace sirius::client

#endif  // EA_CLIENT_BASE_MESSAGE_SENDER_H_
