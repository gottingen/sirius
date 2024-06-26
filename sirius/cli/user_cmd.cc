// Copyright 2023 The Turbo Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include <sirius/cli/user_cmd.h>
#include <sirius/cli/option_context.h>
#include <sirius/base/log.h>
#include <sirius/cli/router_interact.h>
#include <sirius/proto/discovery.interface.pb.h>
#include <sirius/cli/show_help.h>
#include <collie/strings/str_split.h>
#include <sirius/cli/validator.h>
#include <collie/strings/format.h>
namespace sirius::cli {
    /// Set up a subcommand and capture a shared_ptr to a struct that holds all its options.
    /// The variables of the struct are bound to the CLI options.
    /// We use a shared ptr so that the addresses of the variables remain for binding,
    /// You could return the shared pointer if you wanted to access the values in main.
    void setup_user_cmd(collie::App &app) {
        // Create the option and subcommand objects.
        auto opt = UserOptionContext::get_instance();
        auto *ns = app.add_subcommand("user", "user privilege operations");
        ns->callback([ns]() { run_user_cmd(ns); });
        // Add options to sub, binding them to opt.
        //ns->require_subcommand();
        // add sub cmd
        auto cdb = ns->add_subcommand("create", " create user");
        cdb->add_option("-n,--app", opt->app_name, "app name")->required();
        cdb->add_option("-u,--user", opt->user_name, "user name")->required();
        cdb->add_option("-p,--passwd", opt->user_passwd, "user name")->required();
        cdb->callback([]() { run_user_create_cmd(); });

        auto rdb = ns->add_subcommand("remove", " remove user");
        rdb->add_option("-n,--app", opt->app_name, "app name")->required();
        rdb->add_option("-u,--user", opt->user_name, "zone name")->required();
        rdb->add_option("-p,--passwd", opt->user_passwd, "user name")->required();
        rdb->callback([]() { run_user_remove_cmd(); });


        auto add_privilege = ns->add_subcommand("assign", " add user privilege");
        auto *add_option = add_privilege->add_option_group("inputs", "privilege input source");
        add_option->add_option("-i,--ip", opt->user_ips, "user access ip");
        add_option->add_option("-s,--read_servlet", opt->user_rs,
                               "user read able servlet privilege format:zone.servlet");
        add_option->add_option("-S,--write_servlet", opt->user_ws,
                               "user read and write servlet privilege format:zone.servlet");
        add_option->add_option("-z,--read_zone", opt->user_rz, "user read able zone privilege");
        add_option->add_option("-Z,--write_zone", opt->user_wz, "user read and write zone privilege");
        add_option->require_option(1);

        add_privilege->add_option("-n,--app", opt->app_name, "app name")->required();
        add_privilege->add_option("-u,--user", opt->user_name, "user name")->required();
        add_privilege->add_option("-p,--passwd", opt->user_passwd, "user passwd");
        add_privilege->add_option("-f,--force", opt->force, "user passwd")->default_val(false);
        add_privilege->callback([]() { run_user_add_privilege_cmd(); });

        auto remove_privilege = ns->add_subcommand("deassign", " remove user privilege");
        auto *remove_option = remove_privilege->add_option_group("remove_p", "privilege input source");
        remove_option->add_option("-i,--ip", opt->user_ips, "user access ip");
        remove_option->add_option("-s,--read_servlet", opt->user_rs,
                                  "user read able servlet privilege format:zone.servlet");
        remove_option->add_option("-S,--write_servlet", opt->user_ws,
                                  "user read and write servlet privilege format:zone.servlet");
        remove_option->add_option("-z,--read_zone", opt->user_rz, "user read able zone privilege");
        remove_option->add_option("-Z,--write_zone", opt->user_wz, "user read and write zone privilege");
        remove_option->require_option(1);

        remove_privilege->add_option("-n,--app", opt->app_name, "app name")->required();
        remove_privilege->add_option("-u,--user", opt->user_name, "user name")->required();
        remove_privilege->add_option("-p,--passwd", opt->user_passwd, "user passwd");
        remove_privilege->add_option("-f,--force", opt->force, "user passwd")->default_val(false);
        remove_privilege->callback([]() { run_user_remove_privilege_cmd(); });

        auto lns = ns->add_subcommand("list", " list namespaces");
        lns->callback([]() { run_user_list_cmd(); });

        auto fu = ns->add_subcommand("flat", " flat get all user info");
        fu->callback([]() { run_user_flat_cmd(); });

        auto idb = ns->add_subcommand("info", " get user info");
        idb->add_option("-n,--app", opt->app_name, "app name")->required();
        idb->add_option("-u,--user", opt->user_name, "user name")->required();
        idb->add_option("-p,--passwd", opt->user_passwd, "user name");
        idb->add_flag("-s,--show", opt->show_pwd, "show passwd");
        idb->callback([]() { run_user_info_cmd(); });

    }

