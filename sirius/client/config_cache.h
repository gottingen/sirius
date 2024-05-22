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

#include <turbo/container/flat_hash_map.h>
#include <map>
#include <mutex>
#include <collie/utility/status.h>
#include <collie/module/semver.h>
#include <sirius/proto/discovery.struct.pb.h>
#include <sirius/base/fiber.h>

namespace sirius::client {

    /**
     * @ingroup config_client
     * @brief ConfigCache is used to cache the config files downloaded from the meta server.
     *        It is used by the DiscoveryClient to cache the config files downloaded from the meta server.
     */
    class ConfigCache {
    public:
        static ConfigCache *get_instance() {
            static ConfigCache ins;
            return &ins;
        }

        /**
         * @brief init is used to initialize the ConfigCache. It must be called before using the ConfigCache.
         * @return Status::OK if the ConfigCache was initialized successfully. Otherwise, an error status is returned. 
         */ 
        collie::Status init();

        /**
         * @brief add_config is used to add a config to the ConfigCache.
         * @param config [input] is the config to add to the ConfigCache.
         * @return Status::OK if the config was added successfully. Otherwise, an error status is returned. 
         */
        collie::Status add_config(const sirius::proto::ConfigInfo &config);

        /**
         * @brief get_config is used to get a config that matches the name and version from the ConfigCache.
         * @param name [input] is the name of the config to get.
         * @param version [input] is the version of the config to get.
         * @param config [output] is the config received from the ConfigCache.
         * @return Status::OK if the config was received successfully. Otherwise, an error status is returned. 
         */
        collie::Status
        get_config(const std::string &name, const collie::ModuleVersion &version, sirius::proto::ConfigInfo &config);

        /**
         * @brief get_config is used to get the latest version of a config from the ConfigCache.
         * @param name [input] is the name of the config to get the latest version for.
         * @param config is the config received from the ConfigCache.
         * @return Status::OK if the config was received successfully. Otherwise, an error status is returned. 
         */
        collie::Status get_config(const std::string &name, sirius::proto::ConfigInfo &config);

        /**
         * @brief get_config_list is used to get a list of config names from the ConfigCache.
         * @param name [output] is the list of config names received from the ConfigCache.
         * @return Status::OK if the config names were received successfully. Otherwise, an error status is returned. 
         */
        collie::Status get_config_list(std::vector<std::string> &name);

        /**
         * @brief get_config_version_list is used to get a list of config versions from the ConfigCache.
         * @param config_name [input] is the name of the config to get the versions for.
         * @param versions [output] is the list of config versions received from the ConfigCache.
         * @return Status::OK if the config versions were received successfully. Otherwise, an error status is returned. 
         */
        collie::Status
        get_config_version_list(const std::string &config_name, std::vector<collie::ModuleVersion> &versions);

        /**
         * @brief remove_config is used to remove a config from the ConfigCache.
         * @param config_name [input] is the name of the config to remove.
         * @param version [input] is the version of the config to remove.
         * @return Status::OK if the config names were received successfully. Otherwise, an error status is returned. 
         */
        collie::Status remove_config(const std::string &config_name, const collie::ModuleVersion &version);

        /**
         * @brief remove_config is used to remove a config from the ConfigCache.
         * @param config_name [input] is the name of the config to remove.
         * @param version [input] is the version of the config to remove.
         * @return Status::OK if the config was removed successfully. Otherwise, an error status is returned.
         */
        collie::Status remove_config(const std::string &config_name, const std::vector<collie::ModuleVersion> &version);

        /**
         * @brief remove_config_less_than is used to remove a config from the ConfigCache. All versions less than the specified version will be removed.
         * @param config_name [input] is the name of the config to remove all versions less than.
         * @param version [input] is the version of the config to remove all versions less than.
         * @return Status::OK if the config was removed successfully. Otherwise, an error status is returned.
         */
        collie::Status remove_config_less_than(const std::string &config_name, const collie::ModuleVersion &version);

        /**
         * @brief remove_config is used to remove a config from the ConfigCache with all versions.
         * @param config_name [input] is the name of the config to remove.
         * @return Status::OK if the config was removed successfully. Otherwise, an error status is returned.
         */
        collie::Status remove_config(const std::string &config_name);

    private:

        /**
         *
         * @param dir
         * @param config
         * @return
         */
        collie::Status write_config_file(const std::string &dir, const sirius::proto::ConfigInfo &config);

        /**
         *
         * @param dir
         * @param config
         * @return
         */
        collie::Status remove_config_file(const std::string &dir, const sirius::proto::ConfigInfo &config);

        /**
         *
         * @param dir
         * @param config
         * @return
         */
        std::string make_cache_file_path(const std::string &dir, const sirius::proto::ConfigInfo &config);

        /**
         *
         * @param config
         */
        void do_add_config(const sirius::proto::ConfigInfo &config);

    private:
        typedef turbo::flat_hash_map<std::string, std::map<collie::ModuleVersion, sirius::proto::ConfigInfo>> CacheType;
        std::mutex _cache_mutex;
        CacheType _cache_map;
        std::string _cache_dir;
        bool        _init{false};
    };
}  // namespace sirius::client
