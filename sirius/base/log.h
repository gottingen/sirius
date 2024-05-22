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

#include <initializer_list>
#include <sirius/flags/log.h>
#include <collie/log/sinks/rotating_file_sink.h>
#include <collie/log/sinks/daily_file_sink.h>
#include <collie/log/sinks/stdout_color_sinks.h>
#include <collie/log/logging.h>
#include <collie/filesystem/fs.h>
#include <sirius/base/proto_helper.h>

namespace sirius {

    [[nodiscard]] bool initialize_log();

    collie::log::logger* get_logger();
}  // namespace sirius

#define SS_LOG(SEVERITY) LOG_LOGGER(SEVERITY, sirius::get_logger())
#define SS_LOG_IF(SEVERITY, condition) LOG_LOGGER_IF(SEVERITY, (condition), sirius::get_logger())
#define SS_LOG_EVERY_N(SEVERITY, N) LOG_LOGGER_EVERY_N(SEVERITY, N, sirius::get_logger())
#define SS_LOG_IF_EVERY_N(SEVERITY, N, condition) LOG_LOGGER_IF_EVERY_N(SEVERITY, (N), (condition), sirius::get_logger())
#define SS_LOG_FIRST_N(SEVERITY, N) LOG_LOGGER_FIRST_N(SEVERITY, (N), sirius::get_logger())
#define SS_LOG_IF_FIRST_N(SEVERITY, N, condition) LOG_LOGGER_IF_FIRST_N(SEVERITY, (N), (condition), sirius::get_logger())
#define SS_LOG_EVERY_T(SEVERITY, seconds) LOG_LOGGER_EVERY_T(SEVERITY, (seconds), sirius::get_logger())
#define SS_LOG_IF_EVERY_T(SEVERITY, seconds, condition) LOG_LOGGER_IF_EVERY_T(SEVERITY, (seconds), (condition), sirius::get_logger())
#define SS_LOG_ONCE(SEVERITY) LOG_LOGGER_ONCE(SEVERITY, sirius::get_logger())
#define SS_LOG_IF_ONCE(SEVERITY, condition) LOG_LOGGER_IF_ONCE(SEVERITY, (condition), sirius::get_logger())

#define SS_CHECK(condition) CHECK_LOGGER(condition, sirius::get_logger())
#define SS_CHECK_NOTNULL(val) CHECK_LOGGER(collie::ptr(val) != nullptr,sirius::get_logger())
#define SS_CHECK_EQ(val1, val2) CHECK_OP_LOGGER(_EQ, ==, val1, val2, sirius::get_logger())
#define SS_CHECK_NE(val1, val2) CHECK_OP_LOGGER(_NE, !=, val1, val2, sirius::get_logger())
#define SS_CHECK_LE(val1, val2) CHECK_OP_LOGGER(_LE, <=, val1, val2, sirius::get_logger())
#define SS_CHECK_LT(val1, val2) CHECK_OP_LOGGER(_LT, <, val1, val2, sirius::get_logger())
#define SS_CHECK_GE(val1, val2) CHECK_OP_LOGGER(_GE, >=, val1, val2, sirius::get_logger())
#define SS_CHECK_GT(val1, val2) CHECK_OP_LOGGER(_GT, >, val1, val2, sirius::get_logger())
#define SS_CHECK_DOUBLE_EQ(val1, val2) CHECK_DOUBLE_EQ_LOGGER(val1, val2, sirius::get_logger())
#define SS_CHECK_NEAR(val1, val2, margin) CHECK_NEAR_LOGGER(val1, val2, margin, sirius::get_logger())
#define SS_CHECK_INDEX(I, A) CHECK_LOGGER(I < (sizeof(A) / sizeof(A[0])), sirius::get_logger())
#define SS_CHECK_BOUND(B, A) CHECK_LOGGER(B <= (sizeof(A) / sizeof(A[0])), sirius::get_logger())