    /// The function that runs our code.
    /// This could also simply be in the callback lambda itself,
    /// but having a separate function is cleaner.
    void run_user_cmd(collie::App *app) {
        // Do stuff...
        if (app->get_subcommands().empty()) {
            collie::println("{}", app->help());
        }
    }

    void run_user_create_cmd() {
        collie::println(collie::Color::green, "start to create user: {}", UserOptionContext::get_instance()->user_name);
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        ScopeShower ss;
        auto rs = make_user_create(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = RouterInteract::get_instance()->send_request("discovery_manager", request, response);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(OptionContext::get_instance()->router_server, response.errcode(),
                                               request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table));
    }

    void run_user_remove_cmd() {
        collie::println(collie::Color::green, "start to remove user: {}",
                       UserOptionContext::get_instance()->app_name);
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        ScopeShower ss;
        auto rs = make_user_remove(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = RouterInteract::get_instance()->send_request("discovery_manager", request, response);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(OptionContext::get_instance()->router_server, response.errcode(),
                                               request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table));
    }

    void run_user_add_privilege_cmd() {
        collie::println(collie::Color::green, "start to add user privilege: {}",
                       UserOptionContext::get_instance()->user_name);
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        ScopeShower ss;
        auto rs = make_user_add_privilege(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = RouterInteract::get_instance()->send_request("discovery_manager", request, response);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(OptionContext::get_instance()->router_server, response.errcode(),
                                               request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table));
    }

    void run_user_remove_privilege_cmd() {
        collie::println(collie::Color::green, "start to remove user privilege: {}",
                       UserOptionContext::get_instance()->user_name);
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        ScopeShower ss;
        auto rs = make_user_remove_privilege(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = RouterInteract::get_instance()->send_request("discovery_manager", request, response);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(OptionContext::get_instance()->router_server, response.errcode(),
                                               request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table));
    }

    void run_user_list_cmd() {
        collie::println(collie::Color::green, "start to get user list");
        sirius::proto::DiscoveryQueryRequest request;
        sirius::proto::DiscoveryQueryResponse response;
        ScopeShower ss;
        auto rs = make_user_list(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = RouterInteract::get_instance()->send_request("discovery_query", request, response);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(OptionContext::get_instance()->router_server, response.errcode(),
                                               request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table));
        if (response.errcode() != sirius::proto::SUCCESS) {
            return;
        }
        table = show_discovery_query_user_response(response);
        ss.add_table("summary", std::move(table));
    }

    void run_user_flat_cmd() {
        collie::println(collie::Color::green, "start to get user list flatten");
        sirius::proto::DiscoveryQueryRequest request;
        sirius::proto::DiscoveryQueryResponse response;
        ScopeShower ss;
        auto rs = make_user_flat(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = RouterInteract::get_instance()->send_request("discovery_query", request, response);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(OptionContext::get_instance()->router_server, response.errcode(),
                                               request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table));
        if (response.errcode() != sirius::proto::SUCCESS) {
            return;
        }
        table = show_discovery_query_user_flat_response(response);
        ss.add_table("summary", std::move(table));
    }

    void run_user_info_cmd() {
        collie::println(collie::Color::green, "start to get user info");
        sirius::proto::DiscoveryQueryRequest request;
        sirius::proto::DiscoveryQueryResponse response;
        ScopeShower ss;
        auto rs = make_user_info(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = RouterInteract::get_instance()->send_request("discovery_query", request, response);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(OptionContext::get_instance()->router_server, response.errcode(),
                                               request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table));
        if (response.errcode() != sirius::proto::SUCCESS) {
            return;
        }
        table = show_discovery_query_user_response(response);
        ss.add_table("summary", std::move(table));
    }

    collie::table::Table show_discovery_query_user_response(const sirius::proto::DiscoveryQueryResponse &res) {

        auto &users = res.user_privilege();
        collie::table::Table summary;
        summary.add_row({"app", "user", "version", "passwd", "allow access ip", "zone", "servlet"});
        for (auto &user: users) {
            std::string passwd = "******";
            collie::table::Table ip_table;
            for (auto ip: user.ip()) {
                ip_table.add_row({ip});
            }
            collie::table::Table zone_table;
            for (auto zp: user.privilege_zone()) {
                zone_table.add_row(
                        {collie::format("{}:{} {}", zp.zone(), sirius::proto::RW_Name(zp.zone_rw()), zp.force())});
            }

            collie::table::Table servlet_table;
            for (auto sp: user.privilege_servlet()) {
                servlet_table.add_row({collie::format("{}.{}:{} {}", sp.zone(), sp.servlet_name(),
                                                     sirius::proto::RW_Name(sp.servlet_rw()), sp.force())});
            }

            if (UserOptionContext::get_instance()->show_pwd) {
                passwd = user.password();
            }
            summary.add_row(
                    collie::table::Table::Row_t{user.app_name(), user.username(), collie::format("{}", user.version()), passwd,
                                        ip_table,
                                        zone_table,
                                        servlet_table
                    });
            auto last = summary.size() - 1;
            summary[last].format().font_color(collie::Color::green);
        }
        return summary;
    }

