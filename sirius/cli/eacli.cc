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

#include <collie/cli/cli.h>
#include <sirius/cli/option_context.h>
#include <sirius/client/discovery.h>
#include <sirius/client/base_message_sender.h>
#include <sirius/client/router_sender.h>
#include <sirius/client/discovery_sender.h>
#include <sirius/cli/raft_cmd.h>
#include <sirius/cli/discovery.h>

int main(int argc, char **argv) {
    collie::App app{"EA sirius client"};
    auto opt = sirius::cli::OptionContext::get_instance();
    app.add_flag("-V, --verbose", opt->verbose, "verbose detail message default(false)")->default_val(false);
    app.add_option("-s,--server", opt->router_server, "server address default(\"127.0.0.1:8010\")")->default_val(
            "127.0.0.1:8010");
    app.add_option("-d,--discovery_server", opt->discovery_server, "server address default(\"127.0.0.1:8010\")")->default_val(
            "127.0.0.1:8010");
    app.add_flag("-r,--router", opt->router, "server address default(false)")->default_val(false);
    app.add_option("-T,--timeout", opt->timeout_ms, "timeout ms default(2000)");
    app.add_option("-C,--connect", opt->connect_timeout_ms, "connect timeout ms default(100)");
    app.add_option("-R,--retry", opt->max_retry, "max try time default(3)");
    app.add_option("-I,--interval", opt->time_between_discovery_connect_error_ms,
                   "time between discovery server connect error ms default(1000)");
    app.callback([&app] {
        if (app.get_subcommands().empty()) {
            collie::println("{}", app.help());
        }
    });

    auto func = []() {
        auto opt = sirius::cli::OptionContext::get_instance();
        if (opt->verbose) {
            collie::println("cli verbose all operations");
        }
        sirius::client::BaseMessageSender *sender{nullptr};
        if (opt->router) {
            auto rs = sirius::client::RouterSender::get_instance()->init(opt->router_server);
            if (!rs.ok()) {
                collie::println(rs.message());
                exit(0);
            }
            sirius::client::RouterSender::get_instance()->set_connect_time_out(opt->connect_timeout_ms)
                    .set_interval_time(opt->time_between_discovery_connect_error_ms)
                    .set_retry_time(opt->max_retry)
                    .set_verbose(opt->verbose);
            sender = sirius::client::RouterSender::get_instance();
            SS_LOG_IF(INFO, opt->verbose) << "init connect success to router server " << opt->router_server;
        } else {
            sirius::client::DiscoverySender::get_instance()->set_connect_time_out(opt->connect_timeout_ms)
                    .set_interval_time(opt->time_between_discovery_connect_error_ms)
                    .set_retry_time(opt->max_retry)
                    .set_verbose(opt->verbose);
            auto rs = sirius::client::DiscoverySender::get_instance()->init(opt->discovery_server);
            if (!rs.ok()) {
                collie::println("{}", rs.message());
                exit(0);
            }
            sender = sirius::client::DiscoverySender::get_instance();
            SS_LOG_IF(INFO, opt->verbose) << "init connect success to discovery server " << opt->discovery_server;
        }
        auto r = sirius::client::DiscoveryClient::get_instance()->init(sender);
        if (!r.ok()) {
            collie::println("set up discovery server error:{}", r.message());
            exit(0);
        }
    };
    app.parse_complete_callback(func);

    // Call the setup functions for the subcommands.
    // They are kept alive by a shared pointer in the
    // lambda function
    sirius::cli::RaftCmd::setup_raft_cmd(app);
    sirius::cli::DiscoveryCmd::setup_discovery_cmd(app);
    // More setup if needed, i.e., other subcommands etc.

    COLLIE_CLI_PARSE(app, argc, argv);

    return 0;
}
