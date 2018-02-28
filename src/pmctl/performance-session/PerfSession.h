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

#ifndef _PERF_SESSION_H_
#define _PERF_SESSION_H_

#include "LttngController.h"
#include "option/OptionHandler.h"
#include "config/Configuration.h"
#include "utils/Util.h"

class PerfSession
{
public:
    bool initialize();
    bool controlSession(OptionHandler&);

private:
    bool makeWorkDir();
    bool readySession(OptionHandler&);
    void startSession();
    void stopSession();

private:
    LttngController m_lttngCtl;
    Configuration m_config;

    map<string, string> m_lttngEventsMap;
};

#endif

