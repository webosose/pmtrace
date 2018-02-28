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

find_library(PBNJSON_CPP pbnjson_cpp ${LIB_INSTALL_DIR})

pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(${GLIB2_INCLUDE_DIRS})
add_compile_options(-Wall -D__STRICT_ANSI__ ${GLIB2_CFLAGS_OTHER} -pthread)

set(BIN_NAME pmsession-report)
file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE COMMON_SRC_FILES ${PROJECT_SOURCE_DIR}/src/pmctl/common/*.cpp)

add_compile_options(-std=gnu++11)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src/pmctl/common)

add_executable (${BIN_NAME} ${SRC_FILES} ${COMMON_SRC_FILES})

target_link_libraries (${BIN_NAME} "-lpbnjson_c -lpbnjson_cpp")

install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
