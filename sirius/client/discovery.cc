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

#include <sirius/client/discovery.h>
#include <sirius/client/utility.h>
#include <alkaid/files/sequential_read_file.h>
#include <alkaid/files/sequential_write_file.h>
#include <melon/json2pb/json_to_pb.h>
#include <melon/json2pb/pb_to_json.h>
#include <sirius/client/config_info_builder.h>
#include <sirius/client/loader.h>
#include <sirius/client/dumper.h>

namespace sirius::client {

    collie::Status DiscoveryClient::init(BaseMessageSender *sender) {
        _sender = sender;
        return collie::Status::ok_status();
    }



    collie::Status DiscoveryClient::check_config(const std::string &json_content) {
        sirius::proto::ConfigInfo config_pb;
        std::string errmsg;
        if (!json2pb::JsonToProtoMessage(json_content, &config_pb, &errmsg)) {
            return collie::Status::invalid_argument(errmsg);
        }
        return collie::Status::ok_status();
    }

    collie::Status DiscoveryClient::check_config_file(const std::string &config_path) {
        alkaid::SequentialReadFile file;
        auto rs = file.open(config_path);
        if (!rs.ok()) {
            return rs;
        }
        std::string config_data;
        auto rr = file.read(&config_data);
        if (!rr.ok()) {
            return rr.status();
        }
        return check_config(config_data);
    }

    collie::Status DiscoveryClient::dump_config_file(const std::string &config_path, const sirius::proto::ConfigInfo &config) {
        alkaid::SequentialWriteFile file;
        auto rs = file.open(config_path, alkaid::kDefaultTruncateWriteOption);
        if (!rs.ok()) {
            return rs;
        }
        std::string json;
        std::string err;
        if (!json2pb::ProtoMessageToJson(config, &json, &err)) {
            return collie::Status::invalid_argument(err);
        }
        rs = file.write(json);
        if (!rs.ok()) {
            return rs;
        }
        file.close();
        return collie::Status::ok_status();
    }

