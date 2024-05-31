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


#include <sirius/storage/rocks_storage.h>
#include <rocksdb/table.h>
#include <rocksdb/filter_policy.h>
#include <rocksdb/statistics.h>
#include <iostream>
#include <sirius/storage/simple_listener.h>
#include <sirius/storage/transaction_db_bthread_mutex.h>
#include <turbo/strings/numbers.h>
#include <sirius/base/fiber.h>
#include <alkaid/ghc/filesystem.hpp>


namespace sirius {

    const std::string RocksStorage::RAFT_LOG_CF = "raft_log";
    const std::string RocksStorage::DATA_CF = "data";
    const std::string RocksStorage::META_INFO_CF = "meta_info";
    std::atomic<int64_t> RocksStorage::raft_cf_remove_range_count = {0};
    std::atomic<int64_t> RocksStorage::data_cf_remove_range_count = {0};
    std::atomic<int64_t> RocksStorage::mata_cf_remove_range_count = {0};

    RocksStorage::RocksStorage() : _is_init(false), _txn_db(nullptr),
                                   _raft_cf_remove_range_count("raft_cf_remove_range_count"),
                                   _data_cf_remove_range_count("data_cf_remove_range_count"),
                                   _mata_cf_remove_range_count("mata_cf_remove_range_count") {
    }