    collie::table::Table show_discovery_query_user_flat_response(const sirius::proto::DiscoveryQueryResponse &res) {
        collie::table::Table summary;
        auto &users = res.flatten_privileges();

        summary.add_row({"app", "user", "privilege", "servlet_rw", "password"});
        for (auto &user: users) {
            summary.add_row(
                    collie::table::Table::Row_t{user.app_name(), user.username(), user.privilege(),
                                        sirius::proto::RW_Name(user.servlet_rw()), "******"});
            auto last = summary.size() - 1;
            summary[last].format().font_color(collie::Color::green);
        }

        return summary;
    }

    turbo::Status make_user_create(sirius::proto::DiscoveryManagerRequest *req) {
        sirius::proto::UserPrivilege *user_req = req->mutable_user_privilege();
        req->set_op_type(sirius::proto::OP_CREATE_USER);
        auto rs = check_valid_name_type(UserOptionContext::get_instance()->app_name);
        if (!rs.ok()) {
            return rs;
        }
        rs = check_valid_name_type(UserOptionContext::get_instance()->user_name);
        if (!rs.ok()) {
            return rs;
        }
        user_req->set_app_name(UserOptionContext::get_instance()->app_name);
        user_req->set_username(UserOptionContext::get_instance()->user_name);
        user_req->set_password(UserOptionContext::get_instance()->user_passwd);
        return turbo::OkStatus();
    }

    turbo::Status make_user_remove(sirius::proto::DiscoveryManagerRequest *req) {
        sirius::proto::UserPrivilege *user_req = req->mutable_user_privilege();
        req->set_op_type(sirius::proto::OP_DROP_USER);
        auto rs = check_valid_name_type(UserOptionContext::get_instance()->app_name);
        if (!rs.ok()) {
            return rs;
        }
        rs = check_valid_name_type(UserOptionContext::get_instance()->user_name);
        if (!rs.ok()) {
            return rs;
        }
        user_req->set_app_name(UserOptionContext::get_instance()->app_name);
        user_req->set_username(UserOptionContext::get_instance()->user_name);
        user_req->set_password(UserOptionContext::get_instance()->user_passwd);
        return turbo::OkStatus();
    }

    turbo::Status make_user_add_privilege(sirius::proto::DiscoveryManagerRequest *req) {
        req->set_op_type(sirius::proto::OP_ADD_PRIVILEGE);
        auto opt = UserOptionContext::get_instance();
        sirius::proto::UserPrivilege *pri_req = req->mutable_user_privilege();
        auto rs = check_valid_name_type(opt->app_name);
        if (!rs.ok()) {
            return rs;
        }
        rs = check_valid_name_type(opt->user_name);
        if (!rs.ok()) {
            return rs;
        }

        pri_req->set_app_name(opt->app_name);
        pri_req->set_username(opt->user_name);
        for (auto ip: opt->user_ips) {
            pri_req->add_ip(ip);
        }

        // for zone
        std::set<std::string> read_set;
        for (auto &read_zone: opt->user_rz) {
            read_set.insert(read_zone);
        }

        for (auto &write_zone: opt->user_wz) {
            read_set.erase(write_zone);
            sirius::proto::PrivilegeZone pz;
            pz.set_zone(write_zone);
            pz.set_zone_rw(sirius::proto::WRITE);
            pz.set_force(opt->force);
            *pri_req->add_privilege_zone() = pz;
        }

        for (auto &read_zone: read_set) {
            sirius::proto::PrivilegeZone pz;
            pz.set_zone(read_zone);
            pz.set_zone_rw(sirius::proto::READ);
            pz.set_force(opt->force);
            *pri_req->add_privilege_zone() = pz;
        }

        read_set.clear();
        for (auto &read_servlet: opt->user_rs) {
            read_set.insert(read_servlet);
        }

        for (auto &write_servlet: opt->user_ws) {
            read_set.erase(write_servlet);
            sirius::proto::PrivilegeServlet ps;
            std::vector<std::string> names = collie::str_split(write_servlet, ".", collie::SkipEmpty());
            if (names.size() != 2) {
                return turbo::invalid_argument_error(collie::format("bad format of {} should be zone.servlet", write_servlet));
            }
            ps.set_zone(names[0]);
            ps.set_servlet_name(names[1]);
            ps.set_servlet_rw(sirius::proto::WRITE);
            ps.set_force(opt->force);
            *pri_req->add_privilege_servlet() = ps;
        }

        for (auto &read_servlet: read_set) {
            sirius::proto::PrivilegeServlet ps;
            std::vector<std::string> names = collie::str_split(read_servlet, ".", collie::SkipEmpty());
            if (names.size() != 2) {
                return turbo::invalid_argument_error(collie::format("bad format of {} should be zone.servlet", read_servlet));
            }
            ps.set_zone(names[0]);
            ps.set_servlet_name(names[1]);
            ps.set_servlet_rw(sirius::proto::READ);
            ps.set_force(opt->force);
            *pri_req->add_privilege_servlet() = ps;
        }
        return turbo::OkStatus();
    }

