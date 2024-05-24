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
// Created by jeff on 23-11-30.
//
#include <sirius/cli/discovery.h>
#include <sirius/cli/option_context.h>
#include <sirius/cli/show_help.h>
#include <sirius/cli/router_interact.h>
#include <alkaid/files/sequential_read_file.h>
#include <sirius/client/discovery.h>
#include <collie/nlohmann/json.hpp>
#include <sirius/cli/validator.h>
#include <sirius/client/discovery_sender.h>
#include <sirius/client/router_sender.h>
#include <sirius/client/naming.h>
#include <sirius/client/dumper.h>
#include <sirius/client/loader.h>
#include <sirius/client/servlet_instance_builder.h>
#include <sirius/cli/app_cmd.h>
#include <sirius/cli/zone_cmd.h>
#include <sirius/cli/atomic_cmd.h>
#include <sirius/cli/servlet_cmd.h>
#include <sirius/cli/config_cmd.h>
#include <sirius/cli/user_cmd.h>


namespace sirius::cli {

    static int seq_id() {
        static std::atomic<int> seq{0};
        return seq++;
    }

    void DiscoveryCmd::setup_discovery_cmd(collie::App &app) {
        // Create the option and subcommand objects.
        auto opt = DiscoveryOptionContext::get_instance();
        auto *discovery_cmd = app.add_subcommand("discovery", "discovery operations");
        discovery_cmd->callback([discovery_cmd]() { run_discovery_cmd(discovery_cmd); });
        setup_app_cmd(*discovery_cmd);
        setup_zone_cmd(*discovery_cmd);
        ConfigCmd::setup_config_cmd(*discovery_cmd);
        setup_servlet_cmd(*discovery_cmd);
        setup_user_cmd(*discovery_cmd);
        AtomicCmd::setup_atomic_cmd(*discovery_cmd);

        auto dai = discovery_cmd->add_subcommand("mock", " mock servlet serving");
        auto *add_parameters_inputs = dai->add_option_group("parameters_inputs", "config input from parameters");
        auto *add_json_inputs = dai->add_option_group("json_inputs", "config input source from json format");
        add_parameters_inputs->add_option("-n,--app", opt->app_name, "app name")->required();
        add_parameters_inputs->add_option("-z,--zone", opt->zone_name, "zone name")->required();
        add_parameters_inputs->add_option("-s,--servlet", opt->servlet_name, "servlet name")->required();
        add_parameters_inputs->add_option("-a, --address", opt->address, "instance address")->required();
        add_parameters_inputs->add_option("-e, --env", opt->env, "instance env")->required();
        add_parameters_inputs->add_option("-c, --color", opt->color, "instance color")->default_val("default");
        add_parameters_inputs->add_option("-t, --status", opt->status, "instance color")->default_val("NORMAL");
        add_parameters_inputs->add_option("-f, --fiber", opt->fibers, "instance color")->default_val(10);
        add_json_inputs->add_option("-j, --json", opt->json_file, "json input file")->required(true);
        dai->require_option(1);
        dai->callback([]() { run_mock_instance_cmd(); });

        /*
        auto dai = discovery_cmd->add_subcommand("add_instance", " create a instance");
        auto *add_parameters_inputs = dai->add_option_group("parameters_inputs", "config input from parameters");
        auto *add_json_inputs = dai->add_option_group("json_inputs", "config input source from json format");
        add_parameters_inputs->add_option("-n,--app", opt->app_name, "app name")->required();
        add_parameters_inputs->add_option("-z,--zone", opt->zone_name, "zone name")->required();
        add_parameters_inputs->add_option("-s,--servlet", opt->servlet_name, "servlet name")->required();
        add_parameters_inputs->add_option("-a, --address", opt->address, "instance address")->required();
        add_parameters_inputs->add_option("-e, --env", opt->env, "instance env")->required();
        add_parameters_inputs->add_option("-c, --color", opt->color, "instance color")->default_val("default");
        add_parameters_inputs->add_option("-t, --status", opt->status, "instance color")->default_val("NORMAL");
        add_json_inputs->add_option("-j, --json", opt->json_file, "json input file")->required(true);
        dai->require_option(1);
        dai->callback([]() { run_discovery_add_instance_cmd(); });
        */
        auto dri = discovery_cmd->add_subcommand("remove_instance", " remove a instance");
        auto *remove_parameters_inputs = dri->add_option_group("parameters_inputs", "config input from parameters");
        auto *remove_json_inputs = dri->add_option_group("json_inputs", "config input source from json format");
        remove_parameters_inputs->add_option("-n,--app", opt->app_name, "app name")->required();
        remove_parameters_inputs->add_option("-z,--zone", opt->zone_name, "zone name")->required();
        remove_parameters_inputs->add_option("-s,--servlet", opt->servlet_name, "servlet name")->required();
        remove_parameters_inputs->add_option("-a, --address", opt->address, "instance address")->required();
        remove_parameters_inputs->add_option("-e, --env", opt->env, "instance env");
        remove_json_inputs->add_option("-j, --json", opt->json_file, "json input file")->required(true);
        dri->require_option(1);
        dri->callback([]() { run_discovery_remove_instance_cmd(); });

        auto dui = discovery_cmd->add_subcommand("update_instance", " create a instance");
        auto *update_parameters_inputs = dui->add_option_group("parameters_inputs", "config input from parameters");
        auto *update_json_inputs = dui->add_option_group("json_inputs", "config input source from json format");
        update_parameters_inputs->add_option("-n,--app", opt->app_name, "app name")->required();
        update_parameters_inputs->add_option("-z,--zone", opt->zone_name, "zone name")->required();
        update_parameters_inputs->add_option("-s,--servlet", opt->servlet_name, "servlet name")->required();
        update_parameters_inputs->add_option("-a, --address", opt->address, "instance address")->required();
        update_parameters_inputs->add_option("-e, --env", opt->env, "instance env")->required();
        update_parameters_inputs->add_option("-c, --color", opt->color, "instance color")->default_val("default");
        update_parameters_inputs->add_option("-t, --status", opt->status, "instance color")->default_val("NORMAL");
        update_json_inputs->add_option("-j, --json", opt->json_file, "json input file")->required(true);
        dui->require_option(1);
        dui->callback([]() { run_discovery_update_instance_cmd(); });

        auto dl = discovery_cmd->add_subcommand("list", "list instance");
        dl->add_option("-n,--app", opt->app_name, "app name");
        dl->add_option("-z,--zone", opt->zone_name, "zone name");
        dl->add_option("-s,--servlet", opt->servlet_name, "servlet name");
        dl->add_option("-a, --address", opt->address, "instance address");
        dl->callback([]() { run_discovery_list_instance_cmd(); });


        auto di = discovery_cmd->add_subcommand("naming", "info instance");
        di->add_option("-n,--app", opt->app_name, "app name")->required();
        di->add_option("-z,--zone", opt->zones, "zone name")->required();
        di->add_option("-e,--env", opt->envs, "env name")->required();
        di->add_option("-c, --color", opt->colors, "color")->required();
        di->add_option("-t, --status", opt->status, "instance status")->default_val("NORMAL");
        di->callback([]() { run_discovery_info_instance_cmd(); });


        auto dd = discovery_cmd->add_subcommand("dump", " dump instance example to json file");
        dd->add_option("-o,--output", opt->dump_file, "dump file path")->default_val("example_discovery.json");
        dd->add_flag("-q,--quiet", opt->quiet, "quiet or print")->default_val(false);
        dd->callback([]() { run_discovery_dump_cmd(); });
        /// run after cmd line parse complete
    }

