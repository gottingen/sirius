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


#include <sirius/discovery/raft_control.h>
#include <algorithm>
#include <sirius/base/log.h>

namespace melon::raft {
    DECLARE_int32(raft_election_heartbeat_factor);
}
namespace sirius {
    class RaftControlDone : public melon::raft::Closure {
    public:
        RaftControlDone(google::protobuf::RpcController *controller,
                        const sirius::proto::RaftControlRequest *request,
                        sirius::proto::RaftControlResponse *response,
                        google::protobuf::Closure *done,
                        melon::raft::Node *node)
                : _controller(controller),
                  _request(request),
                  _response(response),
                  _done(done),
                  _node(node) {}

        ~RaftControlDone() override {}

        void Run() override{
            auto cntl = static_cast<melon::Controller *>(_controller);
            uint64_t log_id = 0;
            if (cntl->has_log_id()) {
                log_id = cntl->log_id();
            }
            if (status().ok()) {
                LOG(INFO)<< "node:" << _node->node_id().group_id
                            << " " << _node->node_id().peer_id.to_string()
                            << " raft control success, type:" << _request->op_type()
                            << " region_id:" << _request->region_id()
                            << " log_id:" << log_id;
                _response->set_errcode(sirius::proto::SUCCESS);
            } else {
                LOG(WARNING) << "node:" << _node->node_id().group_id
                             << " " << _node->node_id().peer_id.to_string()
                             << " raft control fail, status:" << status().error_code()
                             << " " << status().error_cstr()
                             << " log_id:" << log_id;
                _response->set_errcode(sirius::proto::INTERNAL_ERROR);
                _response->set_errmsg(status().error_cstr());
                _response->set_leader(mutil::endpoint2str(_node->leader_id().addr).c_str());
            }
            //UpdateRegionStatus::get_instance()->reset_region_status(_request->region_id());
            _done->Run();
            delete this;
        }

    private:
        google::protobuf::RpcController *_controller;
        const sirius::proto::RaftControlRequest *_request;
        sirius::proto::RaftControlResponse *_response;
        google::protobuf::Closure *_done;
        melon::raft::Node *_node;
    };

    void _set_peer(google::protobuf::RpcController *controller,
                   const sirius::proto::RaftControlRequest *request,
                   sirius::proto::RaftControlResponse *response,
                   google::protobuf::Closure *done,
                   melon::raft::Node *node);

    int _diff_peers(const std::vector<melon::raft::PeerId> &old_peers,
                    const std::vector<melon::raft::PeerId> &new_peers, melon::raft::PeerId *peer);

    void _trans_leader(google::protobuf::RpcController *controller,
                       const sirius::proto::RaftControlRequest *request,
                       sirius::proto::RaftControlResponse *response,
                       google::protobuf::Closure *done,
                       melon::raft::Node *node);

