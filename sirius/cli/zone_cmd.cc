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
#include <sirius/cli/zone_cmd.h>
#include <sirius/cli/option_context.h>
#include <sirius/base/log.h>
#include <sirius/cli/router_interact.h>
#include <sirius/proto/discovery.interface.pb.h>
#include <sirius/cli/show_help.h>
#include <collie/table/table.h>
#include <collie/strings/format.h>
#include <sirius/cli/validator.h>

namespace sirius::cli {
    /// Set up a subcommand and capture a shared_ptr to a struct that holds all its options.
    /// The variables of the struct are bound to the CLI options.
    /// We use a shared ptr so that the addresses of the variables remain for binding,
    /// You could return the shared pointer if you wanted to access the values in main.
    void setup_zone_cmd(collie::App &app) {
        // Create the option and subcommand objects.
        auto opt = ZoneOptionContext::get_instance();
        auto *ns = app.add_subcommand("zone", "zone operations");
        ns->callback([ns]() { run_zone_cmd(ns); });
        // Add options to sub, binding them to opt.
        //ns->require_subcommand();
        // add sub cmd
        auto cdb = ns->add_subcommand("create", " create zone");
        cdb->add_option("-n,--namespace", opt->namespace_name, "namespace name")->required();
        cdb->add_option("-z,--zone", opt->zone_name, "zone name")->required();
        cdb->add_option("-q, --quota", opt->namespace_quota, "new namespace quota");
        cdb->callback([]() { run_zone_create_cmd(); });

        auto rdb = ns->add_subcommand("remove", " remove zone");
        rdb->add_option("-n,--namespace", opt->namespace_name, "namespace name")->required();
        rdb->add_option("-z,--zone", opt->zone_name, "zone name")->required();
        rdb->callback([]() { run_zone_remove_cmd(); });

        auto mdb = ns->add_subcommand("modify", " modify zone");
        mdb->add_option("-n,--namespace", opt->namespace_name, "namespace name")->required();
        mdb->add_option("-z,--zone", opt->zone_name, "zone name")->required();
        mdb->add_option("-q, --quota", opt->namespace_quota, "new namespace quota");
        mdb->callback([]() { run_zone_modify_cmd(); });

        auto lns = ns->add_subcommand("list", " list namespaces");
        lns->callback([]() { run_zone_list_cmd(); });

        auto idb = ns->add_subcommand("info", " get zone info");
        idb->add_option("-n,--namespace", opt->namespace_name, "namespace name")->required();
        idb->add_option("-z,--zone", opt->zone_name, "zone name")->required();
        idb->callback([]() { run_zone_info_cmd(); });

    }

    /// The function that runs our code.
    /// This could also simply be in the callback lambda itself,
    /// but having a separate function is cleaner.
    void run_zone_cmd(collie::App* app) {
        // Do stuff...
        if(app->get_subcommands().empty()) {
            collie::println("{}", app->help());
        }
    }

    void run_zone_create_cmd() {
        collie::println(collie::Color::green, "start to create namespace: {}", ZoneOptionContext::get_instance()->namespace_name);
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        ScopeShower ss;
        auto rs= make_zone_create(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = RouterInteract::get_instance()->send_request("discovery_manager", request, response);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(OptionContext::get_instance()->router_server, response.errcode(), request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table));
    }
    void run_zone_remove_cmd() {
        collie::println(collie::Color::green, "start to remove namespace: {}", ZoneOptionContext::get_instance()->namespace_name);
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        ScopeShower ss;
        auto rs = make_zone_remove(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = RouterInteract::get_instance()->send_request("discovery_manager", request, response);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(OptionContext::get_instance()->router_server, response.errcode(), request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table));
    }
    void run_zone_modify_cmd() {
        collie::println(collie::Color::green, "start to modify namespace: {}", ZoneOptionContext::get_instance()->namespace_name);
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        ScopeShower ss;
        auto rs = make_zone_modify(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = RouterInteract::get_instance()->send_request("discovery_manager", request, response);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(OptionContext::get_instance()->router_server, response.errcode(), request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table));
    }

    void run_zone_list_cmd() {
        collie::println(collie::Color::green, "start to get zone list");
        sirius::proto::DiscoveryQueryRequest request;
        sirius::proto::DiscoveryQueryResponse response;
        ScopeShower ss;
        auto rs = make_zone_list(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = RouterInteract::get_instance()->send_request("discovery_query", request, response);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(OptionContext::get_instance()->router_server, response.errcode(), request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table));
        if(response.errcode() != sirius::proto::SUCCESS) {
            return;
        }
        table = show_discovery_query_zone_response(response);
        ss.add_table("summary", std::move(table));
    }

