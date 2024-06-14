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
#include <sirius/client/loader.h>
#include <alkaid/files/filesystem.h>
#include <melon/json2pb/json_to_pb.h>
#include <melon/json2pb/pb_to_json.h>

namespace sirius::client {

    turbo::Status Loader::load_proto(const std::string &content, google::protobuf::Message &message) {
        std::string err;
        if (!json2pb::JsonToProtoMessage(content, &message, &err)) {
            return turbo::data_loss_error(err);
        }
        return turbo::OkStatus();
    }


    turbo::Status Loader::load_proto_from_file(const std::string &path, google::protobuf::Message &message) {
        std::string config_data;
        auto lfs = alkaid::Filesystem::localfs();
        STATUS_RETURN_IF_ERROR(lfs->read_file(path, &config_data));
        return load_proto(config_data, message);
    }

}  // namespace sirius::client
