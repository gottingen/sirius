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


#include <string>
#include <fstream>
#include <melon/rpc/server.h>
#include <gflags/gflags.h>
#include <sirius/discovery/discovery_server.h>
#include <sirius/discovery/router_service.h>
#include <sirius/storage/rocks_storage.h>
#include <sirius/base/memory_profile.h>
#include <collie/filesystem/fs.h>
#include <collie/strings/str_split.h>
#include <sirius/flags/discovery.h>

int main(int argc, char **argv) {
    google::SetCommandLineOption("flagfile", "conf/discovery_gflags.conf");
    google::ParseCommandLineFlags(&argc, &argv, true);
    collie::filesystem::path remove_path("init.success");
    collie::filesystem::remove_all(remove_path);
    // Initail log
    if (!sirius::initialize_log()) {
        fprintf(stderr, "log init failed.");
        return -1;
    }
    SS_LOG(INFO) << "log file load success";

    //add service
    melon::Server server;

    if (0 != melon::raft::add_service(&server, sirius::FLAGS_discovery_listen.c_str())) {
        SS_LOG(ERROR) << "Fail to init raft";
        return -1;
    }
    SS_LOG(INFO) << "add raft to discovery server success";

    int ret = 0;
    //this step must be before server.Start
    std::vector<melon::raft::PeerId> peers;
    std::vector<std::string> instances;
    bool completely_deploy = false;

    std::vector<std::string> list_raft_peers = collie::str_split(sirius::FLAGS_discovery_server_peers, ',');
    for (auto &raft_peer: list_raft_peers) {
        SS_LOG(INFO)<< "raft_peer:" << raft_peer.c_str();
        melon::raft::PeerId peer(raft_peer);
        peers.push_back(peer);
    }

    auto *discovery_server = sirius::discovery::DiscoveryServer::get_instance();
    auto *router_server = sirius::discovery::RouterServiceImpl::get_instance();
    auto *sns_server = melon::SnsServiceImpl::get_instance();
    auto rs = router_server->init(sirius::FLAGS_discovery_server_peers);
    if (!rs.ok()) {
        SS_LOG(ERROR) << "Fail init router server " << rs.message();
        return -1;
    }
    rs = sns_server->init(sirius::FLAGS_discovery_server_peers);
    if (!rs.ok()) {
        SS_LOG(ERROR) << "Fail init sns server " << rs.message();
        return -1;
    }
    // registry discovery service
    if (0 != server.AddService(discovery_server, melon::SERVER_DOESNT_OWN_SERVICE)) {
        SS_LOG(ERROR) << "Fail to Add discovery Service";
        return -1;
    }
    // registry router service
    if (0 != server.AddService(router_server, melon::SERVER_DOESNT_OWN_SERVICE)) {
        SS_LOG(ERROR) << "Fail to Add router Service";
        return -1;
    }

    if (0 != server.AddService(sns_server, melon::SERVER_DOESNT_OWN_SERVICE)) {
        SS_LOG(ERROR) << "Fail to Add sns Service";
        return -1;
    }
    // enable ports
    if (server.Start(sirius::FLAGS_discovery_listen.c_str(), nullptr) != 0) {
        SS_LOG(ERROR) << "Fail to start server";
        return -1;
    }
    SS_LOG(INFO)<< "ea discovery server start";
    if (discovery_server->init(peers) != 0) {
        SS_LOG(ERROR) << "discovery server init fail";
        return -1;
    }
    sirius::MemoryGCHandler::get_instance()->init();
    while (!discovery_server->have_data()) {
        fiber_usleep(1000 * 1000);
    }
    std::ofstream init_fs("init.success", std::ofstream::out | std::ofstream::trunc);
    SS_LOG(INFO) << "ea discovery server init success";
    while (!melon::IsAskedToQuit()) {
        fiber_usleep(1000000L);
    }
    SS_LOG(INFO) << "receive kill signal, begin to quit";
    discovery_server->shutdown_raft();
    SS_LOG(INFO) << "ea discovery server shutdown raft";
    discovery_server->close();
    SS_LOG(INFO) << "ea discovery server close";
    sirius::MemoryGCHandler::get_instance()->close();
    SS_LOG(INFO) << "ea discovery server close MemoryGCHandler";
    sirius::RocksStorage::get_instance()->close();
    SS_LOG(INFO) << "ea discovery server close RocksStorage";
    server.Stop(0);
    server.Join();
    SS_LOG(INFO) << "ea discovery server quit success";
    return 0;
}

