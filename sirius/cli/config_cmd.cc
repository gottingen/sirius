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

#include <sirius/cli/config_cmd.h>
#include <sirius/cli/option_context.h>
#include <sirius/cli/show_help.h>
#include <sirius/cli/router_interact.h>
#include <collie/module/semver.h>
#include <alkaid/files/sequential_read_file.h>
#include <alkaid/files/sequential_write_file.h>
#include <turbo/times/time.h>
#include <melon/json2pb/pb_to_json.h>
#include <melon/json2pb/json_to_pb.h>
#include <sirius/client/discovery.h>
#include <sirius/client/dumper.h>
#include <sirius/client/config_info_builder.h>
#include <collie/nlohmann/json.hpp>
#include <collie/strings/format.h>

namespace sirius::cli {

    void ConfigCmd::setup_config_cmd(collie::App &app) {
        // Create the option and subcommand objects.
        auto opt = ConfigOptionContext::get_instance();
        auto *ns = app.add_subcommand("config", "config operations");
        ns->callback([ns]() { run_config_cmd(ns); });

        auto cc = ns->add_subcommand("create", " create config");
        auto *parameters_inputs = cc->add_option_group("parameters_inputs", "config input from parameters");
        auto *json_inputs = cc->add_option_group("json_inputs", "config input source from json format");
        parameters_inputs->add_option("-n,--name", opt->config_name, "config name")->required();
        auto *df_inputs = parameters_inputs->add_option_group("data_or_file", "config input source");
        df_inputs->add_option("-d,--data", opt->config_data, "config content");
        df_inputs->add_option("-f, --file", opt->config_file, "local config file");
        df_inputs->require_option(1);
        parameters_inputs->add_option("-v, --version", opt->config_version, "config version [1.2.3]");
        parameters_inputs->add_option("-t, --type", opt->config_type,
                                      "config type [json|toml|yaml|xml|gflags|text|ini]")->default_val("json");
        json_inputs->add_option("-j, --json", opt->config_json, "local config file form json format");
        cc->require_option(1);
        cc->callback([]() { run_config_create_cmd(); });

        auto cl = ns->add_subcommand("list", " list config");
        cl->add_option("-n,--name", opt->config_name, "config name");
        cl->callback([]() { run_config_list_cmd(); });

        auto cg = ns->add_subcommand("get", " get config");
        cg->add_option("-n,--name", opt->config_name, "config name")->required();
        cg->add_option("-v, --version", opt->config_version, "config version");
        cg->add_option("-o, --output", opt->config_file, "config save file");
        cg->callback([]() { run_config_get_cmd(); });

        auto cr = ns->add_subcommand("remove", " remove config");
        cr->add_option("-n,--name", opt->config_name, "config name")->required();
        cr->add_option("-v, --version", opt->config_version, "config version [1.2.3]");
        cr->callback([]() { run_config_remove_cmd(); });


        auto cd = ns->add_subcommand("dump", " dump config example to json file");
        auto *dump_parameters_inputs = cd->add_option_group("parameters_inputs", "config input from parameters");
        auto *dump_default = cd->add_option_group("default_example", "default config example");
        dump_parameters_inputs->add_option("-n,--name", opt->config_name, "config name")->required(true);
        dump_parameters_inputs->add_option("-v, --version", opt->config_version, "config version")->required(true);
        dump_parameters_inputs->add_option("-c, --content", opt->config_data, "config version")->required(true);
        dump_parameters_inputs->add_option("-t, --type", opt->config_type,
                                           "config type [json|toml|yaml|xml|gflags|text|ini]")->default_val("json");
        dump_parameters_inputs->add_option("-o, --output", opt->config_file, "config save file");
        dump_default->add_option("-e, --example", opt->config_example, "example output file");
        cd->require_option(1);
        cd->callback([]() { run_config_dump_cmd(); });


        auto ct = ns->add_subcommand("test", "test json config file");
        ct->add_option("-f, --file", opt->config_file, "local config file")->required(true);
        ct->callback([]() { run_config_test_cmd(); });

        auto cw = ns->add_subcommand("watch", "watch config");
        cw->add_option("-n, --name", opt->config_watch_list, "local config file")->required(true);
        cw->add_option("-d, --dir", opt->config_watch_dir, "local config file")->default_val("watch_config");
        cw->add_flag("-c, --clean", opt->clean_local, "clean cache")->default_val(false);
        cw->callback([]() { run_config_watch_cmd(); });

    }