    /// The function that runs our code.
    /// This could also simply be in the callback lambda itself,
    /// but having a separate function is cleaner.
    void DiscoveryCmd::run_discovery_cmd(collie::App *app) {
        // Do stuff...
        if (app->get_subcommands().empty()) {
            collie::println("{}", app->help());
        }
    }

    void DiscoveryCmd::run_discovery_add_instance_cmd() {
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        ScopeShower ss;
        auto rs = make_discovery_add_instance(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = RouterInteract::get_instance()->send_request("discovery_manager", request, response);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(OptionContext::get_instance()->router_server, response.errcode(),
                                               request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table));
    }

    static void* run_mock(bool& stop) {
        auto ctx = DiscoveryOptionContext::get_instance();
        sirius::proto::ServletInfo instance;
        instance.set_app_name(ctx->app_name);
        instance.set_zone(ctx->zone_name);
        auto i = seq_id();
        instance.set_servlet_name(ctx->servlet_name + collie::to_str(i));
        instance.set_env(ctx->env);
        instance.set_color(ctx->color);
        instance.set_status(sirius::proto::NORMAL);
        instance.set_address(ctx->address);
        sirius::client::Naming naming;
        auto rs = naming.initialize(OptionContext::get_instance()->router_server);
        if (!rs.ok()) {
            collie::println("initialize naming error:{}", rs.message());
            return nullptr;
        }
        rs = naming.register_server(instance);
        if (!rs.ok()) {
            collie::println("register server error:{}", rs.message());
            return nullptr;
        }
        collie::println("register server:{}.{}.{}#{} {} {} {} {}",
                        instance.app_name(), instance.zone(),
                        instance.servlet_name(), instance.address(),
                        instance.env(), instance.color(),
                        instance.mtime(), sirius::proto::Status_Name(instance.status()));
        while (!stop) {
            fiber_usleep(3 *1000 * 1000);
            rs = naming.update(instance);
            if (!rs.ok()) {
                collie::println("cancel server error:{}", rs.message());
                return nullptr;
            }
            collie::println("update server:{}.{}.{}#{} {} {} {} {}",
                            instance.app_name(), instance.zone(),
                            instance.servlet_name(), instance.address(),
                            instance.env(), instance.color(),
                            instance.mtime(), sirius::proto::Status_Name(instance.status()));
        }
        rs = naming.cancel(instance);
        if (!rs.ok()) {
            collie::println("cancel server error:{}", rs.message());
            return nullptr;
        }
        return nullptr;
    };