#define SS_PLOG(SEVERITY) PLOG_LOGGER(SEVERITY, sirius::get_logger())
#define SS_PLOG_IF(SEVERITY, condition) PLOG_LOGGER_IF(SEVERITY, (condition), sirius::get_logger())
#define SS_PLOG_EVERY_N(SEVERITY, N) PLOG_LOGGER_EVERY_N(SEVERITY, N, sirius::get_logger())
#define SS_PLOG_IF_EVERY_N(SEVERITY, N, condition) PLOG_LOGGER_IF_EVERY_N(SEVERITY, (N), (condition), sirius::get_logger())
#define SS_PLOG_FIRST_N(SEVERITY, N) PLOG_LOGGER_FIRST_N(SEVERITY, (N), true, sirius::get_logger())
#define SS_PLOG_IF_FIRST_N(SEVERITY, N, condition) PLOG_LOGGER_IF_FIRST_N(SEVERITY, (N), (condition), sirius::get_logger())
#define SS_PLOG_EVERY_T(SEVERITY, seconds) PLOG_LOGGER_EVERY_T(SEVERITY, (seconds), sirius::get_logger())
#define SS_PLOG_IF_EVERY_T(SEVERITY, seconds, condition) PLOG_LOGGER_IF_EVERY_T(SEVERITY, (seconds), (condition), sirius::get_logger())
#define SS_PLOG_ONCE(SEVERITY) PLOG_LOGGER_ONCE(SEVERITY, sirius::get_logger())
#define SS_PLOG_IF_ONCE(SEVERITY, condition) PLOG_LOGGER_IF_ONCE(SEVERITY, (condition), sirius::get_logger())

#define SS_PCHECK(condition) PCHECK_LOGGER(condition, sirius::get_logger())
#define SS_PCHECK_PTREQ(val1, val2) PCHECK_PTREQ_LOGGER(val1, val2, sirius::get_logger())
#define SS_PCHECK_NOTNUL(val) PCHECK_NOTNULL_LOGGER(collie::ptr(val) != nullptr, log::default_logger_raw())
#define SS_PCHECK_EQ(val1, val2) PCHECK_EQ_LOGGER(val1, val2, sirius::get_logger())
#define SS_PCHECK_NE(val1, val2) PCHECK_NE_LOGGER(val1, val2, sirius::get_logger())
#define SS_PCHECK_LE(val1, val2) PCHECK_LE_LOGGER(val1, val2, sirius::get_logger())
#define SS_PCHECK_LT(val1, val2) PCHECK_LT_LOGGER(val1, val2, sirius::get_logger())
#define SS_PCHECK_GE(val1, val2) PCHECK_GE_LOGGER(val1, val2, sirius::get_logger())
#define SS_PCHECK_GT(val1, val2) PCHECK_GT_LOGGER(val1, val2, sirius::get_logger())

#define SS_VLOG(verboselevel) VLOG_LOGGER(verboselevel, sirius::get_logger())
#define SS_VLOG_IF(verboselevel, condition) VLOG_IF_LOGGER(verboselevel, (condition), sirius::get_logger())
#define SS_VLOG_EVERY_N(verboselevel, N) VLOG_EVERY_N_LOGGER(verboselevel, N, sirius::get_logger())
#define SS_VLOG_IF_EVERY_N(verboselevel, N, condition) VLOG_IF_EVERY_N_LOGGER(verboselevel, (N), (condition), sirius::get_logger())
#define SS_VLOG_FIRST_N(verboselevel, N) VLOG_FIRST_N_LOGGER(verboselevel, (N), sirius::get_logger())
#define SS_VLOG_IF_FIRST_N(verboselevel, N, condition) VLOG_IF_FIRST_N_LOGGER(verboselevel, (N), (condition), sirius::get_logger())
#define SS_VLOG_EVERY_T(verboselevel, seconds) VLOG_EVERY_T_LOGGER(verboselevel, (seconds), sirius::get_logger())
#define SS_VLOG_IF_EVERY_T(verboselevel, seconds, condition) VLOG_IF_EVERY_T_LOGGER(verboselevel, (seconds), (condition), sirius::get_logger())
#define SS_VLOG_ONCE(verboselevel) VLOG_ONCE_LOGGER(verboselevel, sirius::get_logger())
#define SS_VLOG_IF_ONCE(verboselevel, condition) VLOG_IF_ONCE_LOGGER(verboselevel, (condition), sirius::get_logger())


#if CLOG_DCHECK_IS_ON()

#define SS_DLOG(SEVERITY)                                LOG(SEVERITY)
#define SS_DLOG_IF(SEVERITY, condition)                  LOG_LOGGER_IF(SEVERITY, (condition), sirius::get_logger())
#define SS_DLOG_EVERY_N(SEVERITY, N)                     LOG_LOGGER_EVERY_N(SEVERITY, N, sirius::get_logger())
#define SS_DLOG_IF_EVERY_N(SEVERITY, N, condition)       LOG_LOGGER_IF_EVERY_N(SEVERITY, (N), (condition), sirius::get_logger())
#define SS_DLOG_FIRST_N(SEVERITY, N)                     LOG_LOGGER_FIRST_N(SEVERITY, (N), true, sirius::get_logger())
#define SS_DLOG_IF_FIRST_N(SEVERITY, N, condition)       LOG_LOGGER_IF_FIRST_N(SEVERITY, (N), (condition), sirius::get_logger())
#define SS_DLOG_EVERY_T(SEVERITY, seconds)               LOG_LOGGER_EVERY_T(SEVERITY, (seconds), sirius::get_logger())
#define SS_DLOG_IF_EVERY_T(SEVERITY, seconds, condition) LOG_LOGGER_IF_EVERY_T(SEVERITY, (seconds), (condition), sirius::get_logger())
#define SS_DLOG_ONCE(SEVERITY)                           LOG_LOGGER_ONCE(SEVERITY, sirius::get_logger())
#define SS_DLOG_IF_ONCE(SEVERITY, condition)             LOG_LOGGER_IF_ONCE(SEVERITY, (condition), sirius::get_logger())

