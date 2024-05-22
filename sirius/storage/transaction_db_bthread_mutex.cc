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


#include <melon/fiber/fiber.h>
#include <melon/fiber/condition_variable.h>
#include <melon/fiber/mutex.h>
#include <sirius/storage/transaction_db_bthread_mutex.h>

namespace sirius {

    class TransactionDBBthreadMutex : public rocksdb::TransactionDBMutex {
    public:
        TransactionDBBthreadMutex() {}

        ~TransactionDBBthreadMutex() override {}

        rocksdb::Status Lock() override;

        rocksdb::Status TryLockFor(int64_t timeout_time) override;

        void UnLock() override { _mutex.unlock(); }

        friend class TransactionDBBthreadCond;

    private:
        fiber::Mutex _mutex;
    };

    class TransactionDBBthreadCond : public rocksdb::TransactionDBCondVar {
    public:
        TransactionDBBthreadCond() {}

        ~TransactionDBBthreadCond() override {}

        rocksdb::Status Wait(std::shared_ptr<rocksdb::TransactionDBMutex> mutex) override;

        rocksdb::Status WaitFor(std::shared_ptr<rocksdb::TransactionDBMutex> mutex,
                                int64_t timeout_time) override;

        void Notify() override { _cv.notify_one(); }

        void NotifyAll() override { _cv.notify_all(); }

    private:
        fiber::ConditionVariable _cv;
    };

    std::shared_ptr<rocksdb::TransactionDBMutex>
    TransactionDBBthreadFactory::AllocateMutex() {
        return std::shared_ptr<rocksdb::TransactionDBMutex>(new TransactionDBBthreadMutex());
    }

    std::shared_ptr<rocksdb::TransactionDBCondVar>
    TransactionDBBthreadFactory::AllocateCondVar() {
        return std::shared_ptr<rocksdb::TransactionDBCondVar>(new TransactionDBBthreadCond());
    }

    rocksdb::Status TransactionDBBthreadMutex::Lock() {
        _mutex.lock();
        return rocksdb::Status::OK();
    }

    rocksdb::Status TransactionDBBthreadMutex::TryLockFor(int64_t timeout_time) {
        bool locked = true;

        if (timeout_time == 0) {
            locked = _mutex.try_lock();
        } else {
            _mutex.lock();
        }

        if (!locked) {
            // timeout acquiring mutex
            return rocksdb::Status::TimedOut(rocksdb::Status::SubCode::kMutexTimeout);
        }

        return rocksdb::Status::OK();
    }

    rocksdb::Status TransactionDBBthreadCond::Wait(
            std::shared_ptr<rocksdb::TransactionDBMutex> mutex) {
        auto bthread_mutex = reinterpret_cast<TransactionDBBthreadMutex *>(mutex.get());

        std::unique_lock<fiber_mutex_t> lock(*(bthread_mutex->_mutex.native_handler()), std::adopt_lock);
        _cv.wait(lock);

        // Make sure unique_lock doesn't unlock mutex when it destructs
        lock.release();

        return rocksdb::Status::OK();
    }

    rocksdb::Status TransactionDBBthreadCond::WaitFor(
            std::shared_ptr<rocksdb::TransactionDBMutex> mutex, int64_t timeout_time) {
        rocksdb::Status s;

        auto bthread_mutex = reinterpret_cast<TransactionDBBthreadMutex *>(mutex.get());
        std::unique_lock<fiber_mutex_t> lock(*(bthread_mutex->_mutex.native_handler()), std::adopt_lock);

        if (timeout_time < 0) {
            // If timeout is negative, do not use a timeout
            _cv.wait(lock);
        } else {
            // auto duration = std::chrono::microseconds(timeout_time);
            auto cv_status = _cv.wait_for(lock, timeout_time);

            // Check if the wait stopped due to timing out.
            if (cv_status == ETIMEDOUT) {
                s = rocksdb::Status::TimedOut(rocksdb::Status::SubCode::kMutexTimeout);
            }
        }

        // Make sure unique_lock doesn't unlock mutex when it destructs
        lock.release();

        // CV was signaled, or we spuriously woke up (but didn't time out)
        return s;
    }

}  // namespace sirius