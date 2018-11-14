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

include(FindPkgConfig)

pkg_check_modules(LTTNG_UST lttng-ust>=2.7.0)
include_directories(${LTTNG_UST_INCLUDE_DIRS})
add_compile_options(-Wl,--no-as-needed ${LTTNG_UST_CFLAGS_OTHER})

if(NOT LTTNG_UST_FOUND)
    message("Skip building libmemtracker because lttng-ust isn't found")
    return()
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(MT_MALLOC_SRC_FILES lttng-ust-mtrace-malloc.c)
add_library(lttng-ust-mtrace-malloc SHARED ${MT_MALLOC_SRC_FILES})
target_link_libraries(lttng-ust-mtrace-malloc ${LTTNG_UST_LDFLAGS} dl)
set_target_properties(lttng-ust-mtrace-malloc PROPERTIES
    VERSION ${PMTRACE_VER_STRING}
    SOVERSION ${PMTRACE_VER_MAJOR})

install(TARGETS lttng-ust-mtrace-malloc LIBRARY DESTINATION lib)
