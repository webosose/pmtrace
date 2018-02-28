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

#include "PerfSession.h"

bool PerfSession::initialize()
{
    Definition::LOG->LogDebug("(PerfSession) initialize\n");

    if(!makeWorkDir())
        return false;

    return true;
}

bool PerfSession::controlSession(OptionHandler & opt)
{
    string command = opt.getCmd();

    if(command == Definition::SESSION_CMD_START)
    {
        if(!readySession(opt))
            return false;

        startSession();
        return true;
    }
    else if(command == Definition::SESSION_CMD_STOP)
    {
        stopSession();
        return true;
    }
    else
        return false;
}

bool PerfSession::makeWorkDir()
{
    Definition::LOG->LogDebug("(PerfSession) make directory : %s\n", Definition::RAW_DATA_PATH.c_str());

    if(mkdir(Definition::RAW_DATA_PATH.c_str(), S_IFDIR) != 0 && errno != EEXIST)
    {
        Definition::LOG->LogError("(PerfSession) raw data path is wrong! error msg : %s\n", strerror(errno));
        return false;
    }

    return true;
}

bool PerfSession::readySession(OptionHandler & opt)
{
    if(!m_config.initialize(opt))
    {
        Definition::LOG->LogError("(PerfSession) Fail to initialize session config\n");
        return false;
    }

    if(!m_config.getLttngEventsInfo(m_lttngEventsMap))
    {
        Definition::LOG->LogError("(PerfSession) Fail to get lttng events\n");
        return false;
    }

    return true;
}

void PerfSession::startSession()
{
    Definition::LOG->LogDebug("(PerfSession) startSession function\n");

    m_lttngCtl.prepareLttng();
    m_lttngCtl.setupLttng(m_lttngEventsMap, m_config.hasKernelEvent());
    m_lttngCtl.startLttng();
}

void PerfSession::stopSession()
{
    Definition::LOG->LogDebug("(PerfSession) stopSession function\n");

    m_lttngCtl.stopLttng();
    m_lttngCtl.viewLttng(Definition::LTTNG_FILE_NAME);
    m_lttngCtl.destroyLttng();
    Definition::LOG->LogInfo("lttng data file : %s\n", Definition::LTTNG_FILE_NAME.c_str());
}

