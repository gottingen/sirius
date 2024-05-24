
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


#include <sirius/client/router_sender.h>

namespace sirius::client {

    collie::Status RouterSender::init(const std::string &server) {
        _server = server;
        return collie::Status::ok_status();
    }

    RouterSender &RouterSender::set_server(const std::string &server) {
        std::unique_lock lk(_server_mutex);
        _server = server;
        return *this;
    }

    RouterSender &RouterSender::set_verbose(bool verbose) {
        _verbose = verbose;
        return *this;
    }

    RouterSender &RouterSender::set_time_out(int time_ms) {
        _timeout_ms = time_ms;
        return *this;
    }

    RouterSender &RouterSender::set_connect_time_out(int time_ms) {
        _connect_timeout_ms = time_ms;
        return *this;
    }

    RouterSender &RouterSender::set_interval_time(int time_ms) {
        _between_meta_connect_error_ms = time_ms;
        return *this;
    }

    RouterSender &RouterSender::set_retry_time(int retry) {
        _retry_times = retry;
        return *this;
    }

    collie::Status RouterSender::discovery_manager(const sirius::proto::DiscoveryManagerRequest &request,
                                             sirius::proto::DiscoveryManagerResponse &response, int retry_times) {
        return send_request("discovery_manager", request, response, retry_times);
    }

    collie::Status RouterSender::discovery_manager(const sirius::proto::DiscoveryManagerRequest &request,
                                             sirius::proto::DiscoveryManagerResponse &response) {
        return send_request("discovery_manager", request, response, _retry_times);
    }

    collie::Status RouterSender::discovery_query(const sirius::proto::DiscoveryQueryRequest &request,
                                           sirius::proto::DiscoveryQueryResponse &response, int retry_times) {
        return send_request("discovery_query", request, response, retry_times);
    }

    collie::Status RouterSender::discovery_query(const sirius::proto::DiscoveryQueryRequest &request,
                                           sirius::proto::DiscoveryQueryResponse &response) {
        return send_request("discovery_query", request, response, _retry_times);
    }

    collie::Status RouterSender::discovery_naming(const sirius::proto::ServletNamingRequest &request,
                                    sirius::proto::ServletNamingResponse &response, int retry_time) {
        return send_request("naming", request, response, retry_time);
    }

    collie::Status RouterSender::discovery_naming(const sirius::proto::ServletNamingRequest &request,
                                    sirius::proto::ServletNamingResponse &response) {
        return send_request("naming", request, response, _retry_times);
    }

    collie::Status RouterSender::discovery_register(const sirius::proto::ServletInfo &info,
                                      sirius::proto::DiscoveryRegisterResponse &response, int retry_time) {
        return send_request("registry", info, response, retry_time);
    }

    collie::Status RouterSender::discovery_update(const sirius::proto::ServletInfo &info,
                                    sirius::proto::DiscoveryRegisterResponse &response, int retry_time) {
        return send_request("update", info, response, retry_time);
    }

    collie::Status RouterSender::discovery_cancel(const sirius::proto::ServletInfo &info,
                                    sirius::proto::DiscoveryRegisterResponse &response, int retry_time) {
        return send_request("cancel", info, response, retry_time);
    }


}  // sirius::client

