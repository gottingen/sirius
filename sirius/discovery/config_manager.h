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



#ifndef EA_DISCOVERY_CONFIG_MANAGER_H_
#define EA_DISCOVERY_CONFIG_MANAGER_H_

#include <turbo/container/flat_hash_map.h>
#include <sirius/proto/discovery.interface.pb.h>
#include <turbo/container/flat_hash_map.h>
#include <sirius/discovery/discovery_state_machine.h>
#include "turbo/module/module_version.h"
#include <sirius/discovery/discovery_server.h>
#include <melon/raft/raft.h>
#include <melon/fiber/mutex.h>

namespace sirius::discovery {

    class ConfigManager {
    public:
        static turbo::ModuleVersion kDefaultVersion;
        static ConfigManager *get_instance() {
            static ConfigManager ins;
            return &ins;
        }

        ~ConfigManager();

        ///
        /// \brief preprocess for raft machine, check
        ///        parameter
        /// \param controller
        /// \param request
        /// \param response
        /// \param done
        void process_schema_info(google::protobuf::RpcController *controller,
                                                const sirius::proto::DiscoveryManagerRequest *request,
                                                sirius::proto::DiscoveryManagerResponse *response,
                                                google::protobuf::Closure *done);
        ///
        /// \param request
        /// \param done
        void create_config(const ::sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done);

        ///
        /// \param request
        /// \param done
        void remove_config(const ::sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done);

        ///
        /// \return
        int load_snapshot();

        ///
        /// \param name
        /// \param version
        /// \return
        static std::string make_config_key(const std::string &name, const turbo::ModuleVersion &version);

        ///
        /// \param machine
        void set_discovery_state_machine(DiscoveryStateMachine *machine);
    private:
        ConfigManager();

        ///
        friend class QueryConfigManager;

        ///
        /// \param value
        /// \return
        int load_config_snapshot(const std::string &value);

        ///
        /// \param request
        /// \param done
        void remove_config_all(const ::sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done);

    private:
        DiscoveryStateMachine *_discovery_state_machine;
        fiber_mutex_t _config_mutex;
        turbo::flat_hash_map<std::string, std::map<turbo::ModuleVersion, sirius::proto::ConfigInfo>> _configs;

    };

    ///
    /// inlines
    ///

    inline ConfigManager::ConfigManager() {
        fiber_mutex_init(&_config_mutex, nullptr);
    }

    inline ConfigManager::~ConfigManager() {
        fiber_mutex_destroy(&_config_mutex);
    }

    inline void ConfigManager::set_discovery_state_machine(DiscoveryStateMachine *machine) {
        _discovery_state_machine = machine;
    }
}  // namespace sirius::discovery
#endif  // EA_DISCOVERY_CONFIG_MANAGER_H_