    int32_t RocksStorage::init(const std::string &path) {
        if (_is_init) {
            return 0;
        }
        ghc::filesystem::path dir_path = ghc::filesystem::path(path).parent_path();
        std::error_code ec;
        if(!ghc::filesystem::exists(dir_path, ec)) {
            if(ec) {
                return -1;
            }
            ghc::filesystem::create_directories(dir_path, ec);
            if(ec) {
                return -1;
            }
        }
        std::shared_ptr<rocksdb::EventListener> my_listener = std::make_shared<SimpleListener>();
        rocksdb::BlockBasedTableOptions table_options;
        if (FLAGS_rocks_use_partitioned_index_filters) {
            // use Partitioned Index Filters
            // https://github.com/facebook/rocksdb/wiki/Partitioned-Index-Filters
            table_options.index_type = rocksdb::BlockBasedTableOptions::kTwoLevelIndexSearch;
            table_options.partition_filters = true;
            table_options.metadata_block_size = 4096;
            table_options.cache_index_and_filter_blocks = true;
            table_options.pin_top_level_index_and_filter = true;
            table_options.cache_index_and_filter_blocks_with_high_priority = true;
            table_options.pin_l0_filter_and_index_blocks_in_cache = true;
            table_options.block_cache = rocksdb::NewLRUCache(FLAGS_rocks_block_cache_size_mb * 1024 * 1024LL,
                                                             8, false, FLAGS_rocks_high_pri_pool_ratio);
            // 通过cache控制内存，不需要控制max_open_files
            FLAGS_rocks_max_open_files = -1;
        } else {
            table_options.data_block_index_type = rocksdb::BlockBasedTableOptions::kDataBlockBinaryAndHash;
            if (FLAGS_rocks_use_hyper_clock_cache) {
#if ROCKSDB_MAJOR == 7
                auto cache_opt = rocksdb::HyperClockCacheOptions(FLAGS_rocks_block_cache_size_mb * 1024 * 1024LL, FLAGS_rocks_block_size, 8);
                cache_opt.metadata_charge_policy = rocksdb::kDontChargeCacheMetadata; //cache会比rocks_block_cache_size_mb多占用少量内存
                table_options.block_cache = cache_opt.MakeSharedCache();
#endif
            } else {
                table_options.block_cache = rocksdb::NewLRUCache(FLAGS_rocks_block_cache_size_mb * 1024 * 1024LL, 8);
            }
        }
        table_options.format_version = 4;

        table_options.block_size = FLAGS_rocks_block_size;
        if (FLAGS_rocks_use_ribbon_filter) {
            table_options.filter_policy.reset(rocksdb::NewRibbonFilterPolicy(9.9));
        } else {
            table_options.filter_policy.reset(rocksdb::NewBloomFilterPolicy(10));
        }
        _cache = table_options.block_cache.get();
        rocksdb::Options db_options;
        db_options.IncreaseParallelism(FLAGS_max_background_jobs);
        db_options.create_if_missing = true;
        db_options.use_direct_reads = FLAGS_use_direct_reads;
        db_options.use_direct_io_for_flush_and_compaction = FLAGS_use_direct_io_for_flush_and_compaction;
        db_options.max_open_files = FLAGS_rocks_max_open_files;
        db_options.skip_stats_update_on_db_open = FLAGS_rocks_skip_stats_update_on_db_open;
        db_options.compaction_readahead_size = FLAGS_rocks_compaction_readahead_size;
        db_options.WAL_ttl_seconds = 10 * 60;
        db_options.WAL_size_limit_MB = 0;
        //打开后有些集群内存严重上涨
        //db_options.avoid_unnecessary_blocking_io = true;
        db_options.max_background_compactions = FLAGS_rocks_max_background_compactions;
        if (FLAGS_rocks_kSkipAnyCorruptedRecords) {
            db_options.wal_recovery_mode = rocksdb::WALRecoveryMode::kSkipAnyCorruptedRecords;
        }
        db_options.statistics = rocksdb::CreateDBStatistics();
        db_options.max_subcompactions = FLAGS_rocks_max_subcompactions;
        db_options.max_background_flushes = 2;
        db_options.env->SetBackgroundThreads(2, rocksdb::Env::HIGH);
        db_options.listeners.emplace_back(my_listener);
        rocksdb::TransactionDBOptions txn_db_options;
        LOG(INFO) << "FLAGS_rocks_transaction_lock_timeout_ms:" << FLAGS_rocks_transaction_lock_timeout_ms
                     << " FLAGS_rocks_default_lock_timeout_ms:" << FLAGS_rocks_default_lock_timeout_ms;
        txn_db_options.transaction_lock_timeout = FLAGS_rocks_transaction_lock_timeout_ms;
        txn_db_options.default_lock_timeout = FLAGS_rocks_default_lock_timeout_ms;
        txn_db_options.custom_mutex_factory = std::shared_ptr<rocksdb::TransactionDBMutexFactory>(
                new TransactionDBBthreadFactory());

        //todo
        _log_cf_option.prefix_extractor.reset(
                rocksdb::NewFixedPrefixTransform(sizeof(int64_t) + 1));
        _log_cf_option.OptimizeLevelStyleCompaction();
        _log_cf_option.compaction_pri = rocksdb::kOldestLargestSeqFirst;
        _log_cf_option.table_factory.reset(rocksdb::NewBlockBasedTableFactory(table_options));
        _log_cf_option.compaction_style = rocksdb::kCompactionStyleLevel;
        _log_cf_option.level0_file_num_compaction_trigger = 5;
        _log_cf_option.level0_slowdown_writes_trigger = FLAGS_slowdown_write_sst_cnt;
        _log_cf_option.level0_stop_writes_trigger = FLAGS_stop_write_sst_cnt;
        _log_cf_option.target_file_size_base = FLAGS_target_file_size_base;
        _log_cf_option.max_bytes_for_level_base = 1024 * 1024 * 1024;
        _log_cf_option.level_compaction_dynamic_level_bytes = FLAGS_rocks_data_dynamic_level_bytes;

        _log_cf_option.max_write_buffer_number = FLAGS_max_write_buffer_number;
        _log_cf_option.max_write_buffer_number_to_maintain = _log_cf_option.max_write_buffer_number;
        _log_cf_option.write_buffer_size = FLAGS_write_buffer_size;
        _log_cf_option.min_write_buffer_number_to_merge = FLAGS_min_write_buffer_number_to_merge;

        //todo
        // prefix length: regionid(8 Bytes) tableid(8 Bytes)
        _data_cf_option.prefix_extractor.reset(
                rocksdb::NewFixedPrefixTransform(sizeof(int64_t) * 2));
        _data_cf_option.memtable_prefix_bloom_size_ratio = 0.1;
        _data_cf_option.memtable_whole_key_filtering = true;
        _data_cf_option.OptimizeLevelStyleCompaction();
        _data_cf_option.compaction_pri = static_cast<rocksdb::CompactionPri>(FLAGS_rocks_data_compaction_pri);
        if (FLAGS_rocks_use_sst_partitioner_fixed_prefix) {
            // 按region_id拆分
            _data_cf_option.sst_partitioner_factory = rocksdb::NewSstPartitionerFixedPrefixFactory(sizeof(int64_t));
        }
        _data_cf_option.table_factory.reset(rocksdb::NewBlockBasedTableFactory(table_options));
        _data_cf_option.compaction_style = rocksdb::kCompactionStyleLevel;
        _data_cf_option.optimize_filters_for_hits = FLAGS_rocks_optimize_filters_for_hits;
        _data_cf_option.level0_file_num_compaction_trigger = FLAGS_level0_file_num_compaction_trigger;
        _data_cf_option.level0_slowdown_writes_trigger = FLAGS_slowdown_write_sst_cnt;
        _data_cf_option.level0_stop_writes_trigger = FLAGS_stop_write_sst_cnt;
        _data_cf_option.hard_pending_compaction_bytes_limit = FLAGS_rocks_hard_pending_compaction_g * 1073741824ull;
        _data_cf_option.soft_pending_compaction_bytes_limit = FLAGS_rocks_soft_pending_compaction_g * 1073741824ull;
        _data_cf_option.target_file_size_base = FLAGS_target_file_size_base;
        _data_cf_option.max_bytes_for_level_multiplier = FLAGS_rocks_level_multiplier;
        _data_cf_option.level_compaction_dynamic_level_bytes = FLAGS_rocks_data_dynamic_level_bytes;

        _data_cf_option.max_write_buffer_number = FLAGS_max_write_buffer_number;
        _data_cf_option.max_write_buffer_number_to_maintain = _data_cf_option.max_write_buffer_number;
        _data_cf_option.write_buffer_size = FLAGS_write_buffer_size;
        _data_cf_option.min_write_buffer_number_to_merge = FLAGS_min_write_buffer_number_to_merge;

        _data_cf_option.max_bytes_for_level_base = FLAGS_max_bytes_for_level_base;
        if (FLAGS_l0_compaction_use_lz4) {
            _data_cf_option.compression_per_level = {rocksdb::CompressionType::kNoCompression,
                                                     rocksdb::CompressionType::kLZ4Compression,
                                                     rocksdb::CompressionType::kLZ4Compression,
                                                     rocksdb::CompressionType::kLZ4Compression,
                                                     rocksdb::CompressionType::kLZ4Compression,
                                                     rocksdb::CompressionType::kLZ4Compression,
                                                     rocksdb::CompressionType::kLZ4Compression};
        }

        if (FLAGS_enable_bottommost_compression) {
            _data_cf_option.bottommost_compression_opts.enabled = true;
            _data_cf_option.bottommost_compression = rocksdb::kZSTD;
            _data_cf_option.bottommost_compression_opts.max_dict_bytes = 1 << 14; // 16KB
            _data_cf_option.bottommost_compression_opts.zstd_max_train_bytes = 1 << 18; // 256KB
        }

        _meta_info_option.prefix_extractor.reset(
                rocksdb::NewFixedPrefixTransform(1));
        _meta_info_option.OptimizeLevelStyleCompaction();
        _meta_info_option.compaction_pri = rocksdb::kOldestSmallestSeqFirst;
        _meta_info_option.level_compaction_dynamic_level_bytes = FLAGS_rocks_data_dynamic_level_bytes;
        _meta_info_option.max_write_buffer_number_to_maintain = _meta_info_option.max_write_buffer_number;


        _db_path = path;
        // List Column Family
        std::vector<std::string> column_family_names;
        rocksdb::Status s;
        s = rocksdb::DB::ListColumnFamilies(db_options, path, &column_family_names);
        //db已存在
        if (s.ok()) {
            std::vector<rocksdb::ColumnFamilyDescriptor> column_family_desc;
            std::vector<rocksdb::ColumnFamilyHandle *> handles;
            for (auto &column_family_name: column_family_names) {
                if (column_family_name == RAFT_LOG_CF) {
                    column_family_desc.push_back(rocksdb::ColumnFamilyDescriptor(RAFT_LOG_CF, _log_cf_option));
                } else if (column_family_name == DATA_CF) {
                    column_family_desc.push_back(rocksdb::ColumnFamilyDescriptor(DATA_CF, _data_cf_option));
                } else if (column_family_name == META_INFO_CF) {
                    column_family_desc.push_back(rocksdb::ColumnFamilyDescriptor(META_INFO_CF, _meta_info_option));
                } else {
                    column_family_desc.push_back(
                            rocksdb::ColumnFamilyDescriptor(column_family_name,
                                                            rocksdb::ColumnFamilyOptions()));
                }
            }
            s = rocksdb::TransactionDB::Open(db_options,
                                             txn_db_options,
                                             path,
                                             column_family_desc,
                                             &handles,
                                             &_txn_db);
            if (s.ok()) {
                LOG(INFO)<< "reopen db:" << path << " success";
                for (auto &handle: handles) {
                    _column_families[handle->GetName()] = handle;
                    LOG(INFO) << "open column family:" << handle->GetName();
                }
            } else {
                LOG(ERROR)<< "reopen db:" << path << " fail, err_message:" << s.ToString();
                return -1;
            }
        } else {
            // new db
            s = rocksdb::TransactionDB::Open(db_options, txn_db_options, path, &_txn_db);
            if (s.ok()) {
                LOG(INFO) << "open db:" << path << " success";
            } else {
                LOG(ERROR) << "open db:" << path << " fail, err_message:" << s.ToString();
                return -1;
            }
        }

        if (0 == _column_families.count(RAFT_LOG_CF)) {
            //create raft_log column_familiy
            rocksdb::ColumnFamilyHandle *raft_log_handle;
            s = _txn_db->CreateColumnFamily(_log_cf_option, RAFT_LOG_CF, &raft_log_handle);
            if (s.ok()) {
                LOG(INFO) << "create column family success, column family: " << RAFT_LOG_CF;
                _column_families[RAFT_LOG_CF] = raft_log_handle;
            } else {
                LOG(ERROR) << "create column family fail, column family: " << RAFT_LOG_CF << " err_message: " << s.ToString();
                return -1;
            }
        }
        if (0 == _column_families.count(DATA_CF)) {
            //create data column_family
            rocksdb::ColumnFamilyHandle *data_handle;
            s = _txn_db->CreateColumnFamily(_data_cf_option, DATA_CF, &data_handle);
            if (s.ok()) {
                LOG(INFO) << "create column family success, column family: " << DATA_CF;
                _column_families[DATA_CF] = data_handle;
            } else {
                LOG(ERROR) << "create column family fail, column family: " << DATA_CF << " err_message: " << s.ToString();
                return -1;
            }
        }
        if (0 == _column_families.count(META_INFO_CF)) {
            rocksdb::ColumnFamilyHandle *metainfo_handle;
            s = _txn_db->CreateColumnFamily(_meta_info_option, META_INFO_CF, &metainfo_handle);
            if (s.ok()) {
                LOG(INFO) << "create column family success, column family: " << META_INFO_CF;
                _column_families[META_INFO_CF] = metainfo_handle;
            } else {
                LOG(ERROR) << "create column family fail, column family: " << META_INFO_CF << " err_message: " << s.ToString();
                return -1;
            }
        }
        _is_init = true;
        collect_rocks_options();
        LOG(INFO) << "rocksdb init success";
        return 0;
    }

