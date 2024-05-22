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

#include <sirius/flags/servlet.h>
#include <gflags/gflags.h>

namespace sirius {

    DEFINE_string(servlet_listen, "0.0.0.0:8898", "router default ip port");
    DEFINE_string(servlet_namespace, "default", "servlet namespace");
    DEFINE_string(servlet_zone, "default", "servlet namespace");
    DEFINE_string(servlet_name, "default", "servlet namespace");
    DEFINE_string(servlet_physical, "default", "servlet namespace");
    DEFINE_string(servlet_resource_tag, "default", "servlet namespace");
    DEFINE_string(servlet_env, "default", "servlet namespace");
}  // namespace sirius
