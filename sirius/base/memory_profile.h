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

#include <sirius/base/fiber.h>
#include <sirius/base/thread_safe_map.h>
#include <atomic>

namespace sirius {

    class MemoryGCHandler {
    public:
        virtual ~MemoryGCHandler() {}

        static MemoryGCHandler *get_instance() {
            static MemoryGCHandler _instance;
            return &_instance;
        }

        void memory_gc_thread();

        int init() {
            return 0;
        }

        void close() {
            _shutdown = true;
            _memory_gc_bth.join();
        }

    private:
        MemoryGCHandler() {}

        bool _shutdown = false;
        Fiber _memory_gc_bth;

        DISALLOW_COPY_AND_ASSIGN(MemoryGCHandler);
    };

    class MemTracker {
    public:
        explicit MemTracker(uint64_t log_id, int64_t bytes_limit, MemTracker *parent = nullptr);

        ~MemTracker();

        bool check_bytes_limit() {
            return _bytes_limit > 0 && _bytes_consumed > _bytes_limit;
        }

        bool any_limit_exceeded() {
            if (limit_exceeded() || (_parent != nullptr && _parent->limit_exceeded())) {
                return true;
            }
            return false;
        }

        bool limit_exceeded() const { return _bytes_limit >= 0 && bytes_consumed() > _bytes_limit; }

        void consume(int64_t bytes) {
            _last_active_time = mutil::gettimeofday_us();
            if (bytes <= 0) {
                return;
            }
            _bytes_consumed.fetch_add(bytes, std::memory_order_relaxed);
            if (_parent != nullptr) {
                _parent->_bytes_consumed.fetch_add(bytes, std::memory_order_relaxed);
            }
        }

        void release(int64_t bytes) {
            _last_active_time = mutil::gettimeofday_us();
            if (bytes <= 0) {
                return;
            }
            _bytes_consumed.fetch_sub(bytes, std::memory_order_relaxed);
            if (_parent != nullptr) {
                _parent->_bytes_consumed.fetch_sub(bytes, std::memory_order_relaxed);
            }
        }

        uint64_t log_id() const {
            return _log_id;
        }

        int64_t bytes_limit() const {
            return _bytes_limit;
        }

        int64_t last_active_time() const {
            return _last_active_time;
        }

        int64_t bytes_consumed() const {
            return _bytes_consumed.load(std::memory_order_relaxed);
        }

        bool has_limit_exceeded() const {
            return _limit_exceeded;
        }

        void set_limit_exceeded() {
            _limit_exceeded = true;
        }

        MemTracker *get_parent() {
            return _parent;
        }

    private:

        uint64_t _log_id;
        int64_t _bytes_limit;
        int64_t _last_active_time;
        std::atomic<int64_t> _bytes_consumed;
        MemTracker *_parent = nullptr;
        bool _limit_exceeded;
    };

    typedef std::shared_ptr<MemTracker> SmartMemTracker;

    class MemTrackerPool {
    public:
        static MemTrackerPool *get_instance() {
            static MemTrackerPool _instance;
            return &_instance;
        }

        SmartMemTracker get_mem_tracker(uint64_t log_id);

        void tracker_gc_thread();

        int init();

        void close() {
            _shutdown = true;
            _tracker_gc_bth.join();
        }

    private:
        MemTrackerPool() {}

        int64_t _query_bytes_limit;
        bool _shutdown = false;
        SmartMemTracker _root_tracker = nullptr;
        Fiber _tracker_gc_bth;
        ThreadSafeMap<uint64_t, SmartMemTracker> _mem_tracker_pool;

        DISALLOW_COPY_AND_ASSIGN(MemTrackerPool);
    };


} //namespace sirius
