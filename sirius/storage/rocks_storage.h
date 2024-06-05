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

#include <string>
#include <mizar/db.h>
#include <mizar/convenience.h>
#include <mizar/slice.h>
#include <mizar/cache.h>
#include <mizar/listener.h>
#include <mizar/options.h>
#include <mizar/status.h>
#include <mizar/slice_transform.h>
#include <mizar/utilities/transaction.h>
#include <mizar/utilities/transaction_db.h>
#include <collie/strings/format.h>
#include <sirius/flags/engine.h>
#include <melon/fiber/fiber.h>
#include <melon/var/var.h>

namespace sirius {

    class RocksStorage {
    public:
        static const std::string RAFT_LOG_CF;
        static const std::string DATA_CF;
        static const std::string META_INFO_CF;
        static std::atomic<int64_t> raft_cf_remove_range_count;
        static std::atomic<int64_t> data_cf_remove_range_count;
        static std::atomic<int64_t> mata_cf_remove_range_count;

        virtual ~RocksStorage() = default;

        static RocksStorage *get_instance() {
            static RocksStorage _instance;
            return &_instance;
        }

        int32_t init(const std::string &path);

        mizar::Status write(const mizar::WriteOptions &options, mizar::WriteBatch *updates) {
            return _txn_db->Write(options, updates);
        }

        mizar::Status write(const mizar::WriteOptions &options,
                              mizar::ColumnFamilyHandle *column_family,
                              const std::vector<std::string> &keys,
                              const std::vector<std::string> &values) {
            mizar::WriteBatch batch;
            for (size_t i = 0; i < keys.size(); ++i) {
                batch.Put(column_family, keys[i], values[i]);
            }
            return _txn_db->Write(options, &batch);
        }

        mizar::Status get(const mizar::ReadOptions &options,
                            mizar::ColumnFamilyHandle *column_family,
                            const mizar::Slice &key,
                            std::string *value) {
            return _txn_db->Get(options, column_family, key, value);
        }

        mizar::Status put(const mizar::WriteOptions &options,
                            mizar::ColumnFamilyHandle *column_family,
                            const mizar::Slice &key,
                            const mizar::Slice &value) {
            return _txn_db->Put(options, column_family, key, value);
        }

        mizar::Transaction *begin_transaction(
                const mizar::WriteOptions &write_options,
                const mizar::TransactionOptions &txn_options) {
            return _txn_db->BeginTransaction(write_options, txn_options);
        }

        mizar::Status compact_range(const mizar::CompactRangeOptions &options,
                                      mizar::ColumnFamilyHandle *column_family,
                                      const mizar::Slice *begin,
                                      const mizar::Slice *end) {
            return _txn_db->CompactRange(options, column_family, begin, end);
        }

        mizar::Status flush(const mizar::FlushOptions &options,
                              mizar::ColumnFamilyHandle *column_family) {
            return _txn_db->Flush(options, column_family);
        }

        mizar::Status remove(const mizar::WriteOptions &options,
                               mizar::ColumnFamilyHandle *column_family,
                               const mizar::Slice &key) {
            return _txn_db->Delete(options, column_family, key);
        }

        // Consider setting ReadOptions::ignore_range_deletions = true to speed
        // up reads for key(s) that are known to be unaffected by range deletions.
        mizar::Status remove_range(const mizar::WriteOptions &options,
                                     mizar::ColumnFamilyHandle *column_family,
                                     const mizar::Slice &begin,
                                     const mizar::Slice &end,
                                     bool delete_files_in_range);

        mizar::Iterator *new_iterator(const mizar::ReadOptions &options,
                                        mizar::ColumnFamilyHandle *family) {
            return _txn_db->NewIterator(options, family);
        }

        mizar::Iterator *new_iterator(const mizar::ReadOptions &options, const std::string cf) {
            if (_column_families.count(cf) == 0) {
                return nullptr;
            }
            return _txn_db->NewIterator(options, _column_families[cf]);
        }

        mizar::Status ingest_external_file(mizar::ColumnFamilyHandle *family,
                                             const std::vector<std::string> &external_files,
                                             const mizar::IngestExternalFileOptions &options) {
            return _txn_db->IngestExternalFile(family, external_files, options);
        }

