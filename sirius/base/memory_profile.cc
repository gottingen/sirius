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


#include <sirius/base/memory_profile.h>
#include <sirius/flags/base.h>

namespace sirius {

    void MemoryGCHandler::memory_gc_thread() {
    }

    MemTracker::MemTracker(uint64_t log_id, int64_t bytes_limit, MemTracker *parent) :
            _log_id(log_id), _bytes_limit(bytes_limit),
            _last_active_time(mutil::gettimeofday_us()),
            _bytes_consumed(0), _parent(parent),
            _limit_exceeded(false) {
    }

    MemTracker::~MemTracker() {
        SS_LOG(DEBUG)<< "~MemTracker " << collie::ptr(this) << " log_id:" << _log_id << " used_bytes:" << _bytes_consumed.load();
        int64_t bytes = bytes_consumed();
        if (bytes > 0 && _parent) {
            _parent->release(_bytes_consumed.load());
        }
    }

    void MemTrackerPool::tracker_gc_thread() {
        while (!_shutdown) {
            fiber_usleep_fast_shutdown(FLAGS_memory_gc_interval_s * 1000 * 1000LL, _shutdown);
            std::map<uint64_t, SmartMemTracker> need_erase;
            _mem_tracker_pool.traverse_with_key_value(
                    [&need_erase](const uint64_t &log_id, SmartMemTracker &mem_tracker) {
                        if (mutil::gettimeofday_us() - mem_tracker->last_active_time() >
                            FLAGS_mem_tracker_gc_interval_s * 1000 * 1000LL) {
                            need_erase[log_id] = mem_tracker;
                        }
                    });
            for (auto &iter: need_erase) {
                if (mutil::gettimeofday_us() - iter.second->last_active_time() >
                    FLAGS_mem_tracker_gc_interval_s * 1000 * 1000LL) {
                    _mem_tracker_pool.erase(iter.first);
                }
            }
        }
    }

    int MemTrackerPool::init() {
        _query_bytes_limit = -1;
        if (FLAGS_process_memory_limit_bytes > 0) {
            _query_bytes_limit = FLAGS_process_memory_limit_bytes * FLAGS_query_memory_limit_ratio / 100;
        }
        _root_tracker = std::make_shared<MemTracker>(0, FLAGS_process_memory_limit_bytes, nullptr);
        SS_LOG(INFO)<< "root_limit_size: " << FLAGS_process_memory_limit_bytes << " _query_bytes_limit: " << _query_bytes_limit;
        _tracker_gc_bth.run([this]() { tracker_gc_thread(); });
        return 0;
    }

    SmartMemTracker MemTrackerPool::get_mem_tracker(uint64_t log_id) {
        auto call = [this, log_id](SmartMemTracker &tracker) -> SmartMemTracker {
            tracker = std::make_shared<MemTracker>(log_id, _query_bytes_limit, _root_tracker.get());
            return tracker;
        };
        return _mem_tracker_pool.get_or_put_call(log_id, call);
    }

} //namespace sirius
