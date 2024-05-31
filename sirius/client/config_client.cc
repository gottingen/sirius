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


#include <sirius/client/config_client.h>
#include <sirius/client/utility.h>
#include <sirius/client/config_cache.h>
#include <sirius/flags/client.h>

namespace sirius::client {

    turbo::Status ConfigClient::init() {
        if(_init) {
            return turbo::OkStatus();
        }
        auto rs = ConfigCache::get_instance()->init();
        if (!rs.ok()) {
            LOG(ERROR) << "config cache init error:" << rs.message();
            return rs;
        }

        _shutdown = false;
        _bth.run([this] {
            period_check();
        });
        _init = true;
        return turbo::OkStatus();
    }

    void ConfigClient::stop() {
        _shutdown = true;
    }

    ///
    void ConfigClient::join() {
        _bth.join();
    }

    turbo::Status
    ConfigClient::get_config(const std::string &config_name, const std::string &version, std::string &content,
                             std::string *type) {
        collie::ModuleVersion mv;
        auto rs = string_to_module_version(version, &mv);
        if (!rs.ok()) {
            return rs;
        }
        sirius::proto::ConfigInfo config_pb;
        rs = ConfigCache::get_instance()->get_config(config_name, mv, config_pb);
        if (rs.ok()) {
            content = config_pb.content();
            if (type) {
                *type = config_type_to_string(config_pb.type());
            }
            return turbo::OkStatus();
        }

        rs = DiscoveryClient::get_instance()->get_config(config_name, version, config_pb);
        if (!rs.ok()) {
            return rs;
        }
        content = config_pb.content();
        if (type) {
            *type = config_type_to_string(config_pb.type());
        }
        rs = ConfigCache::get_instance()->add_config(config_pb);
        return turbo::OkStatus();
    }

    turbo::Status ConfigClient::get_config(const std::string &config_name, std::string &content, std::string *version,
                                           std::string *type) {
        collie::ModuleVersion mv;
        sirius::proto::ConfigInfo config_pb;
        auto rs = ConfigCache::get_instance()->get_config(config_name, config_pb);
        if (rs.ok()) {
            content = config_pb.content();
            if (type) {
                *type = config_type_to_string(config_pb.type());
            }
            if (version) {
                *version = version_to_string(config_pb.version());
            }
            return turbo::OkStatus();
        }

        rs = DiscoveryClient::get_instance()->get_config_latest(config_name, config_pb);
        if (!rs.ok()) {
            return rs;
        }
        content = config_pb.content();
        if (type) {
            *type = config_type_to_string(config_pb.type());
        }
        if (version) {
            *version = version_to_string(config_pb.version());
        }
        rs = ConfigCache::get_instance()->add_config(config_pb);
        return rs;
    }

    turbo::Status ConfigClient::watch_config(const std::string &config_name, const ConfigEventListener &listener) {
        collie::ModuleVersion module_version;
        std::unique_lock lock(_watch_mutex);
        if (_watches.find(config_name) != _watches.end()) {
            return turbo::already_exists_error("");
        }
        auto ait = _apply_version.find(config_name);
        if (ait != _apply_version.end()) {
            module_version = ait->second;
        }
        _watches[config_name] = ConfigWatchEntity{module_version, listener};
        return turbo::OkStatus();
    }

    turbo::Status ConfigClient::unwatch_config(const std::string &config_name) {
        std::unique_lock lock(_watch_mutex);
        return do_unwatch_config(config_name);
    }

    turbo::Status ConfigClient::do_unwatch_config(const std::string &config_name) {
        if (_watches.find(config_name) == _watches.end()) {
            return turbo::not_found_error("");
        }
        _watches.erase(config_name);
        return turbo::OkStatus();
    }

    turbo::Status ConfigClient::remove_config(const std::string &config_name) {
        return ConfigCache::get_instance()->remove_config(config_name);
    }

    turbo::Status ConfigClient::remove_config(const std::string &config_name, const std::string &version) {
        collie::ModuleVersion module_version;
        auto rs = string_to_module_version(version, &module_version);
        if (!rs.ok()) {
            return rs;
        }
        return ConfigCache::get_instance()->remove_config(config_name, module_version);
    }

