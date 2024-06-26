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


#ifndef EA_DISCOVERY_DISCOVERY_CONSTANTS_H_
#define EA_DISCOVERY_DISCOVERY_CONSTANTS_H_

#include <string>

namespace sirius::discovery {

    struct DiscoveryConstants {

        static const std::string SCHEMA_IDENTIFY;
        static const std::string MAX_ID_SCHEMA_IDENTIFY;
        static const std::string APP_SCHEMA_IDENTIFY;
        static const std::string ZONE_SCHEMA_IDENTIFY;
        static const std::string SERVLET_SCHEMA_IDENTIFY;

        static const std::string PRIVILEGE_IDENTIFY;

        static const std::string DISCOVERY_IDENTIFY;
        static const std::string DISCOVERY_MAX_ID_IDENTIFY;
        static const std::string DISCOVERY_INSTANCE_IDENTIFY;
        static const std::string INSTANCE_PARAM_CLUSTER_IDENTIFY;

        static const std::string CONFIG_IDENTIFY;
        static const std::string CONFIG_CONTENT_IDENTIFY;

        static const std::string MAX_IDENTIFY;

        /// for schema
        static const std::string MAX_CONFIG_ID_KEY;
        static const std::string MAX_APP_ID_KEY;
        static const std::string MAX_ZONE_ID_KEY;
        static const std::string MAX_SERVLET_ID_KEY;
        static const std::string MAX_INSTANCE_ID_KEY;

        static const int DiscoveryMachineRegion;
        static const int AutoIDMachineRegion;
        static const int TsoMachineRegion;
    };

    namespace tso {
        constexpr int64_t update_timestamp_interval_ms = 50LL; // 50ms
        constexpr int64_t update_timestamp_guard_ms = 1LL; // 1ms
        constexpr int64_t save_interval_ms = 3000LL;  // 3000ms
        constexpr int64_t base_timestamp_ms = 1577808000000LL; // 2020-01-01 12:00:00
        constexpr int logical_bits = 18;
        constexpr int64_t max_logical = 1 << logical_bits;

        inline int64_t clock_realtime_ms() {
            struct timespec tp;
            ::clock_gettime(CLOCK_REALTIME, &tp);
            return tp.tv_sec * 1000ULL + tp.tv_nsec / 1000000ULL - base_timestamp_ms;
        }

        inline uint32_t get_timestamp_internal(int64_t offset) {
            return ((offset >> 18) + base_timestamp_ms) / 1000;
        }

    } // namespace tso

}  // namespace sirius::discovery

#endif  // EA_DISCOVERY_DISCOVERY_CONSTANTS_H_
