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

#include <sirius/storage/rocks_storage.h>

namespace sirius::discovery {
    class DiscoveryRocksdb {
    public:
        virtual ~DiscoveryRocksdb() {}

        static DiscoveryRocksdb *get_instance() {
            static DiscoveryRocksdb _instance;
            return &_instance;
        }

        int init();

        int put_discovery_info(const std::string &key, const std::string &value);

        int put_discovery_info(const std::vector<std::string> &keys,
                          const std::vector<std::string> &values);

        int get_discovery_info(const std::string &key, std::string *value);

        int remove_discovery_info(const std::vector<std::string> &keys);

        int write_discovery_info(const std::vector<std::string> &put_keys,
                            const std::vector<std::string> &put_values,
                            const std::vector<std::string> &delete_keys);

    private:
        DiscoveryRocksdb() {}

        RocksStorage *_rocksdb = nullptr;
        rocksdb::ColumnFamilyHandle *_handle = nullptr;
    }; //class

}  // namespace sirius::discovery
