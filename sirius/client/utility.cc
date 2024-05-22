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

#include <sirius/client/utility.h>
#include <collie/strings/case_conv.h>
#include <collie/strings/str_split.h>
#include <collie/strings/format.h>
#include <turbo/container/flat_hash_set.h>
#include <turbo/strings/numbers.h>

namespace sirius::client {

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

    collie::Status string_to_version(const std::string &str, sirius::proto::Version *v) {
        std::vector<std::string> vs = collie::str_split(str, ".");
        if (vs.size() != 3)
            return collie::Status::invalid_argument("version error, should be like 1.2.3");
        int64_t m;
        if (!turbo::simple_atoi(vs[0], &m)) {
            return collie::Status::invalid_argument("version error, should be like 1.2.3");
        }
        v->set_major(m);
        if (!turbo::simple_atoi(vs[1], &m)) {
            return collie::Status::invalid_argument("version error, should be like 1.2.3");
        }
        v->set_minor(m);
        if (!turbo::simple_atoi(vs[2], &m)) {
            return collie::Status::invalid_argument("version error, should be like 1.2.3");
        }
        v->set_patch(m);
        return collie::Status::ok_status();
    }

    collie::Status string_to_module_version(const std::string &str, collie::ModuleVersion *v) {

        std::vector<std::string> vs = collie::str_split(str, ".");
        if (vs.size() != 3)
            return collie::Status::invalid_argument("version error, should be like 1.2.3");
        int64_t ma;
        if (!turbo::simple_atoi(vs[0], &ma)) {
            return collie::Status::invalid_argument("version error, should be like 1.2.3");
        }
        int64_t mi;
        if (!turbo::simple_atoi(vs[1], &mi)) {
            return collie::Status::invalid_argument("version error, should be like 1.2.3");
        }
        int64_t pa;
        if (!turbo::simple_atoi(vs[2], &pa)) {
            return collie::Status::invalid_argument("version error, should be like 1.2.3");
        }
        *v = collie::ModuleVersion(ma, mi, pa);
        return collie::Status::ok_status();
    }

    std::string version_to_string(const sirius::proto::Version &v) {
        return turbo::format("{}.{}.{}", v.major(), v.minor(), v.patch());
    }

    std::string module_version_to_string(const collie::ModuleVersion &v) {
        return turbo::format("{}.{}.{}", v.major, v.minor, v.patch);
    }

    static turbo::flat_hash_set<char> AllowChar{'a', 'b', 'c', 'd', 'e', 'f', 'g',
                                                'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                                'o', 'p', 'q', 'r', 's', 't',
                                                'u', 'v', 'w', 'x', 'y', 'z',
                                                '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                                                '_',
                                                'A', 'B', 'C', 'D', 'E', 'F', 'G',
                                                'H', 'I', 'J', 'K', 'L', 'M', 'N',
                                                'O', 'P', 'Q', 'R', 'S', 'T',
                                                'U', 'V', 'Q', 'X', 'Y', 'Z',
    };

    collie::Status check_valid_name_type(std::string_view ns) {
        int i = 0;
        for (auto c: ns) {
            if (AllowChar.find(c) == AllowChar.end()) {
                return collie::Status::invalid_argument("the {} char {} of {} is not allow used in name the valid set is[a-z,A-Z,0-9,_]", i, c, ns);
            }
            ++i;
        }
        return collie::Status::ok_status();
    }

}  // namespace sirius::client
