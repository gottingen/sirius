// Copyright 2023 The Turbo Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#pragma once

#include <melon/rpc/channel.h>
#include <turbo/status/status.h>
#include <sirius/base/log.h>
#include <melon/utility/endpoint.h>
#include <melon/rpc/channel.h>
#include <melon/rpc/server.h>
#include <melon/rpc/controller.h>
#include <google/protobuf/descriptor.h>
#include <sirius/cli/option_context.h>
#include <sirius/proto/discovery.interface.pb.h>
#include <turbo/strings/substitute.h>
namespace sirius::cli {

    class RouterInteract {
    public:
        static RouterInteract *get_instance() {
            static RouterInteract ins;
            return &ins;
        }

        template<typename Request, typename Response>
        turbo::Status send_request(const std::string &service_name,
                                   const Request &request,
                                   Response &response) {
            const ::google::protobuf::ServiceDescriptor *service_desc = sirius::proto::DiscoveryService::descriptor();
            const ::google::protobuf::MethodDescriptor *method =
                    service_desc->FindMethodByName(service_name);
            auto verbose =  OptionContext::get_instance()->verbose;
            if (method == nullptr) {
                LOG_IF(ERROR, verbose) << "service name not exist, service: "<<service_name;
                return turbo::invalid_argument_error(turbo::substitute("service name not exist, service: $0", service_name));
            }
            int retry_time = 0;
            uint64_t log_id = mutil::fast_rand();
            do {
                if (retry_time > 0 && OptionContext::get_instance()->max_retry > 0) {
                    fiber_usleep(1000 * OptionContext::get_instance()->time_between_discovery_connect_error_ms);
                }
                melon::Controller cntl;
                cntl.set_log_id(log_id);
                //store has leader address
                melon::ChannelOptions channel_opt;
                channel_opt.timeout_ms =  OptionContext::get_instance()->timeout_ms;
                channel_opt.connect_timeout_ms = OptionContext::get_instance()->connect_timeout_ms;
                melon::Channel short_channel;
                if (short_channel.Init(OptionContext::get_instance()->router_server.c_str(), &channel_opt) != 0) {
                    LOG_IF(WARNING, verbose) << "connect with router server fail. channel Init fail, leader_addr:" << OptionContext::get_instance()->router_server;
                    ++retry_time;
                    continue;
                }
                short_channel.CallMethod(method, &cntl, &request, &response, nullptr);

                LOG_IF(INFO, verbose) << "router_req[" << request.ShortDebugString() << "], router_resp[" << response.ShortDebugString() << "]";
                if (cntl.Failed()) {
                    LOG_IF(WARNING, verbose) << "connect with router server fail. send request fail, error:" << cntl.ErrorText() << ", log_id:" << cntl.log_id();
                    ++retry_time;
                    continue;
                }

                if (response.errcode() != sirius::proto::SUCCESS) {
                    LOG_IF(WARNING, verbose) << "send discovery router server fail, log_id:" << cntl.log_id() << ", response:" << response.ShortDebugString();
                    //return turbo::UnavailableError("send discovery router server fail, log_id:{}, response:{}", cntl.log_id(),
                    //                               response.ShortDebugString());
                    return turbo::OkStatus();
                } else {
                    return turbo::OkStatus();
                }
            } while (retry_time < OptionContext::get_instance()->max_retry);
            return turbo::deadline_exceeded_error(turbo::substitute("try times $0 reach max_try $1 and can not get response.", retry_time, OptionContext::get_instance()->max_retry));

        }

    };
}  // namespace sirius::cli
