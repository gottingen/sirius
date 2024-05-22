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

#include <sirius/cli/proto_help.h>
#include <collie/strings/str_split.h>
#include <collie/strings/case_conv.h>
#include <collie/meta/reflect.h>
#include <turbo/strings/numbers.h>

namespace sirius::cli {

    std::string config_type_to_string(sirius::proto::ConfigType type) {
        switch (type) {
            case sirius::proto::CF_JSON:
                return "json";
            case sirius::proto::CF_TEXT:
                return "text";
            case sirius::proto::CF_INI:
                return "ini";
            case sirius::proto::CF_YAML:
                return "yaml";
            case sirius::proto::CF_XML:
                return "xml";
            case sirius::proto::CF_GFLAGS:
                return "gflags";
            case sirius::proto::CF_TOML:
                return "toml";
            default:
                return "unknown format";
        }
    }

    collie::Result<sirius::proto::ConfigType> string_to_config_type(const std::string &str) {
        auto lc = collie::str_to_lower(str);
        if (lc == "json") {
            return sirius::proto::CF_JSON;
        } else if (lc == "text") {
            return sirius::proto::CF_TEXT;
        } else if (lc == "ini") {
            return sirius::proto::CF_INI;
        } else if (lc == "yaml") {
            return sirius::proto::CF_YAML;
        } else if (lc == "xml") {
            return sirius::proto::CF_XML;
        } else if (lc == "gflags") {
            return sirius::proto::CF_GFLAGS;
        } else if (lc == "toml") {
            return sirius::proto::CF_TOML;
        }
        return collie::Status::invalid_argument("unknown format '{}'", str);
    }

    std::string get_op_string(sirius::proto::OpType type) {
        return sirius::proto::OpType_Name(type);
    }

    std::string get_op_string(sirius::proto::RaftControlOp type) {
        return sirius::proto::RaftControlOp_Name(type);
    }

    std::string get_op_string(sirius::proto::QueryOpType type) {
        return sirius::proto::QueryOpType_Name(type);
    }

    collie::Status string_to_version(const std::string &str, sirius::proto::Version *v) {
        std::vector<std::string> vs = collie::str_split(str, ".");
        if (vs.size() != 3) {
            return collie::Status::invalid_argument("version {} error, should be like 1.2.3", str);
        }
        int64_t m;
        if (!turbo::simple_atoi(vs[0], &m)) {
            return collie::Status::invalid_argument("version {} error, should be like 1.2.3", str);
        }
        v->set_major(m);
        if (!turbo::simple_atoi(vs[1], &m)) {
            return collie::Status::invalid_argument("version {} error, should be like 1.2.3", str);
        }
        v->set_minor(m);
        if (!turbo::simple_atoi(vs[2], &m)) {
            return collie::Status::invalid_argument("version {} error, should be like 1.2.3", str);
        }
        v->set_patch(m);
        return collie::Status::ok_status();
    }

    std::string version_to_string(const sirius::proto::Version &v) {
        return collie::format("{}.{}.{}", v.major(), v.minor(), v.patch());
    }


}  // namespace sirius::cli
