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


#include <sirius/discovery/tso_state_machine.h>
#include <fstream>
#include <collie/rapidjson/rapidjson.h>
#include <collie/rapidjson/reader.h>
#include <collie/rapidjson/writer.h>
#include <collie/rapidjson/document.h>
#include <collie/rapidjson/stringbuffer.h>
#include <collie/rapidjson/prettywriter.h> // for stringify JSON
#include <melon/raft/util.h>
#include <melon/raft/storage.h>
#include <sirius/flags/discovery.h>

namespace sirius::discovery {

    int TsoTimer::init(TSOStateMachine *node, int timeout_ms) {
        int ret = RepeatedTimerTask::init(timeout_ms);
        _node = node;
        return ret;
    }

    void TsoTimer::run() {
        _node->update_timestamp();
    }


    const std::string TSOStateMachine::SNAPSHOT_TSO_FILE = "tso.file";
    const std::string TSOStateMachine::SNAPSHOT_TSO_FILE_WITH_SLASH = "/" + SNAPSHOT_TSO_FILE;

    int TSOStateMachine::init(const std::vector<melon::raft::PeerId> &peers) {
        _tso_update_timer.init(this, tso::update_timestamp_interval_ms);
        _tso_obj.current_timestamp.set_physical(0);
        _tso_obj.current_timestamp.set_logical(0);
        _tso_obj.last_save_physical = 0;
        //int ret = BaseStateMachine::init(peers);
        melon::raft::NodeOptions options;
        options.election_timeout_ms = FLAGS_discovery_election_timeout_ms;
        options.fsm = this;
        options.initial_conf = melon::raft::Configuration(peers);
        options.snapshot_interval_s = FLAGS_discovery_tso_snapshot_interval_s;
        options.log_uri = FLAGS_discovery_log_uri + std::to_string(_dummy_region_id);
        //options.stable_uri = FLAGS_stable_uri + "/discovery";
        options.raft_meta_uri = FLAGS_discovery_stable_uri + _file_path;
        options.snapshot_uri = FLAGS_discovery_snapshot_uri + _file_path;
        int ret = _node.init(options);
        if (ret < 0) {
            SS_LOG(ERROR) << "raft node init fail";
            return ret;
        }
        SS_LOG(INFO) << "raft init success, meat state machine init success";
        return 0;
    }

    void TSOStateMachine::gen_tso(const sirius::proto::TsoRequest *request, sirius::proto::TsoResponse *response) {
        int64_t count = request->count();
        response->set_op_type(request->op_type());
        if (count == 0) {
            response->set_errcode(sirius::proto::INPUT_PARAM_ERROR);
            response->set_errmsg("tso count should be positive");
            return;
        }
        if (!_is_healty) {
            SS_LOG(ERROR) << "TSO has wrong status, retry later";
            response->set_errcode(sirius::proto::RETRY_LATER);
            response->set_errmsg("timestamp not ok, retry later");
            return;
        }
        sirius::proto::TsoTimestamp current;
        bool need_retry = false;
        for (size_t i = 0; i < 50; i++) {
            {
                MELON_SCOPED_LOCK(_tso_mutex);
                int64_t physical = _tso_obj.current_timestamp.physical();
                if (physical != 0) {
                    int64_t new_logical = _tso_obj.current_timestamp.logical() + count;
                    if (new_logical < tso::max_logical) {
                        current.CopyFrom(_tso_obj.current_timestamp);
                        _tso_obj.current_timestamp.set_logical(new_logical);
                        need_retry = false;
                    } else {
                        SS_LOG(WARN) << "logical part outside of max logical interval, retry later, please check ntp time";
                        need_retry = true;
                    }
                } else {
                    SS_LOG(WARN) << "timestamp not ok physical == 0, retry later";
                    need_retry = true;
                }
            }
            if (!need_retry) {
                break;
            } else {
               fiber_usleep(tso::update_timestamp_interval_ms * 1000LL);
            }
        }
        if (need_retry) {
            response->set_errcode(sirius::proto::EXEC_FAIL);
            response->set_errmsg("gen tso failed");
            SS_LOG(ERROR) << "gen tso failed";
            return;
        }
        //TLOG_WARN("gen tso current: ({}, {})", current.physical(), current.logical());
        auto timestamp = response->mutable_start_timestamp();
        timestamp->CopyFrom(current);
        response->set_count(count);
        response->set_errcode(sirius::proto::SUCCESS);
    }