    void DiscoveryCmd::run_mock_instance_cmd() {
        bool stop = false;
        auto ctx = DiscoveryOptionContext::get_instance();
        std::vector<sirius::Fiber> fibers(ctx->fibers);
        for (auto &fiber: fibers) {
            fiber.run([&stop]() {
                run_mock(stop);
            });
        }
        size_t n = 1000;
        while (n > 0) {
            fiber_usleep(5* 1000 * 1000);
            n--;
        }
        stop = true;
        for (auto &fiber: fibers) {
            fiber.join();
        }
    }

    void DiscoveryCmd::run_discovery_remove_instance_cmd() {
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        ScopeShower ss;
        auto rs = make_discovery_remove_instance(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = RouterInteract::get_instance()->send_request("discovery_manager", request, response);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(OptionContext::get_instance()->router_server, response.errcode(),
                                               request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table));
    }

    void DiscoveryCmd::run_discovery_update_instance_cmd() {
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        ScopeShower ss;
        auto rs = make_discovery_update_instance(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = RouterInteract::get_instance()->send_request("discovery_manager", request, response);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(OptionContext::get_instance()->router_server, response.errcode(),
                                               request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table));
    }

    void DiscoveryCmd::run_discovery_list_instance_cmd() {
        sirius::proto::DiscoveryQueryRequest request;
        sirius::proto::DiscoveryQueryResponse response;

        ScopeShower ss;
        auto rs = make_discovery_list_instance(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = sirius::client::DiscoveryClient::get_instance()->discovery_query(request, response, nullptr);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(response.errcode(), request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table), response.errcode() == sirius::proto::SUCCESS);
        if (response.errcode() == sirius::proto::SUCCESS) {
            table = show_query_instance_list_response(response);
            ss.add_table("summary", std::move(table), true);
        }
    }

    void DiscoveryCmd::run_discovery_info_instance_cmd() {
        sirius::proto::ServletNamingRequest request;
        sirius::proto::ServletNamingResponse response;

        ScopeShower ss;
        auto rs = make_discovery_info_instance(&request);
        sirius::client::Naming naming;
        rs = naming.initialize(OptionContext::get_instance()->router_server);
        if (!rs.ok()) {
            collie::println("initialize naming error:{}", rs.message());
            return;
        }
        rs = naming.get_servers(request, response);
        if (!rs.ok()) {
            collie::println("get servers error:{}", rs.message());
            return;
        }

        auto &ins = response.servlets();
        collie::println("instance num:{}", ins.size());
        collie::println("message:{}", response.errmsg());
        for (auto &instance: ins) {
            collie::println("{}.{}.{}#{} {} {} {} {}",
                            instance.app_name(), instance.zone(),
                            instance.servlet_name(), instance.address(),
                            instance.env(), instance.color(),
                            instance.mtime(), sirius::proto::Status_Name(instance.status()));

        }
        /*
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = sirius::client::DiscoveryClient::get_instance()->discovery_query(request, response, nullptr);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(response.errcode(), request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table), response.errcode() == sirius::proto::SUCCESS);
        if (response.errcode() == sirius::proto::SUCCESS) {
            table = show_query_instance_info_response(response);
            ss.add_table("summary", std::move(table), true);
        }
         */
    }

