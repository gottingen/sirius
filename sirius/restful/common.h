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
// Created by jeff on 24-6-23.
//

#pragma once

#include <melon/rpc/restful_service.h>

namespace sirius::restful {

    struct NotFoundProcessor : public melon::RestfulProcessor {
        void process(const melon::RestfulRequest *request, melon::RestfulResponse *response) override {
            auto path = request->unresolved_path();
            response->set_status_code(404);
            response->set_header("Content-Type", "text/plain");
            response->set_body("not found\n");
            response->append_body("Request path: ");
            response->set_header("Access-Control-Allow-Origin", "*");
            response->set_header("Access-Control-Allow-Method","*");
            response->set_header("Access-Control-Allow-Headers","*");
            response->set_header("Access-Control-Allow-Credentials","true");
            response->set_header("Access-Control-Expose-Headers","*");
            response->append_body(path);
            response->append_body("\n");
        }
    };

    struct RootProcessor : public melon::RestfulProcessor {
        void process(const melon::RestfulRequest *request, melon::RestfulResponse *response) override {
            auto path = request->unresolved_path();
            response->set_status_code(200);
            response->set_header("Content-Type", "text/plain");
            response->set_header("Access-Control-Allow-Origin", "*");
            response->set_header("Access-Control-Allow-Method","*");
            response->set_header("Access-Control-Allow-Headers","*");
            response->set_header("Access-Control-Allow-Credentials","true");
            response->set_header("Access-Control-Expose-Headers","*");
            response->set_body("I am  root\n");
            response->append_body("\n");
        }
    };
}
