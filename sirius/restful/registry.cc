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
// Created by jeff on 24-6-23.
//

#include <sirius/restful/registry.h>
#include <sirius/restful/config.h>
#include <sirius/restful/common.h>
#include <melon/rpc/restful_service.h>

namespace sirius::restful {

    turbo::Status registry_server(melon::Server *server) {
        auto service = melon::RestfulService::instance();
        service->set_processor("/config/list", std::make_shared<ListConfigProcessor>());
        service->set_processor("/config/create", std::make_shared<CreateConfigProcessor>());
        service->set_processor("/config/remove", std::make_shared<RemoveConfigProcessor>());
        service->set_processor("/config/version", std::make_shared<ListConfigVersionProcessor>());
        service->set_processor("/config/query", std::make_shared<ConfigQueryProcessor>());
        service->set_not_found_processor(std::make_shared<NotFoundProcessor>());
        service->set_root_processor(std::make_shared<RootProcessor>());
        service->set_mapping_path("ea");
        return service->register_server(server);
    }
}  // namespace sirius::restful