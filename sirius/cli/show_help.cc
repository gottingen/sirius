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

#include <sirius/cli/show_help.h>
#include <sirius/cli/proto_help.h>
#include <turbo/times/civil_time.h>
#include <sirius/cli/option_context.h>
#include <sirius/client/discovery_sender.h>
#include <collie/meta/reflect.h>

namespace sirius::cli {
    using Row_t = collie::table::Table::Row_t;

    ShowHelper::~ShowHelper() {
        std::cout << pre_send_result << std::endl;
        std::cout << rpc_result << std::endl;
        std::cout << result_table << std::endl;
    }

    collie::table::Table ShowHelper::show_response_impl(const std::string_view &server, sirius::proto::ErrCode code, int qt, const std::string &qts, const std::string &msg) {
        collie::table::Table response_result;
        std::string server_role;
        std::string server_addr;
        auto opt = sirius::cli::OptionContext::get_instance();
        if(opt->router) {
            server_role = "router";
            server_addr = opt->router_server;
        } else {
            server_role = "discovery leader";
            server_addr = opt->discovery_server;
        }
        response_result.add_row(Row_t{"status", server_role, "op code", "op string", "error code", "error message"});
        if (code != sirius::proto::SUCCESS) {
            response_result.add_row(
                    Row_t{"fail", server_addr, collie::to_str(qt),qts,NAMEOF_ENUM(code), msg});
        } else {
            response_result.add_row(
                    Row_t{"success", server_addr, collie::to_str(qt),qts,NAMEOF_ENUM(code), msg});
        }
        auto last = response_result.size() - 1;
        response_result[last][0].format().font_color(collie::Color::green).font_style({collie::FontStyle::bold});

        if (code == sirius::proto::SUCCESS) {
            response_result[last][1].format().font_color(collie::Color::yellow);
        } else {
            response_result[last][1].format().font_color(collie::Color::red);
        }
        return response_result;
    }

    collie::table::Table ShowHelper::show_response_impl(sirius::proto::ErrCode code, int qt, const std::string &qts, const std::string &msg) {
        collie::table::Table response_result;
        std::string server_role;
        std::string server_addr;
        auto opt = sirius::cli::OptionContext::get_instance();
        if(opt->router) {
            server_role = "router";
            server_addr = opt->router_server;
        } else {
            server_role = "discovery leader";
            server_addr = sirius::client::DiscoverySender::get_instance()->get_leader();
        }
        response_result.add_row(Row_t{"status", server_role, "op code", "op string", "error code", "error message"});
        if (code != sirius::proto::SUCCESS) {
            response_result.add_row(
                    Row_t{"fail", server_addr, collie::to_str(qt),qts,NAMEOF_ENUM(code), msg});
        } else {
            response_result.add_row(
                    Row_t{"success", server_addr, collie::to_str(qt),qts,NAMEOF_ENUM(code), msg});
        }
        auto last = response_result.size() - 1;
        response_result[last][0].format().font_color(collie::Color::green).font_style({collie::FontStyle::bold});

        if (code == sirius::proto::SUCCESS) {
            response_result[last][1].format().font_color(collie::Color::yellow);
        } else {
            response_result[last][1].format().font_color(collie::Color::red);
        }
        return response_result;
    }

    collie::table::Table ShowHelper::rpc_error_status_impl(const collie::Status &s, int qt, const std::string &qts) {
        collie::table::Table result;
        result.add_row(Row_t{"status", "op code", "op string", "error code", "error message"});
        auto last = result.size() - 1;
        result[last].format().font_color(collie::Color::yellow).font_style({collie::FontStyle::bold});
        if (s.ok()) {
            result.add_row(
                    Row_t{"success", collie::to_str(qt), qts,
                          collie::format("{}", static_cast<int>(s.code())), s.message()});
            last = result.size() - 1;
            result[last][0].format().font_color(collie::Color::green);
            result[last][1].format().font_color(collie::Color::yellow);
            result[last][2].format().font_color(collie::Color::yellow);
            result[last][3].format().font_color(collie::Color::green);
            result[last][4].format().font_color(collie::Color::green);
        } else {
            result.add_row(
                    Row_t{"fail", collie::to_str(qt), qts,
                          collie::format("{}", static_cast<int>(s.code())), s.message()});
            last = result.size() - 1;
            result[last][0].format().font_color(collie::Color::red);
            result[last][1].format().font_color(collie::Color::yellow);
            result[last][2].format().font_color(collie::Color::yellow);
            result[last][3].format().font_color(collie::Color::red);
            result[last][4].format().font_color(collie::Color::red);
        }
        return result;
    }

