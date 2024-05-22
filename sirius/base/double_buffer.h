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

//
// Created by jeff on 23-11-26.
//

#ifndef EA_DOUBLE_BUFFER_H
#define EA_DOUBLE_BUFFER_H

#include "butil/containers/doubly_buffered_data.h"
#include <bthread/butex.h>
#include <bvar/bvar.h>
#include <bthread/bthread.h>
#include <bthread/unstable.h>
#include <butil/third_party/murmurhash3/murmurhash3.h>
#include <butil/containers/doubly_buffered_data.h>
#include <butil/containers/flat_map.h>
#include <butil/endpoint.h>
#include <butil/base64.h>
#include <butil/fast_rand.h>
#include <butil/sha1.h>
#include <bthread/execution_queue.h>
#include <sirius/base/time_cast.h>
#include "ea/flags/base.h"

namespace sirius {

    class ExecutionQueue {
    public:
        ExecutionQueue() {
            bthread::execution_queue_start(&_queue_id, nullptr, run_function, nullptr);
        }

        void run(const std::function<void()> &call) {
            bthread::execution_queue_execute(_queue_id, call);
        }

        void stop() {
            execution_queue_stop(_queue_id);
        }

        void join() {
            execution_queue_join(_queue_id);
        }

    private:
        static int run_function(void *meta, bthread::TaskIterator<std::function<void()>> &iter) {
            if (iter.is_queue_stopped()) {
                return 0;
            }
            for (; iter; ++iter) {
                (*iter)();
            }
            return 0;
        }

        bthread::ExecutionQueueId<std::function<void()>> _queue_id = {0};
    };
    template<typename T, int64_t SLEEP = 1000>
    class DoubleBuffer {
    public:
        DoubleBuffer() {
            bthread::execution_queue_start(&_queue_id, nullptr, run_function, (void *) this);
        }

        T *read() {
            return _data + _index;
        }

        T *read_background() {
            return _data + !_index;
        }

        void swap() {
            _index = !_index;
        }

        void modify(const std::function<void(T &)> &fn) {
            bthread::execution_queue_execute(_queue_id, fn);
        }

    private:
        ExecutionQueue _queue;
        T _data[2];
        int _index = 0;

        static int run_function(void *meta, bthread::TaskIterator<std::function<void(T &)>> &iter) {
            if (iter.is_queue_stopped()) {
                return 0;
            }
            DoubleBuffer *db = (DoubleBuffer *) meta;
            std::vector<std::function<void(T & )>> vec;
            vec.reserve(3);
            for (; iter; ++iter) {
                (*iter)(*db->read_background());
                vec.emplace_back(*iter);
            }
            db->swap();
            bthread_usleep(SLEEP);
            for (auto &c: vec) {
                c(*db->read_background());
            }
            return 0;
        }

        bthread::ExecutionQueueId<std::function<void(T &)>> _queue_id = {0};
    };

    template<typename T>
    class IncrementalUpdate {
    public:
        IncrementalUpdate() {
            fiber_mutex_init(&_mutex, nullptr);
        }

        ~IncrementalUpdate() {
            fiber_mutex_destroy(&_mutex);
        }

        void put_incremental_info(const int64_t apply_index, T &infos) {
            MELON_SCOPED_LOCK(_mutex);
            auto background = _buf.read_background();
            auto frontground = _buf.read();
            //保证bg中最少有一个元素
            if (background->size() <= 0) {
                (*background)[apply_index] = infos;
                _the_earlist_time_for_background.reset();
                return;
            }
            (*background)[apply_index] = infos;
            // 当bg中最早的元素大于回收时间时清理fg，互换bg和fg；这样可以保证清理掉的都是大于超时时间的，极端情况下超时回收时间变为2倍的gc time
            if (_the_earlist_time_for_background.get_time() > FLAGS_incremental_info_gc_time) {
                frontground->clear();
                _buf.swap();
            }
        }

        // 返回值 true:需要全量更新外部处理 false:增量更新，通过update_incremental处理增量
        bool
        check_and_update_incremental(std::function<void(const T &)> update_incremental, int64_t &last_updated_index,
                                     const int64_t applied_index) {
            MELON_SCOPED_LOCK(_mutex);
            auto background = _buf.read_background();
            auto frontground = _buf.read();
            if (frontground->size() == 0 && background->size() == 0) {
                if (last_updated_index < applied_index) {
                    return true;
                }
                return false;
            } else if (frontground->size() == 0 && background->size() > 0) {
                if (last_updated_index < background->begin()->first) {
                    return true;
                } else {
                    auto iter = background->upper_bound(last_updated_index);
                    while (iter != background->end()) {
                        if (iter->first > applied_index) {
                            break;
                        }
                        update_incremental(iter->second);
                        last_updated_index = iter->first;
                        ++iter;
                    }
                    return false;
                }
            } else if (frontground->size() > 0) {
                if (last_updated_index < frontground->begin()->first) {
                    return true;
                } else {
                    auto iter = frontground->upper_bound(last_updated_index);
                    while (iter != frontground->end()) {
                        if (iter->first > applied_index) {
                            break;
                        }
                        update_incremental(iter->second);
                        last_updated_index = iter->first;
                        ++iter;
                    }
                    iter = background->upper_bound(last_updated_index);
                    while (iter != background->end()) {
                        if (iter->first > applied_index) {
                            break;
                        }
                        update_incremental(iter->second);
                        last_updated_index = iter->first;
                        ++iter;
                    }
                    return false;
                }
            }
            return false;
        }

        void clear() {
            auto background = _buf.read_background();
            auto frontground = _buf.read();
            background->clear();
            frontground->clear();
        }

    private:
        DoubleBuffer<std::map<int64_t, T>> _buf;
        fiber_mutex_t _mutex;
        TimeCost _the_earlist_time_for_background;
    };


}
#endif //EA_DOUBLE_BUFFER_H
