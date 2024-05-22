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


#include <sirius/discovery/query_namespace_manager.h>
#include <sirius/base/log.h>

namespace sirius::discovery {
    void QueryNamespaceManager::get_namespace_info(const sirius::proto::DiscoveryQueryRequest *request,
                                                   sirius::proto::DiscoveryQueryResponse *response) {
        NamespaceManager *manager = NamespaceManager::get_instance();
        MELON_SCOPED_LOCK(manager->_namespace_mutex);
        if (!request->has_namespace_name()) {
            for (auto &namespace_info: manager->_namespace_info_map) {
                *(response->add_namespace_infos()) = namespace_info.second;
            }
        } else {
            std::string namespace_name = request->namespace_name();
            if (manager->_namespace_id_map.find(namespace_name) != manager->_namespace_id_map.end()) {
                int64_t id = manager->_namespace_id_map[namespace_name];
                *(response->add_namespace_infos()) = manager->_namespace_info_map[id];
            } else {
                response->set_errcode(sirius::proto::INPUT_PARAM_ERROR);
                response->set_errmsg("namespace not exist");
                SS_LOG(ERROR)<< "namespace: " << namespace_name << " not exist";
            }
        }
    }

}  // namespace sirius::discovery