    void TSOStateMachine::process(google::protobuf::RpcController *controller,
                                  const sirius::proto::TsoRequest *request,
                                  sirius::proto::TsoResponse *response,
                                  google::protobuf::Closure *done) {
        melon::ClosureGuard done_guard(done);
        if (request->op_type() == sirius::proto::OP_QUERY_TSO_INFO) {
            response->set_errcode(sirius::proto::SUCCESS);
            response->set_errmsg("success");
            response->set_op_type(request->op_type());
            response->set_leader(mutil::endpoint2str(_node.leader_id().addr).c_str());
            response->set_system_time(tso::clock_realtime_ms());
            response->set_save_physical(_tso_obj.last_save_physical);
            auto timestamp = response->mutable_start_timestamp();
            timestamp->CopyFrom(_tso_obj.current_timestamp);
            return;
        }
        melon::Controller *cntl = (melon::Controller *) controller;
        uint64_t log_id = 0;
        if (cntl->has_log_id()) {
            log_id = cntl->log_id();
        }
        const auto &remote_side_tmp = mutil::endpoint2str(cntl->remote_side());
        const char *remote_side = remote_side_tmp.c_str();
        if (!_is_leader) {
            response->set_errcode(sirius::proto::NOT_LEADER);
            response->set_errmsg("not leader");
            response->set_op_type(request->op_type());
            response->set_leader(mutil::endpoint2str(_node.leader_id().addr).c_str());
            SS_LOG(WARN) << "state machine not leader, request:" << request->ShortDebugString()
                         << " remote_side:" << remote_side << " log_id:" << log_id;
            return;
        }
        // 获取时间戳在raft外执行
        if (request->op_type() == sirius::proto::OP_GEN_TSO) {
            gen_tso(request, response);
            return;
        }
        mutil::IOBuf data;
        mutil::IOBufAsZeroCopyOutputStream wrapper(&data);
        if (!request->SerializeToZeroCopyStream(&wrapper)) {
            cntl->SetFailed(melon::EREQUEST, "Fail to serialize request");
            return;
        }
        TsoClosure *closure = new TsoClosure;
        closure->cntl = cntl;
        closure->response = response;
        closure->done = done_guard.release();
        closure->common_state_machine = this;
        melon::raft::Task task;
        task.data = &data;
        task.done = closure;
        _node.apply(task);
    }

    void TSOStateMachine::on_apply(melon::raft::Iterator &iter) {
        for (; iter.valid(); iter.next()) {
            melon::raft::Closure *done = iter.done();
            melon::ClosureGuard done_guard(done);
            if (done) {
                ((TsoClosure *) done)->raft_time_cost = ((TsoClosure *) done)->time_cost.get_time();
            }
            mutil::IOBufAsZeroCopyInputStream wrapper(iter.data());
            sirius::proto::TsoRequest request;
            if (!request.ParseFromZeroCopyStream(&wrapper)) {
                SS_LOG(ERROR) << "parse from protobuf fail when on_apply";
                if (done) {
                    if (((TsoClosure *) done)->response) {
                        ((TsoClosure *) done)->response->set_errcode(sirius::proto::PARSE_FROM_PB_FAIL);
                        ((TsoClosure *) done)->response->set_errmsg("parse from protobuf fail");
                    }
                    melon::raft::run_closure_in_fiber(done_guard.release());
                }
                continue;
            }
            if (done && ((TsoClosure *) done)->response) {
                ((TsoClosure *) done)->response->set_op_type(request.op_type());
            }
            switch (request.op_type()) {
                case sirius::proto::OP_RESET_TSO: {
                    reset_tso(request, done);
                    break;
                }
                case sirius::proto::OP_UPDATE_TSO: {
                    update_tso(request, done);
                    break;
                }
                default: {
                    SS_LOG(ERROR) << "unsupport request type, type:" << request.op_type();
                    IF_DONE_SET_RESPONSE(done, sirius::proto::UNKNOWN_REQ_TYPE, "unsupport request type");
                }
            }
            if (done) {
                melon::raft::run_closure_in_fiber(done_guard.release());
            }
        }
    }

