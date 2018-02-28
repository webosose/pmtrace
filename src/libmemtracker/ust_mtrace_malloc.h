/*
 * Copyright (C) 2011  Mathieu Desnoyers <mathieu.desnoyers@efficios.com>
 * Copyright (c) 2016-2018 LG Electronics, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#undef TRACEPOINT_PROVIDER
#define TRACEPOINT_PROVIDER mtrace_malloc

#if !defined(_TRACEPOINT_UST_LIBC_H) || defined(TRACEPOINT_HEADER_MULTI_READ)
#define _TRACEPOINT_UST_LIBC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lttng/tracepoint.h>

TRACEPOINT_EVENT(mtrace_malloc, malloc,
    TP_ARGS(size_t, size, void *, ptr, void *, ip, void *, bt, size_t, depth),
    TP_FIELDS(
        ctf_integer(size_t, size, size)
        ctf_integer_hex(void *, ptr, ptr)
        ctf_sequence(void *, bt, bt, size_t, depth)
    )
)

TRACEPOINT_EVENT(mtrace_malloc, free,
    TP_ARGS(void *, ptr, void *, ip),
    TP_FIELDS(
        ctf_integer_hex(void *, ptr, ptr)
    )
)

TRACEPOINT_EVENT(mtrace_malloc, calloc,
    TP_ARGS(size_t, nmemb, size_t, size, void *, ptr, void *, ip, void *, bt, size_t, depth),
    TP_FIELDS(
        ctf_integer(size_t, nmemb, nmemb)
        ctf_integer(size_t, size, size)
        ctf_integer_hex(void *, ptr, ptr)
        ctf_sequence(void *, bt, bt, size_t, depth)
    )
)

TRACEPOINT_EVENT(mtrace_malloc, realloc,
    TP_ARGS(void *, in_ptr, size_t, size, void *, ptr, void *, ip, void *, bt, size_t, depth),
    TP_FIELDS(
        ctf_integer_hex(void *, in_ptr, in_ptr)
        ctf_integer(size_t, size, size)
        ctf_integer_hex(void *, ptr, ptr)
        ctf_sequence(void *, bt, bt, size_t, depth)
    )
)

TRACEPOINT_EVENT(mtrace_malloc, memalign,
    TP_ARGS(size_t, alignment, size_t, size, void *, ptr, void *, ip, void *, bt, size_t, depth),
    TP_FIELDS(
        ctf_integer(size_t, alignment, alignment)
        ctf_integer(size_t, size, size)
        ctf_integer_hex(void *, ptr, ptr)
        ctf_sequence(void *, bt, bt, size_t, depth)
    )
)

TRACEPOINT_EVENT(mtrace_malloc, posix_memalign,
    TP_ARGS(void *, out_ptr, size_t, alignment, size_t, size, int, result, void *, ip, void *, bt, size_t, depth),
    TP_FIELDS(
        ctf_integer_hex(void *, out_ptr, out_ptr)
        ctf_integer(size_t, alignment, alignment)
        ctf_integer(size_t, size, size)
        ctf_integer(int, result, result)
        ctf_sequence(void *, bt, bt, size_t, depth)
    )
)

#endif /* _TRACEPOINT_UST_LIBC_H */

#undef TRACEPOINT_INCLUDE
#define TRACEPOINT_INCLUDE "./ust_mtrace_malloc.h"

#include <lttng/tracepoint-event.h>

#ifdef __cplusplus
}
#endif