    collie::table::Table ShowHelper::pre_send_error(const collie::Status &s, const sirius::proto::DiscoveryManagerRequest &req) {
        collie::table::Table result;
        result.add_row(Row_t{"status", "op code", "op string", "error message"});
        result[0].format().font_color(collie::Color::green).font_style({collie::FontStyle::bold}).font_align(
                collie::table::FontAlign::center);
        if (!req.has_op_type()) {
            result.add_row(Row_t{"fail", "nil", "nil", "op_type field is required but not set not set"});
            auto last = result.size() - 1;
            result[last].format().font_color(collie::Color::red).font_style(
                    {collie::FontStyle::bold}).font_align(
                    collie::table::FontAlign::center);
        } else if (!s.ok()) {
            result.add_row(
                    Row_t{"fail", collie::format("{}", static_cast<int>(req.op_type())), get_op_string(req.op_type()),
                          s.message()});
            auto last = result.size() - 1;
            result[last][0].format().font_color(collie::Color::red).font_style(
                    {collie::FontStyle::bold}).font_align(
                    collie::table::FontAlign::center);
        } else {
            result.add_row(
                    Row_t{"success", collie::format("{}", static_cast<int>(req.op_type())), get_op_string(req.op_type()),
                          s.message()});
            auto last = result.size() - 1;
            result[last][0].format().font_color(collie::Color::green).font_style(
                    {collie::FontStyle::bold}).font_align(
                    collie::table::FontAlign::center);
        }
        return result;
    }

    collie::table::Table ShowHelper::pre_send_error(const collie::Status &s, const sirius::proto::DiscoveryQueryRequest &req) {
        collie::table::Table result;
        result.add_row(Row_t{"status", "op code", "op string", "error message"});
        result[0].format().font_color(collie::Color::green).font_style({collie::FontStyle::bold}).font_align(
                collie::table::FontAlign::center);
        if (!req.has_op_type()) {
            result.add_row(Row_t{"fail", "nil", "nil", "op_type field is required but not set not set"});
            auto last = result.size() - 1;
            result[last].format().font_color(collie::Color::red).font_style(
                    {collie::FontStyle::bold}).font_align(
                    collie::table::FontAlign::center);
        } else if (!s.ok()) {
            result.add_row(
                    Row_t{"fail", collie::format("{}", static_cast<int>(req.op_type())),
                          get_op_string(req.op_type()),
                          s.message()});
            auto last = result.size() - 1;
            result[last][0].format().font_color(collie::Color::red).font_style(
                    {collie::FontStyle::bold}).font_align(
                    collie::table::FontAlign::center);
        } else {
            result.add_row(
                    Row_t{"success", collie::format("{}", static_cast<int>(req.op_type())),
                          get_op_string(req.op_type()),
                          s.message()});
            auto last = result.size() - 1;
            result[last][0].format().font_color(collie::Color::green).font_style(
                    {collie::FontStyle::bold}).font_align(
                    collie::table::FontAlign::center);
        }
        return result;
    }

    collie::table::Table ShowHelper::pre_send_error(const collie::Status &s, const sirius::proto::RaftControlRequest &req) {
        collie::table::Table result;
        result.add_row(Row_t{"status", "op code", "op string", "error message"});
        result[0].format().font_color(collie::Color::green).font_style({collie::FontStyle::bold}).font_align(
                collie::table::FontAlign::center);
        if (!req.has_op_type()) {
            result.add_row(Row_t{"fail", "nil", "nil", "op_type field is required but not set not set"});
            auto last = result.size() - 1;
            result[last].format().font_color(collie::Color::red).font_style(
                    {collie::FontStyle::bold}).font_align(
                    collie::table::FontAlign::center);
        } else if (!s.ok()) {
            result.add_row(
                    Row_t{"fail", collie::format("{}", static_cast<int>(req.op_type())),
                          get_op_string(req.op_type()),
                          s.message()});
            auto last = result.size() - 1;
            result[last][0].format().font_color(collie::Color::red).font_style(
                    {collie::FontStyle::bold}).font_align(
                    collie::table::FontAlign::center);
        } else {
            result.add_row(
                    Row_t{"success", collie::format("{}", static_cast<int>(req.op_type())),
                          get_op_string(req.op_type()),
                          s.message()});
            auto last = result.size() - 1;
            result[last][0].format().font_color(collie::Color::green).font_style(
                    {collie::FontStyle::bold}).font_align(
                    collie::table::FontAlign::center);
        }
        return result;
    }