    void RocksStorage::collect_rocks_options() {
        // gflag -> option_name
        _rocks_options["level0_file_num_compaction_trigger"] = "level0_file_num_compaction_trigger";
        _rocks_options["slowdown_write_sst_cnt"] = "level0_slowdown_writes_trigger";
        _rocks_options["stop_write_sst_cnt"] = "level0_stop_writes_trigger";
        _rocks_options["rocks_hard_pending_compaction_g"] = "hard_pending_compaction_bytes_limit"; // * 1073741824ull;
        _rocks_options["rocks_soft_pending_compaction_g"] = "soft_pending_compaction_bytes_limit"; // * 1073741824ull;
        _rocks_options["target_file_size_base"] = "target_file_size_base";
        _rocks_options["rocks_level_multiplier"] = "max_bytes_for_level_multiplier";
        _rocks_options["max_write_buffer_number"] = "max_write_buffer_number";
        _rocks_options["write_buffer_size"] = "write_buffer_size";
        _rocks_options["max_bytes_for_level_base"] = "max_bytes_for_level_base";
        _rocks_options["rocks_max_background_compactions"] = "max_background_compactions";
        _rocks_options["rocks_max_subcompactions"] = "max_subcompactions";
        _rocks_options["max_background_jobs"] = "max_background_jobs";
    }

