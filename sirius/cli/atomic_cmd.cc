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

//
// Created by jeff on 23-11-29.
//
#include <sirius/cli/atomic_cmd.h>
#include <sirius/cli/option_context.h>
#include <sirius/cli/show_help.h>
#include <sirius/cli/router_interact.h>
#include <alkaid/files/filesystem.h>
#include <turbo/times/clock.h>
#include <melon/json2pb/pb_to_json.h>
#include <melon/json2pb/json_to_pb.h>
#include <sirius/client/discovery.h>
#include <sirius/client/config_info_builder.h>
#include <collie/nlohmann/json.hpp>
#include <collie/strings/format.h>


namespace sirius::cli {

    void AtomicCmd::setup_atomic_cmd(collie::App &app) {
        auto opt = AtomicOptionContext::get_instance();
        auto *atomic = app.add_subcommand("atomic", "atomic operations");
        atomic->callback([atomic]() { run_atomic_cmd(atomic); });

        auto ac = atomic->add_subcommand("create", " create atomic");
        ac->add_option("-a,--app", opt->servlet_id, "servlet id")->required(true);
        ac->add_option("-i, --id", opt->start_id, "start id");
        ac->callback([]() { run_atomic_create_cmd(); });

        auto ar = atomic->add_subcommand("remove", " remove atomic");
        ar->add_option("-a,--servlet", opt->servlet_id, "servlet id")->required(true);
        ar->callback([]() { run_atomic_remove_cmd(); });

        auto ag = atomic->add_subcommand("gen", " gen atomic");
        ag->add_option("-a,--app", opt->servlet_id, "servlet id")->required(true);
        auto agt = ag->add_option_group("grow type", "start id or increment");
        agt->add_option("-i, --id", opt->start_id, "start id");
        agt->add_option("-c, --count", opt->count, "count id");
        agt->require_option(1);
        agt->required(true);
        ag->callback([]() { run_atomic_gen_cmd(); });

        auto au = atomic->add_subcommand("update", " update atomic");
        au->add_option("-a,--app", opt->servlet_id, "servlet id")->required(true);
        auto gt = au->add_option_group("grow type", "start id or increment");
        gt->add_option("-i, --id", opt->start_id, "start id");
        gt->add_option("-c, --count", opt->increment, "count id");
        gt->require_option(1);
        au->add_flag("-f, --force", opt->force, "force")->default_val(false);
        au->callback([]() { run_atomic_update_cmd(); });
    }

    void AtomicCmd::run_atomic_cmd(collie::App *app) {
        if (app->get_subcommands().empty()) {
            collie::println("{}", app->help());
        }
    }

    void AtomicCmd::run_atomic_create_cmd() {
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        ScopeShower ss;
        request.set_op_type(sirius::proto::OP_ADD_ID_FOR_AUTO_INCREMENT);
        auto opt = AtomicOptionContext::get_instance();
        auto atomic_info = request.mutable_auto_increment();
        atomic_info->set_servlet_id(opt->servlet_id);
        atomic_info->set_start_id(opt->start_id);
        auto rs = sirius::client::DiscoveryClient::get_instance()->discovery_manager(request, response, nullptr);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(response.errcode(), response.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table), response.errcode() == sirius::proto::SUCCESS);
        if (response.errcode() == sirius::proto::SUCCESS) {
            collie::table::Table summary;
            summary.add_row({"servlet id", "start id", "end id"});
            summary.add_row({collie::to_str(opt->servlet_id), collie::to_str(response.start_id()),
                             collie::to_str(response.end_id())});
            ss.add_table("summary", std::move(summary), true);
        }
    }

    void AtomicCmd::run_atomic_remove_cmd() {
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        ScopeShower ss;
        request.set_op_type(sirius::proto::OP_DROP_ID_FOR_AUTO_INCREMENT);
        auto opt = AtomicOptionContext::get_instance();
        auto atomic_info = request.mutable_auto_increment();
        atomic_info->set_servlet_id(opt->servlet_id);
        auto rs = sirius::client::DiscoveryClient::get_instance()->discovery_manager(request, response, nullptr);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(response.errcode(), response.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table), response.errcode() == sirius::proto::SUCCESS);
    };

    void AtomicCmd::run_atomic_gen_cmd() {
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        ScopeShower ss;
        request.set_op_type(sirius::proto::OP_GEN_ID_FOR_AUTO_INCREMENT);
        auto opt = AtomicOptionContext::get_instance();
        auto atomic_info = request.mutable_auto_increment();
        atomic_info->set_servlet_id(opt->servlet_id);
        if (opt->start_id != 0) {
            atomic_info->set_start_id(opt->start_id);
        }
        collie::println("{}", opt->count);
        atomic_info->set_count(opt->count);
        auto rs = sirius::client::DiscoveryClient::get_instance()->discovery_manager(request, response, nullptr);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(response.errcode(), response.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table), response.errcode() == sirius::proto::SUCCESS);
        if (response.errcode() == sirius::proto::SUCCESS) {
            collie::table::Table summary;
            summary.add_row({"servlet id", "start id", "end id"});
            summary.add_row({collie::to_str(opt->servlet_id), collie::to_str(response.start_id()),
                             collie::to_str(response.end_id())});
            ss.add_table("summary", std::move(summary), true);
        }
    }

    void AtomicCmd::run_atomic_update_cmd() {
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        ScopeShower ss;
        request.set_op_type(sirius::proto::OP_UPDATE_FOR_AUTO_INCREMENT);
        auto opt = AtomicOptionContext::get_instance();
        auto atomic_info = request.mutable_auto_increment();
        atomic_info->set_servlet_id(opt->servlet_id);
        if (opt->start_id != 0) {
            atomic_info->set_start_id(opt->start_id);
        }
        if (opt->increment != 0) {
            atomic_info->set_increment_id(opt->increment);
        }
        if (opt->force) {
            atomic_info->set_force(opt->force);
        }
        auto rs = sirius::client::DiscoveryClient::get_instance()->discovery_manager(request, response, nullptr);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(response.errcode(), response.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table), response.errcode() == sirius::proto::SUCCESS);
        if (response.errcode() == sirius::proto::SUCCESS) {
            collie::table::Table summary;
            summary.add_row({"servlet id", "start id"});
            summary.add_row({collie::to_str(opt->servlet_id), collie::to_str(response.start_id())});
            ss.add_table("summary", std::move(summary), true);
        }
    }

}  // namespace sirius::cli
