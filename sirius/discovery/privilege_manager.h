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

#include <unordered_map>
#include <melon/fiber/mutex.h>
#include <sirius/proto/discovery.interface.pb.h>
#include <sirius/discovery/sirius_state_machine.h>
#include <sirius/discovery/sirius_constants.h>

namespace sirius::discovery {
    class PrivilegeManager {
    public:
        friend class QueryPrivilegeManager;

        ///
        /// \return
        static PrivilegeManager *get_instance() {
            static PrivilegeManager instance;
            return &instance;
        }

        ~PrivilegeManager() {
            fiber_mutex_destroy(&_user_mutex);
        }

        ///
        /// \brief privilege proxy called by discovery state machine
        /// \param controller
        /// \param request
        /// \param response
        /// \param done
        void process_user_privilege(google::protobuf::RpcController *controller,
                                    const sirius::proto::DiscoveryManagerRequest *request,
                                    sirius::proto::DiscoveryManagerResponse *response,
                                    google::protobuf::Closure *done);

        ///
        /// \brief create a user
        /// \param request
        /// \param done
        void create_user(const sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done);

        ///
        /// \brief remove user privilege for namespace db and table
        /// \param request
        /// \param done
        void drop_user(const sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done);

        ///
        /// \brief add privilege for user, user should be created
        /// \param request
        /// \param done
        void add_privilege(const sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done);

        ///
        /// \brief drop privilege for user, user should be created
        /// \param request
        /// \param done
        void drop_privilege(const sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done);

        ///
        /// \return
        int load_snapshot();

        ///
        /// \param discovery_state_machine
        void set_discovery_state_machine(DiscoveryStateMachine *discovery_state_machine) {
            _discovery_state_machine = discovery_state_machine;
        }

    private:
        PrivilegeManager() {
            fiber_mutex_init(&_user_mutex, nullptr);
        }

        ///
        /// \param username
        /// \return
        static std::string construct_privilege_key(const std::string &username) {
            return DiscoveryConstants::PRIVILEGE_IDENTIFY + username;
        }

        ///
        /// \param privilege_zone
        /// \param mem_privilege
        static void insert_zone_privilege(const sirius::proto::PrivilegeZone &privilege_zone,
                                       sirius::proto::UserPrivilege &mem_privilege);

        ///
        /// \param privilege_servlet
        /// \param mem_privilege
        static void insert_servlet_privilege(const sirius::proto::PrivilegeServlet &privilege_servlet,
                                   sirius::proto::UserPrivilege &mem_privilege);

        ///
        /// \param ip
        /// \param mem_privilege
        static void insert_ip(const std::string &ip, sirius::proto::UserPrivilege &mem_privilege);

        ///
        /// \param privilege_zone
        /// \param mem_privilege
        static void delete_zone_privilege(const sirius::proto::PrivilegeZone &privilege_zone,
                                       sirius::proto::UserPrivilege &mem_privilege);
        ///
        /// \param privilege_servlet
        /// \param mem_privilege
        static void delete_servlet_privilege(const sirius::proto::PrivilegeServlet &privilege_servlet,
                                    sirius::proto::UserPrivilege &mem_privilege);

        ///
        /// \param ip
        /// \param mem_privilege
        static void delete_ip(const std::string &ip, sirius::proto::UserPrivilege &mem_privilege);

        fiber_mutex_t _user_mutex;
        std::unordered_map<std::string, sirius::proto::UserPrivilege> _user_privilege;

        DiscoveryStateMachine *_discovery_state_machine;
    };//class
}  // namespace sirius::discovery