#define SS_DCHECK(condition) DCHECK_LOGGER(condition, sirius::get_logger())
#define SS_DCHECK_NOTNULL(val) DCHECK_NOTNULL_LOGGER(val, sirius::get_logger())
#define SS_DCHECK_PTREQ(val1, val2) DCHECK_PTREQ_LOGGER(val1, val2, sirius::get_logger())
#define SS_DCHECK_EQ(val1, val2) CHECK_EQ_LOGGER(val1, val2, sirius::get_logger())
#define SS_DCHECK_NE(val1, val2) CHECK_NE_LOGGER(val1, val2, sirius::get_logger())
#define SS_DCHECK_LE(val1, val2) CHECK_LE_LOGGER(val1, val2, sirius::get_logger())
#define SS_DCHECK_LT(val1, val2) CHECK_LT_LOGGER(val1, val2, sirius::get_logger())
#define SS_DCHECK_GE(val1, val2) CHECK_GE_LOGGER(val1, val2, sirius::get_logger())
#define SS_DCHECK_GT(val1, val2) CHECK_GT_LOGGER(val1, val2, sirius::get_logger())

#define SS_DPLOG(SEVERITY)                                  PLOG_LOGGER(SEVERITY, sirius::get_logger())
#define SS_DPLOG_IF(SEVERITY, condition)                    PLOG_LOGGER_IF(SEVERITY, (condition), sirius::get_logger())
#define SS_DPLOG_EVERY_N(SEVERITY, N)                       PLOG_LOGGER_EVERY_N(SEVERITY, N, sirius::get_logger())
#define SS_DPLOG_IF_EVERY_N(SEVERITY, N, condition)         PLOG_LOGGER_IF_EVERY_N(SEVERITY, (N), (condition), sirius::get_logger())
#define SS_DPLOG_FIRST_N(SEVERITY, N)                       PLOG_LOGGER_FIRST_N(SEVERITY, (N), true, sirius::get_logger())
#define SS_DPLOG_IF_FIRST_N(SEVERITY, N, condition)         PLOG_LOGGER_IF_FIRST_N(SEVERITY, (N), (condition), sirius::get_logger())
#define SS_DPLOG_EVERY_T(SEVERITY, seconds)                 PLOG_LOGGER_EVERY_T(SEVERITY, (seconds), sirius::get_logger())
#define SS_DPLOG_IF_EVERY_T(SEVERITY, seconds, condition)   PLOG_LOGGER_IF_EVERY_T(SEVERITY, (seconds), (condition), sirius::get_logger())
#define SS_DPLOG_ONCE(SEVERITY)                             PLOG_LOGGER_ONCE(SEVERITY, sirius::get_logger())
#define SS_DPLOG_IF_ONCE(SEVERITY, condition)               PLOG_LOGGER_IF_ONCE(SEVERITY, (condition), sirius::get_logger())

#define SS_DPCHECK(condition) DPCHECK_LOGGER(condition, sirius::get_logger())
#define SS_DPCHECK_NOTNULL(val) DPCHECK_NOTNULL_LOGGER(val, sirius::get_logger())
#define SS_DPCHECK_PTREQ(val1, val2) DPCHECK_PTREQ_LOGGER(val1, val2, sirius::get_logger())
#define SS_DPCHECK_EQ(val1, val2) PCHECK_EQ_LOGGER(val1, val2, sirius::get_logger())
#define SS_DPCHECK_NE(val1, val2) PCHECK_NE_LOGGER(val1, val2, sirius::get_logger())
#define SS_DPCHECK_LE(val1, val2) PCHECK_LE_LOGGER(val1, val2, sirius::get_logger())
#define SS_DPCHECK_LT(val1, val2) PCHECK_LT_LOGGER(val1, val2, sirius::get_logger())
#define SS_DPCHECK_GE(val1, val2) PCHECK_GE_LOGGER(val1, val2, sirius::get_logger())
#define SS_DPCHECK_GT(val1, val2) PCHECK_GT_LOGGER(val1, val2, sirius::get_logger())

