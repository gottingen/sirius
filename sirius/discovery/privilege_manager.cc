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


#include <sirius/discovery/privilege_manager.h>
#include <melon/rpc/channel.h>
#include <sirius/discovery/sirius_state_machine.h>
#include <sirius/discovery/schema_manager.h>
#include <sirius/discovery/sirius_server.h>
#include <sirius/discovery/sirius_db.h>

namespace sirius::discovery {

    void PrivilegeManager::process_user_privilege(google::protobuf::RpcController *controller,
                                                  const sirius::proto::DiscoveryManagerRequest *request,
                                                  sirius::proto::DiscoveryManagerResponse *response,
                                                  google::protobuf::Closure *done) {
        melon::ClosureGuard done_guard(done);
        auto *cntl = static_cast<melon::Controller *>(controller);
        uint64_t log_id = 0;
        if (cntl->has_log_id()) {
            log_id = cntl->log_id();
        }
        if (!request->has_user_privilege()) {
            ERROR_SET_RESPONSE(response,
                               sirius::proto::INPUT_PARAM_ERROR,
                               "no user_privilege",
                               request->op_type(),
                               log_id);
            return;
        }
        switch (request->op_type()) {
            case sirius::proto::OP_CREATE_USER: {
                if (!request->user_privilege().has_password()) {
                    ERROR_SET_RESPONSE(response,
                                       sirius::proto::INPUT_PARAM_ERROR,
                                       "no password",
                                       request->op_type(),
                                       log_id);
                    return;
                }
                _discovery_state_machine->process(controller, request, response, done_guard.release());
                return;
            }
            case sirius::proto::OP_DROP_USER:
            case sirius::proto::OP_ADD_PRIVILEGE:
            case sirius::proto::OP_DROP_PRIVILEGE: {
                _discovery_state_machine->process(controller, request, response, done_guard.release());
                return;
            }
            default: {
                ERROR_SET_RESPONSE(response,
                                   sirius::proto::INPUT_PARAM_ERROR,
                                   "invalid op_type",
                                   request->op_type(),
                                   log_id);
                return;
            }
        }
    }

