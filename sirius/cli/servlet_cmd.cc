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
#include <sirius/cli/servlet_cmd.h>
#include <sirius/cli/option_context.h>
#include <sirius/base/log.h>
#include <sirius/cli/router_interact.h>
#include <sirius/proto/discovery.interface.pb.h>
#include <sirius/cli/show_help.h>
#include <sirius/cli/validator.h>
#include <collie/strings/format.h>

namespace sirius::cli {
    /// Set up a subcommand and capture a shared_ptr to a struct that holds all its options.
    /// The variables of the struct are bound to the CLI options.
    /// We use a shared ptr so that the addresses of the variables remain for binding,
    /// You could return the shared pointer if you wanted to access the values in main.
    void setup_servlet_cmd(collie::App &app) {
        // Create the option and subcommand objects.
        auto opt = ServletOptionContext::get_instance();
        auto *ns = app.add_subcommand("servlet", "servlet operations");
        ns->callback([ns]() { run_servlet_cmd(ns); });
        // Add options to sub, binding them to opt.
        //ns->require_subcommand();
        // add sub cmd
        auto cdb = ns->add_subcommand("create", " create servlet");
        cdb->add_option("-n,--app", opt->app_name, "app name")->required();
        cdb->add_option("-z,--zone", opt->zone_name, "zone name")->required();
        cdb->add_option("-s,--servlet", opt->servlet_name, "servlet name")->required();
        cdb->add_option("-q, --quota", opt->app_quota, "new app quota");
        cdb->callback([]() { run_servlet_create_cmd(); });

        auto rdb = ns->add_subcommand("remove", " remove servlet");
        rdb->add_option("-n,--app", opt->app_name, "app name")->required();
        rdb->add_option("-z,--zone", opt->zone_name, "zone name")->required();
        rdb->add_option("-s,--servlet", opt->servlet_name, "servlet name")->required();
        rdb->callback([]() { run_servlet_remove_cmd(); });

        auto mdb = ns->add_subcommand("modify", " modify servlet");
        mdb->add_option("-n,--app", opt->app_name, "app name")->required();
        mdb->add_option("-z,--zone", opt->zone_name, "zone name")->required();
        mdb->add_option("-s,--servlet", opt->servlet_name, "zone name")->required();
        mdb->add_option("-q, --quota", opt->app_quota, "new app quota");
        mdb->callback([]() { run_servlet_modify_cmd(); });

        auto lns = ns->add_subcommand("list", " list servlet");
        lns->callback([]() { run_servlet_list_cmd(); });

        auto idb = ns->add_subcommand("info", " get servlet info");
        idb->add_option("-n,--app", opt->app_name, "app name")->required();
        idb->add_option("-z,--zone", opt->zone_name, "zone name")->required();
        idb->add_option("-s,--servlet", opt->servlet_name, "servlet name")->required();
        idb->callback([]() { run_servlet_info_cmd(); });

    }

    /// The function that runs our code.
    /// This could also simply be in the callback lambda itself,
    /// but having a separate function is cleaner.
    void run_servlet_cmd(collie::App* app) {
        // Do stuff...
        if(app->get_subcommands().empty()) {
            collie::println("{}", app->help());
        }
    }

    void run_servlet_create_cmd() {
        collie::println(collie::Color::green, "start to create servlet: {}", ServletOptionContext::get_instance()->servlet_name);
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        ScopeShower ss;
        auto rs= make_servlet_create(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = RouterInteract::get_instance()->send_request("discovery_manager", request, response);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(OptionContext::get_instance()->router_server, response.errcode(), request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table));
    }
    void run_servlet_remove_cmd() {
        collie::println(collie::Color::green, "start to remove app: {}", ServletOptionContext::get_instance()->app_name);
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        ScopeShower ss;
        auto rs = make_servlet_remove(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = RouterInteract::get_instance()->send_request("discovery_manager", request, response);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(OptionContext::get_instance()->router_server, response.errcode(), request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table));
    }
    void run_servlet_modify_cmd() {
        collie::println(collie::Color::green, "start to modify app: {}", ServletOptionContext::get_instance()->app_name);
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        ScopeShower ss;
        auto rs = make_servlet_modify(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = RouterInteract::get_instance()->send_request("discovery_manager", request, response);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(OptionContext::get_instance()->router_server, response.errcode(), request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table));
    }

    void run_servlet_list_cmd() {
        collie::println(collie::Color::green, "start to get servlet list");
        sirius::proto::DiscoveryQueryRequest request;
        sirius::proto::DiscoveryQueryResponse response;
        ScopeShower ss;
        auto rs = make_servlet_list(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = RouterInteract::get_instance()->send_request("discovery_query", request, response);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(OptionContext::get_instance()->router_server, response.errcode(), request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table));
        if(response.errcode() != sirius::proto::SUCCESS) {
            return;
        }
        table = show_discovery_query_servlet_response(response);
        ss.add_table("summary", std::move(table));
    }

