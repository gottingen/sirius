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


#include <sirius/discovery/query_app_manager.h>
#include <sirius/discovery/query_zone_manager.h>
#include <sirius/discovery/query_servlet_manager.h>
#include <sirius/base/log.h>
#include <turbo/times/time.h>
#include <turbo/container/flat_hash_set.h>

namespace sirius::discovery {

    void QueryAppManager::naming(const sirius::proto::ServletNamingRequest *request,
                sirius::proto::ServletNamingResponse *response) {
        AppManager *manager = AppManager::get_instance();
        std::set<int64_t> zone_ids;
        {
            MELON_SCOPED_LOCK(manager->_app_mutex);
            auto it = manager->_app_id_map.find(request->app_name());
            if (it == manager->_app_id_map.end()) {
                response->set_errcode(sirius::proto::INPUT_PARAM_ERROR);
                response->set_errmsg("app not exist");
                return;
            }
            auto app_id = it->second;
            zone_ids = manager->_zone_ids[app_id];
        }
        if(zone_ids.empty()) {
            response->set_errcode(sirius::proto::INPUT_PARAM_ERROR);
            response->set_errmsg("app has no zone");
            return;
        }
        auto *zone_manager = ZoneManager::get_instance();
        auto &query_zone = request->zones();
        std::vector<std::string> zone_names(query_zone.size());
        for (auto &zone: query_zone) {
            zone_names.push_back(zone);
        }
        std::vector<int64_t> query_zone_id;
        zone_manager->get_zone_ids(request->app_name(), zone_names, query_zone_id);
        std::set<int64_t> query_zone_ids;
        for (auto &id: query_zone_id) {
            if (zone_ids.find(id) != zone_ids.end()) {
                query_zone_ids.insert(id);
            }
        }
        if (query_zone_ids.empty()) {
            response->set_errcode(sirius::proto::INPUT_PARAM_ERROR);
            response->set_errmsg("zone not exist");
            return;
        }

        std::set<int64_t> server_ids;
        for(auto &zone_id: query_zone_ids) {
            std::set<int64_t> tmp_server_ids;
            zone_manager->get_servlet_ids(zone_id, tmp_server_ids);
            server_ids.insert(tmp_server_ids.begin(), tmp_server_ids.end());
        }
        if (server_ids.empty()) {
            response->set_errcode(sirius::proto::INPUT_PARAM_ERROR);
            response->set_errmsg("zone has no server");
            return;
        }

        std::set<std::string> env_set;
        env_set.insert(request->env().begin(), request->env().end());
        std::set<std::string> color_set;
        color_set.insert(request->color().begin(), request->color().end());
        auto tnow = turbo::Time::to_time_t(turbo::Time::current_time());
        auto *servlet_manager = ServletManager::get_instance();
        int time_out = 50;
        for(auto &server_id: server_ids) {
            sirius::proto::ServletInfo servlet_info;
            if (servlet_manager->get_servlet_info(server_id, servlet_info) != 0) {
                continue;
            }
            LOG(INFO) << "server_id: " << server_id<< " env: " << servlet_info.env() << " color: " << servlet_info.color() << " mtime: " << servlet_info.mtime() << " tnow: " << tnow;
            if (env_set.find(servlet_info.env()) == env_set.end()) {
                continue;
            }
            if (color_set.find(servlet_info.color()) == color_set.end()) {
                continue;
            }
            if (tnow - servlet_info.mtime() > time_out) {
                continue;
            }
            *(response->add_servlets()) = servlet_info;
        }
        response->set_errcode(sirius::proto::SUCCESS);
    }
    void QueryAppManager::get_app_info(const sirius::proto::DiscoveryQueryRequest *request,
                                                   sirius::proto::DiscoveryQueryResponse *response) {
        AppManager *manager = AppManager::get_instance();
        MELON_SCOPED_LOCK(manager->_app_mutex);
        if (!request->has_app_name()) {
            for (auto &namespace_info: manager->_app_info_map) {
                *(response->add_app_infos()) = namespace_info.second;
            }
        } else {
            std::string namespace_name = request->app_name();
            if (manager->_app_id_map.find(namespace_name) != manager->_app_id_map.end()) {
                int64_t id = manager->_app_id_map[namespace_name];
                *(response->add_app_infos()) = manager->_app_info_map[id];
            } else {
                response->set_errcode(sirius::proto::INPUT_PARAM_ERROR);
                response->set_errmsg("app not exist");
                LOG(ERROR)<< "namespace: " << namespace_name << " not exist";
            }
        }
    }

}  // namespace sirius::discovery
