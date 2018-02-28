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

#include "LttngController.h"

LttngController::LttngController()
: m_setupLttng(false)
{}

void LttngController::prepareLttng()
{
    Definition::LOG->LogDebug("(LttngModule) prepareLttng function\n");

    destroyLttng();
    system("rm -Rf /home/root/lttng-traces/");
    system("lttng create");
}

void LttngController::setupLttng(map<string, string>& eventMap, bool enableKernel)
{
    Definition::LOG->LogDebug("(LttngModule) setupLttng function\n");

    bool allOption = false;

    system("lttng enable-channel channel0 -u");
    system("lttng add-context -c channel0 -u -t procname -t vpid -t vtid");

    if(enableKernel)
    {
        system("lttng enable-channel channel1 -k");
    }

    map<string, string>::iterator iter, ret;
    ret = eventMap.find(Definition::LTTNG_EVENT_ALL);

    if(ret != eventMap.end() && ret->second == Definition::LTTNG_EVENT_USER)
    {
        system("lttng enable-event -c channel0 -u -a");
        allOption = true;
    }

    for(iter = eventMap.begin(); iter != eventMap.end(); iter++)
    {
        if(iter->second == Definition::LTTNG_EVENT_USER && !allOption)
        {
            string event = iter->first;
            string temp = "lttng enable-event -c channel0 -u ";
            temp += event;
            system(temp.c_str());
        }
        else if (enableKernel && iter->second == Definition::LTTNG_EVENT_KERNEL)
        {
            string event = iter->first;
            string temp = "lttng enable-event -c channel1 -k ";
            temp += event;
            system(temp.c_str());
        }
    }

    m_setupLttng = true;
}

void LttngController::startLttng()
{
    Definition::LOG->LogDebug("(LttngModule) startLttng function\n");

    system("lttng start");
}

void LttngController::stopLttng()
{
    Definition::LOG->LogDebug("(LttngModule) stopLttng function\n");

    system("lttng stop");
}

void LttngController::viewLttng(string name)
{
    Definition::LOG->LogDebug("(LttngModule) viewLttng function\n");

    // backup old data
    system("mv /tmp/pmtrace/lttng.txt /tmp/pmtrace/lttng.txt.old 2>/dev/null");

    string temp = "lttng view --viewer 'babeltrace --clock-seconds' > ";
    temp += name;
    Definition::LOG->LogDebug("(LttngModule) command : %s\n", temp.c_str());
    system(temp.c_str());
}

void LttngController::destroyLttng()
{
    Definition::LOG->LogDebug("(LttngModule) destroyLttng function\n");

    system("lttng destroy -a");
    system("kill -9 `ps -ef | grep lttng | grep -v grep | awk '{print $2}'` 2>/dev/null");
    m_setupLttng = false;
}
