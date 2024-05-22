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
#include <sirius/cli/namespace_cmd.h>
#include <sirius/cli/option_context.h>
#include <sirius/base/log.h>
#include <sirius/cli/router_interact.h>
#include <sirius/cli/show_help.h>
#include <sirius/cli/validator.h>

namespace sirius::cli {
    /// Set up a subcommand and capture a shared_ptr to a struct that holds all its options.
    /// The variables of the struct are bound to the CLI options.
    /// We use a shared ptr so that the addresses of the variables remain for binding,
    /// You could return the shared pointer if you wanted to access the values in main.
    void setup_namespace_cmd(collie::App &app) {
        // Create the option and subcommand objects.
        auto opt = NameSpaceOptionContext::get_instance();
        auto *ns = app.add_subcommand("namespace", "namespace operations");
        ns->callback([ns]() { run_namespace_cmd(ns); });
        // Add options to sub, binding them to opt.
        //ns->require_subcommand();
        // add sub cmd
        auto cns = ns->add_subcommand("create", " create namespace");
        cns->add_option("-n,--name", opt->namespace_name, "namespace name")->required();
        cns->add_option("-q, --quota", opt->namespace_quota, "new namespace quota");
        cns->callback([]() { run_ns_create_cmd(); });

        auto rns = ns->add_subcommand("remove", " remove namespace");
        rns->add_option("-n,--name", opt->namespace_name, "namespace name")->required();
        rns->add_option("-q, --quota", opt->namespace_quota, "new namespace quota");
        rns->callback([]() { run_ns_remove_cmd(); });

        auto mns = ns->add_subcommand("modify", " modify namespace");
        mns->add_option("-n,--name", opt->namespace_name, "namespace name")->required();
        mns->add_option("-q, --quota", opt->namespace_quota, "new namespace quota");
        mns->callback([]() { run_ns_modify_cmd(); });

        auto lns = ns->add_subcommand("list", " list namespaces");
        lns->callback([]() { run_ns_list_cmd(); });

        auto ins = ns->add_subcommand("info", " get namespace info");
        ins->add_option("-n,--name", opt->namespace_name, "namespace name")->required();
        ins->callback([]() { run_ns_info_cmd(); });

    }

    /// The function that runs our code.
    /// This could also simply be in the callback lambda itself,
    /// but having a separate function is cleaner.
    void run_namespace_cmd(collie::App *app) {
        // Do stuff...
        if (app->get_subcommands().empty()) {
            collie::println("{}", app->help());
        }
    }