    void run_zone_info_cmd() {
        collie::println(collie::Color::green, "start to get zone list");
        sirius::proto::DiscoveryQueryRequest request;
        sirius::proto::DiscoveryQueryResponse response;
        ScopeShower ss;
        auto rs = make_zone_info(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = RouterInteract::get_instance()->send_request("discovery_query", request, response);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(OptionContext::get_instance()->router_server, response.errcode(), request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table));
        if(response.errcode() != sirius::proto::SUCCESS) {
            return;
        }
        table = show_discovery_query_zone_response(response);
        ss.add_table("summary", std::move(table));
    }

    collie::table::Table show_discovery_query_zone_response(const sirius::proto::DiscoveryQueryResponse &res) {
        auto &zones = res.zone_infos();
        collie::table::Table summary;
        summary.add_row({"namespace", "zone", "id", "version", "quota", "replica number", "resource tag",
                                "region split lines"});
        for (auto &zone: zones) {
            summary.add_row(
                    collie::table::Table::Row_t{zone.namespace_name(), zone.zone(), collie::format("{}", zone.zone_id()),
                                        collie::format("{}", zone.version()),
                                        collie::format("{}", zone.quota()), collie::format("{}", zone.replica_num()), zone.resource_tag(),
                                        collie::format("{}", zone.region_split_lines())});
            auto last = summary.size() - 1;
            summary[last].format().font_color(collie::Color::green);
        }
        return summary;
    }

    collie::Status make_zone_create(sirius::proto::DiscoveryManagerRequest *req) {
        sirius::proto::ZoneInfo *zone_req = req->mutable_zone_info();
        req->set_op_type(sirius::proto::OP_CREATE_ZONE);
        auto rs = check_valid_name_type(ZoneOptionContext::get_instance()->namespace_name);
        if (!rs.ok()) {
            return rs;
        }
        rs = check_valid_name_type(ZoneOptionContext::get_instance()->zone_name);
        if (!rs.ok()) {
            return rs;
        }
        zone_req->set_namespace_name(ZoneOptionContext::get_instance()->namespace_name);
        zone_req->set_zone(ZoneOptionContext::get_instance()->zone_name);
        return collie::Status::ok_status();
    }

    collie::Status make_zone_remove(sirius::proto::DiscoveryManagerRequest *req) {
        sirius::proto::ZoneInfo *zone_req = req->mutable_zone_info();
        req->set_op_type(sirius::proto::OP_DROP_ZONE);
        auto rs = check_valid_name_type(ZoneOptionContext::get_instance()->namespace_name);
        if (!rs.ok()) {
            return rs;
        }
        rs = check_valid_name_type(ZoneOptionContext::get_instance()->zone_name);
        if (!rs.ok()) {
            return rs;
        }
        zone_req->set_namespace_name(ZoneOptionContext::get_instance()->namespace_name);
        zone_req->set_zone(ZoneOptionContext::get_instance()->zone_name);
        return collie::Status::ok_status();
    }

    collie::Status make_zone_modify(sirius::proto::DiscoveryManagerRequest *req) {
        req->set_op_type(sirius::proto::OP_MODIFY_ZONE);
        sirius::proto::ZoneInfo *zone_req = req->mutable_zone_info();
        auto rs = check_valid_name_type(ZoneOptionContext::get_instance()->namespace_name);
        if (!rs.ok()) {
            return rs;
        }
        rs = check_valid_name_type(ZoneOptionContext::get_instance()->zone_name);
        if (!rs.ok()) {
            return rs;
        }
        zone_req->set_namespace_name(ZoneOptionContext::get_instance()->namespace_name);
        zone_req->set_zone(ZoneOptionContext::get_instance()->zone_name);
        return collie::Status::ok_status();
    }

    collie::Status make_zone_list(sirius::proto::DiscoveryQueryRequest *req) {
        req->set_op_type(sirius::proto::QUERY_ZONE);
        return collie::Status::ok_status();
    }

    collie::Status make_zone_info(sirius::proto::DiscoveryQueryRequest *req) {
        req->set_op_type(sirius::proto::QUERY_ZONE);
        auto rs = check_valid_name_type(ZoneOptionContext::get_instance()->namespace_name);
        if (!rs.ok()) {
            return rs;
        }
        rs = check_valid_name_type(ZoneOptionContext::get_instance()->zone_name);
        if (!rs.ok()) {
            return rs;
        }
        req->set_namespace_name(ZoneOptionContext::get_instance()->namespace_name);
        req->set_zone(ZoneOptionContext::get_instance()->zone_name);
        return collie::Status::ok_status();
    }

}  // namespace sirius::cli
