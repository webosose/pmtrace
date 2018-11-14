# Copyright (c) 2016-2018 LG Electronics, Inc.
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

include(FindPkgConfig)

pkg_check_modules(LTTNG_UST lttng-ust>=2.7.0)
include_directories(${LTTNG_UST_INCLUDE_DIRS})
add_compile_options(-Wl,--no-as-needed ${LTTNG_UST_CFLAGS_OTHER})

if(NOT LTTNG_UST_FOUND)
    message("Skip building libmemtracker because lttng-ust isn't found")
    return()
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(MT_NEW_SRC_FILES lttng-ust-mtrace-new.cpp)
add_library(lttng-ust-mtrace-new SHARED ${MT_NEW_SRC_FILES})
target_link_libraries(lttng-ust-mtrace-new ${LTTNG_UST_LDFLAGS} dl)
set_target_properties(lttng-ust-mtrace-new PROPERTIES
    VERSION ${PMTRACE_VER_STRING}
    SOVERSION ${PMTRACE_VER_MAJOR})

install(TARGETS lttng-ust-mtrace-new LIBRARY DESTINATION lib)
