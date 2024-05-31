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


#include <sirius/discovery/discovery_rocksdb.h>
#include <gflags/gflags.h>
#include <sirius/flags/discovery.h>
#include <sirius/base/log.h>

namespace sirius::discovery {

    int DiscoveryRocksdb::init() {
        _rocksdb = RocksStorage::get_instance();
        if (!_rocksdb) {
            LOG(ERROR) << "create rocksdb handler failed";
            return -1;
        }
        int ret = _rocksdb->init(FLAGS_discovery_db_path);
        if (ret != 0) {
            LOG(ERROR) << "rocksdb init failed: code: " << ret;
            return -1;
        }
        _handle = _rocksdb->get_meta_info_handle();
        LOG(WARNING) << "rocksdb init success, db_path: " << FLAGS_discovery_db_path;
        return 0;
    }

    int DiscoveryRocksdb::put_discovery_info(const std::string &key, const std::string &value) {
        rocksdb::WriteOptions write_option;
        write_option.disableWAL = true;
        auto status = _rocksdb->put(write_option, _handle, rocksdb::Slice(key), rocksdb::Slice(value));
        if (!status.ok()) {
            LOG(WARNING) << "put rocksdb fail, err_msg: " << status.ToString()<<", key: "<<key<<", value: "<<value;
            return -1;
        }
        return 0;
    }

    int DiscoveryRocksdb::put_discovery_info(const std::vector<std::string> &keys,
                                   const std::vector<std::string> &values) {
        if (keys.size() != values.size()) {
            LOG(WARNING) << "input keys'size is not equal to values' size";
            return -1;
        }
        rocksdb::WriteOptions write_option;
        write_option.disableWAL = true;
        rocksdb::WriteBatch batch;
        for (size_t i = 0; i < keys.size(); ++i) {
            batch.Put(_handle, keys[i], values[i]);
        }
        auto status = _rocksdb->write(write_option, &batch);
        if (!status.ok()) {
            LOG(WARNING) << "put batch to rocksdb fail, err msg: " << status.ToString();
            return -1;
        }
        return 0;
    }

    int DiscoveryRocksdb::get_discovery_info(const std::string &key, std::string *value) {
        rocksdb::ReadOptions options;
        auto status = _rocksdb->get(options, _handle, rocksdb::Slice(key), value);
        if (!status.ok()) {
            return -1;
        }
        return 0;
    }

    int DiscoveryRocksdb::remove_discovery_info(const std::vector<std::string> &keys) {
        rocksdb::WriteOptions write_option;
        write_option.disableWAL = true;
        rocksdb::WriteBatch batch;
        for (auto &key: keys) {
            batch.Delete(_handle, key);
        }
        auto status = _rocksdb->write(write_option, &batch);
        if (!status.ok()) {
            LOG(WARNING) << "delete batch to rocksdb fail,  " << status.ToString();
            return -1;
        }
        return 0;
    }

    int DiscoveryRocksdb::write_discovery_info(const std::vector<std::string> &put_keys,
                                     const std::vector<std::string> &put_values,
                                     const std::vector<std::string> &delete_keys) {
        if (put_keys.size() != put_values.size()) {
            LOG(WARNING) << "input keys'size is not equal to values' size";
            return -1;
        }
        rocksdb::WriteOptions write_option;
        write_option.disableWAL = true;
        rocksdb::WriteBatch batch;
        for (size_t i = 0; i < put_keys.size(); ++i) {
            batch.Put(_handle, put_keys[i], put_values[i]);
        }
        for (auto &delete_key: delete_keys) {
            batch.Delete(_handle, delete_key);
        }
        auto status = _rocksdb->write(write_option, &batch);
        if (!status.ok()) {
            LOG(WARNING) << "write batch to rocksdb fail,  " << status.ToString();
            return -1;
        }
        return 0;
    }
}  // namespace sirius::discovery
