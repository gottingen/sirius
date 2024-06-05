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


#include <sirius/discovery/query_zone_manager.h>
#include <sirius/base/log.h>

namespace sirius::discovery {
    void QueryZoneManager::get_zone_info(const sirius::proto::DiscoveryQueryRequest *request,
                                                 sirius::proto::DiscoveryQueryResponse *response) {
        ZoneManager *manager = ZoneManager::get_instance();
        MELON_SCOPED_LOCK(manager->_zone_mutex);
        if (!request->has_zone()) {
            for (auto &zone_info: manager->_zone_info_map) {
                *(response->add_zone_infos()) = zone_info.second;
            }
        } else {
            std::string app_name = request->app_name();
            std::string zone = app_name + "\001" + request->zone();
            if (manager->_zone_id_map.find(zone) != manager->_zone_id_map.end()) {
                int64_t id = manager->_zone_id_map[zone];
                *(response->add_zone_infos()) = manager->_zone_info_map[id];
            } else {
                response->set_errmsg("zone not exist");
                response->set_errcode(sirius::proto::INPUT_PARAM_ERROR);
                LOG(ERROR) << "namespace: " << app_name << " zone: " << zone << " not exist";
            }
        }
    }

}  // namespace sirius::discovery
