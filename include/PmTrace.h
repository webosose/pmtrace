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

#ifndef __PMTRACE_H
#define __PMTRACE_H

#ifdef ENABLE_PMTRACE

/**
 * @brief Create a instance of tracepoint.
 *
 * Note that it must be defined on exactly one translation unit.
 * In other words, this shouldn't be defined in two separate C source file.
 */
#ifdef PMTRACE_DEFINE
#define TRACEPOINT_DEFINE
#define TRACEPOINT_PROBE_DYNAMIC_LINKAGE
#endif //PMTRACE_DEFINE

#include "PmTraceProvider.h"
#include "PmTraceMsg.h"


#ifdef __cplusplus
extern "C" {
#endif

void _PmtLog(const char* cat, const char* fmt, ...);
void _PmtBlockEntry(const char* cat, const char* name, const char* fmt, ...);
void _PmtBlockExit(const char* cat, const char* name, const char* fmt, ...);
void _PmtMarker(const char* cat, const char* name, const char* fmt, ...);
void _PmtPerfLog(const char* cat, const char* name, const char* fmt, ...);

#ifdef __cplusplus
}
#endif

/**
 * @brief Argument macros
 *
 * Macros for an argument. It consists of a pair of key and value
 * and has three variations depending on the type of value
 *
 * PMTKVS : for string
 * PMTKVD : for integer
 * PMTKVF : for float
 */
#define PMTKVS(k, v)    k, "%s", v
#define PMTKVD(k, v)    k, %d, v
#define PMTKVF(k, v)    k, %f, v

/**
 * @brief PmLog is for free form tracing.
 *
 * @param cat A category for tracing.
 * @param ... A payload for argument macros (up to 10).
 */
#define PmtLog(cat, ...) \
    _PmtLog(cat, FORMATTED_VA(__VA_ARGS__))

/**
 * @brief PmtBlockEntry is for tracing the duration of a scope.
   Be careful to catch all exit cases.
 *
 * @param cat A category for tracing.
 * @param name A name of duration.
 * @param ... A payload for argument macros (up to 10).
 */
#define PmtBlockEntry(cat, name, ...) \
    _PmtBlockEntry(cat, name, FORMATTED_VA(__VA_ARGS__))

/**
 * @brief PmtBlockExit is for tracing the duration of a scope.
   Be careful to catch all exit cases.
 *
 * @param cat A category for tracing.
 * @param name A name of duration.
 * @param ... A payload for argument macros (up to 10).
 */
#define PmtBlockExit(cat, name, ...) \
    _PmtBlockExit(cat, name, FORMATTED_VA(__VA_ARGS__))

/**
 * @brief PmtMarker is for an important note.
 *
 * @param cat A category for tracing.
 * @param name A name of marker.
 * @param ... A payload for argument macros (up to 10).
 */
#define PmtMarker(cat, name, ...) \
    _PmtMarker(cat, name, FORMATTED_VA(__VA_ARGS__))

#ifdef __cplusplus

class _PmtScopedBlock {
public:
    _PmtScopedBlock(const char* cat, const char* name)
        : blkCat(cat), blkName(name)
    {
        _PmtBlockEntry(blkCat, blkName, const_cast<char*>(""));
    }

    ~_PmtScopedBlock()
    {
        _PmtBlockExit(blkCat, blkName, const_cast<char*>(""));
    }

private:
    const char* blkCat;
    const char* blkName;

    // Prevent heap allocation
    void operator delete(void*);
    void* operator new(size_t);
    _PmtScopedBlock(const _PmtScopedBlock&);
    _PmtScopedBlock& operator=(const _PmtScopedBlock&);
};

/**
 * @brief PmtScopedBlock is for tracing the duration of a block.
 *        Declare this on the head or block and then
 *        it will insert PmtBlockEntry/Exit automatically.
 *
 * Note that it's supported only in C++, not in C. In C code use
 * PmtBlockEntry/Exit pair manually at both ends of a block.
 *
 * @param cat A category for tracing.
 */
#define PmtScopedBlock(cat) \
    _PmtScopedBlock traceScopedBlock(cat, __PRETTY_FUNCTION__)

#endif // __cplusplus

