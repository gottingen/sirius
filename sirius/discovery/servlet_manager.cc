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

#include <sirius/discovery/servlet_manager.h>
#include <sirius/discovery/zone_manager.h>
#include <sirius/discovery/base_state_machine.h>
#include <sirius/discovery/discovery_rocksdb.h>
#include <sirius/discovery/app_manager.h>

namespace sirius::discovery {
    void ServletManager::create_servlet(const sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done) {
        // check legal
        auto &servlet_info = const_cast<sirius::proto::ServletInfo &>(request.servlet_info());
        std::string app_name = servlet_info.app_name();
        std::string zone_name = app_name + "\001" + servlet_info.zone();
        std::string servlet_name = zone_name + "\001" + servlet_info.servlet_name();
        int64_t app_id = AppManager::get_instance()->get_app_id(app_name);
        if (app_id == 0) {
            SS_LOG(WARN) << "request namespace not exist, request:" << request.ShortDebugString();
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "namespace not exist");
            return;
        }
        int64_t zone_id = ZoneManager::get_instance()->get_zone_id(zone_name);
        if (zone_id == 0) {
            SS_LOG(WARN) << "request zone not exist, request:" << zone_name;
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "zone not exist");
            return;
        }

        if (_servlet_id_map.find(servlet_name) != _servlet_id_map.end()) {
            SS_LOG(WARN) << "request zone: " << servlet_name << " already exist";
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "servlet already exist");
            return;
        }

        std::vector<std::string> rocksdb_keys;
        std::vector<std::string> rocksdb_values;

        // prepare zone info
        int64_t tmp_servlet_id = _max_servlet_id + 1;
        servlet_info.set_servlet_id(tmp_servlet_id);
        servlet_info.set_zone_id(zone_id);
        servlet_info.set_app_id(app_id);
        auto t = turbo::Time::time_now().to_time_t();
        servlet_info.set_ctime(t);
        servlet_info.set_mtime(t);

        std::string servlet_value;
        if (!servlet_info.SerializeToString(&servlet_value)) {
            SS_LOG(WARN) << "request serializeToArray fail, request:" << request.ShortDebugString();
            IF_DONE_SET_RESPONSE(done, sirius::proto::PARSE_TO_PB_FAIL, "serializeToArray fail");
            return;
        }
        rocksdb_keys.push_back(construct_servlet_key(tmp_servlet_id));
        rocksdb_values.push_back(servlet_value);

        // persist zone_id
        std::string max_zone_id_value;
        max_zone_id_value.append((char *) &tmp_servlet_id, sizeof(int64_t));
        rocksdb_keys.push_back(construct_max_servlet_id_key());
        rocksdb_values.push_back(max_zone_id_value);

        int ret = DiscoveryRocksdb::get_instance()->put_discovery_info(rocksdb_keys, rocksdb_values);
        if (ret < 0) {
            IF_DONE_SET_RESPONSE(done, sirius::proto::INTERNAL_ERROR, "write db fail");
            return;
        }
        // update memory info
        set_servlet_info(servlet_info);
        set_max_servlet_id(tmp_servlet_id);
        ZoneManager::get_instance()->add_servlet_id(app_id, tmp_servlet_id);
        IF_DONE_SET_RESPONSE(done, sirius::proto::SUCCESS, "success");
        SS_LOG(WARN) << "create zone success, request:" << request.ShortDebugString();
    }

    void ServletManager::drop_servlet(const sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done) {
        // check
        auto &servlet_info = request.servlet_info();
        std::string app_name = servlet_info.app_name();
        std::string zone_name = app_name + "\001" + servlet_info.zone();
        std::string servlet_name = zone_name + "\001" + servlet_info.servlet_name();
        int64_t app_id = AppManager::get_instance()->get_app_id(app_name);
        if (app_id == 0) {
            SS_LOG(WARN) << "request namespace: " << app_name << " not exist";
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "namespace not exist");
            return;
        }
        int64_t zone_id = ZoneManager::get_instance()->get_zone_id(zone_name);
        if (zone_id == 0) {
            SS_LOG(WARN) << "request zone: " << zone_name << " not exist";
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "namespace not exist");
            return;
        }
        if (_servlet_id_map.find(zone_name) == _servlet_id_map.end()) {
            SS_LOG(WARN) << "request zone: " << zone_name << " not exist";
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "zone not exist");
            return;
        }

        int64_t servlet_id = _servlet_id_map[servlet_name];
        // TODO check no instance of servlet
        // persist to rocksdb
        int ret = DiscoveryRocksdb::get_instance()->remove_discovery_info(
                std::vector<std::string>{construct_servlet_key(zone_id)});
        if (ret < 0) {
            SS_LOG(WARN) << "drop zone: " << zone_name << " to rocksdb fail";
            IF_DONE_SET_RESPONSE(done, sirius::proto::INTERNAL_ERROR, "write db fail");
            return;
        }
        // update zone memory info
        erase_servlet_info(servlet_name);
        // update namespace memory info
        ZoneManager::get_instance()->delete_servlet_id(zone_id, servlet_id);
        IF_DONE_SET_RESPONSE(done, sirius::proto::SUCCESS, "success");
        SS_LOG(WARN) << "drop zone success, request:" << request.ShortDebugString();
    }

    void ServletManager::modify_servlet(const sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done) {
        auto &servlet_info = request.servlet_info();
        const std::string &app_name = servlet_info.app_name();
        std::string zone_name = app_name + "\001" + servlet_info.zone();
        std::string servlet_name = zone_name + "\001" + servlet_info.servlet_name();
        int64_t app_id = AppManager::get_instance()->get_app_id(app_name);
        if (app_id == 0) {
            SS_LOG(WARN) << "request app: " << app_name << " not exist";
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "app not exist");
            return;
        }
        int64_t zone_id = ZoneManager::get_instance()->get_zone_id(zone_name);
        if (zone_id == 0) {
            SS_LOG(WARN) << "request zone: " << zone_name << " not exist";
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "zone not exist");
            return;
        }

        if (_servlet_id_map.find(servlet_name) == _servlet_id_map.end()) {
            SS_LOG(WARN) << "request servlet_name: " << servlet_name << " not exist";
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "servlet not exist");
            return;
        }
        int64_t servlet_id = _servlet_id_map[servlet_name];

        sirius::proto::ServletInfo tmp_servlet_info = _servlet_info_map[servlet_id];
        // update servlet info
        if(servlet_info.has_deleted()) {
            tmp_servlet_info.set_deleted(servlet_info.deleted());
        }

        if(servlet_info.has_status()) {
            tmp_servlet_info.set_status(servlet_info.status());
        }
        if(servlet_info.has_color()) {
            tmp_servlet_info.set_color(servlet_info.color());
        }

        tmp_servlet_info.set_env(servlet_info.env());
        tmp_servlet_info.set_address(servlet_info.address());
        tmp_servlet_info.set_mtime(turbo::Time::time_now().to_time_t());

        std::string servlet_value;
        if (!tmp_servlet_info.SerializeToString(&servlet_value)) {
            SS_LOG(ERROR) << "request serializeToArray fail, request:" << request.ShortDebugString();
            IF_DONE_SET_RESPONSE(done, sirius::proto::PARSE_TO_PB_FAIL, "serializeToArray fail");
            return;
        }
        int ret = DiscoveryRocksdb::get_instance()->put_discovery_info(construct_servlet_key(servlet_id), servlet_value);
        if (ret < 0) {
            IF_DONE_SET_RESPONSE(done, sirius::proto::INTERNAL_ERROR, "write db fail");
            return;
        }
        // update zone values in memory
        set_servlet_info(tmp_servlet_info);
        IF_DONE_SET_RESPONSE(done, sirius::proto::SUCCESS, "success");
        SS_LOG(INFO) << "modify zone success, request:" << tmp_servlet_info.ShortDebugString();
    }

    int ServletManager::load_servlet_snapshot(const std::string &value) {
        sirius::proto::ServletInfo servlet_pb;
        if (!servlet_pb.ParseFromString(value)) {
            SS_LOG(WARN) << "parse from pb fail when load zone snapshot, key:" << value;
            return -1;
        }
        SS_LOG(WARN) << "servlet snapshot:" << servlet_pb.ShortDebugString();
        set_servlet_info(servlet_pb);
        // update memory namespace values.
        ZoneManager::get_instance()->add_servlet_id(
                servlet_pb.zone_id(), servlet_pb.servlet_id());
        return 0;
    }
}  //  namespace sirius::discovery
