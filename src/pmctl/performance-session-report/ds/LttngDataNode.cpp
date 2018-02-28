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

#include "LttngDataNode.h"

void LttngDataNode::setTimeStamp(string str, unsigned long long boottime, bool validBoottime)
{
    if(str.size() < 16)
    {
        Definition::LOG->LogError("(LttngDataNode) lttng timestamp value is wrong. : %s\n", str.c_str());
        return;
    }

    string tmp = str.substr(1, str.size()-5);
    vector<string> tokens;
    tokenize(tmp, tokens, ".");
    unsigned long long sec = stoull(tokens[0], NULL, 0);

    if(validBoottime && (boottime <= sec))
    {
        m_timeStamp = to_string(sec - boottime) + tokens[1];
    }
    else
    {
        m_timeStamp = str.substr(5, str.size()-9);
        if(m_timeStamp[0] == '0')
            m_timeStamp.erase(0, 1);
        deleteChars(m_timeStamp, ".");
    }

    m_timeStampl = stoull(m_timeStamp, NULL, 0);
}

void LttngDataNode::setHostName(string str)
{
    vector<string> tokens;
    tokenize(str, tokens, ":");

    m_hostName = tokens[0];
}

void LttngDataNode::setCpuId(string str)
{
    m_cpuId = deleteChars(str, " ");
    m_cpuIdi = atoi(m_cpuId.c_str());
}

void LttngDataNode::setProcessName(string str)
{
    m_processName = deleteChars(str, "\",");
}

void LttngDataNode::setProcessId(string str)
{
    m_processId = deleteChars(str, ", ");
}

void LttngDataNode::setThreadId(string str)
{
    m_threadId = deleteChars(str, ", ");
}

void LttngDataNode::addPayloadMap(string key, string val)
{
    m_payloadDataMap.insert(pair<string, string>(key, val));
}

string LttngDataNode::getPayload(string key)
{
    if(m_payloadDataMap.find(key) != m_payloadDataMap.end())
        return m_payloadDataMap.find(key)->second;
    else
        return Definition::NO_DATA;
}

bool LttngDataNode::dividePair(string& str, string& left, string& right, string delimiters)
{
    vector<string> tokens;
    tokenize(str, tokens, delimiters);

    if(tokens.size() != 2)
    {
        Definition::LOG->LogDebug("(LttngDataNode) Need to check dividePair function - %s\n", str.c_str());
        return false;
    }

    left = deleteChars(tokens[0], " ");
    right = tokens[1];

    return true;
}