/**
 * @brief PmtPerfLog is for post-processable event from PmDaemon.
 */
#ifdef PERFLOG_USE_PMLOG
#include <time.h>
#include <PmLogLib.h>

#define _PmLogMsgPerfLog(x) __PmLogMsgPerfLog##x
#define __PmLogMsgPerfLog0 _PmLogMsgClock2
#define __PmLogMsgPerfLog1 _PmLogMsgClock3
#define __PmLogMsgPerfLog2 _PmLogMsgClock4
#define __PmLogMsgPerfLog3 _PmLogMsgClock5
#define __PmLogMsgPerfLog4 _PmLogMsgClock6
#define __PmLogMsgPerfLog5 _PmLogMsgClock7
#define __PmLogMsgPerfLog6 _PmLogMsgClock8
#define __PmLogMsgPerfLog7 _PmLogMsgClock9
#define __PmLogMsgPerfLog8 _PmLogMsgClock10

#define PmtPerfLog(ctx, msgid, type, group, kv_count, ...) \
    do { \
        struct timespec ts; \
        clock_gettime(CLOCK_MONOTONIC, &ts); \
        _PmLogMsgPerfLog(kv_count)(ctx, Info, msgid, \
            "CLOCK", "%jd.%03d", (intmax_t) ts.tv_sec, ts.tv_nsec / 1000000, \
            "PerfType", "\"%s\"", type, \
            "PerfGroup", "\"%s\"", group, \
            __VA_ARGS__); \
        if (tracepoint_enabled(pmtrace, perflog)) { \
            char payload[128]; \
            snprintf(payload, 128, "{\"CLOCK\":%jd.%03d, \"PerfType\":\"%s\", \"PerfGroup\":\"%s\"}", (intmax_t) ts.tv_sec, ts.tv_nsec / 1000000, type, group); \
            do_tracepoint(pmtrace, perflog, "perflog", msgid, payload); \
        } \
    } while(0)

#else // PERFLOG_USE_PMLOG

#include <time.h>
#include <syslog.h>

#define _PmtPerfLogSyslog(...) \
    syslog(LOG_INFO, FORMATTED_VA(__VA_ARGS__))
#define _PmtPerfLogLttng(cat, name, ...) \
    _PmtPerfLog(cat, name, FORMATTED_VA(__VA_ARGS__))
#define CLOCK_STR_LEN 128

#define PmtPerfLog(ctx, msgid, type, group, ...) \
    do { \
        struct timespec ts; \
        char clk[CLOCK_STR_LEN]; \
        clock_gettime(CLOCK_MONOTONIC, &ts); \
        snprintf(clk, CLOCK_STR_LEN, "%jd.%3d", (intmax_t) ts.tv_sec, ts.tv_nsec / 1000000); \
        _PmtPerfLogSyslog(PMTKVS("ctx", ctx), PMTKVS("CLOCK", clk), PMTKVS("msgid", msgid), PMTKVS("PerfType", type), PMTKVS("PerfGroup", group), __VA_ARGS__); \
        _PmtPerfLogLttng(ctx, msgid, PMTKVS("CLOCK", clk), PMTKVS("PerfType", type), PMTKVS("PerfGroup", group), __VA_ARGS__); \
    } while(0)

#endif

/* TODO: Remove below macros which are for backward compatibility */
#define PMTRACE_BEFORE(name) \
    tracepoint(pmtrace, block_entry, "UNKNOWN", name, "")
#define PMTRACE_AFTER(name) \
    tracepoint(pmtrace, block_exit, "UNKNOWN", name, "")

#else // ENABLE_PMTRACE

#define PmtLog(cat, ...) do {} while(0)
#define PmtBlockEntry(cat, name, ...) do {} while(0)
#define PmtBlockExit(cat, name, ...) do {} while(0)
#define PmtMarker(cat, name, ...) do {} while(0)
#define PmtScopedBlock(cat) do {} while(0)
#define PmtPerfLog(ctx, msgid, type, group, ...) do {} while(0)

/* TODO: Remove below macros which are for backward compatibility */
#define PMTRACE_BEFORE(name) do {} while(0)
#define PMTRACE_AFTER(name) do {} while(0)

#endif // ENABLE_PMTRACE

#endif // __PMTRACE_H