    void run_ns_create_cmd() {
        collie::println(collie::Color::green, "start to create namespace: {}",
                       NameSpaceOptionContext::get_instance()->namespace_name);
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        auto rs = make_namespace_create(&request);
        ScopeShower ss;
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = RouterInteract::get_instance()->send_request("discovery_manager", request, response);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(OptionContext::get_instance()->router_server, response.errcode(), response.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table));
    }

    void run_ns_remove_cmd() {
        collie::println(collie::Color::green, "start to remove namespace: {}",
                       NameSpaceOptionContext::get_instance()->namespace_name);
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        ScopeShower ss;
        auto rs = make_namespace_remove(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = RouterInteract::get_instance()->send_request("discovery_manager", request, response);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(OptionContext::get_instance()->router_server, response.errcode(), response.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table));
    }

    void run_ns_modify_cmd() {
        collie::println(collie::Color::green, "start to modify namespace: {}",
                       NameSpaceOptionContext::get_instance()->namespace_name);
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        ScopeShower ss;
        auto rs = make_namespace_modify(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = RouterInteract::get_instance()->send_request("discovery_manager", request, response);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(OptionContext::get_instance()->router_server, response.errcode(), response.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table));
    }

    void run_ns_list_cmd() {
        collie::println(collie::Color::green, "start to get namespace list");
        sirius::proto::DiscoveryQueryRequest request;
        sirius::proto::DiscoveryQueryResponse response;
        ScopeShower ss;
        auto rs = make_namespace_query(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = RouterInteract::get_instance()->send_request("discovery_query", request, response);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(OptionContext::get_instance()->router_server, response.errcode(), request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table));

        if(response.errcode() != sirius::proto::SUCCESS) {
            return;
        }
        table = show_discovery_query_ns_response(response);
        ss.add_table("summary", std::move(table));
    }

    void run_ns_info_cmd() {
        collie::println(collie::Color::green, "start to get namespace info");
        sirius::proto::DiscoveryQueryRequest request;
        sirius::proto::DiscoveryQueryResponse response;
        ScopeShower ss;
        auto rs = make_namespace_query(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = RouterInteract::get_instance()->send_request("discovery_query", request, response);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(OptionContext::get_instance()->router_server, response.errcode(), request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table));
        if(response.errcode() != sirius::proto::SUCCESS) {
            return;
        }
        table = show_discovery_query_ns_response(response);
        ss.add_table("summary", std::move(table));
    }

    collie::table::Table show_discovery_query_ns_response(const sirius::proto::DiscoveryQueryResponse &res) {
        collie::table::Table result;
        auto &nss = res.namespace_infos();
        result.add_row(
                collie::table::Table::Row_t{"namespace", "id", "version", "quota", "replica number", "resource tag", "region split lines"});
        auto last = result.size() - 1;
        result[last].format().font_color(collie::Color::green);


        for (auto &ns: nss) {
            result.add_row(
                    collie::table::Table::Row_t{ns.namespace_name(),
                          collie::to_str(ns.namespace_id()),
                          collie::to_str(ns.version()),
                          collie::to_str(ns.quota()),
                          collie::to_str(ns.replica_num()),
                          ns.resource_tag(),
                          collie::to_str(ns.region_split_lines())});
            last = result.size() - 1;
            result[last].format().font_color(collie::Color::green);
        }
        return result;
    }

    collie::Status
    make_namespace_create(sirius::proto::DiscoveryManagerRequest *req) {
        sirius::proto::NameSpaceInfo *ns_req = req->mutable_namespace_info();
        auto rs = check_valid_name_type(NameSpaceOptionContext::get_instance()->namespace_name);
        if (!rs.ok()) {
            return rs;
        }
        ns_req->set_namespace_name(NameSpaceOptionContext::get_instance()->namespace_name);
        ns_req->set_quota(NameSpaceOptionContext::get_instance()->namespace_quota);
        req->set_op_type(sirius::proto::OP_CREATE_NAMESPACE);
        return collie::Status::ok_status();
    }

    collie::Status
    make_namespace_remove(sirius::proto::DiscoveryManagerRequest *req) {
        sirius::proto::NameSpaceInfo *ns_req = req->mutable_namespace_info();
        auto rs = check_valid_name_type(NameSpaceOptionContext::get_instance()->namespace_name);
        if (!rs.ok()) {
            return rs;
        }
        ns_req->set_namespace_name(NameSpaceOptionContext::get_instance()->namespace_name);
        req->set_op_type(sirius::proto::OP_DROP_NAMESPACE);
        return collie::Status::ok_status();
    }

    collie::Status
    make_namespace_modify(sirius::proto::DiscoveryManagerRequest *req) {
        sirius::proto::NameSpaceInfo *ns_req = req->mutable_namespace_info();
        auto rs = check_valid_name_type(NameSpaceOptionContext::get_instance()->namespace_name);
        if (!rs.ok()) {
            return rs;
        }
        ns_req->set_namespace_name(NameSpaceOptionContext::get_instance()->namespace_name);
        ns_req->set_quota(NameSpaceOptionContext::get_instance()->namespace_quota);
        req->set_op_type(sirius::proto::OP_MODIFY_NAMESPACE);
        return collie::Status::ok_status();
    }

    collie::Status make_namespace_query(sirius::proto::DiscoveryQueryRequest *req) {
        req->set_op_type(sirius::proto::QUERY_NAMESPACE);
        if (!NameSpaceOptionContext::get_instance()->namespace_name.empty()) {
            req->set_namespace_name(NameSpaceOptionContext::get_instance()->namespace_name);
            auto rs = check_valid_name_type(NameSpaceOptionContext::get_instance()->namespace_name);
            if (!rs.ok()) {
                return rs;
            }
        }
        return collie::Status::ok_status();
    }

}  // namespace sirius::cli
