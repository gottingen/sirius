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


#pragma once

#include <melon/utility/endpoint.h>
#include <melon/rpc/channel.h>
#include <melon/rpc/server.h>
#include <melon/rpc/controller.h>
#include <google/protobuf/descriptor.h>
#include <sirius/proto/discovery.interface.pb.h>
#include <sirius/base/log.h>
#include <sirius/client/base_message_sender.h>

namespace sirius::client {

    /**
     * @ingroup ea_rpc
     * @brief DiscoverySender is used to send messages to the meta server.
     *       It communicates with the meta server and sends messages to the meta server.
     *       It needs to be initialized before use. It need judge the leader of meta server.
     *       If the leader is not found, it will retry to send the request to the meta server.
     *       If the peer is not leader, it will redirect to the leader and retry to send the
     *       request to the meta server.
     * @code
     *      DiscoverySender::get_instance()->init("127.0.0.1:8200");
     *      sirius::proto::DiscoveryManagerRequest request;
     *      sirius::proto::DiscoveryManagerResponse response;
     *      request.set_type(sirius::proto::DiscoveryManagerRequest::ADD);
     *      request.set_name("test");
     *      request.set_version("1.0.0");
     *      request.set_content("test");
     *      auto rs = DiscoverySender::get_instance()->discovery_manager(request, response);
     *      if(!rs.ok()) {
     *          TLOG_ERROR("meta manager error:{}", rs.message());
     *          return;
     *      }
     *      TLOG_INFO("meta manager success");
     *      return;
     *@endcode
     */
    class DiscoverySender : public BaseMessageSender {
    public:
        static const int kRetryTimes = 5;

        /**
         * @brief get_instance is used to get the singleton instance of DiscoverySender.
         * @return
         */
        static DiscoverySender *get_instance() {
            static DiscoverySender _instance;
            return &_instance;
        }

        /**
         * @brief get_backup_instance is used to get the singleton instance of DiscoverySender.
         * @return
         */
        static DiscoverySender *get_backup_instance() {
            static DiscoverySender _instance;
            return &_instance;
        }

        DiscoverySender() = default;

        /**
         * @brief is_inited is used to check if the DiscoverySender is initialized.
         * @return
         */
        bool is_inited() {
            return _is_inited;
        }

        /**
         * @brief init is used to initialize the DiscoverySender. It must be called before using the DiscoverySender.
         * @param raft_nodes [input] is the raft nodes of the meta server.
         * @return Status::OK if the DiscoverySender was initialized successfully. Otherwise, an error status is returned.
         */
        collie::Status init(const std::string &raft_nodes);

        /**
         * @brief init is used to initialize the DiscoverySender. It can be called any time.
         * @param verbose [input] is the verbose flag.
         * @return DiscoverySender itself.
         */
        DiscoverySender &set_verbose(bool verbose);

        /**
         * @brief set_time_out is used to set the timeout for sending a request to the meta server.
         * @param time_ms [input] is the timeout in milliseconds for sending a request to the meta server.
         * @return DiscoverySender itself.
         */
        DiscoverySender &set_time_out(int time_ms);


        /**
         * @brief set_connect_time_out is used to set the timeout for connecting to the meta server.
         * @param time_ms [input] is the timeout in milliseconds for connecting to the meta server.
         * @return DiscoverySender itself.
         */
        DiscoverySender &set_connect_time_out(int time_ms);

        /**
         * @brief set_interval_time is used to set the interval time for retrying to send a request to the meta server.
         * @param time_ms [input] is the interval time in milliseconds for retrying to send a request to the meta server.
         * @return DiscoverySender itself.
         */
        DiscoverySender &set_interval_time(int time_ms);

        /**
         * @brief set_retry_time is used to set the number of times to retry sending a request to the meta server.
         * @param retry [input] is the number of times to retry sending a request to the meta server.
         * @return DiscoverySender itself.
         */
        DiscoverySender &set_retry_time(int retry);

        /**
         * @brief get_leader is used to get the leader address of the meta server.
         * @return the leader address of the meta server.
         */
        std::string get_leader() const;

        /**
         * @brief discovery_manager is used to send a DiscoveryManagerRequest to the meta server for management.
         * @param request [input] is the DiscoveryManagerRequest to send.
         * @param response [output] is the MetaManagerResponse received from the meta server.
         * @param retry_time [input] is the number of times to retry sending the request.
         * @return Status::OK if the request was sent successfully. Otherwise, an error status is returned. 
         */
        collie::Status discovery_manager(const sirius::proto::DiscoveryManagerRequest &request,
                                   sirius::proto::DiscoveryManagerResponse &response, int retry_time) override;

        /**
         * @brief discovery_manager is used to send a DiscoveryManagerRequest to the meta server for management.
         * @param request [input] is the DiscoveryManagerRequest to send.
         * @param response [output] is the MetaManagerResponse received from the meta server.
         * @return Status::OK if the request was sent successfully. Otherwise, an error status is returned. 
         */
        collie::Status discovery_manager(const sirius::proto::DiscoveryManagerRequest &request,
                                   sirius::proto::DiscoveryManagerResponse &response) override;

        /**
         * @brief discovery_query is used to send a DiscoveryQueryRequest to the meta server for querying.
         * @param request [input] is the DiscoveryQueryRequest to send.
         * @param response [output] is the DiscoveryQueryResponse received from the meta server.
         * @param retry_time [input] is the number of times to retry sending the request.
         * @return Status::OK if the request was sent successfully. Otherwise, an error status is returned. 
         */
        collie::Status discovery_query(const sirius::proto::DiscoveryQueryRequest &request,
                                 sirius::proto::DiscoveryQueryResponse &response, int retry_time) override;


