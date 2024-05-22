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


#include <sirius/client/config_cache.h>
#include <sirius/flags/client.h>
#include <sirius/client/utility.h>
#include <alkaid/files/sequential_write_file.h>
#include <alkaid/files/sequential_read_file.h>
#include <sirius/client/loader.h>
#include <sirius/client/dumper.h>

namespace sirius::client {

    collie::Status ConfigCache::init() {
        if(_init) {
            return collie::Status::ok_status();
        }
        _cache_dir = sirius::FLAGS_config_cache_dir;
        if(_cache_dir.empty()) {
            return collie::Status::ok_status();
        }
        std::error_code ec;
        if(!collie::filesystem::exists(_cache_dir, ec)) {
            if(ec) {
                return collie::Status::io_error("check cache dir error");
            }
            collie::filesystem::create_directories(_cache_dir);
            return collie::Status::ok_status();
        }
        collie::filesystem::directory_iterator dir_itr(_cache_dir);
        collie::filesystem::directory_iterator end;
        for(;dir_itr != end; ++dir_itr) {
            if(dir_itr->path() == "." || dir_itr->path() == "..") {
                continue;
            }
            auto file_path = dir_itr->path().string();
            sirius::proto::ConfigInfo info;
            auto rs = Loader::load_proto_from_file(file_path, info);
            if(!rs.ok()) {
                return rs;
            }
            do_add_config(info);
            SS_LOG(INFO) << "loading config cache file:" << file_path;
        }
        _init = true;
        return collie::Status::ok_status();
    }
    collie::Status ConfigCache::add_config(const sirius::proto::ConfigInfo &config) {
        {
            std::unique_lock lock(_cache_mutex);
            if (_cache_map.find(config.name()) == _cache_map.end()) {
                _cache_map[config.name()] = std::map<collie::ModuleVersion, sirius::proto::ConfigInfo>();
            }
            auto it = _cache_map.find(config.name());
            auto version = collie::ModuleVersion(config.version().major(), config.version().minor(),
                                                config.version().patch());
            if (it->second.find(version) != it->second.end()) {
                return collie::Status::already_exists("config already exists");
            }
            it->second[version] = config;
        }
        return write_config_file(_cache_dir, config);
    }

    void ConfigCache::do_add_config(const sirius::proto::ConfigInfo &config) {
            if (_cache_map.find(config.name()) == _cache_map.end()) {
                _cache_map[config.name()] = std::map<collie::ModuleVersion, sirius::proto::ConfigInfo>();
            }
            auto it = _cache_map.find(config.name());
            auto version = collie::ModuleVersion(config.version().major(), config.version().minor(),
                                                config.version().patch());
            it->second[version] = config;
    }

    collie::Status ConfigCache::get_config(const std::string &name, const collie::ModuleVersion &version,
                                          sirius::proto::ConfigInfo &config) {
        std::unique_lock lock(_cache_mutex);
        auto it = _cache_map.find(config.name());
        if (it != _cache_map.end()) {
            auto vit = it->second.find(version);
            if (vit != it->second.end()) {
                config = vit->second;
                return collie::Status::ok_status();
            }
        }
        return collie::Status::not_found("config not found");
    }

    ///
    /// \brief get latest version of config
    /// \param name
    /// \param config
    /// \return
    collie::Status ConfigCache::get_config(const std::string &name, sirius::proto::ConfigInfo &config) {
        std::unique_lock lock(_cache_mutex);
        auto it = _cache_map.find(config.name());
        if (it != _cache_map.end()) {
            auto vit = it->second.rbegin();
            if (vit != it->second.rend()) {
                config = vit->second;
                return collie::Status::ok_status();
            }
        }
        return collie::Status::not_found("config not found");
    }

    ///
    /// \param name
    /// \return
    collie::Status ConfigCache::get_config_list(std::vector<std::string> &configs) {
        std::unique_lock lock(_cache_mutex);
        for (auto it = _cache_map.begin(); it != _cache_map.end(); ++it) {
            configs.push_back(it->first);
        }
        return collie::Status::ok_status();
    }

