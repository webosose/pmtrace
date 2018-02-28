# Copyright (c) 2007-2018 LG Electronics, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required (VERSION 2.8.7)

project(pmtrace C CXX)

set(PMTRACE_VER_MAJOR 1)
set(PMTRACE_VER_MINOR 1)
set(PMTRACE_VER_PATCH 0)
set(PMTRACE_VER_STRING ${PMTRACE_VER_MAJOR}.${PMTRACE_VER_MINOR}.${PMTRACE_VER_PATCH})

include(GNUInstallDirs)
include(webOS/webOS OPTIONAL RESULT_VARIABLE ret)
if(EXISTS ${ret})
    message("<INFO> webos ok:'${ret}'")
    webos_modules_init(1 0 0 QUALIFIER RC4)
    webos_component(1 0 0)
else()
    message("<INFO> not webos")
endif()

include_directories(include)

# "ENABLE_LIBPMTRACE" comes from pmtrace.bb recipe.
# And the value will be set true in all builds except RELEASE mode.
if(ENABLE_LIBPMTRACE)
    install(DIRECTORY files/conf/ DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/pmtrace FILES_MATCHING PATTERN "*conf.json")

    install(PROGRAMS
        files/scripts/acg_migration_tracker.py
        files/scripts/perf_log_viewer.py
        files/scripts/commander.py
        files/scripts/mem_profile.py
        files/scripts/plat_info.py
        files/scripts/smem.arm
        DESTINATION ${CMAKE_INSTALL_BINDIR})

    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/libmemtracker)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/pmctl/performance-control)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/pmctl/performance-session)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/pmctl/performance-session-report)
endif()

# The libpmtrace header files are needed by other components for building with pmtrace.
# However, the libpmtrace lib files will be not installed in RELEASE mode.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/libpmtrace)