    void TSOStateMachine::reset_tso(const sirius::proto::TsoRequest &request,
                                    melon::raft::Closure *done) {
        if (request.has_current_timestamp() && request.has_save_physical()) {
            int64_t physical = request.save_physical();
            sirius::proto::TsoTimestamp current = request.current_timestamp();
            if (physical < _tso_obj.last_save_physical
                || current.physical() < _tso_obj.current_timestamp.physical()) {
                if (!request.force()) {
                    SS_LOG(WARN) << "time fallback save_physical:(" << physical << ", " << _tso_obj.last_save_physical
                                 << ") current:(" << current.physical() << ", " << _tso_obj.current_timestamp.physical()
                                 << ", " << current.logical() << ", " << _tso_obj.current_timestamp.logical();
                    if (done && ((TsoClosure *) done)->response) {
                        sirius::proto::TsoResponse *response = ((TsoClosure *) done)->response;
                        response->set_errcode(sirius::proto::INTERNAL_ERROR);
                        response->set_errmsg("time can't fallback");
                        auto timestamp = response->mutable_start_timestamp();
                        timestamp->CopyFrom(_tso_obj.current_timestamp);
                        response->set_save_physical(_tso_obj.last_save_physical);
                    }
                    return;
                }
            }
            _is_healty = true;
            SS_LOG(WARN) << "reset tso save_physical: " << physical << " current: (" << current.physical()
                         << ", " << current.logical() << ")";
            {
                MELON_SCOPED_LOCK(_tso_mutex);
                _tso_obj.last_save_physical = physical;
                _tso_obj.current_timestamp.CopyFrom(current);
            }
            if (done && ((TsoClosure *) done)->response) {
                sirius::proto::TsoResponse *response = ((TsoClosure *) done)->response;
                response->set_save_physical(physical);
                auto timestamp = response->mutable_start_timestamp();
                timestamp->CopyFrom(current);
                response->set_errcode(sirius::proto::SUCCESS);
                response->set_errmsg("SUCCESS");
            }
        }
    }

    void TSOStateMachine::update_tso(const sirius::proto::TsoRequest &request,
                                     melon::raft::Closure *done) {
        int64_t physical = request.save_physical();
        sirius::proto::TsoTimestamp current = request.current_timestamp();
        if (physical < _tso_obj.last_save_physical
            || current.physical() < _tso_obj.current_timestamp.physical()) {
            SS_LOG(WARN) << "time fallback save_physical:(" << physical << ", " << _tso_obj.last_save_physical
                         << ") current:(" << current.physical() << ", " << _tso_obj.current_timestamp.physical()
                         << ", " << current.logical() << ", " << _tso_obj.current_timestamp.logical() << ")";
            if (done && ((TsoClosure *) done)->response) {
                sirius::proto::TsoResponse *response = ((TsoClosure *) done)->response;
                response->set_errcode(sirius::proto::INTERNAL_ERROR);
                response->set_errmsg("time can't fallback");
            }
            return;
        }
        {
            MELON_SCOPED_LOCK(_tso_mutex);
            _tso_obj.last_save_physical = physical;
            _tso_obj.current_timestamp.CopyFrom(current);
        }

        if (done && ((TsoClosure *) done)->response) {
            sirius::proto::TsoResponse *response = ((TsoClosure *) done)->response;
            response->set_errcode(sirius::proto::SUCCESS);
            response->set_errmsg("SUCCESS");
        }
    }


    int TSOStateMachine::sync_timestamp(const sirius::proto::TsoTimestamp &current_timestamp, int64_t save_physical) {
        sirius::proto::TsoRequest request;
        sirius::proto::TsoResponse response;
        request.set_op_type(sirius::proto::OP_UPDATE_TSO);
        auto timestamp = request.mutable_current_timestamp();
        timestamp->CopyFrom(current_timestamp);
        request.set_save_physical(save_physical);
        mutil::IOBuf data;
        mutil::IOBufAsZeroCopyOutputStream wrapper(&data);
        if (!request.SerializeToZeroCopyStream(&wrapper)) {
            SS_LOG(WARN) << "Fail to serialize request";
            return -1;
        }
        FiberCond sync_cond;
        TsoClosure *c = new TsoClosure(&sync_cond);
        c->response = &response;
        c->done = nullptr;
        c->common_state_machine = this;
        sync_cond.increase();
        melon::raft::Task task;
        task.data = &data;
        task.done = c;
        _node.apply(task);
        sync_cond.wait();
        if (response.errcode() != sirius::proto::SUCCESS) {
            SS_LOG(ERROR) << "sync timestamp failed, request:" << request.ShortDebugString()
                          << " response:" << response.ShortDebugString();
            return -1;
        }
        return 0;
    }

    void TSOStateMachine::update_timestamp() {
        if (!_is_leader) {
            return;
        }
        int64_t now = tso::clock_realtime_ms();
        int64_t prev_physical = 0;
        int64_t prev_logical = 0;
        int64_t last_save = 0;
        {
            MELON_SCOPED_LOCK(_tso_mutex);
            prev_physical = _tso_obj.current_timestamp.physical();
            prev_logical = _tso_obj.current_timestamp.logical();
            last_save = _tso_obj.last_save_physical;
        }
        int64_t delta = now - prev_physical;
        if (delta < 0) {
            SS_LOG(WARN)<< "physical time slow now:" << now << " prev:" << prev_physical;
        }
        int64_t next = now;
        if (delta > tso::update_timestamp_guard_ms) {
            next = now;
        } else if (prev_logical > tso::max_logical / 2) {
            next = now + tso::update_timestamp_guard_ms;
        } else {
            SS_LOG(WARN) << "don't need update timestamp prev:" << prev_physical << " now:" << now << " save:" << last_save;
            return;
        }
        int64_t save = last_save;
        if (save - next <= tso::update_timestamp_guard_ms) {
            save = next + tso::save_interval_ms;
        }
        sirius::proto::TsoTimestamp tp;
        tp.set_physical(next);
        tp.set_logical(0);
        sync_timestamp(tp, save);
    }

