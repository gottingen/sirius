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

#include <sirius/proto/discovery.interface.pb.h>
#include <sirius/discovery/discovery_state_machine.h>

namespace sirius::discovery {

    class SchemaManager {
    public:
        static SchemaManager *get_instance() {
            static SchemaManager instance;
            return &instance;
        }

        ~SchemaManager() {}

        ///
        /// \param controller
        /// \param request
        /// \param response
        /// \param done
        void process_schema_info(google::protobuf::RpcController *controller,
                                 const sirius::proto::DiscoveryManagerRequest *request,
                                 sirius::proto::DiscoveryManagerResponse *response,
                                 google::protobuf::Closure *done);

        ///
        /// \param user_privilege
        /// \return
        int check_and_get_for_privilege(sirius::proto::UserPrivilege &user_privilege);

        ///
        /// \param instance
        /// \return
        int check_and_get_for_instance(sirius::proto::ServletInstance &instance);

        int load_snapshot();

        ///
        /// \param discovery_state_machine
        void set_discovery_state_machine(DiscoveryStateMachine *discovery_state_machine) {
            _discovery_state_machine = discovery_state_machine;
        }

    private:
        SchemaManager() = default;

        int load_max_id_snapshot(const std::string &max_id_prefix,
                                 const std::string &key,
                                 const std::string &value);

        DiscoveryStateMachine *_discovery_state_machine;
    }; //class

}  // namespace sirius::discovery
