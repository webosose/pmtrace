// Copyright (c) 2013-2022 LG Electronics, Inc.
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
#define TRACEPOINT_PROVIDER pmtrace

#undef TRACEPOINT_INCLUDE
#define TRACEPOINT_INCLUDE "./PmTraceProvider.h"

#include <lttng/tracepoint.h>

#ifdef __cplusplus
extern "C"{
#endif /*__cplusplus */

#if !defined(__PMTRACE_PROVIDER_H) || defined(TRACEPOINT_HEADER_MULTI_READ)
#define __PMTRACE_PROVIDER_H

/*
    Tracepoint classes
*/

TRACEPOINT_EVENT_CLASS(
    pmtrace,
    cls_payload,
    TP_ARGS(
        char*, category,
        char*, payload
    ),
    TP_FIELDS(
        ctf_string(cat, category)
        ctf_string(payload, payload)
    )
)

TRACEPOINT_EVENT_CLASS(
    pmtrace,
    cls_name_payload,
    TP_ARGS(
        char*, category,
        char*, name,
        char*, payload
    ),
    TP_FIELDS(
        ctf_string(cat, category)
        ctf_string(name, name)
        ctf_string(payload, payload)
    )
)

/*
    Tracepoint instances
*/

TRACEPOINT_EVENT_INSTANCE(
    pmtrace,
    cls_name_payload,
    block_entry,
    TP_ARGS(
        char*, category,
        char*, name,
        char*, payload
    )
)

TRACEPOINT_EVENT_INSTANCE(
    pmtrace,
    cls_name_payload,
    block_exit,
    TP_ARGS(
        char*, category,
        char*, name,
        char*, payload
    )
)

TRACEPOINT_EVENT_INSTANCE(
    pmtrace,
    cls_payload,
    log,
    TP_ARGS(
        char*, category,
        char*, payload
    )
)

TRACEPOINT_EVENT_INSTANCE(
    pmtrace,
    cls_name_payload,
    marker,
    TP_ARGS(
        char*, category,
        char*, name,
        char*, payload
    )
)

TRACEPOINT_EVENT_INSTANCE(
    pmtrace,
    cls_name_payload,
    perflog,
    TP_ARGS(
        char*, category,
        char*, name,
        char*, payload
    )
)

#endif /* __PMTRACE_PROVIDER_H */

#include <lttng/tracepoint-event.h>

#ifdef __cplusplus
}
#endif /*__cplusplus */
