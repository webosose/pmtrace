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
    static const string SESSION_EVENT_CONF;
    static const string PMDAEMON_LOG_FILE;

    static const string SESSION_CMD_START;
    static const string SESSION_CMD_STOP;

    static const string RAW_DATA_PATH;
    static const string LTTNG_FILE_NAME;

    static const string LTTNG_EVENT_ALL;
    static const string LTTNG_EVENT_USER;
    static const string LTTNG_EVENT_KERNEL;
    static const string NO_DATA;

    static Logger * LOG;
};

#endif

