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
#include <collie/strings/format.h>
#include <sirius/proto/discovery.interface.pb.h>
#include <mizar/db.h>
#include <mizar/utilities/memory_util.h>
#include <collie/rapidjson/rapidjson.h>
#include <collie/rapidjson/reader.h>
#include <melon/raft/log_entry.h>
#include <melon/utility/endpoint.h>

namespace fmt {

    template<>
    struct formatter<mizar::Status::Code> : public formatter<int> {
        auto format(const mizar::Status::Code& a, format_context& ctx) const {
            return formatter<int>::format(static_cast<int>(a), ctx);
        }
    };

    template<>
    struct formatter<sirius::proto::OpType> : public formatter<int> {
        auto format(const sirius::proto::OpType& a, format_context& ctx) const {
            return formatter<int>::format(static_cast<int>(a), ctx);
        }
    };

    template<>
    struct formatter<rapidjson::ParseErrorCode> : public formatter<int> {
        auto format(const rapidjson::ParseErrorCode& a, format_context& ctx) const {
            return formatter<int>::format(static_cast<int>(a), ctx);
        }
    };

    template<>
    struct formatter<melon::raft::EntryType> : public formatter<int> {
        auto format(const melon::raft::EntryType& a, format_context& ctx) const {
            return formatter<int>::format(static_cast<int>(a), ctx);
        }
    };

    template<>
    struct formatter<::sirius::proto::RaftControlOp> : public formatter<int> {
        auto format(const ::sirius::proto::RaftControlOp& a, format_context& ctx) const {
            return formatter<int>::format(static_cast<int>(a), ctx);
        }
    };

    template<>
    struct formatter<mizar::MemoryUtil::UsageType> : public formatter<int> {
        auto format(const mizar::MemoryUtil::UsageType& a, format_context& ctx) const {
            return formatter<int>::format(static_cast<int>(a), ctx);
        }
    };

    template<>
    struct formatter<mutil::EndPoint> : public formatter<std::string> {
        auto format(const mutil::EndPoint& a, format_context& ctx) const {
            return formatter<std::string>::format(mutil::endpoint2str(a).c_str(), ctx);
        }
    };

    template<>
    struct formatter<mutil::EndPointStr> : public formatter<std::string> {
        auto format(const mutil::EndPointStr& a, format_context& ctx) const {
            return formatter<std::string>::format(a.c_str(), ctx);
        }
    };
}  // namespace fmt