    turbo::Status ConfigClient::apply(const std::string &config_name, const collie::ModuleVersion &version) {
        std::unique_lock lock(_watch_mutex);
        return do_apply(config_name, version);
    }

    turbo::Status ConfigClient::apply(const std::string &config_name, const std::string &version) {
        collie::ModuleVersion mv;
        auto rs = string_to_module_version(version, &mv);
        if (!rs.ok()) {
            return rs;
        }
        return apply(config_name, mv);
    }

    turbo::Status ConfigClient::unapply(const std::string &config_name) {
        std::unique_lock lock(_watch_mutex);
        auto rs = do_unwatch_config(config_name);
        (void)(rs);
        return do_unapply(config_name);
    }

    turbo::Status ConfigClient::do_unapply(const std::string &config_name) {
        auto it = _apply_version.find(config_name);
        if (it == _apply_version.end()) {
            return turbo::not_found_error("not found config: " + config_name);
        }
        _apply_version.erase(config_name);
        return turbo::OkStatus();
    }

    turbo::Status ConfigClient::do_apply(const std::string &config_name, const collie::ModuleVersion &version) {
        _apply_version[config_name] = version;
        return turbo::OkStatus();
    }

    void ConfigClient::period_check() {
        static collie::ModuleVersion kZero;
        std::vector<std::pair<std::string, collie::ModuleVersion>> updates;
        turbo::flat_hash_map<std::string, ConfigWatchEntity> watches;
        int sleep_step_us = FLAGS_config_watch_interval_ms * 1000;
        int sleep_round = FLAGS_config_watch_interval_round_s * 1000 * 1000;
        LOG(INFO) << "start config watch background";
        while(!_shutdown) {
            updates.clear();
            watches.clear();
            {
                std::unique_lock lock(_watch_mutex);
                watches = _watches;
            }
            LOG(INFO) << "new round watch size:" << watches.size();
            for(auto &it : watches) {
                sirius::proto::ConfigInfo info;
                collie::ModuleVersion current_version = it.second.notice_version;
                auto rs = DiscoveryClient::get_instance()->get_config_latest(it.first, info);
                if(!rs.ok()) {
                    LOG_IF(WARNING,kZero == it.second.notice_version) << "get config fail:" << rs.message();
                    continue;
                }
                LOG(INFO) << "get config " << info.name() << " version:" << info.version().major() << "."
                             << info.version().minor() << "." << info.version().patch();
                rs = ConfigCache::get_instance()->add_config(info);
                if(!rs.ok() && !turbo::is_already_exists(rs)) {
                    LOG(WARNING) << "add config to cache fail:" << rs.message();
                }
                collie::ModuleVersion new_view(info.version().major(), info.version().minor(), info.version().patch());
                if(current_version == kZero) {
                    if(it.second.listener.on_new_config) {
                        LOG(INFO) << "call new config callback:{}" << info.name();
                        ConfigCallbackData data{info.name(), kZero,new_view,info.content(), config_type_to_string(info.type())};
                        it.second.listener.on_new_config(data);
                    } else {
                        LOG(INFO) << "call new config callback:{} but no call backer",info.name();
                    }
                } else if(current_version < new_view) {
                    if(it.second.listener.on_new_version) {
                        LOG(INFO) << "call new config version, callback:{}" << info.name();
                        ConfigCallbackData data{info.name(), kZero,new_view, info.content(), config_type_to_string(info.type())};
                        it.second.listener.on_new_version(data);
                    } else {
                        LOG(INFO) << "call new config callback:{} but no call backer",info.name();
                    }
                }
                updates.emplace_back(info.name(), new_view);
                fiber_usleep(sleep_step_us);
            }

            {
                std::unique_lock lock(_watch_mutex);
                for(auto &item : updates) {
                    auto it = _watches.find(item.first);
                    if(it == _watches.end()) {
                        continue;
                    }
                    it->second.notice_version = item.second;
                }
            }
            fiber_usleep(sleep_round);

        }
        LOG(INFO) << "config watch background stop...";
    }
}  // namespace sirius::client

