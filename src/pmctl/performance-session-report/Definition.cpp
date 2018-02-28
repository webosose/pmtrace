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

#include "Definition.h"

// lttng user event type
const string Definition::TRACE_TYPE_SOINFO              = "soinfo";
const string Definition::TRACE_TYPE_START               = "start";
const string Definition::TRACE_TYPE_END                 = "end";
const string Definition::TRACE_TYPE_BIN_INFO            = "bin_info";
const string Definition::TRACE_TYPE_BUILD_ID            = "build_id";
const string Definition::TRACE_TYPE_DEBUG_LINK          = "debug_link";
const string Definition::TRACE_TYPE_TOUCHCOORDINATE     = "touchcoordinate";
const string Definition::TRACE_TYPE_MESSAGE             = "message";
const string Definition::TRACE_TYPE_KEYVALUE            = "keyValue";
const string Definition::TRACE_TYPE_BEFORE              = "before";
const string Definition::TRACE_TYPE_AFTER               = "after";
const string Definition::TRACE_TYPE_SCOPE_ENTRY         = "scope_entry";
const string Definition::TRACE_TYPE_SCOPE_EXIT          = "scope_exit";
const string Definition::TRACE_TYPE_FUNCTION_ENTRY      = "function_entry";
const string Definition::TRACE_TYPE_FUNCTION_EXIT       = "function_exit";
const string Definition::TRACE_TYPE_IM_FUNCTION_ENTRY   = "input_change_function_entry";
const string Definition::TRACE_TYPE_IM_FUNCTION_EXIT    = "input_change_function_exit";

// lttng kernel event type
const string Definition::TRACE_KERNEL_SCHED_SWITCH      = "sched_switch";

// new pmtrace event type
const string Definition::TRACE_TYPE_LOG                 = "log";
const string Definition::TRACE_TYPE_BLOCK_ENTRY         = "block_entry";
const string Definition::TRACE_TYPE_BLOCK_EXIT          = "block_exit";
const string Definition::TRACE_TYPE_MARKER              = "marker";

const string Definition::LTTNG_EVENT_ALL                = "all";
const string Definition::LTTNG_EVENT_USER               = "user";
const string Definition::LTTNG_EVENT_KERNEL             = "kernel";

const string Definition::RAW_DATA_PATH                  = "/tmp/pmtrace/log";
const string Definition::SESSION_REPORT_LOG             = "/tmp/pmtrace/log/session-report.log";
const string Definition::DEFAULT_INPUT_FILE             = "/tmp/pmtrace/lttng.txt";
const string Definition::DEFAULT_OUTPUT_FILE            = "/tmp/session-report-result.json";
const string Definition::DEFAULT_CONFIG_FILE            = "/etc/pmtrace/session-report-conf.json";
const string Definition::NO_DATA                        = "NODATA";

Logger * Definition::LOG                                = NULL;

