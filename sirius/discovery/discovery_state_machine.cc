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


#include <sirius/discovery/discovery_state_machine.h>
#include <melon/raft/util.h>
#include <melon/raft/storage.h>
#include <sirius/base/scope_exit.h>
#include <sirius/discovery/privilege_manager.h>
#include <sirius/discovery/schema_manager.h>
#include <sirius/discovery/config_manager.h>
#include <sirius/discovery/namespace_manager.h>
#include <sirius/discovery/zone_manager.h>
#include <sirius/discovery/instance_manager.h>
#include <sirius/discovery/servlet_manager.h>
#include <sirius/storage/rocks_storage.h>
#include <sirius/discovery/query_privilege_manager.h>
#include <sirius/storage/sst_file_writer.h>
#include <sirius/discovery/parse_path.h>

namespace sirius::discovery {


    void DiscoveryStateMachine::on_apply(melon::raft::Iterator &iter) {
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
            SS_LOG(INFO) << "on apply, term:" << iter.term() << ", index:" << iter.index()
                         << ", request op_type:" << sirius::proto::OpType_Name(request.op_type());
            switch (request.op_type()) {
                case sirius::proto::OP_CREATE_USER: {
                    PrivilegeManager::get_instance()->create_user(request, done);
                    break;
                }
                case sirius::proto::OP_DROP_USER: {
                    PrivilegeManager::get_instance()->drop_user(request, done);
                    break;
                }
                case sirius::proto::OP_ADD_PRIVILEGE: {
                    PrivilegeManager::get_instance()->add_privilege(request, done);
                    break;
                }
                case sirius::proto::OP_DROP_PRIVILEGE: {
                    PrivilegeManager::get_instance()->drop_privilege(request, done);
                    break;
                }
                case sirius::proto::OP_CREATE_NAMESPACE: {
                    NamespaceManager::get_instance()->create_namespace(request, done);
                    break;
                }
                case sirius::proto::OP_DROP_NAMESPACE: {
                    NamespaceManager::get_instance()->drop_namespace(request, done);
                    break;
                }
                case sirius::proto::OP_MODIFY_NAMESPACE: {
                    NamespaceManager::get_instance()->modify_namespace(request, done);
                    break;
                }
                case sirius::proto::OP_CREATE_ZONE: {
                    ZoneManager::get_instance()->create_zone(request, done);
                    break;
                }
                case sirius::proto::OP_DROP_ZONE: {
                    ZoneManager::get_instance()->drop_zone(request, done);
                    break;
                }
                case sirius::proto::OP_MODIFY_ZONE: {
                    ZoneManager::get_instance()->modify_zone(request, done);
                    break;
                }
                case sirius::proto::OP_CREATE_SERVLET: {
                    ServletManager::get_instance()->create_servlet(request, done);
                    break;
                }
                case sirius::proto::OP_DROP_SERVLET: {
                    ServletManager::get_instance()->drop_servlet(request, done);
                    break;
                }
                case sirius::proto::OP_MODIFY_SERVLET: {
                    ServletManager::get_instance()->modify_servlet(request, done);
                    break;
                }
                case sirius::proto::OP_CREATE_CONFIG: {
                    ConfigManager::get_instance()->create_config(request, done);
                    break;
                }
                case sirius::proto::OP_REMOVE_CONFIG: {
                    ConfigManager::get_instance()->remove_config(request, done);
                    break;
                }
                case sirius::proto::OP_ADD_INSTANCE: {
                    InstanceManager::get_instance()->add_instance(request, done);
                    break;
                }
                case sirius::proto::OP_DROP_INSTANCE: {
                    InstanceManager::get_instance()->drop_instance(request, done);
                    break;
                }
                case sirius::proto::OP_UPDATE_INSTANCE: {
                    InstanceManager::get_instance()->update_instance(request, done);
                    break;
                }
                default: {
                    SS_LOG(ERROR) << "unknown request type, type:" << request.op_type();
                    IF_DONE_SET_RESPONSE(done, sirius::proto::UNKNOWN_REQ_TYPE, "unknown request type");
                }
            }
            _applied_index = iter.index();
            if (done) {
                melon::raft::run_closure_in_fiber(done_guard.release());
            }
        }
    }

    void DiscoveryStateMachine::on_snapshot_save(melon::raft::SnapshotWriter *writer, melon::raft::Closure *done) {
        SS_LOG(WARN) << "start on snapshot save";
        SS_LOG(WARN) << "max_namespace_id:" << NamespaceManager::get_instance()->get_max_namespace_id()
                     << ", max_zone_id:" << ZoneManager::get_instance()->get_max_zone_id()<< " when on snapshot save";
        //创建snapshot
        rocksdb::ReadOptions read_options;
        read_options.prefix_same_as_start = false;
        read_options.total_order_seek = true;
        auto iter = RocksStorage::get_instance()->new_iterator(read_options,
                                                               RocksStorage::get_instance()->get_meta_info_handle());
        iter->SeekToFirst();
        Fiber bth(&FIBER_ATTR_SMALL);
        std::function<void()> save_snapshot_function = [this, done, iter, writer]() {
            save_snapshot(done, iter, writer);
        };
        bth.run(save_snapshot_function);
    }

    void DiscoveryStateMachine::save_snapshot(melon::raft::Closure *done,
                                         rocksdb::Iterator *iter,
                                         melon::raft::SnapshotWriter *writer) {
        melon::ClosureGuard done_guard(done);
        std::unique_ptr<rocksdb::Iterator> iter_lock(iter);

        std::string snapshot_path = writer->get_path();
        std::string sst_file_path = snapshot_path + "/discovery_info.sst";

        rocksdb::Options option = RocksStorage::get_instance()->get_options(
                RocksStorage::get_instance()->get_meta_info_handle());
        SstFileWriter sst_writer(option);
        SS_LOG(INFO)<< "snapshot path:" << snapshot_path;
        //Open the file for writing
        auto s = sst_writer.open(sst_file_path);
        if (!s.ok()) {
            SS_LOG(WARN) << "Error while opening file " << sst_file_path << ", Error " << s.ToString();
            done->status().set_error(EINVAL, "Fail to open SstFileWriter");
            return;
        }
        for (; iter->Valid(); iter->Next()) {
            auto res = sst_writer.put(iter->key(), iter->value());
            if (!res.ok()) {
                SS_LOG(WARN) << "Error while adding Key: " << iter->key().ToString() << ", Error: " << s.ToString();
                done->status().set_error(EINVAL, "Fail to write SstFileWriter");
                return;
            }
        }
        //close the file
        s = sst_writer.finish();
        if (!s.ok()) {
            SS_LOG(WARN) << "Error while finishing file " << sst_file_path << ", Error " << s.ToString();
            done->status().set_error(EINVAL, "Fail to finish SstFileWriter");
            return;
        }
        if (writer->add_file("/discovery_info.sst") != 0) {
            done->status().set_error(EINVAL, "Fail to add file");
            SS_LOG(ERROR) << "Error while adding file to writer";
            return;
        }
    }

    int DiscoveryStateMachine::on_snapshot_load(melon::raft::SnapshotReader *reader) {
        SS_LOG(WARN) << "start on snapshot load";
        std::string remove_start_key(DiscoveryConstants::SCHEMA_IDENTIFY);
        rocksdb::WriteOptions options;
        auto status = RocksStorage::get_instance()->remove_range(options,
                                                                 RocksStorage::get_instance()->get_meta_info_handle(),
                                                                 remove_start_key,
                                                                 DiscoveryConstants::MAX_IDENTIFY,
                                                                 false);
        if (!status.ok()) {
            SS_LOG(ERROR) << "remove range error when on snapshot load: code=" << status.code() << ", msg=" << status.ToString();
            return -1;
        } else {
            SS_LOG(WARN) << "remove range success when on snapshot load:code:" << status.code() << ", msg:" << status.ToString();
        }
        SS_LOG(WARN) << "clear data success";
        rocksdb::ReadOptions read_options;
        std::unique_ptr<rocksdb::Iterator> iter(RocksStorage::get_instance()->new_iterator(read_options,
                                                                                           RocksStorage::get_instance()->get_meta_info_handle()));
        iter->Seek(DiscoveryConstants::SCHEMA_IDENTIFY);
        for (; iter->Valid(); iter->Next()) {
            SS_LOG(WARN) << "iter key:" << iter->key().ToString() << ", iter value:" << iter->value().ToString();
        }
        std::vector<std::string> files;
        reader->list_files(&files);
        for (auto &file: files) {
            SS_LOG(INFO) << "snapshot load file:" << file;
            if (file == "/discovery_info.sst") {
                std::string snapshot_path = reader->get_path();
                _applied_index = parse_snapshot_index_from_path(snapshot_path, false);
                SS_LOG(INFO) << "_applied_index:" << _applied_index << " path:" << snapshot_path;
                snapshot_path.append("/discovery_info.sst");

                //恢复文件
                rocksdb::IngestExternalFileOptions ifo;
                auto res = RocksStorage::get_instance()->ingest_external_file(
                        RocksStorage::get_instance()->get_meta_info_handle(),
                        {snapshot_path},
                        ifo);
                if (!res.ok()) {
                    SS_LOG(ERROR) << "Error while ingest file " << snapshot_path << ", Error " << res.ToString();
                    return -1;

                }
                auto ret = PrivilegeManager::get_instance()->load_snapshot();
                if (ret != 0) {
                    SS_LOG(ERROR) << "PrivilegeManager load snapshot fail";
                    return -1;
                }
                ret = SchemaManager::get_instance()->load_snapshot();
                if (ret != 0) {
                    SS_LOG(ERROR) << "SchemaManager load snapshot fail";
                    return -1;
                }

                ret = ConfigManager::get_instance()->load_snapshot();
                if (ret != 0) {
                    SS_LOG(ERROR) << "ConfigManager load snapshot fail";
                    return -1;
                }
                ret = InstanceManager::get_instance()->load_snapshot();
                if (ret != 0) {
                    SS_LOG(ERROR)<< "Instance load snapshot fail";
                    return -1;
                }
            }
        }
        set_have_data(true);
        return 0;
    }

    void DiscoveryStateMachine::on_leader_start() {
        SS_LOG(WARN) << "leader start at new term";
        BaseStateMachine::on_leader_start();
        _is_leader.store(true);
    }

    void DiscoveryStateMachine::on_leader_stop() {
        _is_leader.store(false);
        SS_LOG(WARN) << "leader stop";
        BaseStateMachine::on_leader_stop();
    }

}  // namespace sirius::discovery