#define SS_DVLOG(verboselevel) DVLOG_LOGGER(verboselevel, sirius::get_logger())
#define SS_DVLOG_IF(verboselevel, condition) DVLOG_IF_LOGGER(verboselevel, (condition), sirius::get_logger())
#define SS_DVLOG_EVERY_N(verboselevel, N) DVLOG_EVERY_N_LOGGER(verboselevel, N, sirius::get_logger())
#define SS_DVLOG_IF_EVERY_N(verboselevel, N, condition) DVLOG_IF_EVERY_N_LOGGER(verboselevel, (N), (condition), sirius::get_logger())
#define SS_DVLOG_FIRST_N(verboselevel, N) DVLOG_FIRST_N_LOGGER(verboselevel, (N), sirius::get_logger())
#define SS_DVLOG_IF_FIRST_N(verboselevel, N, condition) DVLOG_IF_FIRST_N_LOGGER(verboselevel, (N), (condition), sirius::get_logger())
#define SS_DVLOG_EVERY_T(verboselevel, seconds) DVLOG_EVERY_T_LOGGER(verboselevel, (seconds), sirius::get_logger())
#define SS_DVLOG_IF_EVERY_T(verboselevel, seconds, condition) DVLOG_IF_EVERY_T_LOGGER(verboselevel, (seconds), (condition), sirius::get_logger())
#define SS_DVLOG_ONCE(verboselevel) DVLOG_ONCE_LOGGER(verboselevel, sirius::get_logger())
#define SS_DVLOG_IF_ONCE(verboselevel, condition) DVLOG_IF_ONCE_LOGGER(verboselevel, (condition), sirius::get_logger())

#else // NDEBUG

#define SS_DLOG(SEVERITY)                                DLOG_LOGGER(SEVERITY, sirius::get_logger())
#define SS_DLOG_IF(SEVERITY, condition)                  DLOG_LOGGER_IF(SEVERITY, (condition), sirius::get_logger())
#define SS_DLOG_EVERY_N(SEVERITY, N)                     DLOG_LOGGER_EVERY_N(SEVERITY, N, sirius::get_logger())
#define SS_DLOG_IF_EVERY_N(SEVERITY, N, condition)       DLOG_LOGGER_IF_EVERY_N(SEVERITY, (N), (condition), sirius::get_logger())
#define SS_DLOG_FIRST_N(SEVERITY, N)                     DLOG_LOGGER_FIRST_N(SEVERITY, (N), true, sirius::get_logger())
#define SS_DLOG_IF_FIRST_N(SEVERITY, N, condition)       DLOG_LOGGER_IF_FIRST_N(SEVERITY, (N), (condition), sirius::get_logger())
#define SS_DLOG_EVERY_T(SEVERITY, seconds)               DLOG_LOGGER_EVERY_T(SEVERITY, (seconds), sirius::get_logger())
#define SS_DLOG_IF_EVERY_T(SEVERITY, seconds, condition) DLOG_LOGGER_IF_EVERY_T(SEVERITY, (seconds), (condition), sirius::get_logger())
#define SS_DLOG_ONCE(SEVERITY)                           DLOG_LOGGER_ONCE(SEVERITY, sirius::get_logger())
#define SS_DLOG_IF_ONCE(SEVERITY, condition)             DLOG_LOGGER_IF_ONCE(SEVERITY, (condition), sirius::get_logger())

#define SS_DCHECK(condition) DCHECK_LOGGER(condition, sirius::get_logger())
#define SS_DCHECK_NOTNULL(val) DCHECK_NOTNULL_LOGGER(val, sirius::get_logger())
#define SS_DCHECK_PTREQ(val1, val2) DCHECK_PTREQ_LOGGER(val1, val2, sirius::get_logger())
#define SS_DCHECK_EQ(val1, val2) CHECK_EQ_LOGGER(val1, val2, sirius::get_logger())
#define SS_DCHECK_NE(val1, val2) CHECK_NE_LOGGER(val1, val2, sirius::get_logger())
#define SS_DCHECK_LE(val1, val2) CHECK_LE_LOGGER(val1, val2, sirius::get_logger())
#define SS_DCHECK_LT(val1, val2) CHECK_LT_LOGGER(val1, val2, sirius::get_logger())
#define SS_DCHECK_GE(val1, val2) CHECK_GE_LOGGER(val1, val2, sirius::get_logger())
#define SS_DCHECK_GT(val1, val2) CHECK_GT_LOGGER(val1, val2, sirius::get_logger())


