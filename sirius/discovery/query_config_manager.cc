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

#include <sirius/discovery/query_config_manager.h>
#include <sirius/discovery/config_manager.h>

namespace sirius::discovery {

    void QueryConfigManager::get_config(const ::sirius::proto::DiscoveryQueryRequest *request,
                                        ::sirius::proto::DiscoveryQueryResponse *response) {
        if (!request->has_config_name()) {
            response->set_errmsg("config name not set");
            response->set_errcode(sirius::proto::INPUT_PARAM_ERROR);
            return;
        }
        MELON_SCOPED_LOCK( ConfigManager::get_instance()->_config_mutex);
        auto configs = ConfigManager::get_instance()->_configs;
        auto &name = request->config_name();
        auto it = configs.find(name);
        if (it == configs.end() || it->second.empty()) {
            response->set_errmsg("config not exist");
            response->set_errcode(sirius::proto::INPUT_PARAM_ERROR);
            return;
        }
        collie::ModuleVersion version;

        if (!request->has_config_version()) {
            // use newest
            // version = it->second.rend()->first;
            auto cit = it->second.rbegin();
            *(response->add_config_infos()) = cit->second;
            response->set_errmsg("success");
            response->set_errcode(sirius::proto::SUCCESS);
            return;
        }
        auto &request_version = request->config_version();
        version = collie::ModuleVersion(request_version.major(), request_version.minor(), request_version.patch());

        auto cit = it->second.find(version);
        if (cit == it->second.end()) {
            /// not exists
            response->set_errmsg("config not exist");
            response->set_errcode(sirius::proto::INPUT_PARAM_ERROR);
            return;
        }

        *(response->add_config_infos()) = cit->second;
        response->set_errmsg("success");
        response->set_errcode(sirius::proto::SUCCESS);
    }

    void QueryConfigManager::list_config(const ::sirius::proto::DiscoveryQueryRequest *request,
                                         ::sirius::proto::DiscoveryQueryResponse *response) {
        MELON_SCOPED_LOCK( ConfigManager::get_instance()->_config_mutex);
        auto &configs = ConfigManager::get_instance()->_configs;
        response->mutable_config_infos()->Reserve(configs.size());
        for (auto it = configs.begin(); it != configs.end(); ++it) {
            for(auto vit = it->second.begin(); vit != it->second.end(); ++vit) {
                *(response->add_config_infos()) = vit->second;
            }
        }
        response->set_errmsg("success");
        response->set_errcode(sirius::proto::SUCCESS);
    }

    void QueryConfigManager::list_config_version(const ::sirius::proto::DiscoveryQueryRequest *request,
                                                 ::sirius::proto::DiscoveryQueryResponse *response) {
        if (!request->has_config_name()) {
            response->set_errmsg("config name not set");
            response->set_errcode(sirius::proto::INPUT_PARAM_ERROR);
            return;
        }
        auto &name = request->config_name();
        MELON_SCOPED_LOCK( ConfigManager::get_instance()->_config_mutex);
        auto &configs = ConfigManager::get_instance()->_configs;
        auto it = configs.find(name);
        if (it == configs.end()) {
            response->set_errmsg("config not exist");
            response->set_errcode(sirius::proto::INPUT_PARAM_ERROR);
            return;
        }
        response->mutable_config_infos()->Reserve(it->second.size());
        for (auto vit = it->second.begin(); vit != it->second.end(); ++vit) {
            *(response->add_config_infos()) = vit->second;
        }
        response->set_errmsg("success");
        response->set_errcode(sirius::proto::SUCCESS);
    }

}  // namespace sirius::discovery
