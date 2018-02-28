/*
 * Copyright (C) 2009  Pierre-Marc Fournier
 * Copyright (C) 2011  Mathieu Desnoyers <mathieu.desnoyers@efficios.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#define _GNU_SOURCE

#include <assert.h>
#include <execinfo.h>
#include <sys/types.h>

#define TRACEPOINT_DEFINE
#define TRACEPOINT_CREATE_PROBES
#define TP_IP_PARAM ip
#include "ust_mtrace_malloc.h"

#define MAX_BT_DEPTH 5
#define BT_ARRAY_SIZE MAX_BT_DEPTH+1

/*
    lttng/align.h
*/
#define __ALIGN_MASK(v, mask)   (((v) + (mask)) & ~(mask))
#define ALIGN(v, align)     __ALIGN_MASK(v, (__typeof__(v)) (align) - 1)

/*
    lttng/lttng-dlfcn.h
*/
#define dlopen glibc_dlopen_proto_lies_about_leafness
#define dlclose glibc_dlclose_proto_lies_about_leafness
#define dlsym glibc_dlsym_proto_lies_about_leafness
#define dlerror glibc_dlerror_proto_lies_about_leafness
#include <dlfcn.h>
#undef dlerror
#undef dlsym
#undef dlclose
#undef dlopen

extern void *dlopen(__const char *__file, int __mode);
extern int dlclose(void *__handle) __nonnull ((1));
extern void *dlsym(void *__restrict __handle,
    __const char *__restrict __name) __nonnull ((2));
extern char *dlerror(void);


#define STATIC_CALLOC_LEN 4096
static char static_calloc_buf[STATIC_CALLOC_LEN];
static unsigned long static_calloc_buf_offset;

struct alloc_functions {
    void *(*calloc)(size_t nmemb, size_t size);
    void *(*malloc)(size_t size);
    void (*free)(void *ptr);
    void *(*realloc)(void *ptr, size_t size);
    void *(*memalign)(size_t alignment, size_t size);
    int (*posix_memalign)(void **memptr, size_t alignment, size_t size);
};

static
void *static_calloc(size_t nmemb, size_t size);

static
struct alloc_functions cur_alloc = {
    .calloc = static_calloc
};

static __thread int malloc_nesting = 0;

/*
 * Static allocator to use when initially executing dlsym(). It keeps a
 * size_t value of each object size prior to the object.
 */
static
void *static_calloc_aligned(size_t nmemb, size_t size, size_t alignment)
{
    size_t prev_offset, new_offset, res_offset, aligned_offset;

    if (nmemb * size == 0) {
        return NULL;
    }

    /*
     * Protect static_calloc_buf_offset from concurrent updates
     * using a cmpxchg loop rather than a mutex to remove a
     * dependency on pthread. This will minimize the risk of bad
     * interaction between mutex and malloc instrumentation.
     */
    res_offset = CMM_LOAD_SHARED(static_calloc_buf_offset);
    do {
        prev_offset = res_offset;
        aligned_offset = ALIGN(prev_offset + sizeof(size_t), alignment);
        new_offset = aligned_offset + nmemb * size;
        if (new_offset > sizeof(static_calloc_buf)) {
            fprintf(stderr, "static_calloc_aligned: Exceed local buffer\n");
            abort();
        }
    } while ((res_offset = uatomic_cmpxchg(&static_calloc_buf_offset,
            prev_offset, new_offset)) != prev_offset);
    *(size_t *) &static_calloc_buf[aligned_offset - sizeof(size_t)] = size;
    return &static_calloc_buf[aligned_offset];
}

static
void *static_calloc(size_t nmemb, size_t size)
{
    void *retval;

    retval = static_calloc_aligned(nmemb, size, 1);
    return retval;
}

static
void *static_malloc(size_t size)
{
    void *retval;

    retval = static_calloc_aligned(1, size, 1);
    return retval;
}

static
void static_free(void *ptr)
{
    /* no-op. */
}

static
void *static_realloc(void *ptr, size_t size)
{
    size_t *old_size = NULL;
    void *retval;

    if (size == 0) {
        retval = NULL;
        goto end;
    }

    if (ptr) {
        old_size = (size_t *) ptr - 1;
        if (size <= *old_size) {
            /* We can re-use the old entry. */
            *old_size = size;
            retval = ptr;
            goto end;
        }
    }
    /* We need to expand. Don't free previous memory location. */
    retval = static_calloc_aligned(1, size, 1);
    assert(retval);
    if (ptr)
        memcpy(retval, ptr, *old_size);
end:
    return retval;
}