    void run_servlet_info_cmd() {
        collie::println(collie::Color::green, "start to get servlet list");
        sirius::proto::DiscoveryQueryRequest request;
        sirius::proto::DiscoveryQueryResponse response;
        ScopeShower ss;
        auto rs = make_servlet_info(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = RouterInteract::get_instance()->send_request("discovery_query", request, response);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(OptionContext::get_instance()->router_server, response.errcode(), request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table));
        if(response.errcode() != sirius::proto::SUCCESS) {
            return;
        }
        table = show_discovery_query_servlet_response(response);
        ss.add_table("summary", std::move(table));
    }

    collie::table::Table show_discovery_query_servlet_response(const sirius::proto::DiscoveryQueryResponse &res) {
        auto &servlets = res.servlet_infos();
        collie::table::Table sumary;
        sumary.add_row({"app", "zone", "servlet", "id"});
        for (auto &ns: servlets) {
            sumary.add_row(
                    collie::table::Table::Row_t{ns.app_name(), ns.zone(), ns.servlet_name(), collie::to_str(ns.servlet_id())});
            auto last = sumary.size() - 1;
            sumary[last].format().font_color(collie::Color::green);
        }
        return sumary;
    }

    turbo::Status make_servlet_create(sirius::proto::DiscoveryManagerRequest *req) {
        sirius::proto::ServletInfo *servlet_req = req->mutable_servlet_info();
        req->set_op_type(sirius::proto::OP_CREATE_SERVLET);
        auto rs = check_valid_name_type(ServletOptionContext::get_instance()->app_name);
        if (!rs.ok()) {
            return rs;
        }
        rs = check_valid_name_type(ServletOptionContext::get_instance()->zone_name);
        if (!rs.ok()) {
            return rs;
        }
        rs = check_valid_name_type(ServletOptionContext::get_instance()->servlet_name);
        if (!rs.ok()) {
            return rs;
        }
        servlet_req->set_app_name(ServletOptionContext::get_instance()->app_name);
        servlet_req->set_zone(ServletOptionContext::get_instance()->zone_name);
        servlet_req->set_servlet_name(ServletOptionContext::get_instance()->servlet_name);
        return turbo::OkStatus();
    }

    turbo::Status make_servlet_remove(sirius::proto::DiscoveryManagerRequest *req) {
        sirius::proto::ServletInfo *servlet_req = req->mutable_servlet_info();
        req->set_op_type(sirius::proto::OP_DROP_SERVLET);
        auto rs = check_valid_name_type(ServletOptionContext::get_instance()->app_name);
        if (!rs.ok()) {
            return rs;
        }
        rs = check_valid_name_type(ServletOptionContext::get_instance()->zone_name);
        if (!rs.ok()) {
            return rs;
        }
        rs = check_valid_name_type(ServletOptionContext::get_instance()->servlet_name);
        if (!rs.ok()) {
            return rs;
        }
        servlet_req->set_app_name(ServletOptionContext::get_instance()->app_name);
        servlet_req->set_zone(ServletOptionContext::get_instance()->zone_name);
        servlet_req->set_servlet_name(ServletOptionContext::get_instance()->servlet_name);
        return turbo::OkStatus();
    }

    turbo::Status make_servlet_modify(sirius::proto::DiscoveryManagerRequest *req) {
        req->set_op_type(sirius::proto::OP_MODIFY_SERVLET);
        sirius::proto::ServletInfo *servlet_req = req->mutable_servlet_info();
        auto rs = check_valid_name_type(ServletOptionContext::get_instance()->app_name);
        if (!rs.ok()) {
            return rs;
        }
        rs = check_valid_name_type(ServletOptionContext::get_instance()->zone_name);
        if (!rs.ok()) {
            return rs;
        }
        rs = check_valid_name_type(ServletOptionContext::get_instance()->servlet_name);
        if (!rs.ok()) {
            return rs;
        }
        servlet_req->set_app_name(ServletOptionContext::get_instance()->app_name);
        servlet_req->set_zone(ServletOptionContext::get_instance()->zone_name);
        servlet_req->set_servlet_name(ServletOptionContext::get_instance()->servlet_name);
        return turbo::OkStatus();
    }

    turbo::Status make_servlet_list(sirius::proto::DiscoveryQueryRequest *req) {
        req->set_op_type(sirius::proto::QUERY_SERVLET);
        return turbo::OkStatus();
    }

    turbo::Status make_servlet_info(sirius::proto::DiscoveryQueryRequest *req) {
        req->set_op_type(sirius::proto::QUERY_SERVLET);
        auto rs = check_valid_name_type(ServletOptionContext::get_instance()->app_name);
        if (!rs.ok()) {
            return rs;
        }
        rs = check_valid_name_type(ServletOptionContext::get_instance()->zone_name);
        if (!rs.ok()) {
            return rs;
        }
        rs = check_valid_name_type(ServletOptionContext::get_instance()->servlet_name);
        if (!rs.ok()) {
            return rs;
        }
        req->set_app_name(ServletOptionContext::get_instance()->app_name);
        req->set_zone(ServletOptionContext::get_instance()->zone_name);
        req->set_servlet(ServletOptionContext::get_instance()->servlet_name);

        req->set_app_name(ServletOptionContext::get_instance()->app_name);
        req->set_zone(ServletOptionContext::get_instance()->zone_name);

        return turbo::OkStatus();
    }

}  // namespace sirius::cli
