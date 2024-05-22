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

#include <functional>
#include <melon/utility/macros.h>

namespace sirius {

    class ScopeGuard {
    public:
        explicit ScopeGuard(std::function<void()> exit_func) :
                _exit_func(exit_func) {}

        ~ScopeGuard() {
            if (!_is_release) {
                _exit_func();
            }
        }

        void release() {
            _is_release = true;
        }

    private:
        std::function<void()> _exit_func;
        bool _is_release = false;
        DISALLOW_COPY_AND_ASSIGN(ScopeGuard);
    };

#define SCOPEGUARD_LINENAME_CAT(name, line) name##line
#define SCOPEGUARD_LINENAME(name, line) SCOPEGUARD_LINENAME_CAT(name, line)
#define ON_SCOPE_EXIT(callback) ScopeGuard SCOPEGUARD_LINENAME(scope_guard, __LINE__)(callback)
#ifndef SAFE_DELETE
#define SAFE_DELETE(p) { if(p){delete(p);  (p)=nullptr;} }
#endif

}  // namespace sirius
