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

#include <gflags/gflags_declare.h>

namespace sirius {

    /// for store engine
    DECLARE_int64(flush_memtable_interval_us);
    DECLARE_bool(cstore_scan_fill_cache);
    DECLARE_int32(rocks_transaction_lock_timeout_ms);
    DECLARE_int32(rocks_default_lock_timeout_ms);
    DECLARE_bool(rocks_use_partitioned_index_filters);
    DECLARE_bool(rocks_skip_stats_update_on_db_open);
    DECLARE_int32(rocks_block_size);
    DECLARE_int64(rocks_block_cache_size_mb);
    DECLARE_uint64(rocks_hard_pending_compaction_g);
    DECLARE_uint64(rocks_soft_pending_compaction_g);
    DECLARE_uint64(rocks_compaction_readahead_size);
    DECLARE_int32(rocks_data_compaction_pri);
    DECLARE_double(rocks_level_multiplier);
    DECLARE_double(rocks_high_pri_pool_ratio);
    DECLARE_int32(rocks_max_open_files);
    DECLARE_int32(rocks_max_subcompactions);
    DECLARE_int32(rocks_max_background_compactions);
    DECLARE_bool(rocks_optimize_filters_for_hits);
    DECLARE_int32(slowdown_write_sst_cnt);
    DECLARE_int32(stop_write_sst_cnt);
    DECLARE_bool(rocks_use_ribbon_filter);
    DECLARE_bool(rocks_use_hyper_clock_cache);
    DECLARE_bool(rocks_use_sst_partitioner_fixed_prefix);
    DECLARE_bool(rocks_kSkipAnyCorruptedRecords);
    DECLARE_bool(rocks_data_dynamic_level_bytes);
    DECLARE_int32(max_background_jobs);
    DECLARE_int32(max_write_buffer_number);
    DECLARE_int32(write_buffer_size);
    DECLARE_int32(min_write_buffer_number_to_merge);
    DECLARE_int32(rocks_binlog_max_files_size_gb);
    DECLARE_int32(rocks_binlog_ttl_days);
    DECLARE_int32(level0_file_num_compaction_trigger);
    DECLARE_int32(max_bytes_for_level_base);
    DECLARE_bool(enable_bottommost_compression);
    DECLARE_int32(target_file_size_base);
    DECLARE_int32(addpeer_rate_limit_level);
    DECLARE_bool(delete_files_in_range);
    DECLARE_bool(l0_compaction_use_lz4);
    DECLARE_bool(real_delete_old_binlog_cf);
    DECLARE_bool(rocksdb_fifo_allow_compaction);
    DECLARE_bool(use_direct_io_for_flush_and_compaction);
    DECLARE_bool(use_direct_reads);
    DECLARE_int32(level0_max_sst_num);
    DECLARE_int32(rocksdb_cost_sample);
    DECLARE_int64(qps_statistics_minutes_ago);
    DECLARE_int64(max_tokens_per_second);
    DECLARE_int64(use_token_bucket);
    DECLARE_int64(get_token_weight);
    DECLARE_int64(min_global_extended_percent);
    DECLARE_int64(token_bucket_adjust_interval_s);
    DECLARE_int64(token_bucket_burst_window_ms);
    DECLARE_int64(dml_use_token_bucket);
    DECLARE_int64(sql_token_bucket_timeout_min);
    DECLARE_int64(qos_reject_interval_s);
    DECLARE_int64(qos_reject_ratio);
    DECLARE_int64(qos_reject_timeout_s);
    DECLARE_int64(qos_reject_max_scan_ratio);
    DECLARE_int64(qos_reject_growth_multiple);
    DECLARE_int64(qos_need_reject);
    DECLARE_int64(sign_concurrency);
    DECLARE_bool(disable_wal);
    DECLARE_int64(exec_1pc_out_fsm_timeout_ms);
    DECLARE_int64(exec_1pc_in_fsm_timeout_ms);
    DECLARE_int64(retry_interval_us);
    DECLARE_int32(transaction_clear_delay_ms);
    DECLARE_int32(long_live_txn_interval_ms);
    DECLARE_int64(clean_finished_txn_interval_us);
    DECLARE_int64(one_pc_out_fsm_interval_us);
    // split slow down max time：5s
    DECLARE_int32(transaction_query_primary_region_interval_ms);

}  // namespace sirius
