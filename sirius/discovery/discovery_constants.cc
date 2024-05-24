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


#include <sirius/discovery/discovery_constants.h>

namespace sirius::discovery {

    const std::string DiscoveryConstants::SCHEMA_IDENTIFY(1, 0x01);

    const std::string DiscoveryConstants::MAX_ID_SCHEMA_IDENTIFY(1, 0x01);
    const std::string DiscoveryConstants::APP_SCHEMA_IDENTIFY(1, 0x02);
    const std::string DiscoveryConstants::ZONE_SCHEMA_IDENTIFY(1, 0x09);
    const std::string DiscoveryConstants::SERVLET_SCHEMA_IDENTIFY(1, 0x0A);

    const std::string DiscoveryConstants::PRIVILEGE_IDENTIFY(1, 0x02);

    const std::string DiscoveryConstants::CONFIG_IDENTIFY(1, 0x04);

    const std::string DiscoveryConstants::DISCOVERY_IDENTIFY(1, 0x03);
    const std::string DiscoveryConstants::DISCOVERY_MAX_ID_IDENTIFY(1, 0x01);
    const std::string DiscoveryConstants::DISCOVERY_INSTANCE_IDENTIFY(1, 0x03);
    const std::string DiscoveryConstants::INSTANCE_PARAM_CLUSTER_IDENTIFY(1, 0x04);


    const std::string DiscoveryConstants::MAX_IDENTIFY(1, 0xFF);

    /// for schema
    const std::string DiscoveryConstants::MAX_APP_ID_KEY = "max_app_id";
    const std::string DiscoveryConstants::MAX_ZONE_ID_KEY = "max_zone_id";
    const std::string DiscoveryConstants::MAX_SERVLET_ID_KEY = "max_servlet_id";
    const std::string DiscoveryConstants::MAX_INSTANCE_ID_KEY = "max_instance_id";

    const int DiscoveryConstants::DiscoveryMachineRegion = 0;
    const int DiscoveryConstants::AutoIDMachineRegion = 1;
    const int DiscoveryConstants::TsoMachineRegion = 2;
}  // namespace sirius::discovery