    collie::Status
    DiscoveryClient::create_config(const std::string &config_name, const std::string &content,
                              const std::string &version, const std::string &config_type, int *retry_times) {
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;

        request.set_op_type(sirius::proto::OP_CREATE_CONFIG);
        auto rc = request.mutable_config_info();
        ConfigInfoBuilder builder(rc);
        auto rs = builder.build_from_content(config_name, content, version, config_type);
        if(!rs.ok()) {
            return rs;
        }
        rs = discovery_manager(request, response, retry_times);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != sirius::proto::SUCCESS) {
            return collie::Status::unavailable(response.errmsg());
        }
        return collie::Status::ok_status();
    }

    collie::Status
    DiscoveryClient::create_config(const sirius::proto::ConfigInfo &request,
                              int *retry_times) {
        sirius::proto::DiscoveryManagerRequest meta_request;
        sirius::proto::DiscoveryManagerResponse response;
        meta_request.set_op_type(sirius::proto::OP_CREATE_CONFIG);
        *meta_request.mutable_config_info() = request;
        auto rs = discovery_manager(meta_request, response, retry_times);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != sirius::proto::SUCCESS) {
            return collie::Status::unavailable(response.errmsg());
        }
        return collie::Status::ok_status();
    }

    collie::Status DiscoveryClient::create_config_by_file(const std::string &config_name,
                                                    const std::string &path,
                                                    const std::string &config_type, const std::string &version,
                                                    int *retry_times) {
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        sirius::client::ConfigInfoBuilder builder(request.mutable_config_info());
        auto rs = builder.build_from_file(config_name, path, version, config_type);
        if(!rs.ok()) {
            return rs;
        }
        rs =  discovery_manager(request, response, retry_times);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != sirius::proto::SUCCESS) {
            return collie::Status::unavailable(response.errmsg());
        }
        return collie::Status::ok_status();
    }

    collie::Status DiscoveryClient::create_config_by_json(const std::string &json_path, int *retry_times) {
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        sirius::client::ConfigInfoBuilder builder(request.mutable_config_info());
        auto rs = builder.build_from_json_file(json_path);
        if(!rs.ok()) {
            return rs;
        }
        rs =  discovery_manager(request, response, retry_times);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != sirius::proto::SUCCESS) {
            return collie::Status::unavailable(response.errmsg());
        }
        return collie::Status::ok_status();
    }

    collie::Status DiscoveryClient::list_config(std::vector<std::string> &configs, int *retry_time) {
        sirius::proto::DiscoveryQueryRequest request;
        sirius::proto::DiscoveryQueryResponse response;
        request.set_op_type(sirius::proto::QUERY_LIST_CONFIG);
        auto rs = discovery_query(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != sirius::proto::SUCCESS) {
            return collie::Status::unavailable(response.errmsg());
        }
        auto res_configs = response.config_infos();
        for (auto config: res_configs) {
            configs.push_back(config.name());
        }
        return collie::Status::ok_status();
    }

    collie::Status DiscoveryClient::list_config_version(const std::string &config_name, std::vector<std::string> &versions,
                                                  int *retry_time) {
        sirius::proto::DiscoveryQueryRequest request;
        sirius::proto::DiscoveryQueryResponse response;
        request.set_op_type(sirius::proto::QUERY_LIST_CONFIG_VERSION);
        request.set_config_name(config_name);
        auto rs = discovery_query(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != sirius::proto::SUCCESS) {
            return collie::Status::unavailable(response.errmsg());
        }
        auto res_configs = response.config_infos();
        for (auto &config: res_configs) {
            versions.push_back(version_to_string(config.version()));
        }
        return collie::Status::ok_status();
    }

    collie::Status
    DiscoveryClient::list_config_version(const std::string &config_name, std::vector<collie::ModuleVersion> &versions,
                                    int *retry_time) {
        sirius::proto::DiscoveryQueryRequest request;
        sirius::proto::DiscoveryQueryResponse response;
        request.set_op_type(sirius::proto::QUERY_LIST_CONFIG_VERSION);
        request.set_config_name(config_name);
        auto rs = discovery_query(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != sirius::proto::SUCCESS) {
            return collie::Status::unavailable(response.errmsg());
        }
        auto res_configs = response.config_infos();
        for (auto config: res_configs) {
            versions.emplace_back(config.version().major(),
                                  config.version().minor(), config.version().patch());
        }
        return collie::Status::ok_status();
    }

    collie::Status
    DiscoveryClient::get_config(const std::string &config_name, const std::string &version, sirius::proto::ConfigInfo &config,
                           int *retry_time) {
        sirius::proto::DiscoveryQueryRequest request;
        sirius::proto::DiscoveryQueryResponse response;
        request.set_op_type(sirius::proto::QUERY_GET_CONFIG);
        request.set_config_name(config_name);
        auto rs = string_to_version(version, request.mutable_config_version());
        if (!rs.ok()) {
            return rs;
        }
        rs = discovery_query(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != sirius::proto::SUCCESS) {
            return collie::Status::unavailable(response.errmsg());
        }
        if (response.config_infos_size() != 1) {
            return collie::Status::invalid_argument("bad proto for config list size not 1");
        }

        config = response.config_infos(0);
        return collie::Status::ok_status();
    }

    collie::Status
    DiscoveryClient::get_config(const std::string &config_name, const std::string &version, std::string &config,
                           int *retry_time, std::string *type, uint32_t *time) {
        sirius::proto::ConfigInfo config_pb;
        auto rs = get_config(config_name, version, config_pb, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        config = config_pb.content();
        if (type) {
            *type = config_type_to_string(config_pb.type());
        }
        if (time) {
            *time = config_pb.time();
        }
        return collie::Status::ok_status();
    }

    collie::Status DiscoveryClient::save_config(const std::string &config_name, const std::string &version, std::string &path,
                                          int *retry_time) {
        std::string content;
        auto rs = get_config(config_name, version, content, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        alkaid::SequentialWriteFile file;
        rs = file.open(path, alkaid::kDefaultTruncateWriteOption);
        if (!rs.ok()) {
            return rs;
        }
        rs = file.write(content);
        if (!rs.ok()) {
            return rs;
        }
        file.close();
        return collie::Status::ok_status();
    }

    collie::Status
    DiscoveryClient::save_config(const std::string &config_name, const std::string &version, int *retry_time) {
        std::string content;
        std::string type;
        auto rs = get_config(config_name, version, content, retry_time, &type);
        if (!rs.ok()) {
            return rs;
        }
        alkaid::SequentialWriteFile file;
        auto path = collie::format("{}.{}", config_name, type);
        rs = file.open(path, alkaid::kDefaultTruncateWriteOption);
        if (!rs.ok()) {
            return rs;
        }
        rs = file.write(content);
        if (!rs.ok()) {
            return rs;
        }
        file.close();
        return collie::Status::ok_status();
    }

    collie::Status
    DiscoveryClient::get_config_latest(const std::string &config_name, sirius::proto::ConfigInfo &config,
                                  int *retry_time) {
        sirius::proto::DiscoveryQueryRequest request;
        sirius::proto::DiscoveryQueryResponse response;
        request.set_op_type(sirius::proto::QUERY_GET_CONFIG);
        request.set_config_name(config_name);
        auto rs = discovery_query(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != sirius::proto::SUCCESS) {
            return collie::Status::unavailable(response.errmsg());
        }
        if (response.config_infos_size() != 1) {
            return collie::Status::invalid_argument("bad proto for config list size not 1");
        }

        config = response.config_infos(0);
        return collie::Status::ok_status();
    }

    collie::Status
    DiscoveryClient::get_config_latest(const std::string &config_name, std::string &config, std::string &version,
                                  int *retry_time) {
        sirius::proto::ConfigInfo config_pb;
        auto rs = get_config_latest(config_name, config_pb, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        config = config_pb.content();
        version = version_to_string(config_pb.version());
        return collie::Status::ok_status();
    }

    collie::Status
    DiscoveryClient::get_config_latest(const std::string &config_name, std::string &config, std::string &version,
                                  std::string &type,
                                  int *retry_time) {
        sirius::proto::ConfigInfo config_pb;
        auto rs = get_config_latest(config_name, config_pb, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        config = config_pb.content();
        version = version_to_string(config_pb.version());
        type = config_type_to_string(config_pb.type());
        return collie::Status::ok_status();
    }

    collie::Status
    DiscoveryClient::get_config_latest(const std::string &config_name, std::string &config, collie::ModuleVersion &version,
                                  int *retry_time) {
        sirius::proto::ConfigInfo config_pb;
        auto rs = get_config_latest(config_name, config_pb, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        config = config_pb.content();
        version = collie::ModuleVersion(config_pb.version().major(), config_pb.version().minor(),
                                       config_pb.version().patch());
        return collie::Status::ok_status();
    }

    collie::Status
    DiscoveryClient::get_config_latest(const std::string &config_name, std::string &config, collie::ModuleVersion &version,
                                  std::string &type,
                                  int *retry_time) {
        sirius::proto::ConfigInfo config_pb;
        auto rs = get_config_latest(config_name, config_pb, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        config = config_pb.content();
        version = collie::ModuleVersion(config_pb.version().major(), config_pb.version().minor(),
                                       config_pb.version().patch());
        type = config_type_to_string(config_pb.type());
        return collie::Status::ok_status();
    }

    collie::Status
    DiscoveryClient::get_config_latest(const std::string &config_name, std::string &config, int *retry_time) {
        sirius::proto::ConfigInfo config_pb;
        auto rs = get_config_latest(config_name, config_pb, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        config = config_pb.content();
        return collie::Status::ok_status();
    }

    collie::Status
    DiscoveryClient::remove_config(const std::string &config_name, const std::string &version, int *retry_time) {
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;

        request.set_op_type(sirius::proto::OP_REMOVE_CONFIG);
        auto rc = request.mutable_config_info();
        rc->set_name(config_name);
        auto rs = string_to_version(version, rc->mutable_version());
        if (!rs.ok()) {
            return rs;
        }
        rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != sirius::proto::SUCCESS) {
            return collie::Status::unavailable(response.errmsg());
        }
        return collie::Status::ok_status();
    }

    collie::Status
    DiscoveryClient::remove_config(const std::string &config_name, const collie::ModuleVersion &version, int *retry_time) {
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;

        request.set_op_type(sirius::proto::OP_REMOVE_CONFIG);
        auto rc = request.mutable_config_info();
        rc->set_name(config_name);
        rc->mutable_version()->set_major(version.major);
        rc->mutable_version()->set_minor(version.minor);
        rc->mutable_version()->set_minor(version.patch);
        auto rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != sirius::proto::SUCCESS) {
            return collie::Status::unavailable(response.errmsg());
        }
        return collie::Status::ok_status();
    }

    collie::Status
    DiscoveryClient::remove_config_all_version(const std::string &config_name, int *retry_time) {
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;

        request.set_op_type(sirius::proto::OP_REMOVE_CONFIG);
        auto rc = request.mutable_config_info();
        rc->set_name(config_name);
        auto rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != sirius::proto::SUCCESS) {
            return collie::Status::unavailable(response.errmsg());
        }
        return collie::Status::ok_status();
    }

    collie::Status DiscoveryClient::create_app(sirius::proto::AppInfo &info, int *retry_time) {
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        request.set_op_type(sirius::proto::OP_CREATE_NAMESPACE);

        sirius::proto::AppInfo *ns_req = request.mutable_app_info();
        *ns_req = info;
        auto rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return collie::Status::ok_status();
    }

    collie::Status DiscoveryClient::create_app(const std::string &ns, int64_t quota, int *retry_time) {
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        request.set_op_type(sirius::proto::OP_CREATE_NAMESPACE);

        sirius::proto::AppInfo *ns_req = request.mutable_app_info();
        auto rs = check_valid_name_type(ns);
        if (!rs.ok()) {
            return rs;
        }
        ns_req->set_app_name(ns);
        if (quota != 0) {
            ns_req->set_quota(quota);
        }
        rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return collie::Status::ok_status();
    }

    collie::Status DiscoveryClient::create_namespace_by_json(const std::string &json_str, int *retry_time) {
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        request.set_op_type(sirius::proto::OP_CREATE_NAMESPACE);
        auto rs = Loader::load_proto(json_str, *request.mutable_app_info());
        if (!rs.ok()) {
            return rs;
        }
        rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return collie::Status::ok_status();
    }

    collie::Status DiscoveryClient::create_namespace_by_file(const std::string &path, int *retry_time) {
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        request.set_op_type(sirius::proto::OP_CREATE_NAMESPACE);
        auto rs = Loader::load_proto_from_file(path, *request.mutable_app_info());
        if (!rs.ok()) {
            return rs;
        }
        rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return collie::Status::ok_status();
    }

    collie::Status DiscoveryClient::remove_namespace(const std::string &ns, int *retry_time) {
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        request.set_op_type(sirius::proto::OP_DROP_NAMESPACE);

        sirius::proto::AppInfo *ns_req = request.mutable_app_info();
        auto rs = check_valid_name_type(ns);
        if (!rs.ok()) {
            return rs;
        }
        ns_req->set_app_name(ns);
        rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return collie::Status::ok_status();
    }

    collie::Status DiscoveryClient::modify_app(sirius::proto::AppInfo &ns_info, int *retry_time) {
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        request.set_op_type(sirius::proto::OP_MODIFY_NAMESPACE);
        *request.mutable_app_info() = ns_info;
        auto rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return collie::Status::ok_status();
    }

    collie::Status DiscoveryClient::modify_namespace_by_json(const std::string &json_str, int *retry_time) {
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        request.set_op_type(sirius::proto::OP_MODIFY_NAMESPACE);
        auto rs = Loader::load_proto(json_str, *request.mutable_app_info());
        if (!rs.ok()) {
            return rs;
        }
        rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return collie::Status::ok_status();
    }

    collie::Status DiscoveryClient::modify_namespace_by_file(const std::string &path, int *retry_time) {
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        request.set_op_type(sirius::proto::OP_MODIFY_NAMESPACE);
        auto rs = Loader::load_proto_from_file(path, *request.mutable_app_info());
        if (!rs.ok()) {
            return rs;
        }
        rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return collie::Status::ok_status();
    }

    collie::Status DiscoveryClient::list_namespace(std::vector<std::string> &ns_list, int *retry_time) {
        std::vector<sirius::proto::AppInfo> ns_proto_list;
        auto rs = list_namespace(ns_proto_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &ns: ns_proto_list) {
            ns_list.push_back(ns.app_name());
        }
        return collie::Status::ok_status();
    }

    collie::Status DiscoveryClient::list_namespace(std::vector<sirius::proto::AppInfo> &ns_list, int *retry_time) {
        sirius::proto::DiscoveryQueryRequest request;
        sirius::proto::DiscoveryQueryResponse response;
        request.set_op_type(sirius::proto::QUERY_APP);
        auto rs = discovery_query(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != sirius::proto::SUCCESS) {
            return collie::Status::unavailable(response.errmsg());
        }
        for (auto &ns: response.app_infos()) {
            ns_list.push_back(ns);
        }
        return collie::Status::ok_status();
    }

    collie::Status DiscoveryClient::list_namespace_to_json(std::vector<std::string> &ns_list, int *retry_time) {
        std::vector<sirius::proto::AppInfo> ns_proto_list;
        auto rs = list_namespace(ns_proto_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &ns: ns_proto_list) {
            std::string json_content;
            auto r = Dumper::dump_proto(ns, json_content);
            if (!r.ok()) {
                return r;
            }
            ns_list.push_back(json_content);
        }
        return collie::Status::ok_status();
    }

    collie::Status DiscoveryClient::list_namespace_to_file(const std::string &save_path, int *retry_time) {
        std::vector<std::string> json_list;
        auto rs = list_namespace_to_json(json_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }

        alkaid::SequentialWriteFile file;
        rs = file.open(save_path, alkaid::kDefaultTruncateWriteOption);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &ns: json_list) {
            rs = file.write(ns);
            if (!rs.ok()) {
                return rs;
            }
        }
        file.close();
        return collie::Status::ok_status();
    }

    collie::Status
    DiscoveryClient::get_namespace(const std::string &ns_name, sirius::proto::AppInfo &ns_pb, int *retry_time) {
        sirius::proto::DiscoveryQueryRequest request;
        sirius::proto::DiscoveryQueryResponse response;
        request.set_op_type(sirius::proto::QUERY_APP);
        if (ns_name.empty()) {
            return collie::Status::invalid_argument("app name empty");
        }
        request.set_app_name(ns_name);
        auto rs = discovery_query(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != sirius::proto::SUCCESS) {
            return collie::Status::unavailable(response.errmsg());
        }
        if (response.app_infos_size() != 1) {
            return collie::Status::unavailable("bad proto format for app info size {}", response.app_infos_size());
        }
        ns_pb = response.app_infos(0);
        return collie::Status::ok_status();
    }

    collie::Status DiscoveryClient::get_namespace_json(const std::string &ns_name, std::string &json_str, int *retry_time) {
        sirius::proto::AppInfo ns_pb;
        auto rs = get_namespace(ns_name, ns_pb, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return Dumper::dump_proto(ns_pb, json_str);
    }

    collie::Status
    DiscoveryClient::save_namespace_json(const std::string &ns_name, const std::string &json_path, int *retry_time) {
        sirius::proto::AppInfo ns_pb;
        auto rs = get_namespace(ns_name, ns_pb, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return Dumper::dump_proto_to_file(json_path, ns_pb);
    }


    collie::Status DiscoveryClient::create_zone(sirius::proto::ZoneInfo &zone_info, int *retry_time) {
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        request.set_op_type(sirius::proto::OP_CREATE_ZONE);

        auto *zone_req = request.mutable_zone_info();
        *zone_req = zone_info;
        auto rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return collie::Status::ok_status();
    }

    collie::Status
    DiscoveryClient::create_zone(const std::string &ns, const std::string &zone, int64_t quota, int *retry_time) {
        sirius::proto::ZoneInfo zone_pb;
        zone_pb.set_app_name(ns);
        zone_pb.set_zone(zone);
        if (quota != 0) {
            zone_pb.set_quota(quota);
        }
        return create_zone(zone_pb, retry_time);
    }

    collie::Status DiscoveryClient::create_zone_by_json(const std::string &json_str, int *retry_time) {
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        request.set_op_type(sirius::proto::OP_CREATE_ZONE);
        auto rs = Loader::load_proto(json_str, *request.mutable_zone_info());
        if (!rs.ok()) {
            return rs;
        }
        rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return collie::Status::ok_status();
    }

    collie::Status DiscoveryClient::create_zone_by_file(const std::string &path, int *retry_time) {
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        request.set_op_type(sirius::proto::OP_CREATE_ZONE);
        auto rs = Loader::load_proto_from_file(path, *request.mutable_zone_info());
        if (!rs.ok()) {
            return rs;
        }
        rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return collie::Status::ok_status();
    }

    collie::Status DiscoveryClient::remove_zone(const std::string &ns, const std::string &zone, int *retry_time) {
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        request.set_op_type(sirius::proto::OP_DROP_ZONE);

        auto *zone_req = request.mutable_zone_info();
        zone_req->set_app_name(ns);
        zone_req->set_zone(zone);
        auto rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return collie::Status::ok_status();
    }

    collie::Status DiscoveryClient::modify_zone(sirius::proto::ZoneInfo &zone_info, int *retry_time) {
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        request.set_op_type(sirius::proto::OP_MODIFY_ZONE);

        auto *zone_req = request.mutable_zone_info();
        *zone_req = zone_info;
        auto rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return collie::Status::ok_status();
    }

    collie::Status DiscoveryClient::modify_zone_by_json(const std::string &json_str, int *retry_time) {
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        request.set_op_type(sirius::proto::OP_MODIFY_ZONE);
        auto rs = Loader::load_proto(json_str, *request.mutable_zone_info());
        if (!rs.ok()) {
            return rs;
        }
        rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return collie::Status::ok_status();
    }

    collie::Status DiscoveryClient::modify_zone_by_file(const std::string &path, int *retry_time) {
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        request.set_op_type(sirius::proto::OP_MODIFY_ZONE);
        auto rs = Loader::load_proto_from_file(path, *request.mutable_zone_info());
        if (!rs.ok()) {
            return rs;
        }
        rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return collie::Status::ok_status();
    }

    collie::Status DiscoveryClient::list_zone(std::vector<sirius::proto::ZoneInfo> &zone_list, int *retry_time) {
        sirius::proto::DiscoveryQueryRequest request;
        sirius::proto::DiscoveryQueryResponse response;
        request.set_op_type(sirius::proto::QUERY_ZONE);
        auto rs = discovery_query(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != sirius::proto::SUCCESS) {
            return collie::Status::unavailable(response.errmsg());
        }
        for (auto &zone: response.zone_infos()) {
            zone_list.push_back(zone);
        }
        return collie::Status::ok_status();
    }

    collie::Status
    DiscoveryClient::list_zone(const std::string &ns, std::vector<sirius::proto::ZoneInfo> &zone_list, int *retry_time) {
        std::vector<sirius::proto::ZoneInfo> all_zone_list;
        auto rs = list_zone(zone_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &zone: all_zone_list) {
            if (zone.app_name() == ns) {
                zone_list.push_back(zone);
            }
        }
        return collie::Status::ok_status();
    }

    collie::Status DiscoveryClient::list_zone(std::vector<std::string> &zone_list, int *retry_time) {
        std::vector<sirius::proto::ZoneInfo> zone_proto_list;
        auto rs = list_zone(zone_proto_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &zone: zone_proto_list) {
            zone_list.push_back(collie::format("{},{}", zone.app_name(), zone.zone()));
        }
        return collie::Status::ok_status();
    }

    collie::Status DiscoveryClient::list_zone(std::string &ns, std::vector<std::string> &zone_list, int *retry_time) {
        std::vector<sirius::proto::ZoneInfo> zone_proto_list;
        auto rs = list_zone(ns, zone_proto_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &zone: zone_proto_list) {
            zone_list.push_back(collie::format("{},{}", zone.app_name(), zone.zone()));
        }
        return collie::Status::ok_status();
    }


    collie::Status DiscoveryClient::list_zone_to_json(std::vector<std::string> &zone_list, int *retry_time) {
        std::vector<sirius::proto::ZoneInfo> zone_proto_list;
        auto rs = list_zone(zone_proto_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &zone: zone_proto_list) {
            std::string json_content;
            auto r = Dumper::dump_proto(zone, json_content);
            if (!r.ok()) {
                return r;
            }
            zone_list.push_back(json_content);
        }
        return collie::Status::ok_status();
    }

    collie::Status
    DiscoveryClient::list_zone_to_json(const std::string &ns, std::vector<std::string> &zone_list, int *retry_time) {
        std::vector<sirius::proto::ZoneInfo> zone_proto_list;
        auto rs = list_zone(ns, zone_proto_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &zone: zone_proto_list) {
            std::string json_content;
            auto r = Dumper::dump_proto(zone, json_content);
            if (!r.ok()) {
                return r;
            }
            zone_list.push_back(json_content);
        }
        return collie::Status::ok_status();
    }

    collie::Status DiscoveryClient::list_zone_to_file(const std::string &save_path, int *retry_time) {
        std::vector<std::string> json_list;
        auto rs = list_zone_to_json(json_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }

        alkaid::SequentialWriteFile file;
        rs = file.open(save_path, alkaid::kDefaultTruncateWriteOption);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &zone: json_list) {
            rs = file.write(zone);
            if (!rs.ok()) {
                return rs;
            }
        }
        file.close();
        return collie::Status::ok_status();
    }

    collie::Status DiscoveryClient::list_zone_to_file(const std::string &ns, const std::string &save_path, int *retry_time) {
        std::vector<std::string> json_list;
        auto rs = list_zone_to_json(ns, json_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }

        alkaid::SequentialWriteFile file;
        rs = file.open(save_path, alkaid::kDefaultTruncateWriteOption);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &zone: json_list) {
            rs = file.write(zone);
            if (!rs.ok()) {
                return rs;
            }
        }
        file.close();
        return collie::Status::ok_status();
    }

    collie::Status
    DiscoveryClient::get_zone(const std::string &ns_name, const std::string &zone_name, sirius::proto::ZoneInfo &zone_pb,
                         int *retry_time) {
        sirius::proto::DiscoveryQueryRequest request;
        sirius::proto::DiscoveryQueryResponse response;
        request.set_op_type(sirius::proto::QUERY_ZONE);
        if (ns_name.empty()) {
            return collie::Status::invalid_argument("app name empty");
        }
        request.set_app_name(ns_name);
        request.set_zone(zone_name);
        auto rs = discovery_query(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != sirius::proto::SUCCESS) {
            return collie::Status::unavailable(response.errmsg());
        }
        if (response.zone_infos_size() != 1) {
            return collie::Status::unavailable("bad proto format for zone info size {}", response.zone_infos_size());
        }
        zone_pb = response.zone_infos(0);
        return collie::Status::ok_status();
    }

    collie::Status
    DiscoveryClient::get_zone_json(const std::string &ns_name, const std::string &zone_name, std::string &json_str,
                              int *retry_time) {
        sirius::proto::ZoneInfo zone_pb;
        auto rs = get_zone(ns_name, zone_name, zone_pb, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return Dumper::dump_proto(zone_pb, json_str);
    }

    collie::Status
    DiscoveryClient::save_zone_json(const std::string &ns_name, const std::string &zone_name, const std::string &json_path,
                               int *retry_time) {
        sirius::proto::ZoneInfo zone_pb;
        auto rs = get_zone(ns_name, zone_name, zone_pb, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return Dumper::dump_proto_to_file(json_path, zone_pb);
    }

    collie::Status DiscoveryClient::create_servlet(sirius::proto::ServletInfo &servlet_info, int *retry_time) {
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        request.set_op_type(sirius::proto::OP_CREATE_SERVLET);

        auto *servlet_req = request.mutable_servlet_info();
        *servlet_req = servlet_info;
        auto rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return collie::Status::ok_status();
    }

    collie::Status
    DiscoveryClient::create_servlet(const std::string &ns, const std::string &zone, const std::string &servlet,
                               int *retry_time) {
        sirius::proto::ServletInfo servlet_pb;
        servlet_pb.set_app_name(ns);
        servlet_pb.set_zone(zone);
        servlet_pb.set_servlet_name(servlet);
        return create_servlet(servlet_pb, retry_time);
    }

    collie::Status DiscoveryClient::create_servlet_by_json(const std::string &json_str, int *retry_time) {
        sirius::proto::ServletInfo servlet_pb;
        auto rs = Loader::load_proto(json_str, servlet_pb);
        if (!rs.ok()) {
            return rs;
        }
        return create_servlet(servlet_pb, retry_time);
    }

    collie::Status DiscoveryClient::create_servlet_by_file(const std::string &path, int *retry_time) {
        sirius::proto::ServletInfo servlet_pb;
        auto rs = Loader::load_proto_from_file(path, servlet_pb);
        if (!rs.ok()) {
            return rs;
        }
        return create_servlet(servlet_pb, retry_time);
    }

    collie::Status DiscoveryClient::remove_servlet(const std::string &ns, const std::string &zone, const std::string &servlet,
                                             int *retry_time) {
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        request.set_op_type(sirius::proto::OP_DROP_SERVLET);

        auto *servlet_req = request.mutable_servlet_info();
        servlet_req->set_app_name(ns);
        servlet_req->set_zone(zone);
        servlet_req->set_servlet_name(servlet);
        auto rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return collie::Status::ok_status();
    }

    collie::Status DiscoveryClient::modify_servlet(const sirius::proto::ServletInfo &servlet_info, int *retry_time) {
        sirius::proto::DiscoveryManagerRequest request;
        sirius::proto::DiscoveryManagerResponse response;
        request.set_op_type(sirius::proto::OP_CREATE_SERVLET);

        auto *servlet_req = request.mutable_servlet_info();
        *servlet_req = servlet_info;
        auto rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return collie::Status::ok_status();
    }

    collie::Status DiscoveryClient::modify_servlet_by_json(const std::string &json_str, int *retry_time) {
        sirius::proto::ServletInfo servlet_pb;
        auto rs = Loader::load_proto(json_str, servlet_pb);
        if (!rs.ok()) {
            return rs;
        }
        return modify_servlet(servlet_pb, retry_time);
    }

    collie::Status DiscoveryClient::modify_servlet_by_file(const std::string &path, int *retry_time) {
        sirius::proto::ServletInfo servlet_pb;
        auto rs = Loader::load_proto_from_file(path, servlet_pb);
        if (!rs.ok()) {
            return rs;
        }
        return modify_servlet(servlet_pb, retry_time);
    }

    collie::Status DiscoveryClient::list_servlet(std::vector<sirius::proto::ServletInfo> &servlet_list, int *retry_time) {
        sirius::proto::DiscoveryQueryRequest request;
        sirius::proto::DiscoveryQueryResponse response;
        request.set_op_type(sirius::proto::QUERY_ZONE);
        auto rs = discovery_query(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != sirius::proto::SUCCESS) {
            return collie::Status::unavailable(response.errmsg());
        }
        for (auto &servlet: response.servlet_infos()) {
            servlet_list.push_back(servlet);
        }
        return collie::Status::ok_status();
    }

    collie::Status
    DiscoveryClient::list_servlet(const std::string &ns, std::vector<sirius::proto::ServletInfo> &servlet_list,
                             int *retry_time) {
        std::vector<sirius::proto::ServletInfo> all_servlet_list;
        auto rs = list_servlet(all_servlet_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &servlet: all_servlet_list) {
            if (servlet.app_name() == ns) {
                servlet_list.push_back(servlet);
            }
        }
        return collie::Status::ok_status();
    }

    collie::Status
    DiscoveryClient::list_servlet(const std::string &ns, const std::string &zone,
                             std::vector<sirius::proto::ServletInfo> &servlet_list, int *retry_time) {
        std::vector<sirius::proto::ServletInfo> all_servlet_list;
        auto rs = list_servlet(all_servlet_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &servlet: all_servlet_list) {
            if (servlet.app_name() == ns && servlet.zone() == zone) {
                servlet_list.push_back(servlet);
            }
        }
        return collie::Status::ok_status();
    }

    collie::Status DiscoveryClient::list_servlet(std::vector<std::string> &servlet_list, int *retry_time) {
        std::vector<sirius::proto::ServletInfo> all_servlet_list;
        auto rs = list_servlet(all_servlet_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &servlet: all_servlet_list) {
            servlet_list.push_back(servlet.servlet_name());
        }
        return collie::Status::ok_status();
    }

    collie::Status
    DiscoveryClient::list_servlet(const std::string &ns, std::vector<std::string> &servlet_list, int *retry_time) {
        std::vector<sirius::proto::ServletInfo> all_servlet_list;
        auto rs = list_servlet(all_servlet_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &servlet: all_servlet_list) {
            if (servlet.app_name() == ns) {
                servlet_list.push_back(servlet.servlet_name());
            }
        }
        return collie::Status::ok_status();
    }

    collie::Status
    DiscoveryClient::list_servlet(const std::string &ns, const std::string &zone, std::vector<std::string> &servlet_list,
                             int *retry_time) {
        std::vector<sirius::proto::ServletInfo> all_servlet_list;
        auto rs = list_servlet(all_servlet_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &servlet: all_servlet_list) {
            if (servlet.app_name() == ns && servlet.zone() == zone) {
                servlet_list.push_back(servlet.servlet_name());
            }
        }
        return collie::Status::ok_status();
    }

    collie::Status DiscoveryClient::list_servlet_to_json(std::vector<std::string> &servlet_list, int *retry_time) {
        std::vector<sirius::proto::ServletInfo> servlet_proto_list;
        auto rs = list_servlet(servlet_proto_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &servlet: servlet_proto_list) {
            std::string json_content;
            auto r = Dumper::dump_proto(servlet, json_content);
            if (!r.ok()) {
                return r;
            }
            servlet_list.push_back(json_content);
        }
        return collie::Status::ok_status();
    }

    collie::Status
    DiscoveryClient::list_servlet_to_json(const std::string &ns, std::vector<std::string> &servlet_list, int *retry_time) {
        std::vector<sirius::proto::ServletInfo> servlet_proto_list;
        auto rs = list_servlet(servlet_proto_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &servlet: servlet_proto_list) {
            if (servlet.app_name() == ns) {
                std::string json_content;
                auto r = Dumper::dump_proto(servlet, json_content);
                if (!r.ok()) {
                    return r;
                }
                servlet_list.push_back(json_content);
            }
        }
        return collie::Status::ok_status();
    }

    collie::Status DiscoveryClient::list_servlet_to_json(const std::string &ns, const std::string &zone,
                                                   std::vector<std::string> &servlet_list, int *retry_time) {
        std::vector<sirius::proto::ServletInfo> servlet_proto_list;
        auto rs = list_servlet(servlet_proto_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &servlet: servlet_proto_list) {
            if (servlet.app_name() == ns && servlet.zone() == zone) {
                std::string json_content;
                auto r = Dumper::dump_proto(servlet, json_content);
                if (!r.ok()) {
                    return r;
                }
                servlet_list.push_back(json_content);
            }
        }
        return collie::Status::ok_status();
    }

    collie::Status DiscoveryClient::list_servlet_to_file(const std::string &save_path, int *retry_time) {
        std::vector<std::string> json_list;
        auto rs = list_servlet_to_json(json_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }

        alkaid::SequentialWriteFile file;
        rs = file.open(save_path, alkaid::kDefaultTruncateWriteOption);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &zone: json_list) {
            rs = file.write(zone);
            if (!rs.ok()) {
                return rs;
            }
        }
        file.close();
        return collie::Status::ok_status();
    }

    collie::Status
    DiscoveryClient::list_servlet_to_file(const std::string &ns, const std::string &save_path, int *retry_time) {
        std::vector<std::string> json_list;
        auto rs = list_servlet_to_json(ns, json_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }

        alkaid::SequentialWriteFile file;
        rs = file.open(save_path, alkaid::kDefaultTruncateWriteOption);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &zone: json_list) {
            rs = file.write(zone);
            if (!rs.ok()) {
                return rs;
            }
        }
        file.close();
        return collie::Status::ok_status();
    }

    collie::Status
    DiscoveryClient::list_servlet_to_file(const std::string &ns, const std::string &zone, const std::string &save_path,
                                     int *retry_time) {
        std::vector<std::string> json_list;
        auto rs = list_servlet_to_json(ns, zone, json_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }

        alkaid::SequentialWriteFile file;
        rs = file.open(save_path, alkaid::kDefaultTruncateWriteOption);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &zone: json_list) {
            rs = file.write(zone);
            if (!rs.ok()) {
                return rs;
            }
        }
        file.close();
        return collie::Status::ok_status();
    }

    collie::Status
    DiscoveryClient::get_servlet(const std::string &ns_name, const std::string &zone_name, const std::string &servlet,
                            sirius::proto::ServletInfo &servlet_pb,
                            int *retry_time) {
        sirius::proto::DiscoveryQueryRequest request;
        sirius::proto::DiscoveryQueryResponse response;
        request.set_op_type(sirius::proto::QUERY_ZONE);
        auto rs = discovery_query(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != sirius::proto::SUCCESS) {
            return collie::Status::unavailable(response.errmsg());
        }
        if (response.servlet_infos_size() != 1) {
            return collie::Status::unavailable("bad proto format for servlet infos size: {}", response.servlet_infos_size());
        }
        servlet_pb = response.servlet_infos(0);
        return collie::Status::ok_status();
    }

    collie::Status
    DiscoveryClient::get_servlet_json(const std::string &ns_name, const std::string &zone_name, const std::string &servlet,
                                 std::string &json_str,
                                 int *retry_time) {
        sirius::proto::ServletInfo servlet_pb;
        auto rs = get_servlet(ns_name, zone_name, servlet, servlet_pb, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return Dumper::dump_proto(servlet_pb, json_str);
    }

    collie::Status
    DiscoveryClient::save_servlet_json(const std::string &ns_name, const std::string &zone_name, const std::string &servlet,
                                  const std::string &json_path,
                                  int *retry_time) {
        sirius::proto::ServletInfo servlet_pb;
        auto rs = get_servlet(ns_name, zone_name, servlet, servlet_pb, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        rs = Dumper::dump_proto_to_file(json_path, servlet_pb);
        return rs;

    }

}  // namespace sirius::client

