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

#include <sirius/discovery/servlet_manager.h>

namespace sirius::discovery {
    class QueryServletManager {
    public:
        ~QueryServletManager()  = default;

        static QueryServletManager *get_instance() {
            static QueryServletManager instance;
            return &instance;
        }

        ///
        /// \param request
        /// \param response
        void get_servlet_info(const sirius::proto::DiscoveryQueryRequest *request, sirius::proto::DiscoveryQueryResponse *response);

    private:
        QueryServletManager() {}
    }; //class
} // namespace sirius::discovery