        mizar::ColumnFamilyHandle *get_raft_log_handle();

        mizar::ColumnFamilyHandle *get_data_handle();

        mizar::ColumnFamilyHandle *get_meta_info_handle();

        mizar::TransactionDB *get_db() {
            return _txn_db;
        }

        int32_t create_column_family(std::string cf_name);

        int32_t delete_column_family(std::string cf_name);

        mizar::Options get_options(mizar::ColumnFamilyHandle *family) {
            return _txn_db->GetOptions(family);
        }

        mizar::DBOptions get_db_options() {
            return _txn_db->GetDBOptions();
        }

        mizar::Cache *get_cache() {
            return _cache;
        }

        const mizar::Snapshot *get_snapshot() {
            return _txn_db->GetSnapshot();
        }

        void release_snapshot(const mizar::Snapshot *snapshot) {
            _txn_db->ReleaseSnapshot(snapshot);
        }

        void close() {
            delete _txn_db;
        }

        bool is_any_stall() {
            uint64_t value = 0;
            if (FLAGS_addpeer_rate_limit_level >= 2) {
                _txn_db->GetIntProperty(get_data_handle(), "rocksdb.num-running-compactions", &value);
                if (value >= (uint64_t) FLAGS_rocks_max_background_compactions) {
                    return true;
                }
                _txn_db->GetAggregatedIntProperty("rocksdb.estimate-pending-compaction-bytes", &value);
                uint64_t level0_sst = 0;
                uint64_t pending_compaction_size = 0;
                get_rocks_statistic(level0_sst, pending_compaction_size);
                if (level0_sst > _data_cf_option.level0_slowdown_writes_trigger * 0.6) {
                    return true;
                }
                if (pending_compaction_size > _data_cf_option.soft_pending_compaction_bytes_limit / 2) {
                    return true;
                }
            }
            if (FLAGS_addpeer_rate_limit_level >= 1) {
                _txn_db->GetAggregatedIntProperty("rocksdb.actual-delayed-write-rate", &value);
                if (value > 0) {
                    return true;
                }
                _txn_db->GetAggregatedIntProperty("rocksdb.is-write-stopped", &value);
                if (value > 0) {
                    return true;
                }
            }
            return false;
        }

        bool is_ingest_stall() {
            uint64_t level0_sst = 0;
            uint64_t pending_compaction_size = 0;
            get_rocks_statistic(level0_sst, pending_compaction_size);
            if (level0_sst > FLAGS_level0_max_sst_num) {
                return true;
            }
            return false;
        }

        void set_flush_file_number(const std::string &cf_name, uint64_t file_number) {
            if (cf_name == DATA_CF) {
                _flush_file_number = file_number;
            }
        }

        void collect_rocks_options();

        int get_rocks_statistic(uint64_t &level0_sst, uint64_t &pending_compaction_size) {
            mizar::ColumnFamilyMetaData cf_meta;
            _txn_db->GetColumnFamilyMetaData(get_data_handle(), &cf_meta);
            if (cf_meta.levels.size() == 0) {
                return -1;
            }
            level0_sst = cf_meta.levels[0].files.size();
            _txn_db->GetIntProperty(get_data_handle(), "rocksdb.estimate-pending-compaction-bytes",
                                    &pending_compaction_size);
            return 0;
        }

    private:

        RocksStorage();

        std::string _db_path;

        bool _is_init;

        mizar::TransactionDB *_txn_db;
        mizar::Cache *_cache;

        std::map<std::string, mizar::ColumnFamilyHandle *> _column_families;

        mizar::ColumnFamilyOptions _log_cf_option;
        mizar::ColumnFamilyOptions _data_cf_option;
        mizar::ColumnFamilyOptions _meta_info_option;
        uint64_t _flush_file_number = 0;
        melon::var::Adder<int64_t> _raft_cf_remove_range_count;
        melon::var::Adder<int64_t> _data_cf_remove_range_count;
        melon::var::Adder<int64_t> _mata_cf_remove_range_count;

        std::atomic<int32_t> _split_num;
        fiber::Mutex _options_mutex;
        std::unordered_map<std::string, std::string> _rocks_options;
        std::map<std::string, std::string> _defined_options;
    };
}  // namespace sirius

