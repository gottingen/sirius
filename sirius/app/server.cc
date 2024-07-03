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
#include <sirius/discovery/sirius_server.h>
#include <sirius/storage/rocks_storage.h>
#include <sirius/base/memory_profile.h>
#include <collie/filesystem/fs.h>
#include <sirius/flags/sirius.h>
#include <sirius/service/registry.h>
#include <melon/rpc/webui.h>
#include <turbo/flags/servlet.h>
#include <sirius/service/sirius_service.h>

int main(int argc, char **argv) {
    VLOG(201)<< "111111111111111111111111111111111111111111111111111111";
    LOG(INFO)<< "ea discovery server start";
    turbo::Servlet &servlet = turbo::Servlet::instance();
    auto  [exit, code] = servlet.run(argc, argv);
    if (exit) {
        return code;
    }

    ghc::filesystem::path remove_path("init.success");
    ghc::filesystem::remove_all(remove_path);

    //add service
    melon::Server server;

    if (0 != melon::raft::add_service(&server, turbo::get_flag(FLAGS_sirius_listen).c_str())) {
        LOG(ERROR) << "Fail to init raft";
        return -1;
    }
    LOG(INFO) << "add raft to discovery server success";

    int ret = 0;
    //this step must be before server.Start
    std::vector<melon::raft::PeerId> peers;
    std::vector<std::string> instances;
    bool completely_deploy = false;

    for (auto &raft_peer: turbo::get_flag(FLAGS_sirius_server_peers)) {
        LOG(INFO)<< "raft_peer:" << raft_peer.c_str();
        melon::raft::PeerId peer(raft_peer);
        peers.push_back(peer);
    }

    auto *discovery_server = sirius::discovery::DiscoveryServer::get_instance();
    auto *sirius_service = sirius::SiriusServiceImpl::instance();
    auto *sns_service = sirius::SnsServiceImpl::instance();
    LOG(INFO)<< "start webui";
    melon::WebuiConfig webui_config = melon::WebuiConfig::default_config();
    webui_config.root_path = "www";
    webui_config.mapping_path = "/ea/ui";
    auto *instance = melon::WebuiService::instance();
    auto rs = instance->register_server(webui_config, &server);
    if(!rs.ok()) {
        LOG(ERROR) << "register webui failed: " << rs;
        return -1;
    }
    rs = sirius::restful::registry_server(&server);
    if (!rs.ok()) {
        LOG(ERROR) << "Fail to registry restful service " << rs.message();
        return -1;
    }
    rs = sirius_service->initialize(turbo::get_flag(FLAGS_sirius_server_peers));
    if (!rs.ok()) {
        LOG(ERROR) << "Fail init sirius service " << rs.message();
        return -1;
    }

    if (0 != server.AddService(sirius_service, melon::SERVER_DOESNT_OWN_SERVICE)) {
        LOG(ERROR) << "Fail to Add sirius Service";
        return -1;
    }
    if (0 != server.AddService(sns_service, melon::SERVER_DOESNT_OWN_SERVICE)) {
        LOG(ERROR) << "Fail to Add sns Service";
        return -1;
    }
    // registry discovery service
    if (0 != server.AddService(discovery_server, melon::SERVER_DOESNT_OWN_SERVICE)) {
        LOG(ERROR) << "Fail to Add discovery Service";
        return -1;
    }

    // enable ports
    if (server.Start(turbo::get_flag(FLAGS_sirius_listen).c_str(), nullptr) != 0) {
        LOG(ERROR) << "Fail to start server";
        return -1;
    }
    LOG(INFO)<< "ea discovery server start";
    if (discovery_server->init(peers) != 0) {
        LOG(ERROR) << "discovery server init fail";
        return -1;
    }
    sirius::MemoryGCHandler::get_instance()->init();
    while (!discovery_server->have_data()) {
        fiber_usleep(1000 * 1000);
    }
    std::ofstream init_fs("init.success", std::ofstream::out | std::ofstream::trunc);
    LOG(INFO) << "ea discovery server init success";
    while (!melon::IsAskedToQuit()) {
        fiber_usleep(1000000L);
    }
    LOG(INFO) << "receive kill signal, begin to quit";
    discovery_server->shutdown_raft();
    LOG(INFO) << "ea discovery server shutdown raft";
    discovery_server->close();
    LOG(INFO) << "ea discovery server close";
    sirius::MemoryGCHandler::get_instance()->close();
    LOG(INFO) << "ea discovery server close MemoryGCHandler";
    sirius::RocksStorage::get_instance()->close();
    LOG(INFO) << "ea discovery server close RocksStorage";
    server.Stop(0);
    server.Join();
    LOG(INFO) << "ea discovery server quit success";
    return 0;
}