    ScopeShower::ScopeShower() {
        result_table.add_row({"phase","status"});
        result_table[0].format().font_color(collie::Color::blue);
        result_table[0][1].format().font_align(collie::table::FontAlign::left);
        result_table[0][1].format().font_align(collie::table::FontAlign::center);
    }

    ScopeShower::ScopeShower(const std::string & operation) {
        result_table.add_row({"operation", operation});
        result_table[0][0].format().font_color(collie::Color::yellow).font_align(collie::table::FontAlign::left);
        result_table[0][1].format().font_color(collie::Color::magenta).font_align(collie::table::FontAlign::center);
        result_table.add_row({"phase","status"});
        result_table[1].format().font_color(collie::Color::blue);
        result_table[1][1].format().font_align(collie::table::FontAlign::left);
        result_table[1][1].format().font_align(collie::table::FontAlign::center);

    }
    ScopeShower::~ScopeShower() {
        /*for (auto &it : tables) {
            std::cout<<it<<std::endl;
        }*/
        std::cout<<result_table<<std::endl;
        //std::cout<<std::endl;
    }

    void ScopeShower::add_table(collie::table::Table &&table) {
        tables.push_back(std::move(table));
    }
    void ScopeShower::add_table(const std::string &stage, collie::table::Table &&table, bool ok) {
        auto &t = result_table.add_row({stage,std::move(table)});
        auto last = result_table.size() - 1;
        result_table[last][0].format().font_color(collie::Color::yellow);
        if(ok) {
            result_table[last][1].format().font_color(collie::Color::green);
        } else {
            result_table[last][1].format().font_color(collie::Color::red);
        }
    }

    void ScopeShower::add_table(const std::string &stage, const std::string &msg, bool ok) {
        result_table.add_row({stage,msg});
        auto last = result_table.size() - 1;
        result_table[last][0].format().font_color(collie::Color::yellow);
        if(ok) {
            result_table[last][1].format().font_color(collie::Color::green).font_align(collie::table::FontAlign::center);
        } else {
            result_table[last][1].format().font_color(collie::Color::red).font_align(collie::table::FontAlign::center);
        }
    }

    void ScopeShower::prepare(const collie::Status &status) {
        if(status.ok()) {
            result_table.add_row({"prepare", collie::table::Table().add_row({"ok"})});
            result_table[2][1].format().font_color(collie::Color::green)
                    .font_align(collie::table::FontAlign::center)
                    .font_style({collie::FontStyle::concealed});
        } else {
            result_table.add_row({"prepare", collie::table::Table().add_row({"ok"})});
            result_table[2][1].format().font_color(collie::Color::red)
            .font_align(collie::table::FontAlign::center)
            .font_style({collie::FontStyle::concealed});
        }
        result_table[2][0].format().font_color(collie::Color::yellow);
    }

    std::string ShowHelper::json_format(const std::string &json_str) {
        std::string result = "";
        int level = 0;
        for (std::string::size_type index = 0; index < json_str.size(); index++) {
            char c = json_str[index];

            if (level > 0 && '\n' == json_str[json_str.size() - 1]) {
                result += get_level_str(level);
            }

            switch (c) {
                case '{':
                case '[':
                    result = result + c + "\n";
                    level++;
                    result += get_level_str(level);
                    break;
                case ',':
                    result = result + c + "\n";
                    result += get_level_str(level);
                    break;
                case '}':
                case ']':
                    result += "\n";
                    level--;
                    result += get_level_str(level);
                    result += c;
                    break;
                default:
                    result += c;
                    break;
            }

        }
        return result;
    }

    std::string ShowHelper::get_level_str(int level) {
        std::string levelStr = "";
        for (int i = 0; i < level; i++) {
            levelStr += "\t";
        }
        return levelStr;

    }

}  // namespace sirius::cli
