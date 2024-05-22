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


#pragma once

#include <sirius/discovery/privilege_manager.h>

namespace sirius::discovery {
    class QueryPrivilegeManager {
    public:
        static QueryPrivilegeManager *get_instance() {
            static QueryPrivilegeManager instance;
            return &instance;
        }

        ~QueryPrivilegeManager() = default;

        ///
        /// \param request
        /// \param response
        void get_user_info(const sirius::proto::DiscoveryQueryRequest *request,
                           sirius::proto::DiscoveryQueryResponse *response);

        ///
        /// \param request
        /// \param response
        void
        get_flatten_servlet_privilege(const sirius::proto::DiscoveryQueryRequest *request, sirius::proto::DiscoveryQueryResponse *response);


    private:
        QueryPrivilegeManager() {}

        ///
        /// \param user_privilege
        /// \param namespace_privileges
        static void construct_query_response_for_servlet_privilege(const sirius::proto::UserPrivilege &user_privilege,
                                                            std::map<std::string, std::multimap<std::string, sirius::proto::QueryUserPrivilege>> &namespace_privileges);
    };
}  // namespace sirius::discovery