    [[nodiscard]] collie::Status
    DiscoveryCmd::make_discovery_add_instance(sirius::proto::DiscoveryManagerRequest *req) {
        sirius::proto::ServletInfo *instance_req = req->mutable_servlet_info();
        req->set_op_type(sirius::proto::OP_CREATE_SERVLET);
        if (!DiscoveryOptionContext::get_instance()->json_file.empty()) {
            auto rs = sirius::client::Loader::load_proto_from_file(DiscoveryOptionContext::get_instance()->json_file,
                                                               *instance_req);
            return rs;
        }
        auto rs = check_valid_name_type(DiscoveryOptionContext::get_instance()->app_name);
        if (!rs.ok()) {
            return rs;
        }
        rs = check_valid_name_type(DiscoveryOptionContext::get_instance()->zone_name);
        if (!rs.ok()) {
            return rs;
        }
        rs = check_valid_name_type(DiscoveryOptionContext::get_instance()->servlet_name);
        if (!rs.ok()) {
            return rs;
        }
        auto status = string_to_status(DiscoveryOptionContext::get_instance()->status);
        if (!status.ok()) {
            return status.status();
        }
        instance_req->set_app_name(DiscoveryOptionContext::get_instance()->app_name);
        instance_req->set_zone(DiscoveryOptionContext::get_instance()->zone_name);
        instance_req->set_servlet_name(DiscoveryOptionContext::get_instance()->servlet_name);
        instance_req->set_color(DiscoveryOptionContext::get_instance()->color);
        instance_req->set_env(DiscoveryOptionContext::get_instance()->env);
        instance_req->set_status(status.value_or_die());
        instance_req->set_address(DiscoveryOptionContext::get_instance()->address);
        return collie::Status::ok_status();
    }

    [[nodiscard]] collie::Status
    DiscoveryCmd::make_discovery_remove_instance(sirius::proto::DiscoveryManagerRequest *req) {
        sirius::proto::ServletInfo *instance_req = req->mutable_servlet_info();
        req->set_op_type(sirius::proto::OP_DROP_SERVLET);
        if (!DiscoveryOptionContext::get_instance()->json_file.empty()) {
            auto rs = sirius::client::Loader::load_proto_from_file(DiscoveryOptionContext::get_instance()->json_file,
                                                               *instance_req);
            return rs;
        }
        auto rs = check_valid_name_type(DiscoveryOptionContext::get_instance()->app_name);
        if (!rs.ok()) {
            return rs;
        }
        rs = check_valid_name_type(DiscoveryOptionContext::get_instance()->zone_name);
        if (!rs.ok()) {
            return rs;
        }
        rs = check_valid_name_type(DiscoveryOptionContext::get_instance()->servlet_name);
        if (!rs.ok()) {
            return rs;
        }

        instance_req->set_app_name(DiscoveryOptionContext::get_instance()->app_name);
        instance_req->set_zone(DiscoveryOptionContext::get_instance()->zone_name);
        instance_req->set_servlet_name(DiscoveryOptionContext::get_instance()->servlet_name);
        instance_req->set_address(DiscoveryOptionContext::get_instance()->address);

        return collie::Status::ok_status();
    }

    [[nodiscard]] collie::Status
    DiscoveryCmd::make_discovery_update_instance(sirius::proto::DiscoveryManagerRequest *req) {
        sirius::proto::ServletInfo *instance_req = req->mutable_servlet_info();
        req->set_op_type(sirius::proto::OP_MODIFY_SERVLET);
        if (!DiscoveryOptionContext::get_instance()->json_file.empty()) {
            auto rs = sirius::client::Loader::load_proto_from_file(DiscoveryOptionContext::get_instance()->json_file,
                                                               *instance_req);
            return rs;
        }
        auto rs = check_valid_name_type(DiscoveryOptionContext::get_instance()->app_name);
        if (!rs.ok()) {
            return rs;
        }
        rs = check_valid_name_type(DiscoveryOptionContext::get_instance()->zone_name);
        if (!rs.ok()) {
            return rs;
        }
        rs = check_valid_name_type(DiscoveryOptionContext::get_instance()->servlet_name);
        if (!rs.ok()) {
            return rs;
        }
        auto status = string_to_status(DiscoveryOptionContext::get_instance()->status);
        if (!status.ok()) {
            return status.status();
        }
        instance_req->set_app_name(DiscoveryOptionContext::get_instance()->app_name);
        instance_req->set_zone(DiscoveryOptionContext::get_instance()->zone_name);
        instance_req->set_servlet_name(DiscoveryOptionContext::get_instance()->servlet_name);
        instance_req->set_color(DiscoveryOptionContext::get_instance()->color);
        instance_req->set_env(DiscoveryOptionContext::get_instance()->env);
        instance_req->set_status(status.value_or_die());
        instance_req->set_address(DiscoveryOptionContext::get_instance()->address);
        return collie::Status::ok_status();
    }

