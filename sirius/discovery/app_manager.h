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
#include <set>
#include <mutex>
#include <sirius/proto/discovery.interface.pb.h>
#include <sirius/discovery/sirius_constants.h>
#include <melon/raft/raft.h>
#include <melon/fiber/mutex.h>

namespace sirius::discovery {

    class AppManager {
    public:
        friend class QueryAppManager;

        ~AppManager() {
            fiber_mutex_destroy(&_app_mutex);
        }

        static AppManager *get_instance() {
            static AppManager instance;
            return &instance;
        }

        ///
        /// \brief create app
        ///        failed when exists
        /// \param request
        /// \param done
        void create_app(const sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done);

        ///
        /// \brief remove app
        ///        failed when exists db in the app
        /// \param request
        /// \param done
        void drop_app(const sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done);

        ///
        /// \brief modify app
        ///        name and quota can be modify
        /// \param request
        /// \param done
        void modify_app(const sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done);

        ///
        /// \brief load app snapshot called by discovery state machine
        /// \param value
        /// \return
        int load_app_snapshot(const std::string &value);

        ///
        /// \param max_app_id
        void set_max_app_id(int64_t max_app_id);

        ///
        /// \brief get max id for app
        /// \return

        int64_t get_max_app_id();

        ///
        /// \brief add a zone under a app
        /// \param app_id
        /// \param zone_id
        void add_zone_id(int64_t app_id, int64_t zone_id);

        ///
        /// \brief remove a zone under a app
        /// \param app_id
        /// \param zone_id
        void delete_zone_id(int64_t app_id, int64_t zone_id);

        ///
        /// \brief get app id by app name, and the name
        /// \param app_name
        /// \return
        int64_t get_app_id(const std::string &app_name);

        ///
        /// \brief get app info by app id
        /// \param app_id
        /// \param namespace_info
        /// \return
        int get_app_info(const int64_t &app_id, sirius::proto::AppInfo &namespace_info);

        ///
        /// \brief clear memory values.
        void clear();

    private:
        AppManager();

        ///
        /// \brief set app info for space.
        /// \param namespace_info
        void set_app_info(const sirius::proto::AppInfo &namespace_info);

        ///
        /// \brief erase info for app
        /// \param app_name
        void erase_app_info(const std::string &app_name);

        ///
        /// \brief construct app key
        /// \param app_id
        /// \return
        std::string construct_app_key(int64_t app_id);

        std::string construct_max_app_id_key();

    private:
        //std::mutex                                          _app_mutex;
        fiber_mutex_t _app_mutex;

        int64_t _max_app_id{0};
        // app name -> id
        std::unordered_map<std::string, int64_t> _app_id_map;
        // app id -> info
        std::unordered_map<int64_t, sirius::proto::AppInfo> _app_info_map;
        // app id -> zone ids
        std::unordered_map<int64_t, std::set<int64_t>> _zone_ids; //only in memory, not in rocksdb
    };

    ///
    /// inlines
    ///

    inline void AppManager::set_max_app_id(int64_t max_app_id) {
        MELON_SCOPED_LOCK(_app_mutex);
        _max_app_id = max_app_id;
    }

    inline int64_t AppManager::get_max_app_id() {
        MELON_SCOPED_LOCK(_app_mutex);
        return _max_app_id;
    }

    inline void AppManager::set_app_info(const sirius::proto::AppInfo &app_info) {
        MELON_SCOPED_LOCK(_app_mutex);
        _app_id_map[app_info.app_name()] = app_info.app_id();
        _app_info_map[app_info.app_id()] = app_info;
    }

    inline void AppManager::erase_app_info(const std::string &app_name) {
        MELON_SCOPED_LOCK(_app_mutex);
        int64_t app_id = _app_id_map[app_name];
        _app_id_map.erase(app_name);
        _app_info_map.erase(app_id);
        _zone_ids.erase(app_id);
    }


    inline void AppManager::add_zone_id(int64_t app_id, int64_t zone_id) {
        MELON_SCOPED_LOCK(_app_mutex);
        _zone_ids[app_id].insert(zone_id);
    }

    inline void AppManager::delete_zone_id(int64_t app_id, int64_t zone_id) {
        MELON_SCOPED_LOCK(_app_mutex);
        if (_zone_ids.find(app_id) != _zone_ids.end()) {
            _zone_ids[app_id].erase(zone_id);
        }
    }

    inline int64_t AppManager::get_app_id(const std::string &app_name) {
        MELON_SCOPED_LOCK(_app_mutex);
        if (_app_id_map.find(app_name) == _app_id_map.end()) {
            return 0;
        }
        return _app_id_map[app_name];
    }

    inline int AppManager::get_app_info(const int64_t &app_id, sirius::proto::AppInfo &namespace_info) {
        MELON_SCOPED_LOCK(_app_mutex);
        if (_app_info_map.find(app_id) == _app_info_map.end()) {
            return -1;
        }
        namespace_info = _app_info_map[app_id];
        return 0;
    }

    inline void AppManager::clear() {
        _app_id_map.clear();
        _app_info_map.clear();
        _zone_ids.clear();
    }
    inline AppManager::AppManager() : _max_app_id(0) {
        fiber_mutex_init(&_app_mutex, nullptr);
    }

    inline std::string AppManager::construct_app_key(int64_t app_id) {
        std::string app_key = DiscoveryConstants::SCHEMA_IDENTIFY
                                    + DiscoveryConstants::APP_SCHEMA_IDENTIFY;
        app_key.append((char *) &app_id, sizeof(int64_t));
        return app_key;
    }

    inline std::string AppManager::construct_max_app_id_key() {
        std::string max_app_id_key = DiscoveryConstants::SCHEMA_IDENTIFY
                                           + DiscoveryConstants::MAX_ID_SCHEMA_IDENTIFY
                                           + DiscoveryConstants::MAX_APP_ID_KEY;
        return max_app_id_key;
    }

}  // namespace sirius::discovery
