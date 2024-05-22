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

#pragma once

#include <sirius/discovery/instance_manager.h>

namespace sirius::discovery {

    class QueryInstanceManager {
    public:
        static QueryInstanceManager *get_instance() {
            static QueryInstanceManager ins;
            return &ins;
        }

        ~QueryInstanceManager() = default;

        ///
        /// \param request
        /// \param response
        void query_instance(const sirius::proto::DiscoveryQueryRequest *request, sirius::proto::DiscoveryQueryResponse *response);

        ///
        /// \param request
        /// \param response
        void query_instance_flatten(const sirius::proto::DiscoveryQueryRequest *request, sirius::proto::DiscoveryQueryResponse *response);

    public:
        static void instance_info_to_query(const sirius::proto::ServletInstance &sinstance, sirius::proto::QueryInstance &ins);
    };
}  // namespace sirius::discovery

