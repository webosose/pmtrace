#
# Copyright (C) 2009  Pierre-Marc Fournier
# Copyright (C) 2011  Mathieu Desnoyers <mathieu.desnoyers@efficios.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
#

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

#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/libmemtracker/liblttng-ust-mtrace-malloc)
#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/libmemtracker/liblttng-ust-mtrace-new)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/pmctl/performance-control)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/pmctl/performance-session)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/pmctl/performance-session-report)
endif()

# The libpmtrace header files are needed by other components for building with pmtrace.
# However, the libpmtrace lib files will be not installed in RELEASE mode.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/libpmtrace)
