#
# Copyright 2023 The titan-search Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
###########################################################################
# below are examples, after initialization, modify it to your own.
###########################################################################
###carbin_example
file(GLOB_RECURSE DISCOVERY_SRC "discovery/*.cc")
file(GLOB_RECURSE BASE_SRC "base/*.cc")
file(GLOB_RECURSE FLAGS_SRC "flags/*.cc")
file(GLOB_RECURSE STORAGE_SRC "storage/*.cc")
file(GLOB_RECURSE REST_SRC "restful/*.cc")
file(GLOB_RECURSE CLIENT_SRC "client/*.cc")

carbin_cc_object(
        NAMESPACE sirius
        NAME base_object
        SOURCES
        ${BASE_SRC}
        DEPS
        proto_object
        CXXOPTS
        ${CARBIN_CXX_OPTIONS}
)
carbin_cc_object(
        NAMESPACE sirius
        NAME flags_object
        SOURCES
        ${FLAGS_SRC}
        DEPS
        proto_object
        CXXOPTS
        ${CARBIN_CXX_OPTIONS}
)
carbin_cc_object(
        NAMESPACE sirius
        NAME storage_object
        SOURCES
        ${STORAGE_SRC}
        DEPS
        proto_object
        CXXOPTS
        ${CARBIN_CXX_OPTIONS}
)

carbin_cc_object(
        NAMESPACE sirius
        NAME discovery_object
        SOURCES
        ${DISCOVERY_SRC}
        DEPS
        proto_object
        CXXOPTS
        ${CARBIN_CXX_OPTIONS}
)


carbin_cc_object(
        NAMESPACE sirius
        NAME client_object
        SOURCES
        ${CLIENT_SRC}
        DEPS
        proto_object
        CXXOPTS
        ${CARBIN_CXX_OPTIONS}
)

carbin_cc_object(
        NAMESPACE sirius
        NAME restful
        SOURCES
        ${REST_SRC}
        DEPS
        proto_object
        CXXOPTS
        ${CARBIN_CXX_OPTIONS}
)

carbin_cc_library(
        NAMESPACE sirius
        NAME sirius
        OBJECTS proto_object base_object flags_object storage_object discovery_object client_object restful
        CXXOPTS
        ${CARBIN_CXX_OPTIONS}
        DEPS
        proto_object base_object flags_object storage_object discovery_object client_object restful
        PLINKS
        ${CARBIN_DEPS_LINK}
        PUBLIC
)

carbin_cc_binary(
        NAMESPACE sirius
        NAME sirius
        SOURCES app/server.cc
        CXXOPTS
        ${CARBIN_CXX_OPTIONS}
        LINKS
        ${CARBIN_DEPS_LINK} sirius::sirius_static
        PUBLIC
)
file(GLOB_RECURSE CLI_SRC "cli/*.cc")

carbin_cc_binary(
        NAMESPACE sirius
        NAME sirius_cli
        SOURCES ${CLI_SRC}
        CXXOPTS
        ${CARBIN_CXX_OPTIONS}
        LINKS
        ${CARBIN_DEPS_LINK} sirius::sirius_static
        PUBLIC
)