    turbo::Status make_user_remove_privilege(sirius::proto::DiscoveryManagerRequest *req) {
        req->set_op_type(sirius::proto::OP_DROP_PRIVILEGE);
        auto opt = UserOptionContext::get_instance();
        sirius::proto::UserPrivilege *pri_req = req->mutable_user_privilege();
        auto rs = check_valid_name_type(opt->app_name);
        if (!rs.ok()) {
            return rs;
        }
        rs = check_valid_name_type(opt->user_name);
        if (!rs.ok()) {
            return rs;
        }

        pri_req->set_app_name(opt->app_name);
        pri_req->set_username(opt->user_name);
        for (auto ip: opt->user_ips) {
            pri_req->add_ip(ip);
        }

        // for zone
        std::set<std::string> read_set;
        for (auto &rz: opt->user_rz) {
            read_set.insert(rz);
        }

        for (auto &write_zone: opt->user_wz) {
            read_set.erase(write_zone);
            sirius::proto::PrivilegeZone pz;
            pz.set_zone(write_zone);
            pz.set_zone_rw(sirius::proto::WRITE);
            pz.set_force(opt->force);
            *pri_req->add_privilege_zone() = pz;
        }

        for (auto &read_zone: read_set) {
            sirius::proto::PrivilegeZone pz;
            pz.set_zone(read_zone);
            pz.set_zone_rw(sirius::proto::READ);
            pz.set_force(opt->force);
            *pri_req->add_privilege_zone() = pz;
        }

        read_set.clear();
        for (auto &read_servlet: opt->user_rs) {
            read_set.insert(read_servlet);
        }

        for (auto &write_servlet: opt->user_ws) {
            read_set.erase(write_servlet);
            sirius::proto::PrivilegeServlet ps;
            std::vector<std::string> names = collie::str_split(write_servlet, ".", collie::SkipEmpty());
            if (names.size() != 2) {
                return turbo::invalid_argument_error(collie::format("bad format of {} should be zone.servlet", write_servlet));
            }
            ps.set_zone(names[0]);
            ps.set_servlet_name(names[1]);
            ps.set_servlet_rw(sirius::proto::WRITE);
            ps.set_force(opt->force);
            *pri_req->add_privilege_servlet() = ps;
        }

        for (auto &read_servlet: read_set) {
            sirius::proto::PrivilegeServlet ps;
            std::vector<std::string> names = collie::str_split(read_servlet, ".", collie::SkipEmpty());
            if (names.size() != 2) {
                return turbo::invalid_argument_error(collie::format("bad format of {} should be zone.servlet", read_servlet));
            }
            ps.set_zone(names[0]);
            ps.set_servlet_name(names[1]);
            ps.set_servlet_rw(sirius::proto::READ);
            ps.set_force(opt->force);
            *pri_req->add_privilege_servlet() = ps;
        }

        return turbo::OkStatus();
    }

    turbo::Status make_user_list(sirius::proto::DiscoveryQueryRequest *req) {
        req->set_op_type(sirius::proto::QUERY_USER_PRIVILEGE);
        return turbo::OkStatus();
    }

    turbo::Status make_user_flat(sirius::proto::DiscoveryQueryRequest *req) {
            req->set_op_type(sirius::proto::QUERY_PRIVILEGE_FLATTEN);

        return turbo::OkStatus();
    }

    turbo::Status make_user_info(sirius::proto::DiscoveryQueryRequest *req) {
        req->set_op_type(sirius::proto::QUERY_USER_PRIVILEGE);
        auto rs = check_valid_name_type(UserOptionContext::get_instance()->app_name);
        if (!rs.ok()) {
            return rs;
        }
        rs = check_valid_name_type(UserOptionContext::get_instance()->user_name);
        if (!rs.ok()) {
            return rs;
        }
        req->set_app_name(UserOptionContext::get_instance()->app_name);
        req->set_user_name(UserOptionContext::get_instance()->user_name);
        return turbo::OkStatus();
    }

}  // namespace sirius::cli
