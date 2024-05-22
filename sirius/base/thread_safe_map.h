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

#include <cstdint>
#include <unordered_map>
#include <functional>
#include <melon/fiber/mutex.h>

namespace sirius {

    template<typename KEY, typename VALUE, uint32_t MAP_COUNT = 23>
    class ThreadSafeMap {
        static_assert(MAP_COUNT > 0, "Invalid MAP_COUNT parameters.");
    public:
        ThreadSafeMap() {
            for (uint32_t i = 0; i < MAP_COUNT; i++) {
                fiber_mutex_init(&_mutex[i], nullptr);
            }
        }

        ~ThreadSafeMap() {
            for (uint32_t i = 0; i < MAP_COUNT; i++) {
                fiber_mutex_destroy(&_mutex[i]);
            }
        }

        uint32_t count(const KEY &key) {
            uint32_t idx = map_idx(key);
            MELON_SCOPED_LOCK(_mutex[idx]);
            return _map[idx].count(key);
        }

        uint32_t size() {
            uint32_t size = 0;
            for (uint32_t i = 0; i < MAP_COUNT; i++) {
                MELON_SCOPED_LOCK(_mutex[i]);
                size += _map[i].size();
            }
            return size;
        }

        void set(const KEY &key, const VALUE &value) {
            uint32_t idx = map_idx(key);
            MELON_SCOPED_LOCK(_mutex[idx]);
            _map[idx][key] = value;
        }

        // 已存在则不插入，返回false；不存在则init
        // init函数需要返回0，否则整个insert返回false
        bool insert_init_if_not_exist(const KEY &key, const std::function<int(VALUE &value)> &call) {
            uint32_t idx = map_idx(key);
            MELON_SCOPED_LOCK(_mutex[idx]);
            if (_map[idx].count(key) == 0) {
                if (call(_map[idx][key]) == 0) {
                    return true;
                } else {
                    _map[idx].erase(key);
                    return false;
                }
            } else {
                return false;
            }
        }

        const VALUE get(const KEY &key) {
            uint32_t idx = map_idx(key);
            MELON_SCOPED_LOCK(_mutex[idx]);
            if (_map[idx].count(key) == 0) {
                static VALUE tmp;
                return tmp;
            }
            return _map[idx][key];
        }

        bool call_and_get(const KEY &key, const std::function<void(VALUE &value)> &call) {
            uint32_t idx = map_idx(key);
            MELON_SCOPED_LOCK(_mutex[idx]);
            if (_map[idx].count(key) == 0) {
                return false;
            } else {
                call(_map[idx][key]);
            }
            return true;
        }

        const VALUE get_or_put(const KEY &key, const VALUE &value) {
            uint32_t idx = map_idx(key);
            MELON_SCOPED_LOCK(_mutex[idx]);
            if (_map[idx].count(key) == 0) {
                _map[idx][key] = value;
                return value;
            }
            return _map[idx][key];
        }

        const VALUE get_or_put_call(const KEY &key, const std::function<VALUE(VALUE &value)> &call) {
            uint32_t idx = map_idx(key);
            MELON_SCOPED_LOCK(_mutex[idx]);
            if (_map[idx].count(key) == 0) {
                return call(_map[idx][key]);
            }
            return _map[idx][key];
        }

        VALUE &operator[](const KEY &key) {
            uint32_t idx = map_idx(key);
            MELON_SCOPED_LOCK(_mutex[idx]);
            return _map[idx][key];
        }

        bool exist(const KEY &key) {
            uint32_t idx = map_idx(key);
            MELON_SCOPED_LOCK(_mutex[idx]);
            return _map[idx].count(key) > 0;
        }

        size_t erase(const KEY &key) {
            uint32_t idx = map_idx(key);
            MELON_SCOPED_LOCK(_mutex[idx]);
            return _map[idx].erase(key);
        }

        bool call_and_erase(const KEY &key, const std::function<void(VALUE &value)> &call) {
            uint32_t idx = map_idx(key);
            MELON_SCOPED_LOCK(_mutex[idx]);
            if (_map[idx].count(key) == 0) {
                return false;
            } else {
                call(_map[idx][key]);
                _map[idx].erase(key);
            }
            return true;
        }

        // 会加锁，轻量级操作采用traverse否则用copy
        void traverse(const std::function<void(VALUE &value)> &call) {
            for (uint32_t i = 0; i < MAP_COUNT; i++) {
                MELON_SCOPED_LOCK(_mutex[i]);
                for (auto &pair: _map[i]) {
                    call(pair.second);
                }
            }
        }

        void traverse_with_key_value(const std::function<void(const KEY &key, VALUE &value)> &call) {
            for (uint32_t i = 0; i < MAP_COUNT; i++) {
                MELON_SCOPED_LOCK(_mutex[i]);
                for (auto &pair: _map[i]) {
                    call(pair.first, pair.second);
                }
            }
        }

        void traverse_copy(const std::function<void(VALUE &value)> &call) {
            for (uint32_t i = 0; i < MAP_COUNT; i++) {
                std::unordered_map<KEY, VALUE> tmp;
                {
                    MELON_SCOPED_LOCK(_mutex[i]);
                    tmp = _map[i];
                }
                for (auto &pair: tmp) {
                    call(pair.second);
                }
            }
        }

        void clear() {
            for (uint32_t i = 0; i < MAP_COUNT; i++) {
                MELON_SCOPED_LOCK(_mutex[i]);
                _map[i].clear();
            }
        }

        // 已存在返回true，不存在init则返回false
        template<typename... Args>
        bool init_if_not_exist_else_update(const KEY &key, bool always_update,
                                           const std::function<void(VALUE &value)> &call, Args &&... args) {
            uint32_t idx = map_idx(key);
            MELON_SCOPED_LOCK(_mutex[idx]);
            auto iter = _map[idx].find(key);
            if (iter == _map[idx].end()) {
                _map[idx].insert(std::make_pair(key, VALUE(std::forward<Args>(args)...)));
                if (always_update) {
                    call(_map[idx][key]);
                }
                return false;
            } else {
                //字段存在，才执行回调
                call(iter->second);
                return true;
            }
        }

        bool update(const KEY &key, const std::function<void(VALUE &value)> &call) {
            uint32_t idx = map_idx(key);
            MELON_SCOPED_LOCK(_mutex[idx]);
            auto iter = _map[idx].find(key);
            if (iter != _map[idx].end()) {
                call(iter->second);
                return true;
            } else {
                return false;
            }
        }

        //返回值：true表示执行了全部遍历，false表示遍历中途退出
        bool traverse_with_early_return(const std::function<bool(VALUE &value)> &call) {
            for (uint32_t i = 0; i < MAP_COUNT; i++) {
                MELON_SCOPED_LOCK(_mutex[i]);
                for (auto &pair: _map[i]) {
                    if (!call(pair.second)) {
                        return false;
                    }
                }
            }
            return true;
        }

    private:
        uint32_t map_idx(const KEY &key) {
            return std::hash<KEY>{}(key) % MAP_COUNT;
        }

    private:
        std::unordered_map<KEY, VALUE> _map[MAP_COUNT];
        fiber_mutex_t _mutex[MAP_COUNT];
        DISALLOW_COPY_AND_ASSIGN(ThreadSafeMap);
    };
}  // namespace sirius

