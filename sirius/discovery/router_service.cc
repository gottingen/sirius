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

}  // namespace sirius::discovery

namespace melon {

    void servlet_info_to_peer(::melon::SnsPeer* peer, const sirius::proto::ServletInfo& info) {
        peer->set_app_name(info.app_name());
        peer->set_zone(info.zone());
        peer->set_servlet_name(info.servlet_name());
        peer->set_address(info.address());
        peer->set_env(info.env());
        peer->set_color(info.color());
        peer->set_status(static_cast<melon::PeerStatus>(info.status()));
        peer->set_ctime(info.ctime());
        peer->set_mtime(info.mtime());
        peer->set_deleted(info.deleted());
    }

    void peer_to_servlet_info(sirius::proto::ServletInfo* info, const ::melon::SnsPeer& peer) {
        info->set_app_name(peer.app_name());
        info->set_zone(peer.zone());
        info->set_servlet_name(peer.servlet_name());
        info->set_address(peer.address());
        info->set_env(peer.env());
        info->set_color(peer.color());
        info->set_status(static_cast<int>(peer.status()));
        info->set_ctime(peer.ctime());
        info->set_mtime(peer.mtime());
        info->set_deleted(peer.deleted());
    }

    collie::Status SnsServiceImpl::init(const std::string &discovery_peers) {
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

    void SnsServiceImpl::registry(::google::protobuf::RpcController* controller,
                                  const ::melon::SnsPeer* req,
                                  ::melon::SnsResponse* res,
                                  ::google::protobuf::Closure* done) {
        auto rpc_discovery_func = [controller, req,res, done,this](){
            melon::ClosureGuard done_guard(done);
            ::sirius::proto::DiscoveryManagerRequest request;
            ::sirius::proto::DiscoveryManagerResponse response;
            auto *si = request.mutable_servlet_info();
            peer_to_servlet_info(si, *req);
            request.set_op_type(sirius::proto::OP_CREATE_SERVLET);
            auto ret = _manager_sender.discovery_manager(request, response, 2);
            res->set_errmsg(response.errmsg());
            if(!ret.ok()) {
                res->set_errcode(melon::Errno::OK);
                SS_LOG(ERROR) << "rpc to discovery server:discovery_manager error:" << controller->ErrorText();
            }
            res->set_errcode(melon::Errno::OK);
        };
        sirius::Fiber bth;
        bth.run(rpc_discovery_func);
        bth.join();

    }
    void SnsServiceImpl::update(::google::protobuf::RpcController* controller,
                                const ::melon::SnsPeer* req,
                                ::melon::SnsResponse* res,
                                ::google::protobuf::Closure* done) {
        auto rpc_discovery_func = [controller, req,res, done,this](){
            melon::ClosureGuard done_guard(done);
            ::sirius::proto::DiscoveryManagerRequest request;
            ::sirius::proto::DiscoveryManagerResponse response;
            auto *si = request.mutable_servlet_info();
            peer_to_servlet_info(si, *req);
            request.set_op_type(sirius::proto::OP_MODIFY_SERVLET);
            auto ret = _manager_sender.discovery_manager(request, response, 2);
            res->set_errmsg(response.errmsg());
            if(!ret.ok()) {
                res->set_errcode(melon::Errno::Internal);
                SS_LOG(ERROR) << "rpc to discovery server:discovery_manager error:" << controller->ErrorText();
            }
            res->set_errcode(melon::Errno::OK);
        };
        sirius::Fiber bth;
        bth.run(rpc_discovery_func);
        bth.join();
    }
    void SnsServiceImpl::cancel(::google::protobuf::RpcController* controller,
                                const ::melon::SnsPeer* req,
                                ::melon::SnsResponse* res,
                                ::google::protobuf::Closure* done) {
        auto rpc_discovery_func = [controller, req,res, done,this](){
            melon::ClosureGuard done_guard(done);
            ::sirius::proto::DiscoveryManagerRequest request;
            ::sirius::proto::DiscoveryManagerResponse response;
            auto *si = request.mutable_servlet_info();
            peer_to_servlet_info(si, *req);
            request.set_op_type(sirius::proto::OP_DROP_SERVLET);
            auto ret = _manager_sender.discovery_manager(request, response, 2);
            res->set_errmsg(response.errmsg());
            if(!ret.ok()) {
                res->set_errcode(melon::Errno::Internal);
                SS_LOG(ERROR) << "rpc to discovery server:discovery_manager error:" << controller->ErrorText();
            }
            res->set_errcode(melon::Errno::OK);
        };
        sirius::Fiber bth;
        bth.run(rpc_discovery_func);
        bth.join();
    }

    void SnsServiceImpl::naming(::google::protobuf::RpcController* controller,
                                const ::melon::SnsRequest* req,
                                ::melon::SnsResponse* res,
                                ::google::protobuf::Closure* done) {
        auto rpc_discovery_func = [controller, req,res, done, this](){
            melon::ClosureGuard done_guard(done);
            sirius::proto::ServletNamingRequest request;
            sirius::proto::ServletNamingResponse response;
            request.set_app_name(req->app_name());
            request.mutable_zones()->CopyFrom(req->zones());
            request.mutable_env()->CopyFrom(req->env());
            request.mutable_color()->CopyFrom(req->color());
            auto ret = _query_sender.discovery_naming(request, response, 2);
            if(!ret.ok()) {
                SS_LOG(ERROR) << "rpc to discovery server:naming error:" << controller->ErrorText();
                res->set_errcode(static_cast<melon::Errno>(ret.code()));
                res->set_errmsg(ret.message());
            }
            res->set_errcode(melon::Errno::OK);
            res->set_errmsg("ok");
            for(auto& s : response.servlets()) {
                melon::SnsPeer* peer = res->add_servlets();
                servlet_info_to_peer(peer, s);
            }
        };
        sirius::Fiber bth;
        bth.run(rpc_discovery_func);
        bth.join();

    }
}
