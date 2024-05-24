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


#include <sirius/discovery/query_servlet_manager.h>
#include <sirius/base/log.h>

namespace sirius::discovery {
    void QueryServletManager::get_servlet_info(const sirius::proto::DiscoveryQueryRequest *request,
                                                 sirius::proto::DiscoveryQueryResponse *response) {
        ServletManager *manager = ServletManager::get_instance();
        MELON_SCOPED_LOCK(manager->_servlet_mutex);
        if (!request->has_servlet()) {
            for (auto &servlet_info: manager->_servlet_info_map) {
                *(response->add_servlet_infos()) = servlet_info.second;
            }
        } else {
            std::string app_name = request->app_name();
            std::string zone = app_name + "\001" + request->zone();
            std::string servlet = zone + "\001" + request->servlet();
            if (manager->_servlet_id_map.find(servlet) != manager->_servlet_id_map.end()) {
                int64_t id = manager->_servlet_id_map[servlet];
                *(response->add_servlet_infos()) = manager->_servlet_info_map[id];
            } else {
                response->set_errmsg("servlet not exist");
                response->set_errcode(sirius::proto::INPUT_PARAM_ERROR);
                SS_LOG(ERROR)<< "namespace: " << app_name << " zone: " << zone << " servlet: " << servlet << " not exist";
            }
        }
    }

}  // namespace sirius::discovery
