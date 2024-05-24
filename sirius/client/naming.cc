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

#include <sirius/client/naming.h>

namespace sirius::client {

    collie::Status Naming::initialize(const std::string &servers) {
        _router_sender = std::make_unique<RouterSender>();
        return _router_sender->init(servers);
    }

    collie::Status Naming::register_server(const sirius::proto::ServletInfo &info) {
        sirius::proto::DiscoveryRegisterResponse res;
        auto rs = _router_sender->discovery_register(info, res);
        // TODO: check res
        return rs;
    }

    collie::Status Naming::update(const sirius::proto::ServletInfo &info) {
        sirius::proto::DiscoveryRegisterResponse res;
        auto rs = _router_sender->discovery_update(info, res);
        return rs;
    }

    collie::Status Naming::get_servers(const sirius::proto::ServletNamingRequest &request, sirius::proto::ServletNamingResponse &response) {
        return _router_sender->discovery_naming(request, response);
    }

    collie::Status Naming::cancel(const sirius::proto::ServletInfo &info) {
        sirius::proto::DiscoveryRegisterResponse res;
        return _router_sender->discovery_cancel(info, res);
    }

}  // namespace sirius::client
