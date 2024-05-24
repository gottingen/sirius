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

#include <collie/utility/status.h>
#include <sirius/proto/discovery.interface.pb.h>
#include <sirius/client/router_sender.h>

namespace sirius::client {

    class Naming {
    public:
        Naming() = default;
        ~Naming() = default;

        collie::Status initialize(const std::string &servers);

        collie::Status register_server(const sirius::proto::ServletInfo &info);

        collie::Status update(const sirius::proto::ServletInfo &info);

        collie::Status get_servers(const sirius::proto::ServletNamingRequest &request, sirius::proto::ServletNamingResponse &response);

        collie::Status cancel(const sirius::proto::ServletInfo &info);
    private:
        std::unique_ptr<RouterSender> _router_sender;
    };
}  // namespace sirius::client
