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

const string Definition::SESSION_EVENT_CONF     = "/etc/pmtrace/session-event-conf.json";
const string Definition::PMDAEMON_LOG_FILE      = "pmdaemon.log";

const string Definition::SESSION_CMD_START      = "start";
const string Definition::SESSION_CMD_STOP       = "stop";

const string Definition::RAW_DATA_PATH          = "/tmp/pmtrace/";
const string Definition::LTTNG_FILE_NAME        = "/tmp/pmtrace/lttng.txt";

const string Definition::LTTNG_EVENT_ALL        = "all";
const string Definition::LTTNG_EVENT_USER       = "user";
const string Definition::LTTNG_EVENT_KERNEL     = "kernel";

const string Definition::NO_DATA                = "NODATA";
Logger * Definition::LOG                        = NULL;

