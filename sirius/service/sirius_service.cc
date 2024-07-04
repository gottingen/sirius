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

#include <sirius/service/sirius_service.h>
#include <sirius/service/api_proxy.h>
#include <sirius/proto/discovery.interface.pb.h>
#include <melon/rpc/closure_guard.h>
#include <sirius/base/fiber.h>

namespace sirius {

    turbo::Status SiriusServiceImpl::initialize(const std::vector<std::string> &discovery_addr) {
        return ApiProxy::instance()->initialize(discovery_addr);
    }


    void SiriusServiceImpl::create_config(::google::protobuf::RpcController *controller,
                       const ::eapi::sirius::ConfigInfo *request,
                       ::eapi::CommonResponse *response,
                       ::google::protobuf::Closure *done) {
        auto create_config_func = [this, controller, request, response, done]() {
            melon::ClosureGuard guard(done);
            turbo::Status status = ApiProxy::instance()->create_config(request, response);
            if (!status.ok()) {
                controller->SetFailed(status.to_string());
            }
            
        };

        sirius::Fiber bth;
        bth.run(create_config_func);
        bth.join();
    }

    void SiriusServiceImpl::delete_config(::google::protobuf::RpcController *controller,
                       const ::eapi::sirius::ConfigInfo *request,
                       ::eapi::CommonResponse *response,
                       ::google::protobuf::Closure *done) {

        auto delete_config_func = [this, controller, request, response, done]() {
            melon::ClosureGuard guard(done);
            turbo::Status status = ApiProxy::instance()->delete_config(request, response);
            if (!status.ok()) {
                controller->SetFailed(status.to_string());
            }
            
        };

        sirius::Fiber bth;
        bth.run(delete_config_func);
        bth.join();
    }

    void SiriusServiceImpl::get_config(::google::protobuf::RpcController *controller,
                    const ::eapi::sirius::ConfigInfo *request,
                    ::eapi::sirius::ConfigListResponse *response,
                    ::google::protobuf::Closure *done) {
        auto get_config_func = [this, controller, request, response, done]() {
            melon::ClosureGuard guard(done);
            turbo::Status status = ApiProxy::instance()->get_config(request, response);
            if (!status.ok()) {
                controller->SetFailed(status.to_string());
            }
            
        };

        sirius::Fiber bth;
        bth.run(get_config_func);
        bth.join();
    }

    void SiriusServiceImpl::list_config_versions(::google::protobuf::RpcController* controller,
                              const ::eapi::sirius::ConfigQueryRequest* request,
                              ::eapi::sirius::ConfigListResponse* response,
                              ::google::protobuf::Closure* done)  {

    }

    void SiriusServiceImpl::list_config(::google::protobuf::RpcController *controller,
                     const ::eapi::sirius::ConfigQueryRequest *request,
                     ::eapi::sirius::ConfigListResponse *response,
                     ::google::protobuf::Closure *done) {
        auto list_config_func = [this, controller, request, response, done]() {
            melon::ClosureGuard guard(done);
            turbo::Status status = ApiProxy::instance()->list_config(request, response);
            if (!status.ok()) {
                controller->SetFailed(status.to_string());
            }
            
        };

        sirius::Fiber bth;
        bth.run(list_config_func);
        bth.join();
    }

    void SiriusServiceImpl::create_app(::google::protobuf::RpcController *controller,
                    const ::eapi::sirius::AppInfo *request,
                    ::eapi::CommonResponse *response,
                    ::google::protobuf::Closure *done) {
        auto create_app_func = [this, controller, request, response, done]() {
            melon::ClosureGuard guard(done);
            turbo::Status status = ApiProxy::instance()->create_app(request, response);
            if (!status.ok()) {
                controller->SetFailed(status.to_string());
            }
            
        };

        sirius::Fiber bth;
        bth.run(create_app_func);
        bth.join();
    }