static
void *static_memalign(size_t alignment, size_t size)
{
    void *retval;

    retval = static_calloc_aligned(1, size, alignment);
    return retval;
}

static
int static_posix_memalign(void **memptr, size_t alignment, size_t size)
{
    void *ptr;

    /* Check for power of 2, larger than void *. */
    if (alignment & (alignment - 1)
            || alignment < sizeof(void *)
            || alignment == 0) {
        goto end;
    }
    ptr = static_calloc_aligned(1, size, alignment);
    *memptr = ptr;
end:
    return 0;
}


static
void setup_static_allocator(void)
{
    assert(cur_alloc.calloc == NULL);
    cur_alloc.calloc = static_calloc;
    assert(cur_alloc.malloc == NULL);
    cur_alloc.malloc = static_malloc;
    assert(cur_alloc.free == NULL);
    cur_alloc.free = static_free;
    assert(cur_alloc.realloc == NULL);
    cur_alloc.realloc = static_realloc;
    assert(cur_alloc.memalign == NULL);
    cur_alloc.memalign = static_memalign;
    assert(cur_alloc.posix_memalign == NULL);
    cur_alloc.posix_memalign = static_posix_memalign;
}

static
void lookup_all_symbols(void)
{
    struct alloc_functions af;

    /*
     * Temporarily redirect allocation functions to
     * static_calloc_aligned, and free function to static_free
     * (no-op), until the dlsym lookup has completed.
     */
    setup_static_allocator();

    /* Perform the actual lookups */
    af.calloc = dlsym(RTLD_NEXT, "calloc");
    af.malloc = dlsym(RTLD_NEXT, "malloc");
    af.free = dlsym(RTLD_NEXT, "free");
    af.realloc = dlsym(RTLD_NEXT, "realloc");
    af.memalign = dlsym(RTLD_NEXT, "memalign");
    af.posix_memalign = dlsym(RTLD_NEXT, "posix_memalign");

    /* Populate the new allocator functions */
    memcpy(&cur_alloc, &af, sizeof(cur_alloc));
}

void *malloc(size_t size)
{
    void *retval;
    void *bt[BT_ARRAY_SIZE];
    size_t depth;

    malloc_nesting++;
    if (cur_alloc.malloc == NULL) {
        cur_alloc.malloc = dlsym(RTLD_NEXT, "malloc");
        cur_alloc.calloc = dlsym(RTLD_NEXT, "calloc");
    }
    retval = cur_alloc.malloc(size);
    if (malloc_nesting == 1) {
        if (tracepoint_enabled(mtrace_malloc, malloc)) {
            depth = backtrace(bt, BT_ARRAY_SIZE);

            do_tracepoint(mtrace_malloc, malloc,
            size, retval, __builtin_return_address(0),
            bt+1, depth-1);
        }
    }
end:
    malloc_nesting--;
    return retval;
}

void free(void *ptr)
{
    malloc_nesting++;
    /*
     * Check whether the memory was allocated with
     * static_calloc_align, in which case there is nothing to free.
     */
    if (caa_unlikely((char *)ptr >= static_calloc_buf &&
            (char *)ptr < static_calloc_buf + STATIC_CALLOC_LEN)) {
        goto end;
    }

    if (malloc_nesting == 1) {
        if (tracepoint_enabled(mtrace_malloc, free)) {
            do_tracepoint(mtrace_malloc, free,
                ptr, __builtin_return_address(0));
        }
    }

    if (cur_alloc.free == NULL) {
        cur_alloc.free = dlsym(RTLD_NEXT, "free");
        if (cur_alloc.free == NULL) {
            fprintf(stderr, "freewrap: unable to find free\n");
            abort();
        }
    }
    cur_alloc.free(ptr);
end:
    malloc_nesting--;
}

void *calloc(size_t nmemb, size_t size)
{
    void *retval;
    void *bt[BT_ARRAY_SIZE];
    size_t depth;

    malloc_nesting++;
    if (cur_alloc.calloc == NULL) {
        cur_alloc.calloc = dlsym(RTLD_NEXT, "calloc");
        if (cur_alloc.calloc == NULL) {
            fprintf(stderr, "callocwrap: unable to find calloc\n");
            abort();
        }
    }
    retval = cur_alloc.calloc(nmemb, size);
    if (malloc_nesting == 1) {
        depth = backtrace(bt, BT_ARRAY_SIZE);   // Exception to avoid deadlock
        if (tracepoint_enabled(mtrace_malloc, calloc)) {
            do_tracepoint(mtrace_malloc, calloc,
                nmemb, size, retval, __builtin_return_address(0),
                bt+1, depth-1);
        }
    }
end:
    malloc_nesting--;
    return retval;
}

