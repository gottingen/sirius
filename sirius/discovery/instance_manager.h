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

#ifndef EA_DISCOVERY_INSTANCE_MANAGER_H_
#define EA_DISCOVERY_INSTANCE_MANAGER_H_

#include <turbo/container/flat_hash_map.h>
#include "turbo/container/flat_hash_set.h"
#include <sirius/proto/discovery.interface.pb.h>
#include <melon/raft/raft.h>
#include <melon/fiber/mutex.h>
#include <sirius/discovery/discovery_constants.h>
#include <sirius/base/time_cast.h>
#include <sirius/discovery/zone_manager.h>
#include <sirius/discovery/servlet_manager.h>

namespace sirius::discovery {

    class InstanceManager {
    public:
        static InstanceManager *get_instance() {
            static InstanceManager ins;
            return &ins;
        }

        ~InstanceManager();

        ///
        /// \param request
        /// \param done
        void add_instance(const sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done);

        ///
        /// \param request
        /// \param done
        void drop_instance(const sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done);

        ///
        /// \param request
        /// \param done
        void update_instance(const sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done);

        ///
        /// \param value
        /// \return
        int load_instance_snapshot(const std::string &value);

        void clear();

        ///
        /// \return
        int load_snapshot();

    private:
        InstanceManager();

        std::string construct_instance_key(const std::string &address);

        void set_instance_info(const sirius::proto::ServletInstance &instance_info);

        void remove_instance_info(const std::string &address);

    private:
        friend class QueryInstanceManager;
        fiber_mutex_t _instance_mutex;
        turbo::flat_hash_map<std::string, sirius::proto::ServletInstance>       _instance_info;
        turbo::flat_hash_map<std::string, TimeCost>                           _removed_instance;

        /// namespace --> instance
        turbo::flat_hash_map<std::string, turbo::flat_hash_set<std::string>>  _namespace_instance;
        /// key zone[namespace + 0x01+zone] value:instance address
        turbo::flat_hash_map<std::string, turbo::flat_hash_set<std::string>>  _zone_instance;
        /// key zone[namespace + 0x01+zone + 0x01 + servlet] value:instance address
        turbo::flat_hash_map<std::string, turbo::flat_hash_set<std::string>>  _servlet_instance;
    };

    /// inlines

    inline InstanceManager::InstanceManager() {
        fiber_mutex_init(&_instance_mutex, nullptr);
    }

    inline InstanceManager::~InstanceManager() {
        fiber_mutex_destroy(&_instance_mutex);
    }

    inline std::string InstanceManager::construct_instance_key(const std::string &address) {
        std::string instance_key = DiscoveryConstants::DISCOVERY_IDENTIFY
                                   + DiscoveryConstants::DISCOVERY_INSTANCE_IDENTIFY;
        instance_key.append(address);
        return instance_key;
    }

    inline void InstanceManager::clear() {
        _instance_info.clear();
        _removed_instance.clear();
        _namespace_instance.clear();
        _zone_instance.clear();
        _servlet_instance.clear();
    }

}  // namespace sirius::discovery

#endif  // EA_DISCOVERY_INSTANCE_MANAGER_H_
