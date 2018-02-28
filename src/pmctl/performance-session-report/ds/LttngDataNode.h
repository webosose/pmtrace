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

#ifndef _LTTNG_DATA_NODE_H_
#define _LTTNG_DATA_NODE_H_

#include "../Definition.h"
#include "utils/Util.h"

class LttngDataNode
{
public:
    void setTimeStamp(string, unsigned long long, bool);
    void setHostName(string);
    void setCpuId(string);
    void setProcessName(string);
    void setProcessId(string);
    void setThreadId(string);
    void addPayloadMap(string, string);

    string getTimeStamp() { return m_timeStamp; }
    unsigned long long getTimeStampl() { return m_timeStampl; }
    string getHostName() { return m_hostName; }
    string getProvider() { return m_providerName; }
    string getCpuId() { return m_cpuId; }
    int getCpuIdi() { return m_cpuIdi; }
    string getProcessName() { return m_processName; }
    string getProcessId() { return m_processId; }
    string getThreadId() { return m_threadId; }
    map<string, string> getPayloadMap() { return m_payloadDataMap; }
    string getPayload(string key);

protected:
    bool dividePair(string& str, string& left, string& right, string delimiters);

protected:
    string m_timeStamp;
    unsigned long long m_timeStampl;
    string m_hostName;
    string m_providerName;
    string m_cpuId;
    int m_cpuIdi;
    string m_processName;
    string m_processId;
    string m_threadId;
    map<string, string> m_payloadDataMap;
    string m_logFileName;
};

#endif

