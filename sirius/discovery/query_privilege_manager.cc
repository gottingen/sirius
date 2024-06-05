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


#include <sirius/discovery/query_privilege_manager.h>
#include <collie/strings/trim.h>


namespace sirius::discovery {
    void QueryPrivilegeManager::get_user_info(const sirius::proto::DiscoveryQueryRequest *request,
                                              sirius::proto::DiscoveryQueryResponse *response) {
        PrivilegeManager *manager = PrivilegeManager::get_instance();
        MELON_SCOPED_LOCK(manager->_user_mutex);
        if (!request->has_user_name()) {
            for (auto &user_info: manager->_user_privilege) {
                auto privilege = response->add_user_privilege();
                *privilege = user_info.second;
            }
        } else {
            std::string user_name = request->user_name();
            if (manager->_user_privilege.find(user_name) != manager->_user_privilege.end()) {
                auto privilege = response->add_user_privilege();
                *privilege = manager->_user_privilege[user_name];
            } else {
                response->set_errcode(sirius::proto::INPUT_PARAM_ERROR);
                response->set_errmsg("username not exist");
            }
        }
    }

    void QueryPrivilegeManager::get_flatten_servlet_privilege(const sirius::proto::DiscoveryQueryRequest *request,
                                                      sirius::proto::DiscoveryQueryResponse *response) {
        PrivilegeManager *manager = PrivilegeManager::get_instance();
        MELON_SCOPED_LOCK(manager->_user_mutex);
        std::string user_name = request->user_name();
        collie::trim_all(&user_name);
        std::string app_name = request->app_name();
        collie::trim_all(&app_name);
        std::map<std::string, std::multimap<std::string, sirius::proto::QueryUserPrivilege>> namespace_privileges;
        if (user_name.empty() && app_name.empty()) {
            for (auto &privilege_info: manager->_user_privilege) {
                construct_query_response_for_servlet_privilege(privilege_info.second, namespace_privileges);
            }
        }
        if (!user_name.empty()
            && manager->_user_privilege.find(user_name) != manager->_user_privilege.end()) {
            construct_query_response_for_servlet_privilege(manager->_user_privilege[user_name], namespace_privileges);
        }
        if (!app_name.empty()) {
            for (auto &privilege_info: manager->_user_privilege) {
                if (privilege_info.second.app_name() != app_name) {
                    continue;
                }
                construct_query_response_for_servlet_privilege(privilege_info.second, namespace_privileges);
            }
        }
        for (auto &namespace_privilege: namespace_privileges) {
            for (auto &user_privilege: namespace_privilege.second) {
                sirius::proto::QueryUserPrivilege *privilege_info = response->add_flatten_privileges();
                *privilege_info = user_privilege.second;
            }
        }
    }


    void QueryPrivilegeManager::construct_query_response_for_servlet_privilege(const sirius::proto::UserPrivilege &user_privilege,
                                                                       std::map<std::string, std::multimap<std::string, sirius::proto::QueryUserPrivilege>> &namespace_privileges) {
        std::string app_name = user_privilege.app_name();
        std::string username = user_privilege.username();
        for (auto &privilege_zone: user_privilege.privilege_zone()) {
            sirius::proto::QueryUserPrivilege flatten_privilege;
            flatten_privilege.set_username(username);
            flatten_privilege.set_app_name(app_name);
            flatten_privilege.set_servlet_rw(privilege_zone.zone_rw());
            flatten_privilege.set_privilege(privilege_zone.zone() + ".*");
            std::multimap<std::string, sirius::proto::QueryUserPrivilege> user_privilege_map;
            if (namespace_privileges.find(app_name) != namespace_privileges.end()) {
                user_privilege_map = namespace_privileges[app_name];
            }
            user_privilege_map.insert(std::pair<std::string, sirius::proto::QueryUserPrivilege>(username, flatten_privilege));
            namespace_privileges[app_name] = user_privilege_map;
        }
        for (auto &privilege_servlet: user_privilege.privilege_servlet()) {
            sirius::proto::QueryUserPrivilege flatten_privilege;
            flatten_privilege.set_username(username);
            flatten_privilege.set_app_name(app_name);
            flatten_privilege.set_servlet_rw(privilege_servlet.servlet_rw());
            flatten_privilege.set_privilege(privilege_servlet.zone() + "." + privilege_servlet.servlet_name());
            std::multimap<std::string, sirius::proto::QueryUserPrivilege> user_privilege_map;
            if (namespace_privileges.find(app_name) != namespace_privileges.end()) {
                user_privilege_map = namespace_privileges[app_name];
            }
            user_privilege_map.insert(std::pair<std::string, sirius::proto::QueryUserPrivilege>(username, flatten_privilege));
            namespace_privileges[app_name] = user_privilege_map;
        }
    }

}  // namespace sirius::discovery
