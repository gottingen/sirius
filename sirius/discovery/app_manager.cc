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


#include <sirius/discovery/app_manager.h>
#include <sirius/discovery/discovery_rocksdb.h>
#include <sirius/discovery/base_state_machine.h>

namespace sirius::discovery {

    void AppManager::create_app(const sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done) {
        auto &app_info = const_cast<sirius::proto::AppInfo &>(request.app_info());
        std::string app_name = app_info.app_name();
        if (_app_id_map.find(app_name) != _app_id_map.end()) {
            SS_LOG(WARN) << "request app:" << app_name << " has been existed";
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "app already existed");
            return;
        }
        std::vector<std::string> rocksdb_keys;
        std::vector<std::string> rocksdb_values;

        // prepare app info
        int64_t tmp_app_id = _max_app_id + 1;
        app_info.set_app_id(tmp_app_id);
        app_info.set_version(1);

        std::string app_value;
        if (!app_info.SerializeToString(&app_value)) {
            SS_LOG(WARN) << "request serializeToArray fail, request:" << request.ShortDebugString();
            IF_DONE_SET_RESPONSE(done, sirius::proto::PARSE_TO_PB_FAIL, "serializeToArray fail");
            return;
        }
        rocksdb_keys.push_back(construct_app_key(tmp_app_id));
        rocksdb_values.push_back(app_value);

        // save to rocksdb
        std::string max_app_id_value;
        max_app_id_value.append((char *) &tmp_app_id, sizeof(int64_t));
        rocksdb_keys.push_back(construct_max_app_id_key());
        rocksdb_values.push_back(max_app_id_value);

        int ret = DiscoveryRocksdb::get_instance()->put_discovery_info(rocksdb_keys, rocksdb_values);
        if (ret < 0) {
            IF_DONE_SET_RESPONSE(done, sirius::proto::INTERNAL_ERROR, "write db fail");
            return;
        }
        // update values in memory
        set_app_info(app_info);
        set_max_app_id(tmp_app_id);
        IF_DONE_SET_RESPONSE(done, sirius::proto::SUCCESS, "success");
        SS_LOG(INFO) << "create app success, request:" << request.ShortDebugString();
    }

    void AppManager::drop_app(const sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done) {
        auto &app_info = request.app_info();
        const std::string& app_name = app_info.app_name();
        if (_app_id_map.find(app_name) == _app_id_map.end()) {
            SS_LOG(WARN) << "request app:" << app_name << " not exist";
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "app not exist");
            return;
        }

        int64_t app_id = _app_id_map[app_name];
        if (!_zone_ids[app_id].empty()) {
            SS_LOG(WARN) << "request app:" << app_name << " has zone";
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "app has servlet");
            return;
        }

        std::string app_key = construct_app_key(app_id);

        int ret = DiscoveryRocksdb::get_instance()->remove_discovery_info(std::vector<std::string>{app_key});
        if (ret < 0) {
            IF_DONE_SET_RESPONSE(done, sirius::proto::INTERNAL_ERROR, "write db fail");
            return;
        }

        erase_app_info(app_name);
        IF_DONE_SET_RESPONSE(done, sirius::proto::SUCCESS, "success");
        SS_LOG(INFO) << "drop app success, request:" << request.ShortDebugString();
    }

    void AppManager::modify_app(const sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done) {
        auto &app_info = request.app_info();
        auto &app_name = app_info.app_name();
        if (_app_id_map.find(app_name) == _app_id_map.end()) {
            SS_LOG(WARN) << "request app:" << app_name << " not exist";
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "app not exist");
            return;
        }

        int64_t app_id = _app_id_map[app_name];
        sirius::proto::AppInfo tmp_info = _app_info_map[app_id];
        if (app_info.has_quota()) {
            tmp_info.set_quota(app_info.quota());
        }
        tmp_info.set_version(tmp_info.version() + 1);

        std::string app_value;
        if (!tmp_info.SerializeToString(&app_value)) {
            SS_LOG(WARN) << "request serializeToArray fail, request:" << request.ShortDebugString();
            IF_DONE_SET_RESPONSE(done, sirius::proto::PARSE_TO_PB_FAIL, "serializeToArray fail");
            return;
        }

        int ret = DiscoveryRocksdb::get_instance()->put_discovery_info(construct_app_key(app_id), app_value);
        if (ret < 0) {
            IF_DONE_SET_RESPONSE(done, sirius::proto::INTERNAL_ERROR, "write db fail");
            return;
        }

        set_app_info(tmp_info);
        IF_DONE_SET_RESPONSE(done, sirius::proto::SUCCESS, "success");
        SS_LOG(INFO) << "modify app success, request:" << request.ShortDebugString();
    }

    int AppManager::load_app_snapshot(const std::string &value) {
        sirius::proto::AppInfo app_pb;
        if (!app_pb.ParseFromString(value)) {
            SS_LOG(ERROR) << "parse from pb fail when load app snapshot, value: " << value;
            return -1;
        }
        SS_LOG(INFO) << "load app snapshot success, app_pb:" << app_pb.ShortDebugString();
        set_app_info(app_pb);
        return 0;
    }

}  // namespace sirius::discovery
