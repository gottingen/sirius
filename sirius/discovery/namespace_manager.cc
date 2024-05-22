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


#include <sirius/discovery/namespace_manager.h>
#include <sirius/discovery/discovery_rocksdb.h>
#include <sirius/discovery/base_state_machine.h>

namespace sirius::discovery {

    void NamespaceManager::create_namespace(const sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done) {
        auto &namespace_info = const_cast<sirius::proto::NameSpaceInfo &>(request.namespace_info());
        std::string namespace_name = namespace_info.namespace_name();
        if (_namespace_id_map.find(namespace_name) != _namespace_id_map.end()) {
            SS_LOG(WARN) << "request namespace:" << namespace_name << " has been existed";
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "namespace already existed");
            return;
        }
        std::vector<std::string> rocksdb_keys;
        std::vector<std::string> rocksdb_values;

        // prepare namespace info
        int64_t tmp_namespace_id = _max_namespace_id + 1;
        namespace_info.set_namespace_id(tmp_namespace_id);
        namespace_info.set_version(1);

        std::string namespace_value;
        if (!namespace_info.SerializeToString(&namespace_value)) {
            SS_LOG(WARN) << "request serializeToArray fail, request:" << request.ShortDebugString();
            IF_DONE_SET_RESPONSE(done, sirius::proto::PARSE_TO_PB_FAIL, "serializeToArray fail");
            return;
        }
        rocksdb_keys.push_back(construct_namespace_key(tmp_namespace_id));
        rocksdb_values.push_back(namespace_value);

        // save to rocksdb
        std::string max_namespace_id_value;
        max_namespace_id_value.append((char *) &tmp_namespace_id, sizeof(int64_t));
        rocksdb_keys.push_back(construct_max_namespace_id_key());
        rocksdb_values.push_back(max_namespace_id_value);

        int ret = DiscoveryRocksdb::get_instance()->put_discovery_info(rocksdb_keys, rocksdb_values);
        if (ret < 0) {
            IF_DONE_SET_RESPONSE(done, sirius::proto::INTERNAL_ERROR, "write db fail");
            return;
        }
        // update values in memory
        set_namespace_info(namespace_info);
        set_max_namespace_id(tmp_namespace_id);
        IF_DONE_SET_RESPONSE(done, sirius::proto::SUCCESS, "success");
        SS_LOG(INFO) << "create namespace success, request:" << request.ShortDebugString();
    }

    void NamespaceManager::drop_namespace(const sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done) {
        auto &namespace_info = request.namespace_info();
        std::string namespace_name = namespace_info.namespace_name();
        if (_namespace_id_map.find(namespace_name) == _namespace_id_map.end()) {
            SS_LOG(WARN) << "request namespace:" << namespace_name << " not exist";
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "namespace not exist");
            return;
        }

        int64_t namespace_id = _namespace_id_map[namespace_name];
        if (!_zone_ids[namespace_id].empty()) {
            SS_LOG(WARN) << "request namespace:" << namespace_name << " has zone";
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "namespace has servlet");
            return;
        }

        std::string namespace_key = construct_namespace_key(namespace_id);

        int ret = DiscoveryRocksdb::get_instance()->remove_discovery_info(std::vector<std::string>{namespace_key});
        if (ret < 0) {
            IF_DONE_SET_RESPONSE(done, sirius::proto::INTERNAL_ERROR, "write db fail");
            return;
        }

        erase_namespace_info(namespace_name);
        IF_DONE_SET_RESPONSE(done, sirius::proto::SUCCESS, "success");
        SS_LOG(INFO) << "drop namespace success, request:" << request.ShortDebugString();
    }

    void NamespaceManager::modify_namespace(const sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done) {
        auto &namespace_info = request.namespace_info();
        std::string namespace_name = namespace_info.namespace_name();
        if (_namespace_id_map.find(namespace_name) == _namespace_id_map.end()) {
            SS_LOG(WARN) << "request namespace:" << namespace_name << " not exist";
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "namespace not exist");
            return;
        }
        //目前支持改quota
        int64_t namespace_id = _namespace_id_map[namespace_name];
        sirius::proto::NameSpaceInfo tmp_info = _namespace_info_map[namespace_id];
        if (namespace_info.has_quota()) {
            tmp_info.set_quota(namespace_info.quota());
        }
        if (namespace_info.has_resource_tag()) {
            tmp_info.set_resource_tag(namespace_info.resource_tag());
        }
        if (namespace_info.has_byte_size_per_record()) {
            tmp_info.set_byte_size_per_record(namespace_info.byte_size_per_record());
        }
        if (namespace_info.has_replica_num()) {
            tmp_info.set_replica_num(namespace_info.replica_num());
        }
        if (namespace_info.has_region_split_lines()) {
            tmp_info.set_region_split_lines(namespace_info.region_split_lines());
        }
        tmp_info.set_version(tmp_info.version() + 1);

        //持久化新的namespace信息
        std::string namespace_value;
        if (!tmp_info.SerializeToString(&namespace_value)) {
            SS_LOG(WARN) << "request serializeToArray fail, request:" << request.ShortDebugString();
            IF_DONE_SET_RESPONSE(done, sirius::proto::PARSE_TO_PB_FAIL, "serializeToArray fail");
            return;
        }

        int ret = DiscoveryRocksdb::get_instance()->put_discovery_info(construct_namespace_key(namespace_id), namespace_value);
        if (ret < 0) {
            IF_DONE_SET_RESPONSE(done, sirius::proto::INTERNAL_ERROR, "write db fail");
            return;
        }

        //更新内存值
        set_namespace_info(tmp_info);
        IF_DONE_SET_RESPONSE(done, sirius::proto::SUCCESS, "success");
        SS_LOG(INFO) << "modify namespace success, request:" << request.ShortDebugString();
    }

    int NamespaceManager::load_namespace_snapshot(const std::string &value) {
        sirius::proto::NameSpaceInfo namespace_pb;
        if (!namespace_pb.ParseFromString(value)) {
            SS_LOG(ERROR) << "parse from pb fail when load namespace snapshot, value: " << value;
            return -1;
        }
        SS_LOG(INFO) << "load namespace snapshot success, namespace_pb:" << namespace_pb.ShortDebugString();
        set_namespace_info(namespace_pb);
        return 0;
    }

}  // namespace sirius::discovery
