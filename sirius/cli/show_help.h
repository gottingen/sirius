// Copyright 2023 The Turbo Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#pragma once

#include <collie/utility/status.h>
#include <sirius/proto/discovery.interface.pb.h>
#include <collie/table/table.h>
#include <sirius/cli/proto_help.h>

namespace sirius::cli {
    class ShowHelper {
    public:
        ~ShowHelper();

        static collie::table::Table
        show_response(const std::string_view &server, sirius::proto::ErrCode code, sirius::proto::QueryOpType qt,
                      const std::string &msg);

        static collie::table::Table
        show_response(sirius::proto::ErrCode code, sirius::proto::QueryOpType qt,
                      const std::string &msg);

        static collie::table::Table
        show_response(const std::string_view &server, sirius::proto::ErrCode code, sirius::proto::OpType qt,
                      const std::string &msg);

        static collie::table::Table show_response(sirius::proto::ErrCode code, sirius::proto::OpType qt,
                                          const std::string &msg);

        static collie::table::Table show_response(sirius::proto::ErrCode code, sirius::proto::RaftControlOp qt,
                                          const std::string &msg);

        static collie::table::Table rpc_error_status(const collie::Status &s, sirius::proto::OpType qt);

        static collie::table::Table rpc_error_status(const collie::Status &s, sirius::proto::QueryOpType qt);

        static collie::table::Table rpc_error_status(const collie::Status &s, sirius::proto::RaftControlOp qt);

        static collie::table::Table pre_send_error(const collie::Status &s, const sirius::proto::DiscoveryManagerRequest &req);


        static collie::table::Table pre_send_error(const collie::Status &s, const sirius::proto::DiscoveryQueryRequest &req);

        static collie::table::Table pre_send_error(const collie::Status &s, const sirius::proto::RaftControlRequest &req);

        static std::string json_format(const std::string &json_str);

    private:
        static collie::table::Table
        show_response_impl(const std::string_view &server, sirius::proto::ErrCode code, int qt, const std::string &qts,
                           const std::string &msg);

        static collie::table::Table
        show_response_impl(sirius::proto::ErrCode code, int qt, const std::string &qts, const std::string &msg);

        static collie::table::Table rpc_error_status_impl(const collie::Status &s, int qt, const std::string &qts);

        static std::string get_level_str(int level);
    private:
        using Row_t = collie::table::Table::Row_t;
        collie::table::Table pre_send_result;
        collie::table::Table rpc_result;
        collie::table::Table result_table;

    };

    ///
    /// inlines
    ///
    inline collie::table::Table
    ShowHelper::show_response(const std::string_view &server, sirius::proto::ErrCode code, sirius::proto::QueryOpType qt,
                              const std::string &msg) {
        return show_response_impl(server, code, static_cast<int>(qt), get_op_string(qt), msg);
    }

    inline collie::table::Table
    ShowHelper::show_response(sirius::proto::ErrCode code, sirius::proto::OpType qt,
                              const std::string &msg) {
        return show_response_impl(code, static_cast<int>(qt), get_op_string(qt), msg);
    }

    inline collie::table::Table
    ShowHelper::show_response(sirius::proto::ErrCode code, sirius::proto::RaftControlOp qt,
                              const std::string &msg) {
        return show_response_impl(code, static_cast<int>(qt), get_op_string(qt), msg);
    }

    inline collie::table::Table
    ShowHelper::show_response(sirius::proto::ErrCode code, sirius::proto::QueryOpType qt,
                              const std::string &msg) {
        return show_response_impl(code, static_cast<int>(qt), get_op_string(qt), msg);
    }

    inline collie::table::Table
    ShowHelper::show_response(const std::string_view &server, sirius::proto::ErrCode code, sirius::proto::OpType qt,
                              const std::string &msg) {
        return show_response_impl(server, code, static_cast<int>(qt), get_op_string(qt), msg);
    }

    inline collie::table::Table ShowHelper::rpc_error_status(const collie::Status &s, sirius::proto::OpType qt) {
        return rpc_error_status_impl(s, static_cast<int>(qt), get_op_string(qt));
    }

    inline collie::table::Table ShowHelper::rpc_error_status(const collie::Status &s, sirius::proto::QueryOpType qt) {
        return rpc_error_status_impl(s, static_cast<int>(qt), get_op_string(qt));
    }

    inline collie::table::Table ShowHelper::rpc_error_status(const collie::Status &s, sirius::proto::RaftControlOp qt) {
        return rpc_error_status_impl(s, static_cast<int>(qt), get_op_string(qt));
    }

    struct ScopeShower {
        ~ScopeShower();

        ScopeShower();

        explicit ScopeShower(const std::string &operation);

        void add_table(collie::table::Table &&table);

        void add_table(const std::string &stage, collie::table::Table &&table, bool ok = true);

        void add_table(const std::string &stage, const std::string &msg, bool ok);

        void prepare(const collie::Status &status);

        std::vector<collie::table::Table> tables;
        collie::table::Table result_table;
    };
}  // namespace sirius::cli

#define PREPARE_ERROR_RETURN(show, rs, request) \
    do {                                            \
        if(!rs.ok()) {                        \
            show.add_table("prepare", std::move(ShowHelper::pre_send_error(rs, request))); \
            return;                                            \
        }                                               \
    }while(0)

#define PREPARE_ERROR_RETURN_OR_OK(show, rs, request) \
    do {                                            \
        if(!rs.ok()) {                        \
            show.add_table("prepare", std::move(ShowHelper::pre_send_error(rs, request)), false); \
            return;                                            \
        } else {                                               \
            show.add_table("prepare", "ok", true);                   \
        }                                                      \
    }while(0)

#define RPC_ERROR_RETURN_OR_OK(show, rs, request) \
    do {                                            \
        if(!rs.ok()) {                               \
            show.add_table("rpc", std::move(ShowHelper::rpc_error_status(rs, request.op_type())), false);\
            return;                                    \
        } else {                                        \
            show.add_table("rpc","ok", true);                    \
        }                                             \
    }while(0)
