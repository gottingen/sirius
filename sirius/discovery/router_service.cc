// Copyright (c) 2020 Baidu, Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <sirius/proto/discovery.interface.pb.h>
#include <sirius/discovery/router_service.h>
#include <sirius/base/fiber.h>

namespace sirius::discovery {

    collie::Status RouterServiceImpl::init(const std::string &discovery_peers) {
        if(_is_init) {
            return  collie::Status::ok_status();
        }
        auto rs = _manager_sender.init(discovery_peers);
        if(!rs.ok()) {
            return rs;
        }
        rs = _query_sender.init(discovery_peers);
        if(!rs.ok()) {
            return rs;
        }
        _is_init = true;
        return collie::Status::ok_status();
    }
    void RouterServiceImpl::discovery_manager(::google::protobuf::RpcController* controller,
                      const ::sirius::proto::DiscoveryManagerRequest* request,
                      ::sirius::proto::DiscoveryManagerResponse* response,
                      ::google::protobuf::Closure* done) {

        auto rpc_discovery_func = [controller, request,response, done,this](){
            melon::ClosureGuard done_guard(done);
            auto ret = _manager_sender.discovery_manager(*request, *response, 2);
            if(!ret.ok()) {
                SS_LOG(ERROR) << "rpc to discovery server:discovery_manager error:" << controller->ErrorText();
            }
        };
        Fiber bth;
        bth.run(rpc_discovery_func);
        bth.join();
    }

    void RouterServiceImpl::discovery_query(::google::protobuf::RpcController* controller,
               const ::sirius::proto::DiscoveryQueryRequest* request,
               ::sirius::proto::DiscoveryQueryResponse* response,
               ::google::protobuf::Closure* done) {
        auto rpc_discovery_func = [controller, request,response, done, this](){
            melon::ClosureGuard done_guard(done);
            auto ret = _query_sender.discovery_query(*request, *response, 2);
            if(!ret.ok()) {
                SS_LOG(ERROR) << "rpc to discovery server:discovery_manager error:" << controller->ErrorText();
            }
        };
        Fiber bth;
        bth.run(rpc_discovery_func);
        bth.join();
    }
    void RouterServiceImpl::registry(::google::protobuf::RpcController *controller,
                  const ::sirius::proto::ServletInfo *request,
                  ::sirius::proto::DiscoveryRegisterResponse *response,
                  ::google::protobuf::Closure *done) {
        auto rpc_discovery_func = [controller, request,response, done,this](){
            melon::ClosureGuard done_guard(done);
            ::sirius::proto::DiscoveryManagerRequest req;
            ::sirius::proto::DiscoveryManagerResponse res;
            req.mutable_servlet_info()->CopyFrom(*request);
            req.set_op_type(sirius::proto::OP_CREATE_SERVLET);
            auto ret = _manager_sender.discovery_manager(req, res, 2);
            response->set_errcode(res.errcode());
            response->set_errmsg(res.errmsg());
            if(!ret.ok()) {
                SS_LOG(ERROR) << "rpc to discovery server:discovery_manager error:" << controller->ErrorText();
            }
        };
        Fiber bth;
        bth.run(rpc_discovery_func);
        bth.join();
    }

    void RouterServiceImpl::update(::google::protobuf::RpcController *controller,
                const ::sirius::proto::ServletInfo *request,
                ::sirius::proto::DiscoveryRegisterResponse *response,
                ::google::protobuf::Closure *done)  {
        auto rpc_discovery_func = [controller, request,response, done,this](){
            melon::ClosureGuard done_guard(done);
            ::sirius::proto::DiscoveryManagerRequest req;
            ::sirius::proto::DiscoveryManagerResponse res;
            req.mutable_servlet_info()->CopyFrom(*request);
            req.set_op_type(sirius::proto::OP_MODIFY_SERVLET);
            auto ret = _manager_sender.discovery_manager(req, res, 2);
            response->set_errcode(res.errcode());
            response->set_errmsg(res.errmsg());
            if(!ret.ok()) {
                SS_LOG(ERROR) << "rpc to discovery server:discovery_manager error:" << controller->ErrorText();
            }
        };
        Fiber bth;
        bth.run(rpc_discovery_func);
        bth.join();
    }

    void RouterServiceImpl::cancel(::google::protobuf::RpcController *controller,
                const ::sirius::proto::ServletInfo *request,
                ::sirius::proto::DiscoveryRegisterResponse *response,
                ::google::protobuf::Closure *done) {
        auto rpc_discovery_func = [controller, request,response, done,this](){
            melon::ClosureGuard done_guard(done);
            ::sirius::proto::DiscoveryManagerRequest req;
            ::sirius::proto::DiscoveryManagerResponse res;
            req.mutable_servlet_info()->CopyFrom(*request);
            req.set_op_type(sirius::proto::OP_DROP_SERVLET);
            auto ret = _manager_sender.discovery_manager(req, res, 2);
            response->set_errcode(res.errcode());
            response->set_errmsg(res.errmsg());
            if(!ret.ok()) {
                SS_LOG(ERROR) << "rpc to discovery server:discovery_manager error:" << controller->ErrorText();
            }
        };
        Fiber bth;
        bth.run(rpc_discovery_func);
        bth.join();

    }

    void RouterServiceImpl::naming(google::protobuf::RpcController *controller,
                const sirius::proto::ServletNamingRequest *request,
                sirius::proto::ServletNamingResponse *response,
                google::protobuf::Closure *done) {
        auto rpc_discovery_func = [controller, request,response, done, this](){
            melon::ClosureGuard done_guard(done);
            auto ret = _query_sender.discovery_naming(*request, *response, 2);
            if(!ret.ok()) {
                SS_LOG(ERROR) << "rpc to discovery server:naming error:" << controller->ErrorText();
            }
        };
        Fiber bth;
        bth.run(rpc_discovery_func);
        bth.join();
    }

}  // namespace sirius::discovery