    ///
    /// \param config_name
    /// \param versions
    /// \return
    collie::Status
    ConfigCache::get_config_version_list(const std::string &config_name, std::vector<collie::ModuleVersion> &versions) {
        std::unique_lock lock(_cache_mutex);
        auto it = _cache_map.find(config_name);
        if (it != _cache_map.end()) {
            for (auto vit = it->second.begin(); vit != it->second.end(); ++vit) {
                versions.push_back(vit->first);
            }
            return collie::Status::ok_status();
        }
        return collie::Status::not_found("config not found");
    }

    ///
    /// \brief remove signal version of config
    /// \param config_name
    /// \param version
    /// \return
    collie::Status ConfigCache::remove_config(const std::string &config_name, const collie::ModuleVersion &version) {
        std::unique_lock lock(_cache_mutex);
        auto it = _cache_map.find(config_name);
        if (it != _cache_map.end()) {
            auto vit = it->second.find(version);
            if (vit != it->second.end()) {
                auto rs = remove_config_file(_cache_dir, vit->second);
                TURBO_UNUSED(rs);
                it->second.erase(vit);
                if (it->second.empty()) {
                    _cache_map.erase(it);
                }
                return collie::Status::ok_status();
            }
        }
        return collie::Status::not_found("config not found");
    }

    ///
    /// \param config_name
    /// \param version
    /// \return
    collie::Status
    ConfigCache::remove_config(const std::string &config_name, const std::vector<collie::ModuleVersion> &versions) {
        std::unique_lock lock(_cache_mutex);
        auto it = _cache_map.find(config_name);
        if (it != _cache_map.end()) {
            for (auto &version: versions) {
                auto vit = it->second.find(version);
                if (vit != it->second.end()) {
                    auto rs = remove_config_file(_cache_dir, vit->second);
                    TURBO_UNUSED(rs);
                    it->second.erase(vit);
                }
            }
            if (it->second.empty()) {
                _cache_map.erase(it);
            }
            return collie::Status::ok_status();
        }
        return collie::Status::not_found("config not found");
    }

    ///
    /// \brief remove config versions less than the given version
    /// \param config_name
    /// \param version
    /// \return
    collie::Status
    ConfigCache::remove_config_less_than(const std::string &config_name, const collie::ModuleVersion &version) {
        std::unique_lock lock(_cache_mutex);
        auto it = _cache_map.find(config_name);
        if (it != _cache_map.end()) {
            auto vit = it->second.lower_bound(version);
            for (auto rit = it->second.begin(); rit != vit; ++rit) {
                auto rs = remove_config_file(_cache_dir, rit->second);
                TURBO_UNUSED(rs);
            }
            it->second.erase(it->second.begin(), vit);
            if (it->second.empty()) {
                _cache_map.erase(it);
            }
            return collie::Status::ok_status();
        }
        return collie::Status::not_found("config not found");
    }

    ///
    /// \brief remove all version of config
    /// \param config_name
    /// \return
    collie::Status ConfigCache::remove_config(const std::string &config_name) {
        std::unique_lock lock(_cache_mutex);
        auto it = _cache_map.find(config_name);
        if (it != _cache_map.end()) {
            for (auto &vit: it->second) {
                auto rs = remove_config_file(_cache_dir, vit.second);
                TURBO_UNUSED(rs);
            }
            _cache_map.erase(it);
            return collie::Status::ok_status();
        }
        return collie::Status::not_found("config not found");
    }

    collie::Status ConfigCache::write_config_file(const std::string &dir, const sirius::proto::ConfigInfo &config) {
        if (dir.empty()) {
            return collie::Status::ok_status();
        }
        auto file_path = make_cache_file_path(dir, config);
        return sirius::client::Dumper::dump_proto_to_file(file_path, config);
    }

    collie::Status ConfigCache::remove_config_file(const std::string &dir, const sirius::proto::ConfigInfo &config) {
        if (dir.empty()) {
            return collie::Status::ok_status();
        }
        auto file_path = make_cache_file_path(dir, config);
        collie::filesystem::remove(file_path);
        return collie::Status::ok_status();
    }

    std::string ConfigCache::make_cache_file_path(const std::string &dir, const sirius::proto::ConfigInfo &config) {
        return collie::format("{}/{}-{}.{}.{}.{}", dir,
                             config.name(),
                             config.version().minor(),
                             config.version().minor(),
                             config.version().patch(),
                             config_type_to_string(config.type()));
    }
}  // namespace sirius::client