    rocksdb::Status RocksStorage::remove_range(const rocksdb::WriteOptions &options,
                                               rocksdb::ColumnFamilyHandle *column_family,
                                               const rocksdb::Slice &begin,
                                               const rocksdb::Slice &end,
                                               bool delete_files_in_range) {
        auto raft_cf = get_raft_log_handle();
        auto data_cf = get_data_handle();
        auto mata_cf = get_meta_info_handle();
        if (raft_cf != nullptr && column_family->GetID() == raft_cf->GetID()) {
            _raft_cf_remove_range_count << 1;
            raft_cf_remove_range_count++;
        } else if (data_cf != nullptr && column_family->GetID() == data_cf->GetID()) {
            _data_cf_remove_range_count << 1;
            data_cf_remove_range_count++;
        } else if (mata_cf != nullptr && column_family->GetID() == mata_cf->GetID()) {
            _mata_cf_remove_range_count << 1;
            mata_cf_remove_range_count++;
        }

        if (delete_files_in_range && FLAGS_delete_files_in_range) {
            auto s = rocksdb::DeleteFilesInRange(_txn_db, column_family, &begin, &end, false);
            if (!s.ok()) {
                return s;
            }
        }
        rocksdb::TransactionDBWriteOptimizations opt;
        opt.skip_concurrency_control = true;
        opt.skip_duplicate_key_check = true;
        rocksdb::WriteBatch batch;
        batch.DeleteRange(column_family, begin, end);
        return _txn_db->Write(options, opt, &batch);
    }