    void DiscoveryCmd::run_discovery_dump_cmd() {
        sirius::proto::ServletInfo instance;
        sirius::client::ServletInstanceBuilder builder(&instance);
        builder.set_namespace("ex_namespace")
                .set_zone("ex_zone")
                .set_servlet("ex_servlet")
                .set_env("ex_env")
                .set_color("green")
                .set_status("NORMAL")
                .set_address("127.0.0.1:12345");
        auto rs = sirius::client::Dumper::dump_proto_to_file(DiscoveryOptionContext::get_instance()->dump_file, instance);
        if (!rs.ok()) {
            collie::println("dump example discovery instance error:{}", rs.message());
            return;
        }

        if (!DiscoveryOptionContext::get_instance()->quiet) {
            std::string json_str;
            rs = sirius::client::Dumper::dump_proto(instance, json_str);
            if (!rs.ok()) {
                collie::println("dump example discovery instance error:{}", rs.message());
                return;
            }
            std::cout << ShowHelper::json_format(json_str) << std::endl;
        }

    }

    [[nodiscard]] collie::Status
    DiscoveryCmd::make_discovery_list_instance(sirius::proto::DiscoveryQueryRequest *req) {
        req->set_op_type(sirius::proto::QUERY_SERVLET);
        auto opt = DiscoveryOptionContext::get_instance();
        if (opt->app_name.empty()) {
            return collie::Status::ok_status();
        }
        req->set_app_name(opt->app_name);
        if (opt->zone_name.empty()) {
            return collie::Status::ok_status();
        }
        req->set_zone(opt->zone_name);
        if (opt->servlet_name.empty()) {
            return collie::Status::ok_status();
        }
        req->set_servlet(opt->servlet_name);
        return collie::Status::ok_status();
    }

    [[nodiscard]] collie::Status
    DiscoveryCmd::make_discovery_info_instance(sirius::proto::ServletNamingRequest *req) {
        auto opt = DiscoveryOptionContext::get_instance();
        req->set_app_name(opt->app_name);
        for(auto &zone: opt->zones) {
            req->add_zones(zone);
        }
        for(auto &env: opt->envs) {
            req->add_env(env);
        }
        for(auto &color: opt->colors) {
            req->add_color(color);
        }
        return collie::Status::ok_status();
    }

    [[nodiscard]] collie::Result<sirius::proto::Status> DiscoveryCmd::string_to_status(const std::string &status) {
        sirius::proto::Status ret;
        if (sirius::proto::Status_Parse(status, &ret)) {
            return ret;
        }
        return collie::Status::invalid_argument("unknown status");
    }

    collie::table::Table DiscoveryCmd::show_query_instance_list_response(const sirius::proto::DiscoveryQueryResponse &res) {
        collie::table::Table result;
        auto &instance_list = res.flatten_instances();
        result.add_row(collie::table::Table::Row_t{"instance num", collie::to_str(instance_list.size())});
        auto last = result.size() - 1;
        result[last].format().font_color(collie::Color::green);
        result.add_row(collie::table::Table::Row_t{"number", "instance"});
        last = result.size() - 1;
        result[last].format().font_color(collie::Color::green);
        int i = 0;
        std::vector<sirius::proto::QueryInstance> sorted_list;
        for (auto &ns: instance_list) {
            sorted_list.push_back(ns);
        }
        auto less_fun = [](const sirius::proto::QueryInstance &lhs, const sirius::proto::QueryInstance &rhs) -> bool {
            return std::less()(lhs.address(), rhs.address());
        };
        std::sort(sorted_list.begin(), sorted_list.end(), less_fun);
        for (auto &ns: sorted_list) {
            result.add_row(collie::table::Table::Row_t{collie::to_str(i++),
                                               collie::format("{}.{}.{}#{}", ns.app_name(), ns.zone_name(),
                                                             ns.servlet_name(), ns.address())});
            last = result.size() - 1;
            result[last].format().font_color(collie::Color::yellow);

        }
        return result;
    }

    collie::table::Table DiscoveryCmd::show_query_instance_info_response(const sirius::proto::DiscoveryQueryResponse &res) {
        collie::table::Table result;
        auto &instance = res.servlet_infos(0);
        result.add_row(collie::table::Table::Row_t{"uri", "address", "env", "color","lmtime","status"});
        result.add_row(
                collie::table::Table::Row_t{
                        collie::format("{}.{}.{}", instance.app_name(), instance.zone(),
                                      instance.servlet_name()),
                        instance.address(),
                        instance.env(),
                        instance.color(),
                        collie::to_str(instance.mtime()),
                        sirius::proto::Status_Name(instance.status())
                }
        );

        return result;
    }

}  // namespace sirius::cli