void *realloc(void *ptr, size_t size)
{
    void *retval;
    void *bt[BT_ARRAY_SIZE];
    size_t depth;

    malloc_nesting++;
    if (caa_unlikely((char *)ptr >= static_calloc_buf &&
            (char *)ptr < static_calloc_buf + STATIC_CALLOC_LEN)) {
        size_t *old_size;

        old_size = (size_t *) ptr - 1;
        retval = cur_alloc.calloc(1, size);
        if (retval) {
            memcpy(retval, ptr, *old_size);
        }
        ptr = NULL;
        goto end;
    }

    if (cur_alloc.realloc == NULL) {
        cur_alloc.realloc = dlsym(RTLD_NEXT, "realloc");
        if (cur_alloc.realloc == NULL) {
            fprintf(stderr, "reallocwrap: unable to find realloc\n");
            abort();
        }
    }
    retval = cur_alloc.realloc(ptr, size);
end:
    if (malloc_nesting == 1) {
        if (tracepoint_enabled(mtrace_malloc, realloc)) {
            depth = backtrace(bt, BT_ARRAY_SIZE);
            do_tracepoint(mtrace_malloc, realloc,
                ptr, size, retval, __builtin_return_address(0),
                bt+1, depth-1);
        }
    }
    malloc_nesting--;
    return retval;
}

void *memalign(size_t alignment, size_t size)
{
    void *retval;
    void *bt[BT_ARRAY_SIZE];
    size_t depth;

    malloc_nesting++;
    if (cur_alloc.memalign == NULL) {
        cur_alloc.memalign = dlsym(RTLD_NEXT, "memalign");
        if (cur_alloc.memalign == NULL) {
            fprintf(stderr, "memalignwrap: unable to find memalign\n");
            abort();
        }
    }
    retval = cur_alloc.memalign(alignment, size);
    if (malloc_nesting == 1) {
        if (tracepoint_enabled(mtrace_malloc, memalign)) {
            depth = backtrace(bt, BT_ARRAY_SIZE);
            do_tracepoint(mtrace_malloc, memalign,
                alignment, size, retval,
                __builtin_return_address(0),
                bt+1, depth-1);
        }
    }
    malloc_nesting--;
    return retval;
}

int posix_memalign(void **memptr, size_t alignment, size_t size)
{
    int retval;
    void *bt[BT_ARRAY_SIZE];
    size_t depth;

    malloc_nesting++;
    if (cur_alloc.posix_memalign == NULL) {
        cur_alloc.posix_memalign = dlsym(RTLD_NEXT, "posix_memalign");
        if (cur_alloc.posix_memalign == NULL) {
            fprintf(stderr, "posix_memalignwrap: unable to find posix_memalign\n");
            abort();
        }
    }
    retval = cur_alloc.posix_memalign(memptr, alignment, size);
    if (malloc_nesting == 1) {
        if (tracepoint_enabled(mtrace_malloc, posix_memalign)) {
            depth = backtrace(bt, BT_ARRAY_SIZE);
            do_tracepoint(mtrace_malloc, posix_memalign,
                *memptr, alignment, size,
                retval, __builtin_return_address(0),
                bt+1, depth-1);
        }
    }
    malloc_nesting--;
    return retval;
}

__attribute__((constructor))
void lttng_ust_malloc_wrapper_init(void)
{
    if (cur_alloc.calloc == NULL)
        cur_alloc.calloc = dlsym(RTLD_NEXT, "calloc");

    if (cur_alloc.malloc == NULL)
        cur_alloc.malloc = dlsym(RTLD_NEXT, "malloc");

    if (cur_alloc.free == NULL)
        cur_alloc.free = dlsym(RTLD_NEXT, "free");

    if (cur_alloc.realloc = NULL)
        cur_alloc.realloc = dlsym(RTLD_NEXT, "realloc");

    if (cur_alloc.memalign == NULL)
        cur_alloc.memalign = dlsym(RTLD_NEXT, "memalign");

    if (cur_alloc.posix_memalign == NULL)
        cur_alloc.posix_memalign = dlsym(RTLD_NEXT, "posix_memalign");
}