    void common_raft_control(google::protobuf::RpcController *controller,
                             const sirius::proto::RaftControlRequest *request,
                             sirius::proto::RaftControlResponse *response,
                             google::protobuf::Closure *done,
                             melon::raft::Node *node) {
        auto cntl = static_cast<melon::Controller *>(controller);
        uint64_t log_id = 0;
        if (cntl->has_log_id()) {
            log_id = cntl->log_id();
        }
        response->set_region_id(request->region_id());
        melon::ClosureGuard done_guard(done);


        switch (request->op_type()) {
            case sirius::proto::SetPeer : {
                _set_peer(controller, request, response, done_guard.release(), node);
                return;
            }
            case sirius::proto::SnapShot : {
                RaftControlDone *snapshot_done =
                        new RaftControlDone(cntl, request, response, done_guard.release(), node);
                node->snapshot(snapshot_done);
                return;
            }
            case sirius::proto::ShutDown : {
                RaftControlDone *shutdown_done =
                        new RaftControlDone(cntl, request, response, done_guard.release(), node);
                node->shutdown(shutdown_done);
                return;
            }
            case sirius::proto::TransLeader : {
                _trans_leader(controller, request, response, done_guard.release(), node);
                return;
            }
            case sirius::proto::GetLeader : {
                mutil::EndPoint leader_addr = node->leader_id().addr;
                if (leader_addr != mutil::EndPoint()) {
                    response->set_errcode(sirius::proto::SUCCESS);
                    response->set_leader(mutil::endpoint2str(leader_addr).c_str());
                } else {
                    LOG(ERROR) << "node:" << node->node_id().group_id
                                 << " " << node->node_id().peer_id.to_string()
                                 << " get leader fail, log_id:" << log_id;
                    response->set_errcode(sirius::proto::INTERNAL_ERROR);
                    response->set_errmsg("get leader fail");
                }
                return;
            }
            case sirius::proto::ListPeer: {
                mutil::EndPoint leader_addr = node->leader_id().addr;
                if (leader_addr != mutil::EndPoint()) {
                    response->set_leader(mutil::endpoint2str(leader_addr).c_str());
                    std::vector<melon::raft::PeerId> peers;
                    auto s = node->list_peers(&peers);
                    if(!s.ok()) {
                        LOG(ERROR) << "node:" << node->node_id().group_id
                                     << " " << node->node_id().peer_id.to_string()
                                     << " list peers fail, log_id:" << log_id;
                        response->set_errcode(sirius::proto::INTERNAL_ERROR);
                        response->set_errmsg("list peers fail");
                        return;
                    }
                    for(auto &peer : peers) {
                        response->add_peers(mutil::endpoint2str(peer.addr).c_str());
                    }
                    response->set_errcode(sirius::proto::SUCCESS);
                    return;
                } else {
                    LOG(ERROR) << "node:" << node->node_id().group_id
                                 << " " << node->node_id().peer_id.to_string()
                                 << " get leader fail, log_id:" << log_id;
                    response->set_errcode(sirius::proto::INTERNAL_ERROR);
                    response->set_errmsg("get leader fail");
                }
                return;
            }
            case sirius::proto::ResetVoteTime : {
                node->reset_election_timeout_ms(request->election_time());
                response->set_errcode(sirius::proto::SUCCESS);
                return;
            }
            default:
                LOG(ERROR) << "node:" << node->node_id().group_id
                             << " " << node->node_id().peer_id.to_string()
                             << " unsupport request type:" << request->op_type()
                             << " log_id:" << log_id;
                return;
        }
    }

