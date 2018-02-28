// Copyright (c) 2016-2018 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#undef TRACEPOINT_PROVIDER
#define TRACEPOINT_PROVIDER mtrace_new

#if !defined(_TRACEPOINT_UST_LIBC_H) || defined(TRACEPOINT_HEADER_MULTI_READ)
#define _TRACEPOINT_UST_LIBC_H

#include <lttng/tracepoint.h>


TRACEPOINT_EVENT(mtrace_new, new,
    TP_ARGS(size_t, size, void *, ptr, void *, ip, void *, bt, size_t, depth),
    TP_FIELDS(
        ctf_integer(size_t, size, size)
        ctf_integer_hex(void *, ptr, ptr)
        ctf_sequence(void *, bt, bt, size_t, depth)
    )
)

TRACEPOINT_EVENT(mtrace_new, new_arr,
    TP_ARGS(size_t, size, void *, ptr, void *, ip, void *, bt, size_t, depth),
    TP_FIELDS(
        ctf_integer(size_t, size, size)
        ctf_integer_hex(void *, ptr, ptr)
        ctf_sequence(void *, bt, bt, size_t, depth)
    )
)

TRACEPOINT_EVENT(mtrace_new, delete,
    TP_ARGS(void *, ptr, void *, ip),
    TP_FIELDS(
        ctf_integer_hex(void *, ptr, ptr)
    )
)

TRACEPOINT_EVENT(mtrace_new, delete_arr,
    TP_ARGS(void *, ptr, void *, ip),
    TP_FIELDS(
        ctf_integer_hex(void *, ptr, ptr)
    )
)

#endif /* _TRACEPOINT_UST_LIBC_H */

#undef TRACEPOINT_INCLUDE
#define TRACEPOINT_INCLUDE "./ust_mtrace_new.h"

#include <lttng/tracepoint-event.h>
