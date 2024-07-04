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

#include <sirius/discovery/query_sns_manager.h>
#include <sirius/discovery/query_app_manager.h>
#include <sirius/discovery/query_zone_manager.h>
#include <sirius/discovery/query_servlet_manager.h>
#include <turbo/log/logging.h>
#include <turbo/times/time.h>
#include <turbo/container/flat_hash_set.h>
#include <sirius/flags/sns.h>
#include <turbo/flags/flag.h>

namespace sirius::discovery {

    void QuerySnsManager::naming(const sirius::proto::DiscoveryQueryRequest *req,
                                 sirius::proto::DiscoveryQueryResponse *res) {
        const auto *request = &req->servlet_naming();
        auto *response = res->mutable_servlet_naming();
        AppManager *manager = AppManager::get_instance();
        res->set_errcode(eapi::kOk);
        res->set_errmsg("ok");
        std::set<int64_t> zone_ids;
        {
            MELON_SCOPED_LOCK(manager->_app_mutex);
            auto it = manager->_app_id_map.find(request->app_name());
            if (it == manager->_app_id_map.end()) {
                response->set_code(eapi::kInvalidArgument);
                response->set_message("app not exist");
                return;
            }
            auto app_id = it->second;
            zone_ids = manager->_zone_ids[app_id];
        }
        if(zone_ids.empty()) {
            response->set_code(eapi::kInvalidArgument);
            response->set_message("app has no zone");
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
            response->set_code(eapi::kInvalidArgument);
            response->set_message("zone not exist");
            return;
        }

        std::set<int64_t> server_ids;
        for(auto &zone_id: query_zone_ids) {
            std::set<int64_t> tmp_server_ids;
            zone_manager->get_servlet_ids(zone_id, tmp_server_ids);
            server_ids.insert(tmp_server_ids.begin(), tmp_server_ids.end());
        }
        if (server_ids.empty()) {
            response->set_code(eapi::kInvalidArgument);
            response->set_message("zone has no server");
            return;
        }

        std::set<std::string> env_set;
        env_set.insert(request->env().begin(), request->env().end());
        std::set<std::string> color_set;
        color_set.insert(request->color().begin(), request->color().end());
        auto tnow = turbo::Time::to_time_t(turbo::Time::current_time());
        auto *servlet_manager = ServletManager::get_instance();
        int time_out = turbo::get_flag(FLAGS_sns_max_update_interval);
        for(auto &server_id: server_ids) {
            eapi::sirius::ServletInfo servlet_info;
            if (servlet_manager->get_servlet_info(server_id, servlet_info) != 0) {
                continue;
            }
            if(servlet_info.manager_status() == eapi::sirius::TOMBSTONE || servlet_info.manager_status() == eapi::sirius::OFFLINE) {
                continue;
            }

            VLOG(turbo::V_DEBUG) << "server_id: " << server_id<< " env: " << servlet_info.env() << " color: " << servlet_info.color() << " mtime: " << servlet_info.mtime() << " tnow: " << tnow;
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
        response->set_code(eapi::kOk);
        response->set_message("ok");
        res->set_errcode(eapi::kOk);
        res->set_errmsg("ok");
    }

}  // namespace sirius::discovery

