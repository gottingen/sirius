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

#include <sirius/cli/raft_cmd.h>
#include <sirius/cli/option_context.h>
#include <sirius/cli/show_help.h>
#include <sirius/client/config_info_builder.h>
#include <collie/strings/format.h>

namespace sirius::cli {

    void RaftCmd::setup_raft_cmd(collie::App &app) {
        auto opt = RaftOptionContext::get_instance();
        auto *ns = app.add_subcommand("raft", "raft control operations");
        ns->callback([ns]() { run_raft_cmd(ns); });

        ns->add_option("-m,--discovery_server", opt->discovery_server, "server address default(\"127.0.0.1:8010\")")->default_val(
                "127.0.0.1:8010");

        auto cg = ns->add_subcommand("status", "cluster status");
        cg->add_option("-c,--cluster", opt->cluster, "cluster [discovery|tso|atomic]")->required();
        cg->callback([]() { run_status_cmd(); });

        auto cs = ns->add_subcommand("snapshot", "cluster snapshot");
        cs->add_option("-c,--cluster", opt->cluster, "cluster [discovery|tso|atomic]")->required();
        cs->callback([]() { run_snapshot_cmd(); });

        auto cv = ns->add_subcommand("vote", "cluster vote");
        cv->add_option("-c,--cluster", opt->cluster, "cluster [discovery|tso|atomic]")->required();
        cv->add_option("-t,--time", opt->vote_time_ms, "election time ms")->required();
        cv->callback([]() { run_vote_cmd(); });

        auto cd = ns->add_subcommand("shutdown", "cluster shutdown");
        cd->add_option("-c,--cluster", opt->cluster, "cluster [discovery|tso|atomic]")->required();
        cd->callback([]() { run_shutdown_cmd(); });

        auto cset = ns->add_subcommand("set", "cluster set peer");
        cset->add_option("-c,--cluster", opt->cluster, "cluster [discovery|tso|atomic]")->required();
        cset->add_option("-o,--old", opt->old_peers, "old peers")->required();
        cset->add_option("-n,--new", opt->new_peers, "new peers")->required();
        cset->add_option("-f,--force", opt->force, "new peers")->default_val(false);
        cset->callback([]() { run_set_cmd(); });

        auto ct = ns->add_subcommand("trans", "cluster trans leader");
        ct->add_option("-c,--cluster", opt->cluster, "cluster [discovery|tso|atomic]")->required();
        ct->add_option("-n,--new-leader", opt->new_leader, "cluster new leader")->required();
        ct->callback([]() { run_trans_cmd(); });

        auto func = []() {
            auto opt = RaftOptionContext::get_instance();
            auto r = opt->sender.init(opt->discovery_server);
            if(!r.ok()) {
                collie::println(collie::Color::red, "init error:{}", opt->discovery_server);
                exit(0);
            }
        };
        ns->parse_complete_callback(func);
    }

    void RaftCmd::run_raft_cmd(collie::App *app) {
        if (app->get_subcommands().empty()) {
            collie::println("{}", app->help());
        }
    }