#define SS_DPLOG(SEVERITY)                                  DPLOG_LOGGER(SEVERITY, sirius::get_logger())
#define SS_DPLOG_IF(SEVERITY, condition)                    DPLOG_LOGGER_IF(SEVERITY, (condition), sirius::get_logger())
#define SS_DPLOG_EVERY_N(SEVERITY, N)                       DPLOG_LOGGER_EVERY_N(SEVERITY, N, sirius::get_logger())
#define SS_DPLOG_IF_EVERY_N(SEVERITY, N, condition)         DPLOG_LOGGER_IF_EVERY_N(SEVERITY, (N), (condition), sirius::get_logger())
#define SS_DPLOG_FIRST_N(SEVERITY, N)                       DPLOG_LOGGER_FIRST_N(SEVERITY, (N), true, sirius::get_logger())
#define SS_DPLOG_IF_FIRST_N(SEVERITY, N, condition)         DPLOG_LOGGER_IF_FIRST_N(SEVERITY, (N), (condition), sirius::get_logger())
#define SS_DPLOG_EVERY_T(SEVERITY, seconds)                 DPLOG_LOGGER_EVERY_T(SEVERITY, (seconds), sirius::get_logger())
#define SS_DPLOG_IF_EVERY_T(SEVERITY, seconds, condition)   DPLOG_LOGGER_IF_EVERY_T(SEVERITY, (seconds), (condition), sirius::get_logger())
#define SS_DPLOG_ONCE(SEVERITY)                             DPLOG_LOGGER_ONCE(SEVERITY, sirius::get_logger())
#define SS_DPLOG_IF_ONCE(SEVERITY, condition)               DPLOG_LOGGER_IF_ONCE(SEVERITY, (condition), sirius::get_logger())

#define SS_DPCHECK(condition)        DPCHECK_LOGGER(condition, sirius::get_logger())
#define SS_DPCHECK_NOTNULL(val)      DPCHECK_NOTNULL_LOGGER(val, sirius::get_logger())
#define SS_DPCHECK_PTREQ(val1, val2) DPCHECK_PTREQ_LOGGER(val1, val2, sirius::get_logger())
#define SS_DPCHECK_EQ(val1, val2) DPCHECK_EQ_LOGGER(val1, val2, sirius::get_logger())
#define SS_DPCHECK_NE(val1, val2) DPCHECK_NE_LOGGER(val1, val2, sirius::get_logger())
#define SS_DPCHECK_LE(val1, val2) DPCHECK_LE_LOGGER(val1, val2, sirius::get_logger())
#define SS_DPCHECK_LT(val1, val2) DPCHECK_LT_LOGGER(val1, val2, sirius::get_logger())
#define SS_DPCHECK_GE(val1, val2) DPCHECK_GE_LOGGER(val1, val2, sirius::get_logger())
#define SS_DPCHECK_GT(val1, val2) DPCHECK_GT_LOGGER(val1, val2, sirius::get_logger())

#define SS_DVLOG(verboselevel) DVLOG_LOGGER(verboselevel, sirius::get_logger())
#define SS_DVLOG_IF(verboselevel, condition) DVLOG_IF_LOGGER(verboselevel, (condition), sirius::get_logger())
#define SS_DVLOG_EVERY_N(verboselevel, N) DVLOG_EVERY_N_LOGGER(verboselevel, N, sirius::get_logger())
#define SS_DVLOG_IF_EVERY_N(verboselevel, N, condition) DVLOG_IF_EVERY_N_LOGGER(verboselevel, (N), (condition), sirius::get_logger())
#define SS_DVLOG_FIRST_N(verboselevel, N) DVLOG_FIRST_N_LOGGER(verboselevel, (N), sirius::get_logger())
#define SS_DVLOG_IF_FIRST_N(verboselevel, N, condition) DVLOG_IF_FIRST_N_LOGGER(verboselevel, (N), (condition), sirius::get_logger())
#define SS_DVLOG_EVERY_T(verboselevel, seconds) DVLOG_EVERY_T_LOGGER(verboselevel, (seconds), sirius::get_logger())
#define SS_DVLOG_IF_EVERY_T(verboselevel, seconds, condition) DVLOG_IF_EVERY_T_LOGGER(verboselevel, (seconds), (condition), sirius::get_logger())
#define SS_DVLOG_ONCE(verboselevel) DVLOG_ONCE_LOGGER(verboselevel, sirius::get_logger())
#define SS_DVLOG_IF_ONCE(verboselevel, condition) DVLOG_IF_ONCE_LOGGER(verboselevel, (condition), sirius::get_logger())

#endif

