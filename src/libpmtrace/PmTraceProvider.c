// Copyright (c) 2013-2018 LG Electronics, Inc.
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

#include <stdarg.h>

#define TRACEPOINT_CREATE_PROBES
#define TRACEPOINT_DEFINE
#include "PmTraceProvider.h"

#define MAXSTRBUFLEN    128

#define CREATE_MSG_FROM_VA(str) \
    do { \
        va_list args; \
        va_start(args, fmt); \
        vsnprintf(str, MAXSTRBUFLEN, fmt, args); \
        va_end(args); \
    } while(0)

void _PmtLog(const char* cat, const char* fmt, ...) {
    if (tracepoint_enabled(pmtrace, log)) {
        char payload[MAXSTRBUFLEN];

        CREATE_MSG_FROM_VA(payload);
        do_tracepoint(pmtrace, log, (char*)cat, payload);
    }
}

void _PmtBlockEntry(const char* cat, const char* name, const char* fmt, ...) {
    if (tracepoint_enabled(pmtrace, block_entry)) {
        char payload[MAXSTRBUFLEN];

        CREATE_MSG_FROM_VA(payload);
        do_tracepoint(pmtrace, block_entry, (char*)cat, (char*)name, payload);
    }
}

void _PmtBlockExit(const char* cat, const char* name, const char* fmt, ...) {
    if (tracepoint_enabled(pmtrace, block_exit)) {
        char payload[MAXSTRBUFLEN];

        CREATE_MSG_FROM_VA(payload);
        do_tracepoint(pmtrace, block_exit, (char*)cat, (char*)name, payload);
    }
}

void _PmtMarker(const char* cat, const char* name, const char* fmt, ...) {
    if (tracepoint_enabled(pmtrace, marker)) {
        char payload[MAXSTRBUFLEN];

        CREATE_MSG_FROM_VA(payload);
        do_tracepoint(pmtrace, marker, (char*)cat, (char*)name, payload);
    }
}

void _PmtPerfLog(const char* cat, const char* name, const char* fmt, ...) {
    if (tracepoint_enabled(pmtrace, perflog)) {
        char payload[MAXSTRBUFLEN];

        CREATE_MSG_FROM_VA(payload);
        do_tracepoint(pmtrace, perflog, (char*)cat, (char*)name, payload);
    }
}
