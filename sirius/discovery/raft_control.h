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


#pragma once

#include <melon/raft/raft.h>
#include <melon/rpc/channel.h>
#include <melon/rpc/server.h>
#include <melon/rpc/controller.h>
#include <sirius/proto/raft.pb.h>

namespace sirius {
    extern void common_raft_control(google::protobuf::RpcController *controller,
                                    const sirius::proto::RaftControlRequest *request,
                                    sirius::proto::RaftControlResponse *response,
                                    google::protobuf::Closure *done,
                                    melon::raft::Node *node);
}