    void _set_peer(google::protobuf::RpcController *controller,
                   const sirius::proto::RaftControlRequest *request,
                   sirius::proto::RaftControlResponse *response,
                   google::protobuf::Closure *done,
                   melon::raft::Node *node) {
        auto cntl =
                static_cast<melon::Controller *>(controller);
        melon::ClosureGuard done_guard(done);
        uint64_t log_id = 0;
        if (cntl->has_log_id()) {
            log_id = cntl->log_id();
        }
        bool is_force = request->has_force() && request->force();
        std::vector<melon::raft::PeerId> old_peers;
        std::vector<melon::raft::PeerId> new_peers;
        for (int i = 0; i < request->old_peers_size(); i++) {
            melon::raft::PeerId peer;
            if (peer.parse(request->old_peers(i)) != 0) {
                response->set_errcode(sirius::proto::INPUT_PARAM_ERROR);
                response->set_errmsg("old peer parse fail");
                return;
            }
            old_peers.push_back(peer);
        }
        for (int i = 0; i < request->new_peers_size(); i++) {
            melon::raft::PeerId peer;
            if (peer.parse(request->new_peers(i)) != 0) {
                response->set_errcode(sirius::proto::INPUT_PARAM_ERROR);
                response->set_errmsg("new peer parse fail");
                return;
            }
            new_peers.push_back(peer);
        }
        if (is_force) {
            melon::raft::Configuration new_conf(new_peers);
            auto status = node->reset_peers(new_conf);
            if (!status.ok()) {
                LOG(ERROR) << "node:" << node->node_id().group_id
                             << " " << node->node_id().peer_id.to_string()
                             << " set peer fail, status:" << status.error_code()
                                << " " << status.error_cstr()
                                << " log_id:" << log_id;
                response->set_errcode(sirius::proto::INTERNAL_ERROR);
                response->set_errmsg("force set peer fail");
            } else {
                response->set_errcode(sirius::proto::SUCCESS);
                response->set_errmsg("force set peer success");
            }
            return;
        }
        std::vector<melon::raft::PeerId> inner_peers;
        auto status = node->list_peers(&inner_peers);
        if (!status.ok() && status.error_code() == 1) {
            response->set_errcode(sirius::proto::NOT_LEADER);
            response->set_leader(mutil::endpoint2str(node->leader_id().addr).c_str());
            LOG(WARNING) << "node:" << node->node_id().group_id
                         << " " << node->node_id().peer_id.to_string()
                         << " list peers fail, not leader, status:" << status.error_code()
                         << " " << status.error_cstr()
                         << " log_id:" << log_id;
            return;
        }
        if (!status.ok()) {
            response->set_errcode(sirius::proto::PEER_NOT_EQUAL);
            response->set_errmsg("node list peer fail");
            LOG(WARNING) << "node:" << node->node_id().group_id
                         << " " << node->node_id().peer_id.to_string()
                         << " list peers fail, status:" << status.error_code()
                            << " " << status.error_cstr()
                            << " log_id:" << log_id;
            return;
        }
        if (inner_peers.size() != old_peers.size()) {
            LOG(WARNING) << "peer size is not equal when set peer, node:" << node->node_id().group_id
                         << " " << node->node_id().peer_id.to_string()
                         << " inner_peer.size: " << inner_peers.size()
                         << " old_peer.size: " << old_peers.size()
                         << " remote_side: " << mutil::endpoint2str(cntl->remote_side())
                         << " log_id:" << log_id;
            response->set_errcode(sirius::proto::PEER_NOT_EQUAL);
            response->set_errmsg("peer size not equal");
            return;
        }
        for (auto &inner_peer: inner_peers) {
            auto iter = std::find(old_peers.begin(), old_peers.end(), inner_peer);
            if (iter == old_peers.end()) {
                LOG(WARNING) << "old_peer not equal to list peers, node:" << node->node_id().group_id
                             << " " << node->node_id().peer_id.to_string()
                             << " inner_peer: " << mutil::endpoint2str(inner_peer.addr)
                             << " log_id:" << log_id;
                response->set_errcode(sirius::proto::PEER_NOT_EQUAL);
                response->set_errmsg("peer not equal");
                return;
            }
        }
        melon::raft::PeerId peer;
        // add peer
        if (new_peers.size() == old_peers.size() + 1) {
            if (0 == _diff_peers(old_peers, new_peers, &peer)) {
                auto set_peer_done =
                        new RaftControlDone(cntl, request, response, done_guard.release(), node);
                node->add_peer(peer, set_peer_done);
            } else {
                response->set_errcode(sirius::proto::INPUT_PARAM_ERROR);
                response->set_errmsg("diff peer fail when add peer");
                LOG(ERROR) << "node:" << node->node_id().group_id
                             << " " << node->node_id().peer_id.to_string()
                             << " set peer fail, log_id:" << log_id;
                return;
            }
        } else if (old_peers.size() == new_peers.size() + 1) {
            if (0 == _diff_peers(old_peers, new_peers, &peer)) {
                bool self_faulty = false;
                bool other_faulty = false;
                melon::raft::NodeStatus status;
                node->get_status(&status);
                for (auto iter: status.stable_followers) {
                    if (iter.first == peer) {
                        if (iter.second.consecutive_error_times > melon::raft::FLAGS_raft_election_heartbeat_factor) {
                            self_faulty = true;
                            break;
                        }
                    } else {
                        if (iter.second.consecutive_error_times > melon::raft::FLAGS_raft_election_heartbeat_factor) {
                            LOG(WARNING) << "node:" << node->node_id().group_id
                                         << " " << node->node_id().peer_id.to_string()
                                         << " peer:" << iter.first.to_string()
                                         << " is faulty, log_id:" << log_id;
                            other_faulty = true;
                        }
                    }
                }
                if (self_faulty || (!self_faulty && !other_faulty)) {
                    auto  set_peer_done =
                            new RaftControlDone(cntl, request, response, done_guard.release(), node);
                    node->remove_peer(peer, set_peer_done);
                } else {
                    response->set_errcode(sirius::proto::INPUT_PARAM_ERROR);
                    response->set_errmsg("other peer is faulty");
                    LOG(ERROR) << "node:" << node->node_id().group_id
                                 << " " << node->node_id().peer_id.to_string()
                                 << " set peer fail, log_id:" << log_id;
                    return;
                }
            } else {
                response->set_errcode(sirius::proto::INPUT_PARAM_ERROR);
                response->set_errmsg("diff peer fail when remove peer");
                LOG(WARNING) << "node:" << node->node_id().group_id
                             << " " << node->node_id().peer_id.to_string()
                             << " set peer fail, log_id:" << log_id;
                return;
            }
        } else {
            response->set_errcode(sirius::proto::INPUT_PARAM_ERROR);
            response->set_errmsg("set peer fail");
            LOG(WARNING) << "node:" << node->node_id().group_id
                         << " " << node->node_id().peer_id.to_string()
                         << " set peer fail, log_id:" << log_id;
        }
    }

