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


#include <sirius/discovery/zone_manager.h>
#include <sirius/discovery/base_state_machine.h>
#include <sirius/discovery/discovery_rocksdb.h>
#include <sirius/discovery/app_manager.h>
#include <sirius/base/log.h>

namespace sirius::discovery {
    void ZoneManager::create_zone(const sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done) {
        // check legal
        auto &zone_info = const_cast<sirius::proto::ZoneInfo &>(request.zone_info());
        std::string app_name = zone_info.app_name();
        std::string zone_name = app_name + "\001" + zone_info.zone();
        int64_t app_id = AppManager::get_instance()->get_app_id(app_name);
        if (app_id == 0) {
            SS_LOG(WARN) << "request app not exist, request:" << request.ShortDebugString();
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "app not exist");
            return;
        }
        if (_zone_id_map.find(zone_name) != _zone_id_map.end()) {
            SS_LOG(WARN) << "request zone already exist, request:" << request.ShortDebugString();
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "zone already exist");
            return;
        }

        std::vector<std::string> rocksdb_keys;
        std::vector<std::string> rocksdb_values;

        // prepare zone info
        int64_t tmp_zone_id = _max_zone_id + 1;
        zone_info.set_zone_id(tmp_zone_id);
        zone_info.set_app_id(app_id);
        zone_info.set_version(1);

        std::string zone_value;
        if (!zone_info.SerializeToString(&zone_value)) {
            SS_LOG(WARN) << "request serializeToArray fail, request:" << request.ShortDebugString();
            IF_DONE_SET_RESPONSE(done, sirius::proto::PARSE_TO_PB_FAIL, "serializeToArray fail");
            return;
        }
        rocksdb_keys.push_back(construct_zone_key(tmp_zone_id));
        rocksdb_values.push_back(zone_value);

        // persist zone_id
        std::string max_zone_id_value;
        max_zone_id_value.append((char *) &tmp_zone_id, sizeof(int64_t));
        rocksdb_keys.push_back(construct_max_zone_id_key());
        rocksdb_values.push_back(max_zone_id_value);

        int ret = DiscoveryRocksdb::get_instance()->put_discovery_info(rocksdb_keys, rocksdb_values);
        if (ret < 0) {
            IF_DONE_SET_RESPONSE(done, sirius::proto::INTERNAL_ERROR, "write db fail");
            return;
        }
        // update memory info
        set_zone_info(zone_info);
        set_max_zone_id(tmp_zone_id);
        AppManager::get_instance()->add_zone_id(app_id, tmp_zone_id);
        IF_DONE_SET_RESPONSE(done, sirius::proto::SUCCESS, "success");
        SS_LOG(INFO) << "create zone success, request:" << request.ShortDebugString();
    }

    void ZoneManager::drop_zone(const sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done) {
        // check
        auto &zone_info = request.zone_info();
        std::string app_name = zone_info.app_name();
        std::string zone_name = app_name + "\001" + zone_info.zone();
        int64_t app_id = AppManager::get_instance()->get_app_id(app_name);
        if (app_id == 0) {
            SS_LOG(WARN) << "request app not exist, request:" << request.ShortDebugString();
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "app not exist");
            return;
        }
        if (_zone_id_map.find(zone_name) == _zone_id_map.end()) {
            SS_LOG(WARN) << "request zone not exist, request:" << request.ShortDebugString();
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "zone not exist");
            return;
        }
        int64_t zone_id = _zone_id_map[zone_name];
        if (!_servlet_ids[zone_id].empty()) {
            SS_LOG(WARN) << "request zone has servlet, request:" << request.ShortDebugString();
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "zone has servlet");
            return;
        }
        // persist to rocksdb
        int ret = DiscoveryRocksdb::get_instance()->remove_discovery_info(
                std::vector<std::string>{construct_zone_key(zone_id)});
        if (ret < 0) {
            SS_LOG(WARN) << "drop zone: " << zone_name << " to rocksdb fail";
            IF_DONE_SET_RESPONSE(done, sirius::proto::INTERNAL_ERROR, "write db fail");
            return;
        }
        // update zone memory info
        erase_zone_info(zone_name);
        // update app memory info
        AppManager::get_instance()->delete_zone_id(app_id, zone_id);
        IF_DONE_SET_RESPONSE(done, sirius::proto::SUCCESS, "success");
        SS_LOG(INFO) << "drop zone success, request:" << request.ShortDebugString();
    }

    void ZoneManager::modify_zone(const sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done) {
        auto &zone_info = request.zone_info();
        std::string app_name = zone_info.app_name();
        std::string zone_name = app_name + "\001" + zone_info.zone();
        int64_t app_id = AppManager::get_instance()->get_app_id(app_name);
        if (app_id == 0) {
            SS_LOG(WARN) << "request app not exist, request:" << request.ShortDebugString();
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "app not exist");
            return;
        }
        if (_zone_id_map.find(zone_name) == _zone_id_map.end()) {
            SS_LOG(WARN) << "request zone not exist, request:" << request.ShortDebugString();
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "zone not exist");
            return;
        }
        int64_t zone_id = _zone_id_map[zone_name];

        sirius::proto::ZoneInfo tmp_zone_info = _zone_info_map[zone_id];
        tmp_zone_info.set_version(tmp_zone_info.version() + 1);
        if (zone_info.has_quota()) {
            tmp_zone_info.set_quota(zone_info.quota());
        }
        std::string zone_value;
        if (!tmp_zone_info.SerializeToString(&zone_value)) {
            SS_LOG(WARN) << "request serializeToArray fail, request:" << request.ShortDebugString();
            IF_DONE_SET_RESPONSE(done, sirius::proto::PARSE_TO_PB_FAIL, "serializeToArray fail");
            return;
        }
        int ret = DiscoveryRocksdb::get_instance()->put_discovery_info(construct_zone_key(zone_id), zone_value);
        if (ret < 0) {
            IF_DONE_SET_RESPONSE(done, sirius::proto::INTERNAL_ERROR, "write db fail");
            return;
        }
        // update zone values in memory
        set_zone_info(tmp_zone_info);
        IF_DONE_SET_RESPONSE(done, sirius::proto::SUCCESS, "success");
        SS_LOG(INFO) << "modify zone success, request:" << request.ShortDebugString();
    }

    int ZoneManager::load_zone_snapshot(const std::string &value) {
        sirius::proto::ZoneInfo zone_pb;
        if (!zone_pb.ParseFromString(value)) {
            SS_LOG(ERROR) << "parse from pb fail when load zone snapshot, key:" << value;
            return -1;
        }
        SS_LOG(INFO) << "zone snapshot:" << zone_pb.ShortDebugString();
        set_zone_info(zone_pb);
        // update memory app values.
        AppManager::get_instance()->add_zone_id(
                zone_pb.app_id(),
                zone_pb.zone_id());
        return 0;
    }
}  //  namespace sirius::discovery
