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

#include <sirius/proto/discovery.interface.pb.h>
#include <collie/utility/result.h>

namespace sirius::cli {

    std::string config_type_to_string(sirius::proto::ConfigType type);

    collie::Result<sirius::proto::ConfigType> string_to_config_type(const std::string &str);

    std::string get_op_string(sirius::proto::OpType type);

    std::string get_op_string(sirius::proto::RaftControlOp type);

    std::string get_op_string(sirius::proto::QueryOpType type);

    collie::Status string_to_version(const std::string &str, sirius::proto::Version*v);

    std::string version_to_string(const sirius::proto::Version &v);


}  // namespace sirius::cli
