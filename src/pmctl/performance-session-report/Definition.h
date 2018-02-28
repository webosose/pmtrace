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

#ifndef _DEFINITION_H_
#define _DEFINITION_H_

#include "log/Logger.h"

class Definition
{
public:
    static const string TRACE_TYPE_SOINFO;
    static const string TRACE_TYPE_START;
    static const string TRACE_TYPE_END;
    static const string TRACE_TYPE_BIN_INFO;
    static const string TRACE_TYPE_BUILD_ID;
    static const string TRACE_TYPE_DEBUG_LINK;
    static const string TRACE_TYPE_TOUCHCOORDINATE;
    static const string TRACE_TYPE_MESSAGE;
    static const string TRACE_TYPE_KEYVALUE;
    static const string TRACE_TYPE_BEFORE;
    static const string TRACE_TYPE_AFTER;
    static const string TRACE_TYPE_SCOPE_ENTRY;
    static const string TRACE_TYPE_SCOPE_EXIT;
    static const string TRACE_TYPE_FUNCTION_ENTRY;
    static const string TRACE_TYPE_FUNCTION_EXIT;
    static const string TRACE_TYPE_IM_FUNCTION_ENTRY;
    static const string TRACE_TYPE_IM_FUNCTION_EXIT;

    static const string TRACE_KERNEL_SCHED_SWITCH;

    // new pmtrace event type
    static const string TRACE_TYPE_LOG;
    static const string TRACE_TYPE_BLOCK_ENTRY;
    static const string TRACE_TYPE_BLOCK_EXIT;
    static const string TRACE_TYPE_MARKER;

    static const string LTTNG_EVENT_ALL;
    static const string LTTNG_EVENT_USER;
    static const string LTTNG_EVENT_KERNEL;

    static const string RAW_DATA_PATH;
    static const string SESSION_REPORT_LOG;
    static const string DEFAULT_INPUT_FILE;
    static const string DEFAULT_OUTPUT_FILE;
    static const string DEFAULT_CONFIG_FILE;
    static const string NO_DATA;
    static Logger * LOG;

    static const int JSON_START = 1;
    static const int JSON_END = 2;
    static const int BUFF_SIZE = 1024;
    static const int PID = 1;
    static const int TID = 2;
    static const int RETURN_TRUE = 1;
    static const int RETURN_FALSE = 0;
    static const int RETURN_ERROR = -99999;
    static const int CPU_USAGE_CORE_MAX = 100;
    static const int CPU_USAGE_TOTAL_MAX = 100;
    static const int LTTNG_SETUP_WAITING_TIME = 1;
    static const long DEFAULT_CPU_USAGE_PERIOD = 10000l; // 0.01 sec

    static const int MSG_INDEX_TIMESTAMP = 0;
    static const int MSG_INDEX_HOSTNAME = 2;
    static const int MSG_INDEX_PROVIDER_AND_EVENT = 3;
    static const int MSG_INDEX_CPU_ID = 7;
    static const int MSG_INDEX_PROCESS_NAME = 12;
    static const int MSG_INDEX_PROCESS_ID = 15;
    static const int MSG_INDEX_THREAD_ID = 18;
    static const int MSG_INDEX_MINIMUM_USER = Definition::MSG_INDEX_THREAD_ID + 2;
    static const int MSG_INDEX_MINIMUM_KERNEL = Definition::MSG_INDEX_PROCESS_ID + 2;

    static const int CPU_CORE_VIEW_ID = -1;
    static const int CPU_PROCESS_VIEW_ID = -2;
    static const int CPU_CORE_BASE_NUMBER = -1;
    static const int GROUP_VIEW_ID_BASE = CPU_PROCESS_VIEW_ID - 1;
};

#endif