        /**
         * @brief discovery_query is used to send a DiscoveryQueryRequest to the meta server for querying.
         * @param request [input] is the DiscoveryQueryRequest to send.
         * @param response [output] is the DiscoveryQueryResponse received from the meta server.
         * @return Status::OK if the request was sent successfully. Otherwise, an error status is returned. 
         */
        collie::Status discovery_query(const sirius::proto::DiscoveryQueryRequest &request,
                                 sirius::proto::DiscoveryQueryResponse &response) override;

        /**
         * @brief send_request is used to send a request to the meta server.
         * @param service_name [input] is the name of the service to send the request to.
         * @param request [input] is the request to send.
         * @param response [output] is the response received from the meta server.
         * @param retry_times [input] is the number of times to retry sending the request.
         * @return Status::OK if the request was sent successfully. Otherwise, an error status is returned. 
         */
        template<typename Request, typename Response>
        collie::Status send_request(const std::string &service_name,
                                   const Request &request,
                                   Response &response, int retry_times);

    private:

        /**
         *
         * @param addr
         */
        void set_leader_address(const mutil::EndPoint &addr);

    private:
        std::string _meta_raft_group;
        std::string _meta_nodes;
        std::vector<mutil::EndPoint> _servlet_nodes;
        int32_t _request_timeout = 30000;
        int32_t _connect_timeout = 5000;
        bool _is_inited{false};
        std::mutex _master_leader_mutex;
        mutil::EndPoint _master_leader_address;
        int _between_meta_connect_error_ms{1000};
        int _retry_times{kRetryTimes};
        bool _verbose{false};
    };

    template<typename Request, typename Response>
    inline collie::Status DiscoverySender::send_request(const std::string &service_name,
                                                  const Request &request,
                                                  Response &response, int retry_times) {
        const ::google::protobuf::ServiceDescriptor *service_desc = sirius::proto::DiscoveryService::descriptor();
        const ::google::protobuf::MethodDescriptor *method =
                service_desc->FindMethodByName(service_name);
        if (method == nullptr) {
            SS_LOG_IF(ERROR, _verbose)<< "service name not exist, service:"<<service_name;
            return collie::Status::unavailable("service name not exist, service:{}", service_name);
        }
        int retry_time = 0;
        bool is_select_leader{false};
        uint64_t log_id = mutil::fast_rand();
        do {
            if (!is_select_leader && retry_time > 0 && _between_meta_connect_error_ms > 0) {
                fiber_usleep(1000 * _between_meta_connect_error_ms);
            }
            melon::Controller cntl;
            cntl.set_log_id(log_id);
            std::unique_lock<std::mutex> lck(_master_leader_mutex);
            mutil::EndPoint leader_address = _master_leader_address;
            lck.unlock();
            melon::ChannelOptions channel_opt;
            channel_opt.timeout_ms = _request_timeout;
            channel_opt.connect_timeout_ms = _connect_timeout;
            melon::Channel short_channel;
            is_select_leader = leader_address.ip == mutil::IP_ANY;
            //store has leader address
            if (is_select_leader) {
                SS_LOG_IF(INFO, _verbose) << "master address null, select leader first";
                auto seed = mutil::fast_rand() % _servlet_nodes.size();
                leader_address = _servlet_nodes[seed];
            }
            SS_LOG_IF(INFO, _verbose & is_select_leader) << "select leader address:" << mutil::endpoint2str(leader_address).c_str();
            if (short_channel.Init(leader_address, &channel_opt) != 0) {
                SS_LOG_IF(ERROR, _verbose) << "connect with meta server fail. channel Init fail, leader_addr:"
                                           << mutil::endpoint2str(leader_address).c_str();
                set_leader_address(mutil::EndPoint());
                ++retry_time;
                continue;
            }
            short_channel.CallMethod(method, &cntl, &request, &response, nullptr);
            SS_LOG_IF(INFO, _verbose) << "meta_req[" << request.ShortDebugString() << "], meta_resp["
                                      << response.ShortDebugString() << "]";
            if (cntl.Failed()) {
                SS_LOG_IF(WARN, _verbose) << "connect with server fail. send request fail, error:" << cntl.ErrorText()
                                          << ", log_id:" << cntl.log_id();
                set_leader_address(mutil::EndPoint());
                ++retry_time;
                continue;
            }
            if (response.errcode() == sirius::proto::HAVE_NOT_INIT) {
                SS_LOG_IF(WARN, _verbose) << "connect with server fail. HAVE_NOT_INIT  log_id:" << cntl.log_id();
                set_leader_address(mutil::EndPoint());
                ++retry_time;
                continue;
            }
            if (response.errcode() == sirius::proto::NOT_LEADER) {
                SS_LOG_IF(WARN, _verbose) << "connect with server fail. not leader, redirect to :"
                                          << response.leader() << ", log_id:" << cntl.log_id();
                mutil::EndPoint leader_addr;
                mutil::str2endpoint(response.leader().c_str(), &leader_addr);
                set_leader_address(leader_addr);
                // select leader do not cost retry times
                ++retry_time;
                continue;
            }
            /// success, The node being tried happens to be leader
            if (_master_leader_address.ip == mutil::IP_ANY && leader_address.ip != mutil::IP_ANY) {
                SS_LOG_IF(INFO, _verbose) << "set leader ip:" << mutil::endpoint2str(leader_address).c_str();
                set_leader_address(leader_address);
            }
            return collie::Status::ok_status();
        } while (retry_time < retry_times);
        return collie::Status::unavailable("can not connect server after {} times try", retry_times);
    }


}  // namespace sirius::client