    void SiriusServiceImpl::update_app(::google::protobuf::RpcController *controller,
                    const ::eapi::sirius::AppInfo *request,
                    ::eapi::CommonResponse *response,
                    ::google::protobuf::Closure *done) {
        auto update_app_func = [this, controller, request, response, done]() {
            melon::ClosureGuard guard(done);
            turbo::Status status = ApiProxy::instance()->update_app(request, response);
            if (!status.ok()) {
                controller->SetFailed(status.to_string());
            }
            
        };

        sirius::Fiber bth;
        bth.run(update_app_func);
        bth.join();
    }

    void SiriusServiceImpl::delete_app(::google::protobuf::RpcController *controller,
                    const ::eapi::sirius::AppInfo *request,
                    ::eapi::CommonResponse *response,
                    ::google::protobuf::Closure *done) {
        auto delete_app_func = [this, controller, request, response, done]() {
            melon::ClosureGuard guard(done);
            turbo::Status status = ApiProxy::instance()->delete_app(request, response);
            if (!status.ok()) {
                controller->SetFailed(status.to_string());
            }
            
        };

        sirius::Fiber bth;
        bth.run(delete_app_func);
        bth.join();

    }

    void SiriusServiceImpl::get_app(::google::protobuf::RpcController *controller,
                 const ::eapi::sirius::SnsQueryRequest *request,
                 ::eapi::sirius::AppListResponse *response,
                 ::google::protobuf::Closure *done) {
        auto get_app_func = [this, controller, request, response, done]() {
            melon::ClosureGuard guard(done);
            turbo::Status status = ApiProxy::instance()->get_app(request, response);
            if (!status.ok()) {
                controller->SetFailed(status.to_string());
            }
            
        };

        sirius::Fiber bth;
        bth.run(get_app_func);
        bth.join();
    }

    void SiriusServiceImpl::list_app(::google::protobuf::RpcController *controller,
                  const ::eapi::sirius::SnsQueryRequest *request,
                  ::eapi::sirius::AppListResponse *response,
                  ::google::protobuf::Closure *done) {
        auto list_app_func = [this, controller, request, response, done]() {
            melon::ClosureGuard guard(done);
            turbo::Status status = ApiProxy::instance()->list_app(request, response);
            if (!status.ok()) {
                controller->SetFailed(status.to_string());
            }
            
        };

        sirius::Fiber bth;
        bth.run(list_app_func);
        bth.join();
    }

    void SiriusServiceImpl::create_zone(::google::protobuf::RpcController *controller,
                     const ::eapi::sirius::ZoneInfo *request,
                     ::eapi::CommonResponse *response,
                     ::google::protobuf::Closure *done) {
        auto create_zone_func = [this, controller, request, response, done]() {
            melon::ClosureGuard guard(done);
            turbo::Status status = ApiProxy::instance()->create_zone(request, response);
            if (!status.ok()) {
                controller->SetFailed(status.to_string());
            }
        };

        sirius::Fiber bth;
        bth.run(create_zone_func);
        bth.join();
    }

    void SiriusServiceImpl::update_zone(::google::protobuf::RpcController *controller,
                     const ::eapi::sirius::ZoneInfo *request,
                     ::eapi::CommonResponse *response,
                     ::google::protobuf::Closure *done) {
        auto update_zone_func = [this, controller, request, response, done]() {
            melon::ClosureGuard guard(done);
            turbo::Status status = ApiProxy::instance()->update_zone(request, response);
            if (!status.ok()) {
                controller->SetFailed(status.to_string());
            }
            
        };

        sirius::Fiber bth;
        bth.run(update_zone_func);
        bth.join();
    }

    void SiriusServiceImpl::delete_zone(::google::protobuf::RpcController *controller,
                     const ::eapi::sirius::ZoneInfo *request,
                     ::eapi::CommonResponse *response,
                     ::google::protobuf::Closure *done) {
        auto delete_zone_func = [this, controller, request, response, done]() {
            melon::ClosureGuard guard(done);
            turbo::Status status = ApiProxy::instance()->delete_zone(request, response);
            if (!status.ok()) {
                controller->SetFailed(status.to_string());
            }
            
        };

        sirius::Fiber bth;
        bth.run(delete_zone_func);
        bth.join();
    }

