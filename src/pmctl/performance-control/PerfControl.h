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

#ifndef _PERF_CONTROL_
#define _PERF_CONTROL_

#include <iostream>
#include <cstring>
using namespace std;

class PerfControl
{
public:
    PerfControl(int argc, char ** argv);
    ~PerfControl();

    bool execModule();

private:
    bool runModule(string);

public:
    static const string MODULE_SESSION;
    static const string MODULE_SESSION_REPORT;
    static const string MODULE_PERFLOG_REPORT;
    static const string MODULE_MEMORY_PROFILE;

    static const string COMMAND_SESSION;
    static const string COMMAND_SESSION_REPORT;
    static const string COMMAND_PERFLOG_REPORT;
    static const string COMMAND_MEMORY_PROFILE;

private:
    string m_module;
    bool m_isDebug;
    int m_argc;
    char ** m_argv;
};

#endif