    void RaftCmd::run_status_cmd() {
        sirius::proto::RaftControlRequest request;
        sirius::proto::RaftControlResponse response;
        ScopeShower ss;
        request.set_op_type(sirius::proto::ListPeer);
        auto id = to_region_id();
        if(!id.ok()) {
            collie::println("unknown cluster");
            return;
        }
        PREPARE_ERROR_RETURN_OR_OK(ss, id.status(), request);
        request.set_region_id(id.value());
        auto rs = RaftOptionContext::get_instance()->sender.send_request("raft_control", request, response, 1);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(response.errcode(), request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table), response.errcode() == sirius::proto::SUCCESS);
        if (response.errcode() == sirius::proto::SUCCESS) {
            table = show_raft_result(response);
            ss.add_table("summary", std::move(table), true);
        }
    }

    void RaftCmd::run_snapshot_cmd() {
        sirius::proto::RaftControlRequest request;
        sirius::proto::RaftControlResponse response;
        ScopeShower ss;
        request.set_op_type(sirius::proto::SnapShot);
        auto id = to_region_id();
        if(!id.ok()) {
            collie::println("unknown cluster");
            return;
        }
        PREPARE_ERROR_RETURN_OR_OK(ss, id.status(), request);
        request.set_region_id(id.value());
        auto rs = RaftOptionContext::get_instance()->sender.send_request("raft_control", request, response, 1);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(response.errcode(), request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table), response.errcode() == sirius::proto::SUCCESS);
    }

    void RaftCmd::run_vote_cmd() {
        sirius::proto::RaftControlRequest request;
        sirius::proto::RaftControlResponse response;
        ScopeShower ss;
        request.set_op_type(sirius::proto::ResetVoteTime);
        auto id = to_region_id();
        if(!id.ok()) {
            collie::println("unknown cluster");
            return;
        }
        PREPARE_ERROR_RETURN_OR_OK(ss, id.status(), request);
        request.set_region_id(id.value());
        auto rs = RaftOptionContext::get_instance()->sender.send_request("raft_control", request, response, 1);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(response.errcode(), request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table), response.errcode() == sirius::proto::SUCCESS);
    }

    void RaftCmd::run_shutdown_cmd() {
        sirius::proto::RaftControlRequest request;
        sirius::proto::RaftControlResponse response;
        ScopeShower ss;
        request.set_op_type(sirius::proto::ShutDown);
        auto id = to_region_id();
        if(!id.ok()) {
            collie::println("unknown cluster");
            return;
        }
        PREPARE_ERROR_RETURN_OR_OK(ss, id.status(), request);
        request.set_region_id(id.value());
        auto rs = RaftOptionContext::get_instance()->sender.send_request("raft_control", request, response, 1);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(response.errcode(), request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table), response.errcode() == sirius::proto::SUCCESS);
    }

    void RaftCmd::run_set_cmd() {
        sirius::proto::RaftControlRequest request;
        sirius::proto::RaftControlResponse response;
        ScopeShower ss;
        request.set_op_type(sirius::proto::SetPeer);
        auto id = to_region_id();
        if(!id.ok()) {
            collie::println("unknown cluster");
            return;
        }
        auto opt = RaftOptionContext::get_instance();
        for(auto old : opt->old_peers) {
            request.add_old_peers(old);
        }
        for(auto peer : opt->new_peers) {
            request.add_old_peers(peer);
        }
        if(opt->force) {
            request.set_force(true);
        }
        PREPARE_ERROR_RETURN_OR_OK(ss, id.status(), request);
        request.set_region_id(id.value());
        auto rs = RaftOptionContext::get_instance()->sender.send_request("raft_control", request, response, 1);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(response.errcode(), request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table), response.errcode() == sirius::proto::SUCCESS);
        if (response.errcode() == sirius::proto::SUCCESS) {
            table = show_raft_result(response);
            ss.add_table("summary", std::move(table), true);
        }
    }

    void RaftCmd::run_trans_cmd() {
        sirius::proto::RaftControlRequest request;
        sirius::proto::RaftControlResponse response;
        ScopeShower ss;
        request.set_op_type(sirius::proto::TransLeader);
        auto id = to_region_id();
        if(!id.ok()) {
            collie::println("unknown cluster");
            return;
        }
        auto opt = RaftOptionContext::get_instance();
        PREPARE_ERROR_RETURN_OR_OK(ss, id.status(), request);
        request.set_new_leader(opt->new_leader);
        request.set_region_id(id.value());
        auto rs = RaftOptionContext::get_instance()->sender.send_request("raft_control", request, response, 1);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(response.errcode(), request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table), response.errcode() == sirius::proto::SUCCESS);
        if (response.errcode() == sirius::proto::SUCCESS) {
            table = show_raft_result(response);
            ss.add_table("summary", std::move(table), true);
        }
    }

    turbo::Result<int> RaftCmd::to_region_id() {
        auto opt = RaftOptionContext::get_instance();
        if(opt->cluster == "discovery") {
            return 0;
        }
        if(opt->cluster == "tso") {
            return 2;
        }
        if(opt->cluster == "atomic") {
            return 1;
        }
        return turbo::invalid_argument_error("unknown cluster: " + opt->cluster);
    }

    collie::table::Table RaftCmd::show_raft_result(sirius::proto::RaftControlResponse &res) {
        collie::table::Table summary;
        summary.add_row({"leader","peers"});
        collie::table::Table peers;
        for(auto &peer : res.peers()) {
            peers.add_row({peer});
        }
        summary.add_row({collie::table::Table().add_row({res.leader()}), peers});
        return summary;
    }
}  // namespace sirius::cli
