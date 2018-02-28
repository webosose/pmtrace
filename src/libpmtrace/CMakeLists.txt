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

#
# src/libpmtrace/CMakeLists.txt
#

project(PmTrace)

include(FindPkgConfig)

pkg_check_modules(LTTNG_UST lttng-ust>=2.7.0)
include_directories(${LTTNG_UST_INCLUDE_DIRS})
add_compile_options(-Wl,--no-as-needed ${LTTNG_UST_CFLAGS_OTHER})

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include DESTINATION ${CMAKE_INSTALL_PREFIX})

set(CONF_FILE ${CMAKE_SOURCE_DIR}/files/pkgconfig/PmTrace.pc)

# "ENABLE_LIBPMTRACE" comes from pmtrace.bb recipe.
# And the value will be set true in all builds except RELEASE mode.
if(ENABLE_LIBPMTRACE AND LTTNG_UST_FOUND)
    set(EXTRA_LIBS "-ldl -lPmTrace")
    set(EXTRA_CFLAGS "-DENABLE_PMTRACE")

    if(DEFAULT_LOGGING STREQUAL "pmlog")
        set(EXTRA_CFLAGS ${EXTRA_CFLAGS} "-DPERFLOG_USE_PMLOG")
    endif()

    configure_file(${CONF_FILE}.in ${CONF_FILE} @ONLY)
    install(FILES ${CONF_FILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/pkgconfig/)
else()
    configure_file(${CONF_FILE}.in ${CONF_FILE} @ONLY)
    install(FILES ${CONF_FILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/pkgconfig/)
    message(STATUS "libpmtrace is disabled")
    return()
endif()

set(SRC_FILES PmTraceProvider.c)
add_library(PmTrace SHARED ${SRC_FILES})
target_link_libraries(PmTrace ${LTTNG_UST_LDFLAGS} dl)
set_target_properties(PmTrace PROPERTIES
    VERSION ${PMTRACE_VER_STRING}
    SOVERSION ${PMTRACE_VER_MAJOR})
install(TARGETS PmTrace LIBRARY DESTINATION lib)
