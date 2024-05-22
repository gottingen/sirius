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
#include <sirius/discovery/query_instance_manager.h>
#include <collie/strings/format.h>

namespace sirius::discovery {

    void QueryInstanceManager::query_instance(const sirius::proto::DiscoveryQueryRequest *request, sirius::proto::DiscoveryQueryResponse *response) {
        if(!request->has_instance_address()) {
            response->set_errcode(sirius::proto::INPUT_PARAM_ERROR);
            response->set_errmsg("no instance address");
            return;
        }
        auto manager = InstanceManager::get_instance();
        MELON_SCOPED_LOCK(manager->_instance_mutex);
        auto it = manager->_instance_info.find(request->instance_address());
        if(it == manager->_instance_info.end()) {
            response->set_errcode(sirius::proto::INPUT_PARAM_ERROR);
            response->set_errmsg("instance not exists");
            return;
        }
        *response->add_instance() = it->second;
        response->set_errcode(sirius::proto::SUCCESS);
        response->set_errmsg("success");
    }

    void QueryInstanceManager::query_instance_flatten(const sirius::proto::DiscoveryQueryRequest *request, sirius::proto::DiscoveryQueryResponse *response) {
        auto manager = InstanceManager::get_instance();
        if(!request->has_namespace_name() || request->namespace_name().empty()) {
            MELON_SCOPED_LOCK(manager->_instance_mutex);
            for(auto &it : manager->_instance_info) {
                sirius::proto::QueryInstance ins;
                instance_info_to_query(it.second, ins);
                *response->add_flatten_instances() = std::move(ins);
            }
            response->set_errcode(sirius::proto::SUCCESS);
            response->set_errmsg("success");
            return;
        }
        if(!request->has_zone() || request->zone().empty()) {
            MELON_SCOPED_LOCK(manager->_instance_mutex);
            auto it = manager->_namespace_instance.find(request->namespace_name());
            if(it == manager->_namespace_instance.end()) {
                response->set_errcode(sirius::proto::INPUT_PARAM_ERROR);
                auto msg = collie::format("no instance in namespace {}", request->namespace_name());
                response->set_errmsg(msg);
                return;
            }
            for(auto address : it->second) {
                sirius::proto::QueryInstance ins;
                auto &info = manager->_instance_info[address];
                instance_info_to_query(info, ins);
                *response->add_flatten_instances() = std::move(ins);
            }
            response->set_errcode(sirius::proto::SUCCESS);
            response->set_errmsg("success");
            return;
        }
        if(!request->has_servlet() || request->servlet().empty()) {
            MELON_SCOPED_LOCK(manager->_instance_mutex);
            auto zone_key = ZoneManager::make_zone_key(request->namespace_name(), request->zone());
            auto it = manager->_zone_instance.find(zone_key);
            if(it == manager->_zone_instance.end()) {
                response->set_errcode(sirius::proto::INPUT_PARAM_ERROR);
                auto msg = collie::format("no instance in namespace {}.{}", request->namespace_name(), request->zone());
                response->set_errmsg(msg);
                return;
            }
            for(auto address : it->second) {
                sirius::proto::QueryInstance ins;
                auto &info = manager->_instance_info[address];
                instance_info_to_query(info, ins);
                *response->add_flatten_instances() = std::move(ins);
            }
            response->set_errcode(sirius::proto::SUCCESS);
            response->set_errmsg("success");
            return;
        }

        MELON_SCOPED_LOCK(manager->_instance_mutex);
        auto servlet_key = ServletManager::make_servlet_key(request->namespace_name(), request->zone(), request->servlet());
        auto it = manager->_servlet_instance.find(servlet_key);
        if(it == manager->_servlet_instance.end()) {
            response->set_errcode(sirius::proto::INPUT_PARAM_ERROR);
            auto msg = collie::format("no instance in {}.{}.{}", request->namespace_name(), request->zone(), request->servlet());
            response->set_errmsg(msg);
            return;
        }
        for(auto address : it->second) {
            sirius::proto::QueryInstance ins;
            auto &info = manager->_instance_info[address];
            instance_info_to_query(info, ins);
            *response->add_flatten_instances() = std::move(ins);
        }
        response->set_errcode(sirius::proto::SUCCESS);
        response->set_errmsg("success");
    }

    void QueryInstanceManager::instance_info_to_query(const sirius::proto::ServletInstance &sinstance, sirius::proto::QueryInstance &ins) {
        ins.set_namespace_name(sinstance.namespace_name());
        ins.set_zone_name(sinstance.zone_name());
        ins.set_servlet_name(sinstance.servlet_name());
        ins.set_env(sinstance.env());
        ins.set_color(sinstance.color());
        ins.set_version(sinstance.version());
        ins.set_status(sinstance.status());
        ins.set_address(sinstance.address());
    }
}  // namespace sirius::discovery
