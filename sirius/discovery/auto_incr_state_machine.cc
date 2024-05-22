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


#include <sirius/discovery/auto_incr_state_machine.h>
#include <fstream>
#include <collie/rapidjson/rapidjson.h>
#include <collie/rapidjson/reader.h>
#include <collie/rapidjson/writer.h>
#include <collie/rapidjson/document.h>
#include <collie/rapidjson/stringbuffer.h>
#include <collie/rapidjson/prettywriter.h> // for stringify JSON
#include <melon/raft/util.h>
#include <melon/raft/storage.h>
#include "turbo/strings/numbers.h"
#include <sirius/base/fiber.h>

namespace sirius::discovery {
    void AutoIncrStateMachine::on_apply(melon::raft::Iterator &iter) {
        for (; iter.valid(); iter.next()) {
            melon::raft::Closure *done = iter.done();
            melon::ClosureGuard done_guard(done);
            if (done) {
                ((DiscoveryServerClosure *) done)->raft_time_cost = ((DiscoveryServerClosure *) done)->time_cost.get_time();
            }
            mutil::IOBufAsZeroCopyInputStream wrapper(iter.data());
            sirius::proto::DiscoveryManagerRequest request;
            if (!request.ParseFromZeroCopyStream(&wrapper)) {
                SS_LOG(ERROR) << "parse from protobuf fail when on_apply";
                if (done) {
                    if (((DiscoveryServerClosure *) done)->response) {
                        ((DiscoveryServerClosure *) done)->response->set_errcode(sirius::proto::PARSE_FROM_PB_FAIL);
                        ((DiscoveryServerClosure *) done)->response->set_errmsg("parse from protobuf fail");
                    }
                    melon::raft::run_closure_in_fiber(done_guard.release());
                }
                continue;
            }
            if (done && ((DiscoveryServerClosure *) done)->response) {
                ((DiscoveryServerClosure *) done)->response->set_op_type(request.op_type());
            }
            SS_LOG(DEBUG)<< "on apply, term:" << iter.term() << ", index:" << iter.index()
                        << ", request op_type:" << sirius::proto::OpType_Name(request.op_type());
            switch (request.op_type()) {
                case sirius::proto::OP_ADD_ID_FOR_AUTO_INCREMENT: {
                    add_servlet_id(request, done);
                    break;
                }
                case sirius::proto::OP_DROP_ID_FOR_AUTO_INCREMENT: {
                    drop_servlet_id(request, done);
                    break;
                }
                case sirius::proto::OP_GEN_ID_FOR_AUTO_INCREMENT: {
                    gen_id(request, done);
                    break;
                }
                case sirius::proto::OP_UPDATE_FOR_AUTO_INCREMENT: {
                    update(request, done);
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

    void AutoIncrStateMachine::add_servlet_id(const sirius::proto::DiscoveryManagerRequest &request,
                                            melon::raft::Closure *done) {
        auto &increment_info = request.auto_increment();
        int64_t servlet_id = increment_info.servlet_id();
        uint64_t start_id = increment_info.start_id();
        if (_auto_increment_map.find(servlet_id) != _auto_increment_map.end()) {
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "servlet id has exist");
            SS_LOG(ERROR) << "servlet_id: " << servlet_id << " has exist when add servlet id for auto increment";
            return;
        }
        _auto_increment_map[servlet_id] = start_id;
        if (done && ((DiscoveryServerClosure *) done)->response) {
            ((DiscoveryServerClosure *) done)->response->set_errcode(sirius::proto::SUCCESS);
            ((DiscoveryServerClosure *) done)->response->set_op_type(request.op_type());
            ((DiscoveryServerClosure *) done)->response->set_start_id(start_id);
            ((DiscoveryServerClosure *) done)->response->set_errmsg("SUCCESS");
        }
        SS_LOG(INFO) << "add servlet id for auto_increment success, request:" << request.ShortDebugString();
    }

    void AutoIncrStateMachine::drop_servlet_id(const sirius::proto::DiscoveryManagerRequest &request,
                                             melon::raft::Closure *done) {
        auto &increment_info = request.auto_increment();
        int64_t servlet_id = increment_info.servlet_id();
        if (_auto_increment_map.find(servlet_id) == _auto_increment_map.end()) {
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "servlet id not exist");
            SS_LOG(WARN) << "servlet id: " << servlet_id << " not exist when drop servlet id for auto increment";
            return;
        }
        _auto_increment_map.erase(servlet_id);
        if (done && ((DiscoveryServerClosure *) done)->response) {
            ((DiscoveryServerClosure *) done)->response->set_errcode(sirius::proto::SUCCESS);
            ((DiscoveryServerClosure *) done)->response->set_op_type(request.op_type());
            ((DiscoveryServerClosure *) done)->response->set_errmsg("SUCCESS");
        }
        SS_LOG(INFO) << "drop servlet id for auto_increment success, request:" << request.ShortDebugString();
    }

    void AutoIncrStateMachine::gen_id(const sirius::proto::DiscoveryManagerRequest &request,
                                      melon::raft::Closure *done) {
        auto &increment_info = request.auto_increment();
        int64_t servlet_id = increment_info.servlet_id();
        if (_auto_increment_map.find(servlet_id) == _auto_increment_map.end()) {
            SS_LOG(WARN) << "servlet id: " << servlet_id << " has no auto_increment field";
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "servlet has no auto increment");
            return;
        }
        uint64_t old_start_id = _auto_increment_map[servlet_id];
        if (increment_info.has_start_id() && old_start_id < increment_info.start_id() + 1) {
            old_start_id = increment_info.start_id() + 1;
        }
        _auto_increment_map[servlet_id] = old_start_id + increment_info.count();
        if (done && ((DiscoveryServerClosure *) done)->response) {
            ((DiscoveryServerClosure *) done)->response->set_errcode(sirius::proto::SUCCESS);
            ((DiscoveryServerClosure *) done)->response->set_op_type(request.op_type());
            ((DiscoveryServerClosure *) done)->response->set_start_id(old_start_id);
            ((DiscoveryServerClosure *) done)->response->set_end_id(_auto_increment_map[servlet_id]);
            ((DiscoveryServerClosure *) done)->response->set_errmsg("SUCCESS");
        }
        SS_LOG(DEBUG)<< "gen_id for auto_increment success, request:" << request.ShortDebugString();
    }

    void AutoIncrStateMachine::update(const sirius::proto::DiscoveryManagerRequest &request,
                                      melon::raft::Closure *done) {
        auto &increment_info = request.auto_increment();
        int64_t servlet_id = increment_info.servlet_id();
        if (_auto_increment_map.find(servlet_id) == _auto_increment_map.end()) {
            SS_LOG(WARN) << "servlet id: " << servlet_id << " has no auto_increment field";
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "servlet has no auto increment");
            return;
        }
        if (!increment_info.has_start_id() && !increment_info.has_increment_id()) {
            SS_LOG(WARN) << "star_id or increment_id all not exist, servlet_id:" << servlet_id;
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR,
                                 "star_id or increment_id all not exist");
            return;
        }
        if (increment_info.has_start_id() && increment_info.has_increment_id()) {
            SS_LOG(WARN) << "star_id and increment_id all exist, servlet_id:" << servlet_id;
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR,
                                 "star_id and increment_id all exist");
            return;
        }
        uint64_t old_start_id = _auto_increment_map[servlet_id];
        // backwards
        if (increment_info.has_start_id()
            && old_start_id > increment_info.start_id() + 1
            && (!increment_info.has_force() || increment_info.force() == false)) {
            SS_LOG(WARN) << "request not illegal, max_id not support back, servlet_id:" << servlet_id;
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "not support rollback");
            return;
        }
        if (increment_info.has_start_id()) {
            _auto_increment_map[servlet_id] = increment_info.start_id() + 1;
        } else {
            _auto_increment_map[servlet_id] += increment_info.increment_id();
        }
        if (done && ((DiscoveryServerClosure *) done)->response) {
            ((DiscoveryServerClosure *) done)->response->set_errcode(sirius::proto::SUCCESS);
            ((DiscoveryServerClosure *) done)->response->set_op_type(request.op_type());
            ((DiscoveryServerClosure *) done)->response->set_start_id(_auto_increment_map[servlet_id]);
            ((DiscoveryServerClosure *) done)->response->set_errmsg("SUCCESS");
        }
        SS_LOG(INFO) << "update start_id for auto_increment success, request:" << request.ShortDebugString();
    }

    void AutoIncrStateMachine::on_snapshot_save(melon::raft::SnapshotWriter *writer, melon::raft::Closure *done) {
        SS_LOG(INFO) << "start on snapshot save";
        std::string max_id_string;
        save_auto_increment(max_id_string);
        Fiber bth(&FIBER_ATTR_SMALL);
        std::function<void()> save_snapshot_function = [this, done, writer, max_id_string]() {
            save_snapshot(done, writer, max_id_string);
        };
        bth.run(save_snapshot_function);
    }

    int AutoIncrStateMachine::on_snapshot_load(melon::raft::SnapshotReader *reader) {
        SS_LOG(INFO) << "start on snapshot load";
        std::vector<std::string> files;
        reader->list_files(&files);
        for (auto &file: files) {
            SS_LOG(INFO) << "snapshot load file:" << file;
            if (file == "/max_id.json") {
                std::string max_id_file = reader->get_path() + "/max_id.json";
                if (load_auto_increment(max_id_file) != 0) {
                    SS_LOG(WARN) << "load auto increment max_id fail";
                    return -1;
                }
            }
        }
        set_have_data(true);
        return 0;
    }

    void AutoIncrStateMachine::save_auto_increment(std::string &max_id_string) {
        rapidjson::Document root;
        root.SetObject();
        rapidjson::Document::AllocatorType &alloc = root.GetAllocator();
        for (auto &max_id_pair: _auto_increment_map) {
            std::string servlet_id_string = std::to_string(max_id_pair.first);
            rapidjson::Value servlet_id_val(rapidjson::kStringType);
            servlet_id_val.SetString(servlet_id_string.c_str(), servlet_id_string.size(), alloc);

            rapidjson::Value max_id_value(rapidjson::kNumberType);
            max_id_value.SetUint64(max_id_pair.second);

            root.AddMember(servlet_id_val, max_id_value, alloc);
        }
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> json_writer(buffer);
        root.Accept(json_writer);
        max_id_string = buffer.GetString();
        SS_LOG(INFO) << "save auto increment success, max_id_string:" << max_id_string;
    }

    void AutoIncrStateMachine::save_snapshot(melon::raft::Closure *done,
                                             melon::raft::SnapshotWriter *writer,
                                             std::string max_id_string) {
        melon::ClosureGuard done_guard(done);
        std::string snapshot_path = writer->get_path();
        std::string max_id_path = snapshot_path + "/max_id.json";
        std::ofstream extra_fs(max_id_path,
                               std::ofstream::out | std::ofstream::trunc);
        extra_fs.write(max_id_string.data(), max_id_string.size());
        extra_fs.close();
        if (writer->add_file("/max_id.json") != 0) {
            done->status().set_error(EINVAL, "Fail to add file");
            SS_LOG(ERROR) << "Error while adding file to writer";
            return;
        }
    }

    int AutoIncrStateMachine::load_auto_increment(const std::string &max_id_file) {
        _auto_increment_map.clear();
        std::ifstream extra_fs(max_id_file);
        std::string extra((std::istreambuf_iterator<char>(extra_fs)),
                          std::istreambuf_iterator<char>());
        return parse_json_string(extra);
    }

    int AutoIncrStateMachine::parse_json_string(const std::string &json_string) {
        rapidjson::Document root;
        try {
            root.Parse<0>(json_string.c_str());
            if (root.HasParseError()) {
                rapidjson::ParseErrorCode code = root.GetParseError();
                SS_LOG(WARN) << "parse extra file error, [code:" << code << "][json:" << json_string << "]";
                return -1;
            }
        } catch (...) {
            SS_LOG(WARN) << "parse extra file error, json:" << json_string;
            return -1;
        }
        for (auto json_iter = root.MemberBegin(); json_iter != root.MemberEnd(); ++json_iter) {
            int64_t servlet_id;
            auto r = turbo::simple_atoi(json_iter->name.GetString(), &servlet_id);
            if (r != 0) {
                SS_LOG(WARN) << "parse servlet_id fail, servlet_id:" << json_iter->name.GetString();
                continue;
            }
            uint64_t max_id = json_iter->value.GetUint64();
            SS_LOG(INFO) << "load auto increment, servlet_id:" << servlet_id << ", max_id:" << max_id;
            _auto_increment_map[servlet_id] = max_id;
        }
        return 0;
    }
}  // namespace sirius::discovery