    void PrivilegeManager::create_user(const sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done) {
        auto &user_privilege = const_cast<sirius::proto::UserPrivilege &>(request.user_privilege());
        std::string username = user_privilege.username();
        if (_user_privilege.find(username) != _user_privilege.end()) {
            LOG(INFO) << "request username has been created, username:" << user_privilege.username();
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "username has been repeated");
            return;
        }
        int ret = SchemaManager::get_instance()->check_and_get_for_privilege(user_privilege);
        if (ret < 0) {
            LOG(WARNING) << "request not illegal, request:" << request.ShortDebugString();
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "request invalid");
            return;
        }
        user_privilege.set_version(1);
        // construct key and value
        std::string value;
        if (!user_privilege.SerializeToString(&value)) {
            LOG(WARNING) << "request serializeToArray fail, request:" << request.ShortDebugString();
            IF_DONE_SET_RESPONSE(done, sirius::proto::PARSE_TO_PB_FAIL, "serializeToArray fail");
            return;
        }
        // write date to rocksdb
        ret = DiscoveryRocksdb::get_instance()->put_discovery_info(construct_privilege_key(username), value);
        if (ret < 0) {
            LOG(WARNING) << "add username:" << username << " privilege to rocksdb fail";
            IF_DONE_SET_RESPONSE(done, sirius::proto::INTERNAL_ERROR, "write db fail");
            return;
        }
        // update memory values
        MELON_SCOPED_LOCK(_user_mutex);
        _user_privilege[username] = user_privilege;
        IF_DONE_SET_RESPONSE(done, sirius::proto::SUCCESS, "success");
        LOG(INFO) << "create user success, request:" << request.ShortDebugString();
    }

    void PrivilegeManager::drop_user(const sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done) {
        std::string username = request.user_privilege().username();
        if (_user_privilege.find(username) == _user_privilege.end()) {
            LOG(WARNING) << "request username not exist, username:" << username;
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "username not exist");
            return;
        }

        // update rocksdb
        auto ret = DiscoveryRocksdb::get_instance()->remove_discovery_info(
                std::vector<std::string>{construct_privilege_key(username)});
        if (ret < 0) {
            LOG(WARNING) << "drop username:" << username << " privilege to rocksdb fail";
            IF_DONE_SET_RESPONSE(done, sirius::proto::INTERNAL_ERROR, "delete from db fail");
            return;
        }
        // update memory
        MELON_SCOPED_LOCK(_user_mutex);
        _user_privilege.erase(username);
        IF_DONE_SET_RESPONSE(done, sirius::proto::SUCCESS, "success");
        LOG(INFO) << "drop user success, request:" << request.ShortDebugString();
    }

    void PrivilegeManager::add_privilege(const sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done) {
        auto &user_privilege = const_cast<sirius::proto::UserPrivilege &>(request.user_privilege());
        std::string username = user_privilege.username();
        if (_user_privilege.find(username) == _user_privilege.end()) {
            LOG(WARNING) << "request username not exist, username:" << username;
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "username not exist");
            return;
        }
        int ret = SchemaManager::get_instance()->check_and_get_for_privilege(user_privilege);
        if (ret < 0) {
            LOG(WARNING) << "request not illegal, request:" << request.ShortDebugString();
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "request invalid");
            return;
        }
        sirius::proto::UserPrivilege tmp_mem_privilege = _user_privilege[username];
        for (auto &privilege_zone: user_privilege.privilege_zone()) {
            insert_zone_privilege(privilege_zone, tmp_mem_privilege);
        }
        for (auto &privilege_servlet: user_privilege.privilege_servlet()) {
            insert_servlet_privilege(privilege_servlet, tmp_mem_privilege);
        }
        for (auto &ip: user_privilege.ip()) {
            insert_ip(ip, tmp_mem_privilege);
        }
        if (user_privilege.has_need_auth_addr()) {
            tmp_mem_privilege.set_need_auth_addr(user_privilege.need_auth_addr());
        }
        if (user_privilege.has_resource_tag()) {
            tmp_mem_privilege.set_resource_tag(user_privilege.resource_tag());
        }
        tmp_mem_privilege.set_version(tmp_mem_privilege.version() + 1);
        // 构造key 和 value
        std::string value;
        if (!tmp_mem_privilege.SerializeToString(&value)) {
            LOG(WARNING) << "request serializeToArray fail, request:" << request.ShortDebugString();
            IF_DONE_SET_RESPONSE(done, sirius::proto::PARSE_TO_PB_FAIL, "serializeToArray fail");
            return;
        }
        // write date to rocksdb
        ret = DiscoveryRocksdb::get_instance()->put_discovery_info(construct_privilege_key(username), value);
        if (ret != 0) {
            LOG(WARNING)<< "add username:" << username << " privilege to rocksdb fail";
            IF_DONE_SET_RESPONSE(done, sirius::proto::INTERNAL_ERROR, "write db fail");
            return;
        }
        MELON_SCOPED_LOCK(_user_mutex);
        _user_privilege[username] = tmp_mem_privilege;
        IF_DONE_SET_RESPONSE(done, sirius::proto::SUCCESS, "success");
        LOG(INFO) << "add privilege success, request:" << request.ShortDebugString();
    }

    void PrivilegeManager::drop_privilege(const sirius::proto::DiscoveryManagerRequest &request, melon::raft::Closure *done) {
        auto &user_privilege = const_cast<sirius::proto::UserPrivilege &>(request.user_privilege());
        std::string username = user_privilege.username();
        if (_user_privilege.find(username) == _user_privilege.end()) {
            LOG(WARNING) << "request username not exist, username:" << username;
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "username not exist");
            return;
        }
        int ret = SchemaManager::get_instance()->check_and_get_for_privilege(user_privilege);
        if (ret < 0) {
            LOG(WARNING) << "request not illegal, request:" << request.ShortDebugString();
            IF_DONE_SET_RESPONSE(done, sirius::proto::INPUT_PARAM_ERROR, "request invalid");
            return;
        }
        sirius::proto::UserPrivilege tmp_mem_privilege = _user_privilege[username];

        for (auto &privilege_zone: user_privilege.privilege_zone()) {
            delete_zone_privilege(privilege_zone, tmp_mem_privilege);
        }
        for (auto &privilege_servlet: user_privilege.privilege_servlet()) {
            delete_servlet_privilege(privilege_servlet, tmp_mem_privilege);
        }

        for (auto &ip: user_privilege.ip()) {
            delete_ip(ip, tmp_mem_privilege);
        }
        if (user_privilege.has_need_auth_addr()) {
            tmp_mem_privilege.set_need_auth_addr(user_privilege.need_auth_addr());
        }
        if (user_privilege.has_resource_tag() && tmp_mem_privilege.has_resource_tag() &&
            user_privilege.resource_tag() == tmp_mem_privilege.resource_tag()) {
            tmp_mem_privilege.clear_resource_tag();
        }
        tmp_mem_privilege.set_version(tmp_mem_privilege.version() + 1);
        // 构造key 和 value
        std::string value;
        if (!tmp_mem_privilege.SerializeToString(&value)) {
            LOG(WARNING) << "request serializeToArray fail, request:" << request.ShortDebugString();
            IF_DONE_SET_RESPONSE(done, sirius::proto::PARSE_TO_PB_FAIL, "serializeToArray fail");
            return;
        }
        // write date to rocksdb
        ret = DiscoveryRocksdb::get_instance()->put_discovery_info(construct_privilege_key(username), value);
        if (ret < 0) {
            LOG(WARNING) << "add username:" << username << " privilege to rocksdb fail";
            IF_DONE_SET_RESPONSE(done, sirius::proto::INTERNAL_ERROR, "write db fail");
            return;
        }
        MELON_SCOPED_LOCK(_user_mutex);
        _user_privilege[username] = tmp_mem_privilege;
        IF_DONE_SET_RESPONSE(done, sirius::proto::SUCCESS, "success");
        LOG(INFO) << "drop privilege success, request:" << request.ShortDebugString();
    }


    int PrivilegeManager::load_snapshot() {
        _user_privilege.clear();
        std::string privilege_prefix = DiscoveryConstants::PRIVILEGE_IDENTIFY;
        mizar::ReadOptions read_options;
        read_options.prefix_same_as_start = true;
        read_options.total_order_seek = false;
        RocksStorage *db = RocksStorage::get_instance();
        std::unique_ptr<mizar::Iterator> iter(
                db->new_iterator(read_options, db->get_meta_info_handle()));
        iter->Seek(privilege_prefix);
        for (; iter->Valid(); iter->Next()) {
            std::string username(iter->key().ToString(), privilege_prefix.size());
            sirius::proto::UserPrivilege user_privilege;
            if (!user_privilege.ParseFromString(iter->value().ToString())) {
                LOG(ERROR) << "parse from pb fail when load privilege snapshot, key:" << iter->key().data();
                return -1;
            }
            LOG(WARNING) << "user_privilege:" << user_privilege.ShortDebugString();
            MELON_SCOPED_LOCK(_user_mutex);
            _user_privilege[username] = user_privilege;
        }
        return 0;
    }

    void PrivilegeManager::insert_zone_privilege(const sirius::proto::PrivilegeZone &privilege_zone,
                                                 sirius::proto::UserPrivilege &mem_privilege) {
        bool whether_exist = false;
        for (auto &mem_zone: *mem_privilege.mutable_privilege_zone()) {
            if (mem_zone.zone_id() == privilege_zone.zone_id()) {
                whether_exist = true;

                if (privilege_zone.force()) {
                    mem_zone.set_zone_rw(privilege_zone.zone_rw());
                } else {
                    if (privilege_zone.zone_rw() > mem_zone.zone_rw()) {
                        mem_zone.set_zone_rw(privilege_zone.zone_rw());
                    }
                }
                break;
            }
        }
        if (!whether_exist) {
            sirius::proto::PrivilegeZone *ptr_zone = mem_privilege.add_privilege_zone();
            *ptr_zone = privilege_zone;
        }
    }

    void PrivilegeManager::insert_servlet_privilege(const sirius::proto::PrivilegeServlet &privilege_servlet,
                                                    sirius::proto::UserPrivilege &mem_privilege) {
        bool whether_exist = false;
        int64_t zone_id = privilege_servlet.zone_id();
        int64_t servlet_id = privilege_servlet.servlet_id();
        for (auto &mem_privilege_servlet: *mem_privilege.mutable_privilege_servlet()) {
            if (mem_privilege_servlet.zone_id() == zone_id
                && mem_privilege_servlet.servlet_id() == servlet_id) {
                whether_exist = true;

                if (privilege_servlet.force()) {
                    mem_privilege_servlet.set_servlet_rw(privilege_servlet.servlet_rw());
                } else {
                    if (privilege_servlet.servlet_rw() > mem_privilege_servlet.servlet_rw()) {
                        mem_privilege_servlet.set_servlet_rw(privilege_servlet.servlet_rw());
                    }
                }

                break;
            }
        }
        if (!whether_exist) {
            sirius::proto::PrivilegeServlet *ptr_table = mem_privilege.add_privilege_servlet();
            *ptr_table = privilege_servlet;
        }
    }


    void PrivilegeManager::insert_ip(const std::string &ip,
                                     sirius::proto::UserPrivilege &mem_privilege) {
        bool whether_exist = false;
        for (auto &mem_ip: mem_privilege.ip()) {
            if (mem_ip == ip) {
                whether_exist = true;
            }
        }
        if (!whether_exist) {
            mem_privilege.add_ip(ip);
        }
    }

    void PrivilegeManager::delete_zone_privilege(const sirius::proto::PrivilegeZone &privilege_zone,
                                                 sirius::proto::UserPrivilege &mem_privilege) {
        sirius::proto::UserPrivilege copy_mem_privilege = mem_privilege;
        mem_privilege.clear_privilege_zone();
        for (auto &copy_zone: copy_mem_privilege.privilege_zone()) {
            if (copy_zone.zone_id() == privilege_zone.zone_id()) {
                //收回写权限
                if (privilege_zone.has_zone_rw() &&
                    privilege_zone.zone_rw() < copy_zone.zone_rw()) {
                    auto add_zone = mem_privilege.add_privilege_zone();
                    *add_zone = privilege_zone;
                }
            } else {
                auto add_zone = mem_privilege.add_privilege_zone();
                *add_zone = copy_zone;

            }
        }
    }

    void PrivilegeManager::delete_servlet_privilege(const sirius::proto::PrivilegeServlet &privilege_servlet,
                                                    sirius::proto::UserPrivilege &mem_privilege) {
        int64_t zone_id = privilege_servlet.zone_id();
        int64_t servlet_id = privilege_servlet.servlet_id();
        sirius::proto::UserPrivilege copy_mem_privilege = mem_privilege;
        mem_privilege.clear_privilege_servlet();
        for (auto &copy_servlet: copy_mem_privilege.privilege_servlet()) {
            if (zone_id == copy_servlet.zone_id() && servlet_id == copy_servlet.servlet_id()) {
                if (privilege_servlet.has_servlet_rw() &&
                    privilege_servlet.servlet_rw() < copy_servlet.servlet_rw()) {
                    auto add_servlet = mem_privilege.add_privilege_servlet();
                    *add_servlet = privilege_servlet;
                }
            } else {
                auto add_servlet = mem_privilege.add_privilege_servlet();
                *add_servlet = copy_servlet;
            }
        }
    }

    void PrivilegeManager::delete_ip(const std::string &ip,
                                     sirius::proto::UserPrivilege &mem_privilege) {
        sirius::proto::UserPrivilege copy_mem_privilege = mem_privilege;
        mem_privilege.clear_ip();
        for (auto &copy_ip: copy_mem_privilege.ip()) {
            if (copy_ip != ip) {
                mem_privilege.add_ip(copy_ip);
            }
        }
    }

}  // namespace sirius::discovery