    /// The function that runs our code.
    /// This could also simply be in the callback lambda itself,
    /// but having a separate function is cleaner.
    void ConfigCmd::run_config_cmd(collie::App *app) {
        // Do stuff...
        if (app->get_subcommands().empty()) {
            collie::println("{}", app->help());
        }
    }

    void ConfigCmd::run_config_create_cmd() {
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        ScopeShower ss;
        request.set_op_type(sirius::proto::OP_CREATE_CONFIG);
        auto opt = ConfigOptionContext::get_instance();
        auto config_info = request.mutable_config_info();
        sirius::client::ConfigInfoBuilder builder(config_info);
        /// json builder
        turbo::Status rs;
        if (!opt->config_json.empty()) {
            rs = builder.build_from_json_file(opt->config_json);
        } else {
            /// parameter
            if (!opt->config_file.empty()) {
                rs = builder.build_from_file(opt->config_name,
                                             opt->config_file,
                                             opt->config_version,
                                             opt->config_type);
            } else {
                rs = builder.build_from_content(opt->config_name,
                                                opt->config_data,
                                                opt->config_version,
                                                opt->config_type);
            }
        }
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = sirius::client::DiscoveryClient::get_instance()->discovery_manager(request, response, nullptr);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(response.errcode(), response.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table), response.errcode() == sirius::proto::SUCCESS);
    }

    void ConfigCmd::run_config_dump_cmd() {
        sirius::proto::ConfigInfo request;

        ScopeShower ss;
        auto opt = ConfigOptionContext::get_instance();
        turbo::Status rs;
        std::string file_path;
        if (!opt->config_example.empty()) {
            rs = make_example_config_dump(&request);
            file_path = opt->config_example;
        } else {
            file_path = opt->config_file;
            sirius::client::ConfigInfoBuilder builder(&request);
            rs = builder.build_from_content(opt->config_name, opt->config_data, opt->config_version, opt->config_type);
        }

        if (!rs.ok()) {
            ss.add_table("prepare", rs.to_string(), false);
            return;
        } else {
            ss.add_table("prepare", "ok", true);
        }
        alkaid::SequentialWriteFile file;
        rs = file.open(file_path, alkaid::kDefaultTruncateWriteOption);
        if (!rs.ok()) {
            ss.add_table("prepare file", rs.to_string(), false);
            return;
        } else {
            ss.add_table("prepare file", "ok", true);
        }
        std::string json;
        std::string err;
        rs = sirius::client::Dumper::dump_proto(request, json);
        if (!rs.ok()) {
            ss.add_table("convert", rs.to_string(), false);
            return;
        } else {
            ss.add_table("convert", "ok", true);
        }

        rs = file.write(json);
        if (!rs.ok()) {
            ss.add_table("write", rs.to_string(), false);
            return;
        } else {
            ss.add_table("write", "ok", true);
        }
        file.close();
        ss.add_table("summary", collie::format("success write to  file: {}", file_path), true);
    }

    void ConfigCmd::run_config_test_cmd() {
        sirius::proto::ConfigInfo request;
        ScopeShower ss;
        if (ConfigOptionContext::get_instance()->config_file.empty()) {
            ss.add_table("prepare", "no input file", false);
            return;
        }
        std::string content;
        alkaid::SequentialReadFile file;
        auto rs = file.open(ConfigOptionContext::get_instance()->config_file);
        if (!rs.ok()) {
            ss.add_table("open file", rs.to_string(), false);
            return;
        }
        ss.add_table("open file", "ok", true);
        auto r = file.read(&content);
        if (!r.ok()) {
            ss.add_table("read file", rs.to_string(), false);
            return;
        }
        ss.add_table("read file", "ok", true);
        sirius::client::ConfigInfoBuilder builder(&request);
        rs = builder.build_from_json(content);
        if (!rs.ok()) {
            ss.add_table("convert", rs.to_string(), false);
            return;
        }
        ss.add_table("convert", "ok", true);
        collie::println("name size:{}", request.name().size());
        collie::table::Table result_table;
        result_table.add_row(collie::table::Table::Row_t{"name", request.name()});
        result_table.add_row(collie::table::Table::Row_t{"version", collie::format("{}.{}.{}", request.version().major(),
                                                                          request.version().minor(),
                                                                          request.version().patch())});
        result_table.add_row(collie::table::Table::Row_t{"type", config_type_to_string(request.type())});
        result_table.add_row(collie::table::Table::Row_t{"size", collie::to_str(request.content().size())});
        turbo::Time cs = turbo::Time::from_time_t(request.time());
        result_table.add_row(collie::table::Table::Row_t{"time", turbo::Time::format(cs)});
        result_table.add_row(collie::table::Table::Row_t{"content", request.content()});
        ss.add_table("result", std::move(result_table), true);
    }

    void ConfigCmd::run_config_list_cmd() {
        if (!ConfigOptionContext::get_instance()->config_name.empty()) {
            run_config_version_list_cmd();
            return;
        }
        sirius::proto::DiscoveryQueryRequest request;
        sirius::proto::DiscoveryQueryResponse response;

        ScopeShower ss;
        auto rs = make_config_list(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = sirius::client::DiscoveryClient::get_instance()->discovery_query(request, response, nullptr);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(response.errcode(), request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table), response.errcode() == sirius::proto::SUCCESS);
        if (response.errcode() == sirius::proto::SUCCESS) {
            table = show_query_ops_config_list_response(response);
            ss.add_table("summary", std::move(table), true);
        }
    }

    void ConfigCmd::run_config_version_list_cmd() {
        sirius::proto::DiscoveryQueryRequest request;
        sirius::proto::DiscoveryQueryResponse response;

        ScopeShower ss;
        auto rs = make_config_list_version(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = sirius::client::DiscoveryClient::get_instance()->discovery_query(request, response, nullptr);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(response.errcode(), request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table), response.errcode() == sirius::proto::SUCCESS);
        if (response.errcode() == sirius::proto::SUCCESS) {
            table = show_query_ops_config_list_version_response(response);
            ss.add_table("summary", std::move(table), true);
        }
    }

    void ConfigCmd::run_config_get_cmd() {
        sirius::proto::DiscoveryQueryRequest request;
        sirius::proto::DiscoveryQueryResponse response;

        ScopeShower ss("get config info");
        auto rs = make_config_get(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = sirius::client::DiscoveryClient::get_instance()->discovery_query(request, response, nullptr);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(response.errcode(), request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table), true);
        if (response.errcode() != sirius::proto::SUCCESS) {
            return;
        }
        turbo::Status save_status;
        if (!ConfigOptionContext::get_instance()->config_file.empty()) {
            save_status = save_config_to_file(ConfigOptionContext::get_instance()->config_file, response);
        }
        table = show_query_ops_config_get_response(response, save_status);
        ss.add_table("summary", std::move(table), true);
    }

    turbo::Status ConfigCmd::save_config_to_file(const std::string &path, const sirius::proto::DiscoveryQueryResponse &res) {
        alkaid::SequentialWriteFile file;
        auto s = file.open(path, alkaid::kDefaultTruncateWriteOption);
        if (!s.ok()) {
            return s;
        }
        s = file.write(res.config_infos(0).content());
        if (!s.ok()) {
            return s;
        }
        file.close();
        return turbo::OkStatus();
    }

    void ConfigCmd::run_config_remove_cmd() {
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;

        ScopeShower ss;
        auto rs = make_config_remove(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = sirius::client::DiscoveryClient::get_instance()->discovery_manager(request, response, nullptr);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(response.errcode(), response.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table), true);
    }

    [[nodiscard]] turbo::Status
    ConfigCmd::make_example_config_dump(sirius::proto::ConfigInfo *req) {
        req->set_name("example");
        req->set_time(static_cast<int>(turbo::Time::current_seconds()));
        req->set_type(sirius::proto::CF_JSON);
        auto v = req->mutable_version();
        v->set_major(1);
        v->set_minor(2);
        v->set_patch(3);

        nlohmann::json json_content;
        json_content["servlet"] = "sug";
        json_content["zone"]["name"] = "ea_search";
        json_content["zone"]["user"] = "jeff";
        json_content["zone"]["instance"] = {"192.168.1.2","192.168.1.3","192.168.1.3"};
        nlohmann::to_string(json_content);
        req->set_content(nlohmann::to_string(json_content));
        std::cout<<json_content<<std::endl;
        return turbo::OkStatus();
    }

    [[nodiscard]] turbo::Status
    ConfigCmd::make_config_list(sirius::proto::DiscoveryQueryRequest *req) {
        req->set_op_type(sirius::proto::QUERY_LIST_CONFIG);
        return turbo::OkStatus();
    }

    [[nodiscard]] turbo::Status
    ConfigCmd::make_config_list_version(sirius::proto::DiscoveryQueryRequest *req) {
        req->set_op_type(sirius::proto::QUERY_LIST_CONFIG_VERSION);
        auto opt = ConfigOptionContext::get_instance();
        req->set_config_name(opt->config_name);
        return turbo::OkStatus();
    }

    [[nodiscard]] turbo::Status
    ConfigCmd::make_config_get(sirius::proto::DiscoveryQueryRequest *req) {
        req->set_op_type(sirius::proto::QUERY_GET_CONFIG);
        auto opt = ConfigOptionContext::get_instance();
        req->set_config_name(opt->config_name);
        if (!opt->config_version.empty()) {
            auto v = req->mutable_config_version();
            return string_to_version(opt->config_version, v);
        }
        return turbo::OkStatus();
    }

    [[nodiscard]] turbo::Status
    ConfigCmd::make_config_remove(sirius::proto::DiscoveryManagerRequest *req) {
        req->set_op_type(sirius::proto::OP_REMOVE_CONFIG);
        auto rc = req->mutable_config_info();
        auto opt = ConfigOptionContext::get_instance();
        rc->set_name(opt->config_name);
        if (!opt->config_version.empty()) {
            auto v = rc->mutable_version();
            return string_to_version(opt->config_version, v);
        }
        return turbo::OkStatus();
    }

    collie::table::Table ConfigCmd::show_query_ops_config_list_response(const sirius::proto::DiscoveryQueryResponse &res) {
        collie::table::Table result;
        auto &config_list = res.config_infos();
        result.add_row(collie::table::Table::Row_t{"config size", collie::to_str(config_list.size())});
        auto last = result.size() - 1;
        result[last].format().font_color(collie::Color::green);
        result.add_row(collie::table::Table::Row_t{"number", "config"});
        last = result.size() - 1;
        result[last].format().font_color(collie::Color::green);
        int i = 0;
        std::vector<sirius::proto::ConfigInfo> sorted_list;
        for (auto &ns: config_list) {
            sorted_list.push_back(ns);
        }
        auto less_fun = [](const sirius::proto::ConfigInfo & lhs, const sirius::proto::ConfigInfo &rhs) -> bool{
            return std::less()(lhs.name(), rhs.name());
        };
        std::sort(sorted_list.begin(), sorted_list.end(),less_fun);
        for (auto &ns: sorted_list) {
            result.add_row(collie::table::Table::Row_t{collie::to_str(i++), ns.name()});
            last = result.size() - 1;
            result[last].format().font_color(collie::Color::yellow);

        }
        return result;
    }

    collie::table::Table ConfigCmd::show_query_ops_config_list_version_response(const sirius::proto::DiscoveryQueryResponse &res) {
        collie::table::Table result;
        auto &config_versions = res.config_infos();
        result.add_row(collie::table::Table::Row_t{"version num", collie::to_str(config_versions.size())});
        auto last = result.size() - 1;
        result[last].format().font_color(collie::Color::green);
        result.add_row(collie::table::Table::Row_t{"number", "version"});
        last = result.size() - 1;
        result[last].format().font_color(collie::Color::green);
        int i = 0;
        for (auto &ns: config_versions) {
            result.add_row(
                    collie::table::Table::Row_t{collie::to_str(i++),
                                        collie::format("{}.{}.{}", ns.version().major(), ns.version().minor(),
                                                      ns.version().patch())});
            last = result.size() - 1;
            result[last].format().font_color(collie::Color::yellow);

        }
        return result;
    }

    collie::table::Table ConfigCmd::show_query_ops_config_get_response(const sirius::proto::DiscoveryQueryResponse &res,
                                                               const turbo::Status &save_status) {
        collie::table::Table result_table;
        auto config = res.config_infos(0);
        result_table.add_row(collie::table::Table::Row_t{"version", collie::format("{}.{}.{}", config.version().major(),
                                                                          config.version().minor(),
                                                                          config.version().patch())});
        auto last = result_table.size() - 1;
        result_table[last].format().font_color(collie::Color::green);
        result_table.add_row(collie::table::Table::Row_t{"type", config_type_to_string(config.type())});
        last = result_table.size() - 1;
        result_table[last].format().font_color(collie::Color::green);
        result_table.add_row(collie::table::Table::Row_t{"size", collie::to_str(config.content().size())});
        last = result_table.size() - 1;
        result_table[last].format().font_color(collie::Color::green);
        turbo::Time cs = turbo::Time::from_time_t(config.time());
        result_table.add_row(collie::table::Table::Row_t{"time", turbo::Time::format(cs)});
        last = result_table.size() - 1;
        result_table[last].format().font_color(collie::Color::green);
        if (!ConfigOptionContext::get_instance()->config_file.empty()) {
            result_table.add_row(collie::table::Table::Row_t{"file", ConfigOptionContext::get_instance()->config_file});
            last = result_table.size() - 1;
            result_table[last].format().font_color(collie::Color::green);
            result_table.add_row(collie::table::Table::Row_t{"status", save_status.ok() ? "ok" : save_status.message()});
            last = result_table.size() - 1;
            result_table[last].format().font_color(collie::Color::green);
        }

        return result_table;
    }

    void ConfigCmd::run_config_watch_cmd() {
        auto opt= ConfigOptionContext::get_instance();
        auto  rs = sirius::client::ConfigClient::get_instance()->init();
        if(opt->clean_local) {
            collie::println("remove local config cache dir:{}",opt->config_watch_dir);
            ghc::filesystem::remove_all(opt->config_watch_dir);
        }

        if(!ghc::filesystem::exists(opt->config_watch_dir)) {
            ghc::filesystem::create_directories(opt->config_watch_dir);
        }
        if(!rs.ok()) {
            collie::println("watch error:{}", rs.to_string());
        }

        auto new_config_func = [](const sirius::client::ConfigCallbackData &data) ->void  {
            auto opt= ConfigOptionContext::get_instance();
            auto rs = save_config_to_file(opt->config_watch_dir,data);
            if(rs.ok()) {
                collie::println(collie::Color::green, "on new config:{} version:{}.{}.{} type:{}", data.config_name, data.new_version.major,
                                                      data.new_version.minor, data.new_version.patch, data.type);
            } else {
                collie::println("{}", rs.to_string());
            }
            rs = sirius::client::ConfigClient::get_instance()->apply(data.config_name, data.new_version);
            if(rs.ok()) {
                collie::println(collie::Color::green, "apply new config:{} version:{}.{}.{} type:{}", data.config_name, data.new_version.major,
                               data.new_version.minor, data.new_version.patch, data.type);
            } else {
                collie::println("{}", rs.to_string());
            }
        };
        auto new_version_func = [](const sirius::client::ConfigCallbackData &data) ->void  {
            auto opt= ConfigOptionContext::get_instance();
            auto rs = save_config_to_file(opt->config_watch_dir,data);
            if(rs.ok()) {
                collie::println(collie::Color::green, "on new config version:{} version:{}.{}.{} type:{}", data.config_name, data.new_version.major,
                               data.new_version.minor, data.new_version.patch, data.type);
            }else {
                collie::println("{}", rs.to_string());
            }
            rs = sirius::client::ConfigClient::get_instance()->apply(data.config_name, data.new_version);
            if(rs.ok()) {
                collie::println(collie::Color::green, "apply new config version:{} version:{}.{}.{} type:{}", data.config_name, data.new_version.major,
                               data.new_version.minor, data.new_version.patch, data.type);
            }else {
                collie::println("{}", rs.to_string());
            }
        };
        sirius::client::ConfigEventListener listener{new_config_func, new_version_func};
        for(auto &it : opt->config_watch_list) {
            rs = sirius::client::ConfigClient::get_instance()->watch_config(it, listener);
            if(!rs.ok()) {
                collie::println(collie::Color::red,"{}", rs.to_string());
            }
        }
        while (1) {
            sleep(1);
        }
    }

    turbo::Status ConfigCmd::save_config_to_file(const std::string &basedir, const sirius::client::ConfigCallbackData &data) {
        std::string file_name = collie::format("{}/{}-{}.{}.{}.{}", basedir, data.config_name, data.new_version.major,
                                              data.new_version.minor, data.new_version.patch, data.type);
        if(ghc::filesystem::exists(file_name)) {
            return turbo::already_exists_error(turbo::substitute("write file [$0] already exists", file_name));
        }
        alkaid::SequentialWriteFile file;
        auto rs = file.open(file_name, alkaid::kDefaultTruncateWriteOption);
        if(!rs.ok()){
            return rs;
        }
        rs = file.write(data.new_content);
        if(!rs.ok()){
            return rs;
        }
        file.close();
        return turbo::OkStatus();
    }
}  // namespace sirius::cli
