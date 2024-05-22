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


#include <sirius/discovery/instance_manager.h>
#include <sirius/discovery/schema_manager.h>
#include <sirius/discovery/discovery_rocksdb.h>

namespace sirius::discovery {

    void InstanceManager::add_instance(const sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done) {
        auto &instance_info = const_cast<sirius::proto::ServletInstance &>(request.instance_info());
        const std::string &address = instance_info.address();

        auto ret = SchemaManager::get_instance()->check_and_get_for_instance(instance_info);
        if (ret < 0) {
            SS_LOG(WARN) << "request not illegal, request: " << request.ShortDebugString();
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "request invalid");
            return;
        }
        if (_instance_info.find(address) != _instance_info.end()) {
            SS_LOG(WARN) << "request instance:" << address << " has been existed";
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "instance already existed");
            return;
        }
        auto it = _removed_instance.find(address);
        if(it != _removed_instance.end()) {
            if(it->second.get_time_s() < int64_t(3600)) {
                SS_LOG(WARN) << "request instance:" << address << " has been removed in 1 hour";
                IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "removed in 1 hour");
                return;
            }
        }
        std::vector<std::string> rocksdb_keys;
        std::vector<std::string> rocksdb_values;

        // prepare address info
        instance_info.set_version(1);

        std::string instance_value;
        if (!instance_info.SerializeToString(&instance_value)) {
            SS_LOG(WARN) << "request serializeToArray fail, request: " << request.ShortDebugString();
            IF_DONE_SET_RESPONSE(done, sirius::proto::PARSE_TO_PB_FAIL, "serializeToArray fail");
            return;
        }
        rocksdb_keys.push_back(construct_instance_key(address));
        rocksdb_values.push_back(instance_value);

        // save to rocksdb

        ret = DiscoveryRocksdb::get_instance()->put_discovery_info(rocksdb_keys, rocksdb_values);
        if (ret < 0) {
            IF_DONE_SET_RESPONSE(done, sirius::proto::INTERNAL_ERROR, "write db fail");
            return;
        }
        // update values in memory
        MELON_SCOPED_LOCK(_instance_mutex);
        set_instance_info(instance_info);
        IF_DONE_SET_RESPONSE(done, sirius::proto::SUCCESS, "success");
        SS_LOG(INFO) << "create instance success, request: " << request.ShortDebugString();
    }

    void InstanceManager::drop_instance(const sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done) {
        auto &instance_info = request.instance_info();
        std::string address = instance_info.address();
        if (_instance_info.find(address) == _instance_info.end()) {
            SS_LOG(WARN) << "request address:" << address << " not exist";
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "address not exist");
            return;
        }

        std::string instance_key = construct_instance_key(address);

        int ret = DiscoveryRocksdb::get_instance()->remove_discovery_info(std::vector<std::string>{instance_key});
        if (ret < 0) {
            IF_DONE_SET_RESPONSE(done, sirius::proto::INTERNAL_ERROR, "write db fail");
            return;
        }

        remove_instance_info(address);
        IF_DONE_SET_RESPONSE(done, sirius::proto::SUCCESS, "success");
        SS_LOG(INFO) << "drop instance success, request: " << request.ShortDebugString();
    }

    void InstanceManager::update_instance(const sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done) {
        auto &instance_info = request.instance_info();
        std::string address = instance_info.address();
        if (_instance_info.find(address) == _instance_info.end()) {
            SS_LOG(WARN) << "request address:" << address << " not exist";
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "address not exist");
            return;
        }

        auto tmp_instance_pb = _instance_info[address];
        if(instance_info.has_status()) {
            tmp_instance_pb.set_status(instance_info.status());
        }
        if(instance_info.has_color()) {
            tmp_instance_pb.set_color(instance_info.color());
        }

        if(instance_info.has_env()) {
            tmp_instance_pb.set_env(instance_info.env());
        }

        if(instance_info.has_weight()) {
            tmp_instance_pb.set_weight(instance_info.weight());
        }
        tmp_instance_pb.set_version(tmp_instance_pb.version() + 1);
        std::string instance_key = construct_instance_key(address);

        std::string instance_value;
        if (!tmp_instance_pb.SerializeToString(&instance_value)) {
            SS_LOG(WARN) << "request serializeToArray fail, request: " << request.ShortDebugString();
            IF_DONE_SET_RESPONSE(done, sirius::proto::PARSE_TO_PB_FAIL, "serializeToArray fail");
            return;
        }

        int ret = DiscoveryRocksdb::get_instance()->put_discovery_info(instance_key, instance_value);
        if (ret < 0) {
            IF_DONE_SET_RESPONSE(done, sirius::proto::INTERNAL_ERROR, "write db fail");
            return;
        }

        MELON_SCOPED_LOCK(_instance_mutex);
        set_instance_info(tmp_instance_pb);
        IF_DONE_SET_RESPONSE(done, sirius::proto::SUCCESS, "success");
        SS_LOG(INFO) << "update instance success, request: " << request.ShortDebugString();
    }

    int InstanceManager::load_instance_snapshot(const std::string &value) {
        sirius::proto::ServletInstance instance_pb;
        if (!instance_pb.ParseFromString(value)) {
            SS_LOG(ERROR) << "parse from pb fail when load instance snapshot, value: " << value;
            return -1;
        }
        SS_LOG(WARN) << "instance snapshot:" << instance_pb.ShortDebugString();
        set_instance_info(instance_pb);
        return 0;
    }

    void InstanceManager::set_instance_info(const sirius::proto::ServletInstance &instance_info) {
        _instance_info[instance_info.address()] = instance_info;
        _removed_instance.erase(instance_info.address());
        if(_namespace_instance.find(instance_info.namespace_name()) == _namespace_instance.end()) {
            _namespace_instance[instance_info.namespace_name()] = turbo::flat_hash_set<std::string>();
        }
        _namespace_instance[instance_info.namespace_name()].insert(instance_info.address());

        auto zone_key = ZoneManager::make_zone_key(instance_info.namespace_name(), instance_info.zone_name());
        if(_zone_instance.find(zone_key) == _zone_instance.end()) {
            _zone_instance[zone_key] = turbo::flat_hash_set<std::string>();
        }
        _zone_instance[zone_key].insert(instance_info.address());

        auto servlet_key = ServletManager::make_servlet_key(zone_key, instance_info.servlet_name());
        if(_servlet_instance.find(servlet_key) == _servlet_instance.end()) {
            _servlet_instance[servlet_key] = turbo::flat_hash_set<std::string>();
        }
        _servlet_instance[servlet_key].insert(instance_info.address());
    }

    void InstanceManager::remove_instance_info(const std::string &address) {
        MELON_SCOPED_LOCK(_instance_mutex);
        auto &info = _instance_info[address];
        _removed_instance[address] = TimeCost();
        _namespace_instance.erase(info.namespace_name());
        auto zone_key = ZoneManager::make_zone_key(info.namespace_name(), info.zone_name());
        _zone_instance.erase(zone_key);
        auto servlet_key = ServletManager::make_servlet_key(zone_key, info.servlet_name());
        _servlet_instance.erase(servlet_key);
        _instance_info.erase(address);

    }

    int InstanceManager::load_snapshot() {
        MELON_SCOPED_LOCK( InstanceManager::get_instance()->_instance_mutex);
        SS_LOG(INFO) << "start to load instance snapshot";
        clear();
        std::string config_prefix = DiscoveryConstants::DISCOVERY_IDENTIFY;
        rocksdb::ReadOptions read_options;
        read_options.prefix_same_as_start = true;
        read_options.total_order_seek = false;
        RocksStorage *db = RocksStorage::get_instance();
        std::unique_ptr<rocksdb::Iterator> iter(
                db->new_iterator(read_options, db->get_meta_info_handle()));
        iter->Seek(config_prefix);
        for (; iter->Valid(); iter->Next()) {
            if(load_instance_snapshot(iter->value().ToString()) != 0) {
                return -1;
            }
        }
        SS_LOG(INFO) << "load instance snapshot done";
        return 0;
    }

}  // namespace sirius::discovery
