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


#include <sirius/client/discovery_sender.h>
#include <melon/raft/route_table.h>
#include <melon/raft/raft.h>
#include <melon/raft/util.h>
#include <collie/strings/str_split.h>

namespace sirius::client {


    collie::Status DiscoverySender::init(const std::string & raft_nodes) {
        _master_leader_address.ip = mutil::IP_ANY;
        std::vector<std::string> peers = collie::str_split(raft_nodes, collie::ByAnyChar(",;\t\n "));
        for (auto &peer : peers) {
            mutil::EndPoint end_point;
            if(mutil::str2endpoint(peer.c_str(), &end_point) != 0) {
                return collie::Status::invalid_argument("invalid address {}", peer);
            }
            _servlet_nodes.push_back(end_point);
        }
        return collie::Status::ok_status();
    }


    std::string DiscoverySender::get_leader() const {
        SS_LOG_IF(INFO, _verbose) << "get master address:" << mutil::endpoint2str(_master_leader_address).c_str();
        return mutil::endpoint2str(_master_leader_address).c_str();
    }

    void DiscoverySender::set_leader_address(const mutil::EndPoint &addr) {
        std::unique_lock<std::mutex> lock(_master_leader_mutex);
        _master_leader_address = addr;
        SS_LOG_IF(INFO, _verbose) << "set master address:" << mutil::endpoint2str(_master_leader_address).c_str();
    }

    collie::Status DiscoverySender::discovery_manager(const sirius::proto::DiscoveryManagerRequest &request,
                                           sirius::proto::DiscoveryManagerResponse &response, int retry_times) {
        return send_request("discovery_manager", request, response, retry_times);
    }

    collie::Status DiscoverySender::discovery_manager(const sirius::proto::DiscoveryManagerRequest &request,
                                           sirius::proto::DiscoveryManagerResponse &response) {
        return send_request("discovery_manager", request, response, _retry_times);
    }

    collie::Status DiscoverySender::discovery_query(const sirius::proto::DiscoveryQueryRequest &request,
                                         sirius::proto::DiscoveryQueryResponse &response, int retry_times) {
        return send_request("discovery_query", request, response, retry_times);
    }

    collie::Status DiscoverySender::discovery_query(const sirius::proto::DiscoveryQueryRequest &request,
                                         sirius::proto::DiscoveryQueryResponse &response) {
        return send_request("discovery_query", request, response, _retry_times);
    }

    collie::Status DiscoverySender::discovery_naming(const sirius::proto::ServletNamingRequest &request,
                                    sirius::proto::ServletNamingResponse &response, int retry_time) {
        return send_request("naming", request, response, retry_time);
    }

    collie::Status DiscoverySender::discovery_naming(const sirius::proto::ServletNamingRequest &request,
                                                     sirius::proto::ServletNamingResponse &response) {
        return send_request("naming", request, response, _retry_times);
    }


    DiscoverySender &DiscoverySender::set_verbose(bool verbose) {
        _verbose = verbose;
        return *this;
    }

    DiscoverySender &DiscoverySender::set_time_out(int time_ms) {
        _request_timeout = time_ms;
        return *this;
    }

    DiscoverySender &DiscoverySender::set_connect_time_out(int time_ms) {
        _connect_timeout = time_ms;
        return *this;
    }

    DiscoverySender &DiscoverySender::set_interval_time(int time_ms) {
        _between_meta_connect_error_ms = time_ms;
        return *this;
    }

    DiscoverySender &DiscoverySender::set_retry_time(int retry) {
        _retry_times = retry;
        return *this;
    }

}  // sirius::client