    int32_t RocksStorage::delete_column_family(std::string cf_name) {
        if (_column_families.count(cf_name) == 0) {
            LOG(ERROR) << "column_family: " << cf_name << " not exist";
            return -1;
        }
        rocksdb::ColumnFamilyHandle *cf_handler = _column_families[cf_name];
        auto res = _txn_db->DropColumnFamily(cf_handler);
        if (!res.ok()) {
            LOG(ERROR) << "drop column_family: " << cf_name << " fail, err_message: " << res.ToString();
            return -1;
        }
        res = _txn_db->DestroyColumnFamilyHandle(cf_handler);
        if (!res.ok()) {
            LOG(ERROR) << "destroy column_family: " << cf_name << " fail, err_message: " << res.ToString();
            return -1;
        }
        _column_families.erase(cf_name);
        return 0;
    }

    int32_t RocksStorage::create_column_family(std::string cf_name) {
        if (_column_families.count(cf_name) != 0) {
            LOG(ERROR) << "column_family: " << cf_name << " already exist";
            return -1;
        }
        rocksdb::ColumnFamilyHandle *cf_handler = nullptr;
        auto s = _txn_db->CreateColumnFamily(_data_cf_option, cf_name, &cf_handler);
        if (s.ok()) {
            LOG(WARNING) << "create column family: " << cf_name << " success";
            _column_families[cf_name] = cf_handler;
        } else {
            LOG(ERROR) << "create column family: " << cf_name << " fail, err_message: " << s.ToString();
            return -1;
        }
        _column_families[cf_name] = cf_handler;
        return 0;
    }

    rocksdb::ColumnFamilyHandle *RocksStorage::get_raft_log_handle() {
        if (!_is_init) {
            LOG(ERROR) << "rocksdb has not been inited";
            return nullptr;
        }
        if (0 == _column_families.count(RAFT_LOG_CF)) {
            LOG(ERROR) << "rocksdb has no raft log cf";
            return nullptr;
        }
        return _column_families[RAFT_LOG_CF];
    }

    rocksdb::ColumnFamilyHandle *RocksStorage::get_data_handle() {
        if (!_is_init) {
            LOG(ERROR) << "rocksdb has not been inited";
            return nullptr;
        }
        if (0 == _column_families.count(DATA_CF)) {
            LOG(ERROR) << "rocksdb has no data column family";
            return nullptr;
        }
        return _column_families[DATA_CF];
    }

    rocksdb::ColumnFamilyHandle *RocksStorage::get_meta_info_handle() {
        if (!_is_init) {
            LOG(ERROR) << "rocksdb has not been inited";
            return nullptr;
        }
        if (0 == _column_families.count(META_INFO_CF)) {
            LOG(ERROR) << "rocksdb has no meta info column family";
            return nullptr;
        }
        return _column_families[META_INFO_CF];
    }

}