    void TSOStateMachine::on_leader_start() {
        SS_LOG(WARN) << "tso leader start";
        int64_t now = tso::clock_realtime_ms();
        sirius::proto::TsoTimestamp current;
        current.set_physical(now);
        current.set_logical(0);
        int64_t last_save = _tso_obj.last_save_physical;
        if (last_save - now < tso::update_timestamp_interval_ms) {
            current.set_physical(last_save + tso::update_timestamp_guard_ms);
            last_save = now + tso::save_interval_ms;
        }
        auto func = [this, last_save, current]() {
            SS_LOG(WARN) << "leader start current(phy:" << current.physical() << ",log:" << current.logical()
                         << ") save:" << last_save;
            int ret = sync_timestamp(current, last_save);
            if (ret < 0) {
                _is_healty = false;
            }
            SS_LOG(WARN) << "sync timestamp ok";
            _is_leader.store(true);
            _tso_update_timer.start();
        };
        Fiber bth;
        bth.run(func);
    }

    void TSOStateMachine::on_leader_stop() {
        _tso_update_timer.stop();
        SS_LOG(WARN) << "tso leader stop";
        BaseStateMachine::on_leader_stop();
    }

    void TSOStateMachine::on_snapshot_save(melon::raft::SnapshotWriter *writer, melon::raft::Closure *done) {
        SS_LOG(WARN) << "start on snapshot save";
        std::string sto_str = std::to_string(_tso_obj.last_save_physical);
        Fiber bth(&FIBER_ATTR_SMALL);
        std::function<void()> save_snapshot_function = [this, done, writer, sto_str]() {
            save_snapshot(done, writer, sto_str);
        };
        bth.run(save_snapshot_function);
    }

    void TSOStateMachine::save_snapshot(melon::raft::Closure *done,
                                        melon::raft::SnapshotWriter *writer,
                                        std::string sto_str) {
        melon::ClosureGuard done_guard(done);
        std::string snapshot_path = writer->get_path();
        std::string save_path = snapshot_path + SNAPSHOT_TSO_FILE_WITH_SLASH;
        std::ofstream extra_fs(save_path,
                               std::ofstream::out | std::ofstream::trunc);
        extra_fs.write(sto_str.data(), sto_str.size());
        extra_fs.close();
        if (writer->add_file(SNAPSHOT_TSO_FILE_WITH_SLASH) != 0) {
            done->status().set_error(EINVAL, "Fail to add file");
            SS_LOG(WARN) << "Error while adding file to writer";
            return;
        }
        SS_LOG(WARN) << "save physical string:" << sto_str << " when snapshot";
    }

    int TSOStateMachine::on_snapshot_load(melon::raft::SnapshotReader *reader) {
        SS_LOG(WARN) << "start on snapshot load";
        std::vector<std::string> files;
        reader->list_files(&files);
        for (auto &file: files) {
            SS_LOG(WARN) << "snapshot load file:" << file;
            if (file == SNAPSHOT_TSO_FILE_WITH_SLASH) {
                std::string tso_file = reader->get_path() + SNAPSHOT_TSO_FILE_WITH_SLASH;
                if (load_tso(tso_file) != 0) {
                    SS_LOG(WARN) << "load tso fail";
                    return -1;
                }
                break;
            }
        }
        set_have_data(true);
        return 0;
    }

    int TSOStateMachine::load_tso(const std::string &tso_file) {
        std::ifstream extra_fs(tso_file);
        std::string extra((std::istreambuf_iterator<char>(extra_fs)),
                          std::istreambuf_iterator<char>());
        try {
            _tso_obj.last_save_physical = std::stol(extra);
        } catch (std::invalid_argument &) {
            SS_LOG(WARN) << "Invalid_argument: " << extra;
            return -1;
        }
        catch (std::out_of_range &) {
            SS_LOG(WARN)<< "Out of range: " << extra;
            return -1;
        }
        catch (...) {
            SS_LOG(WARN) << "error happen: " << extra;
            return -1;
        }
        return 0;
    }

}  // namespace sirius::discovery