    void SiriusServiceImpl::get_zone(::google::protobuf::RpcController *controller,
                  const ::eapi::sirius::SnsQueryRequest *request,
                  ::eapi::sirius::ZoneListResponse *response,
                  ::google::protobuf::Closure *done) {
        auto get_zone_func = [this, controller, request, response, done]() {
            melon::ClosureGuard guard(done);
            turbo::Status status = ApiProxy::instance()->get_zone(request, response);
            if (!status.ok()) {
                controller->SetFailed(status.to_string());
            }
            
        };

        sirius::Fiber bth;
        bth.run(get_zone_func);
        bth.join();
    }

    void SiriusServiceImpl::list_zone(::google::protobuf::RpcController *controller,
                   const ::eapi::sirius::SnsQueryRequest *request,
                   ::eapi::sirius::ZoneListResponse *response,
                   ::google::protobuf::Closure *done) {
        auto list_zone_func = [this, controller, request, response, done]() {
            melon::ClosureGuard guard(done);
            turbo::Status status = ApiProxy::instance()->list_zone(request, response);
            if (!status.ok()) {
                controller->SetFailed(status.to_string());
            }
            
        };

        sirius::Fiber bth;
        bth.run(list_zone_func);
        bth.join();
    }

    void SiriusServiceImpl::offline_zone(::google::protobuf::RpcController* controller,
                      const ::eapi::sirius::SnsManageRequest* request,
                      ::eapi::CommonResponse* response,
                      ::google::protobuf::Closure* done) {
        auto offline_zone_func = [this, controller, request, response, done]() {
            melon::ClosureGuard guard(done);
            turbo::Status status = ApiProxy::instance()->offline_zone(request, response);
            if (!status.ok()) {
                controller->SetFailed(status.to_string());
            }
            
        };

        sirius::Fiber bth;
        bth.run(offline_zone_func);
        bth.join();
    }
    void SiriusServiceImpl::online_zone(::google::protobuf::RpcController* controller,
                     const ::eapi::sirius::SnsManageRequest* request,
                     ::eapi::CommonResponse* response,
                     ::google::protobuf::Closure* done) {
        auto online_zone_func = [this, controller, request, response, done]() {
            melon::ClosureGuard guard(done);
            turbo::Status status = ApiProxy::instance()->online_zone(request, response);
            if (!status.ok()) {
                controller->SetFailed(status.to_string());
            }
            
        };

        sirius::Fiber bth;
        bth.run(online_zone_func);
        bth.join();
    }
    void SiriusServiceImpl::offline_servlet(::google::protobuf::RpcController* controller,
                         const ::eapi::sirius::SnsManageRequest* request,
                         ::eapi::CommonResponse* response,
                         ::google::protobuf::Closure* done)  {
        auto offline_servlet_func = [this, controller, request, response, done]() {
            melon::ClosureGuard guard(done);
            turbo::Status status = ApiProxy::instance()->offline_servlet(request, response);
            if (!status.ok()) {
                controller->SetFailed(status.to_string());
            }
            
        };

        sirius::Fiber bth;
        bth.run(offline_servlet_func);
        bth.join();
    }

    void SiriusServiceImpl::online_servlet(::google::protobuf::RpcController* controller,
                        const ::eapi::sirius::SnsManageRequest* request,
                        ::eapi::CommonResponse* response,
                        ::google::protobuf::Closure* done) {
        auto online_servlet_func = [this, controller, request, response, done]() {
            melon::ClosureGuard guard(done);
            turbo::Status status = ApiProxy::instance()->online_servlet(request, response);
            if (!status.ok()) {
                controller->SetFailed(status.to_string());
            }
            
        };

        sirius::Fiber bth;
        bth.run(online_servlet_func);
        bth.join();
    }
    void SiriusServiceImpl::tombstone_servlet(::google::protobuf::RpcController* controller,
                           const ::eapi::sirius::SnsManageRequest* request,
                           ::eapi::CommonResponse* response,
                           ::google::protobuf::Closure* done) {
        auto tombstone_servlet_func = [this, controller, request, response, done]() {
            melon::ClosureGuard guard(done);
            turbo::Status status = ApiProxy::instance()->tombstone_servlet(request, response);
            if (!status.ok()) {
                controller->SetFailed(status.to_string());
            }
            
        };

        sirius::Fiber bth;
        bth.run(tombstone_servlet_func);
        bth.join();
    }

