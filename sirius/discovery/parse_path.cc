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

#include <sirius/discovery/parse_path.h>
#include <collie/strings/str_split.h>

namespace sirius::discovery {
    int64_t parse_snapshot_index_from_path(const std::string &snapshot_path, bool use_dirname) {
        mutil::FilePath path(snapshot_path);
        std::string tmp_path;
        if (use_dirname) {
            tmp_path = path.DirName().BaseName().value();
        } else {
            tmp_path = path.BaseName().value();
        }
        std::vector<std::string> split_vec;
        std::vector<std::string> snapshot_index_vec;
        split_vec = collie::str_split(tmp_path, '/', collie::SkipEmpty());
        snapshot_index_vec = collie::str_split(split_vec.back(), '_', collie::SkipEmpty());
        int64_t snapshot_index = 0;
        if (snapshot_index_vec.size() == 2) {
            snapshot_index = atoll(snapshot_index_vec[1].c_str());
        }
        return snapshot_index;
    }

}  // namespace sirius::discovery
