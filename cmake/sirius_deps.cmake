#
# Copyright 2023 The Carbin Authors.
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
############################################################
# system pthread and rt, dl
############################################################
set(CARBIN_SYSTEM_DYLINK)
if (APPLE)
    find_library(CoreFoundation CoreFoundation)
    list(APPEND CARBIN_SYSTEM_DYLINK ${CoreFoundation} pthread)
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    list(APPEND CARBIN_SYSTEM_DYLINK rt dl pthread)
endif ()

if (CARBIN_BUILD_TEST)
    enable_testing()
    #include(require_gtest)
    #include(require_gmock)
    #include(require_doctest)
endif (CARBIN_BUILD_TEST)

if (CARBIN_BUILD_BENCHMARK)
    #include(require_benchmark)
endif ()
include(GNUInstallDirs)
set(EA_ROOT /opt/EA/inf)
set(EA_ROOT_INCLUDE ${EA_ROOT}/${CMAKE_INSTALL_INCLUDEDIR})
set(EA_ROOT_LIB ${EA_ROOT}/${CMAKE_INSTALL_LIBDIR})
list(APPEND CMAKE_PREFIX_PATH "/opt/EA/inf")

find_package(mizar REQUIRED)
find_package(Threads REQUIRED)
find_package(melon REQUIRED)
find_package(turbo REQUIRED)
find_package(collie REQUIRED)
find_package(alkaid REQUIRED)
get_target_property(MIZAR_LIB mizar::mizar LOCATION)
message(STATUS "mizar found at ${MIZAR_LIB}")
include_directories(${collie_INCLUDE_DIR})
include_directories(${turbo_INCLUDE_DIR})
include_directories(${melon_INCLUDE_DIR})
include_directories(${alkaid_INCLUDE_DIR})
# lz4
find_path(LZ4_INCLUDE_DIRS NAMES lz4.h HINTS ${EA_ROOT_INCLUDE})
find_library(LZ4_LIBRARIES NAMES lz4 HINTS ${EA_ROOT_LIB})
if (NOT LZ4_INCLUDE_DIRS OR NOT LZ4_LIBRARIES)
    message(FATAL_ERROR "lz4 not found")
endif ()
find_path(ZSTD_INCLUDE_DIRS NAMES zstd.h HINTS ${zstd_ROOT_DIR}/include)
find_library(ZSTD_LIBRARIES NAMES zstd HINTS ${zstd_ROOT_DIR}/lib)
if (NOT ZSTD_INCLUDE_DIRS OR NOT ZSTD_LIBRARIES)
    message(FATAL_ERROR "zstd not found")
endif ()
find_package(ZLIB REQUIRED)
find_package(BZip2 REQUIRED)
if(BZIP2_INCLUDE_DIRS)
    include_directories(${BZIP2_INCLUDE_DIRS})
else()
    include_directories(${BZIP2_INCLUDE_DIR})
endif()
############################################################
#
# add you libs to the CARBIN_DEPS_LINK variable eg as turbo
# so you can and system pthread and rt, dl already add to
# CARBIN_SYSTEM_DYLINK, using it for fun.
##########################################################
set(CARBIN_DEPS_LINK
        melon::melon_static
        turbo::turbo_static
        alkaid::alkaid_static
        ${MIZAR_LIB}
        ${THIRDPARTY_LIBS}
        ${MELON_DEPS_LIBS}
        ${CARBIN_SYSTEM_DYLINK}
        )
list(REMOVE_DUPLICATES CARBIN_DEPS_LINK)
carbin_print_list_label("Denpendcies:" CARBIN_DEPS_LINK)





