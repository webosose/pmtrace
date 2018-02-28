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

#include <dlfcn.h>
#include <execinfo.h>
#include <sys/types.h>

#define TRACEPOINT_DEFINE
#define TRACEPOINT_CREATE_PROBES
#define TP_IP_PARAM ip
#include "ust_mtrace_new.h"

#define MAX_BT_DEPTH 5
#define BT_ARRAY_SIZE MAX_BT_DEPTH+1

#define LOOKUP_FUNCTION(ptr, type, func) \
    if ((ptr) == NULL) { \
        (ptr) = (type)dlsym(RTLD_NEXT, #func); \
        if ((ptr) == NULL) { \
            fprintf(stderr, "Unable to find " #func "\n"); \
            abort(); \
        } \
    }

static struct alloc_functions {
    void *(*malloc)(size_t size);
    void (*free)(void *ptr);
} cur_alloc;

void lookup_functions(void) {
    LOOKUP_FUNCTION(cur_alloc.malloc, void* (*)(size_t), malloc);
    LOOKUP_FUNCTION(cur_alloc.free, void (*)(void *), free);
}

void * operator new(size_t size) {
    void *retval;
    void *bt[BT_ARRAY_SIZE];
    size_t depth;

    LOOKUP_FUNCTION(cur_alloc.malloc, void* (*)(size_t), malloc);
    retval = cur_alloc.malloc(size);

    if (tracepoint_enabled(mtrace_new, new)) {
        depth = backtrace(bt, BT_ARRAY_SIZE);
        do_tracepoint(mtrace_new, new,
            size, retval, __builtin_return_address(0),
            bt+1, depth-1);
    }
    return retval;
}

void * operator new[] (size_t size) {
    void *retval;
    void *bt[BT_ARRAY_SIZE];
    size_t depth;

    LOOKUP_FUNCTION(cur_alloc.malloc, void* (*)(size_t), malloc);
    retval = cur_alloc.malloc(size);

    if (tracepoint_enabled(mtrace_new, new_arr)) {
        depth = backtrace(bt, BT_ARRAY_SIZE);
        do_tracepoint(mtrace_new, new_arr,
            size, retval, __builtin_return_address(0),
            bt+1, depth-1);
    }
    return retval;
}

void operator delete (void *ptr) {
    LOOKUP_FUNCTION(cur_alloc.free, void (*)(void *), free);
    if (tracepoint_enabled(mtrace_new, delete)) {
        do_tracepoint(mtrace_new, delete,
            ptr, __builtin_return_address(0));
    }

    cur_alloc.free(ptr);
}

void operator delete[] (void *ptr) {
    LOOKUP_FUNCTION(cur_alloc.free, void (*)(void *), free);
    if (tracepoint_enabled(mtrace_new, delete_arr)) {
        do_tracepoint(mtrace_new, delete_arr,
            ptr, __builtin_return_address(0));
    }

    cur_alloc.free(ptr);
}

__attribute__((constructor))
static void register_alloc_functions() {
    lookup_functions();
}