    void _trans_leader(google::protobuf::RpcController *controller,
                       const sirius::proto::RaftControlRequest *request,
                       sirius::proto::RaftControlResponse *response,
                       google::protobuf::Closure *done,
                       melon::raft::Node *node) {
        melon::ClosureGuard done_guard(done);
        auto cntl =
                static_cast<melon::Controller *>(controller);
        uint64_t log_id = 0;
        if (cntl->has_log_id()) {
            log_id = cntl->log_id();
        }
        melon::raft::PeerId peer;
        if (peer.parse(request->new_leader()) != 0) {
            response->set_errcode(sirius::proto::INPUT_PARAM_ERROR);
            response->set_errmsg("new leader parse fail");
            return;
        }
        // return 0 or -1
        int ret = node->transfer_leadership_to(peer);
        if (ret != 0) {
            response->set_errcode(sirius::proto::NOT_LEADER);
            response->set_leader(mutil::endpoint2str(node->leader_id().addr).c_str());
            LOG(WARNING) << "node:" << node->node_id().group_id
                         << " " << node->node_id().peer_id.to_string()
                         << " transfer leader fail, log_id:" << log_id;
            return;
        }
        std::vector<melon::raft::PeerId> peers;
        auto s = node->list_peers(&peers);
        if(!s.ok()) {
            LOG(ERROR) << "node:" << node->node_id().group_id
                          << " " << node->node_id().peer_id.to_string()
                          << " list peers fail, log_id:" << log_id;
            response->set_errcode(sirius::proto::INTERNAL_ERROR);
            response->set_errmsg("list peers fail");
            return;
        }
        for(auto &peer : peers) {
            response->add_peers(mutil::endpoint2str(peer.addr).c_str());
        }
        response->set_errcode(sirius::proto::SUCCESS);
        response->set_leader(request->new_leader());
    }

    int _diff_peers(const std::vector<melon::raft::PeerId> &old_peers,
                    const std::vector<melon::raft::PeerId> &new_peers, melon::raft::PeerId *peer) {
        melon::raft::Configuration old_conf(old_peers);
        melon::raft::Configuration new_conf(new_peers);
        if (old_peers.size() == new_peers.size() - 1 && new_conf.contains(old_peers)) {
            // add peer
            for (size_t i = 0; i < old_peers.size(); i++) {
                new_conf.remove_peer(old_peers[i]);
            }
            std::vector<melon::raft::PeerId> peers;
            new_conf.list_peers(&peers);
            //CHECK(1 == peers.size());
            if (1 != peers.size()) {
                return -1;
            }
            *peer = peers[0];
            return 0;
        } else if (old_peers.size() == new_peers.size() + 1 && old_conf.contains(new_peers)) {
            // remove peer
            for (size_t i = 0; i < new_peers.size(); i++) {
                old_conf.remove_peer(new_peers[i]);
            }
            std::vector<melon::raft::PeerId> peers;
            old_conf.list_peers(&peers);
            if (1 != peers.size()) {
                return -1;
            }
            *peer = peers[0];
            return 0;
        } else {
            return -1;
        }
    }

}//namespace