    void SiriusServiceImpl::list_servlet(::google::protobuf::RpcController* controller,
                      const ::eapi::sirius::SnsQueryRequest* request,
                      ::eapi::sirius::ServletListResponse* response,
                      ::google::protobuf::Closure* done)  {
        auto list_servlet_func = [this, controller, request, response, done]() {
            melon::ClosureGuard guard(done);
            turbo::Status status = ApiProxy::instance()->list_servlet(request, response);
            if (!status.ok()) {
                controller->SetFailed(status.to_string());
            }
            
        };

        sirius::Fiber bth;
        bth.run(list_servlet_func);
        bth.join();
    }
    void SiriusServiceImpl::get_servlet(::google::protobuf::RpcController* controller,
                     const ::eapi::sirius::SnsQueryRequest* request,
                     ::eapi::sirius::ServletListResponse* response,
                     ::google::protobuf::Closure* done)  {
        auto get_servlet_func = [this, controller, request, response, done]() {
            melon::ClosureGuard guard(done);
            turbo::Status status = ApiProxy::instance()->get_servlet(request, response);
            if (!status.ok()) {
                controller->SetFailed(status.to_string());
            }
            
        };

        sirius::Fiber bth;
        bth.run(get_servlet_func);
        bth.join();
    }


    void servlet_info_to_peer(::melon::SnsPeer* peer, const eapi::sirius::ServletInfo& info) {
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

    void peer_to_servlet_info(eapi::sirius::ServletInfo* info, const ::melon::SnsPeer& peer) {
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

    void SnsServiceImpl::registry(::google::protobuf::RpcController* controller,
                                  const ::melon::SnsPeer* req,
                                  ::melon::SnsResponse* res,
                                  ::google::protobuf::Closure* done) {
        auto rpc_discovery_func = [controller, req,res, done,this](){
            melon::ClosureGuard done_guard(done);
            eapi::sirius::ServletInfo info;
            peer_to_servlet_info(&info, *req);
            eapi::CommonResponse response;
            auto rs = ApiProxy::instance()->create_servlet(&info, &response);
            if(!rs.ok()) {
                controller->SetFailed(rs.to_string());
            }
            res->set_errcode(static_cast<melon::Errno>(response.code()));
            res->set_errmsg(response.message());
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
            eapi::sirius::ServletInfo info;
            peer_to_servlet_info(&info, *req);
            eapi::CommonResponse response;
            auto rs = ApiProxy::instance()->update_servlet(&info, &response);
            if(!rs.ok()) {
                controller->SetFailed(rs.to_string());
                return ;
            }
            res->set_errcode(static_cast<melon::Errno>(response.code()));
            res->set_errmsg(response.message());
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
            eapi::sirius::ServletInfo info;
            peer_to_servlet_info(&info, *req);
            eapi::CommonResponse response;
            auto rs = ApiProxy::instance()->delete_servlet(&info, &response);
            if(!rs.ok()) {
                controller->SetFailed(rs.to_string());
                return ;
            }
            res->set_errcode(static_cast<melon::Errno>(response.code()));
            res->set_errmsg(response.message());
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
            eapi::sirius::ServletNamingRequest request;
            eapi::sirius::ServletNamingResponse response;
            request.set_app_name(req->app_name());
            request.mutable_zones()->CopyFrom(req->zones());
            request.mutable_env()->CopyFrom(req->env());
            request.mutable_color()->CopyFrom(req->color());
            auto ret = ApiProxy::instance()->naming(&request, &response);
            if(!ret.ok()) {
                LOG(ERROR) << "rpc to discovery server:naming error:" << controller->ErrorText();
                res->set_errcode(static_cast<melon::Errno>(ret.code()));
                res->set_errmsg(ret.to_string());
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

}  // namespace sirius
