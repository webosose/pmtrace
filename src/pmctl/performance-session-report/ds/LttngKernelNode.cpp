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

#include "LttngKernelNode.h"

LttngKernelNode::LttngKernelNode()
{
    m_logFileName = Definition::SESSION_REPORT_LOG;
    m_logFileName.append(".kernel.node");
}

void LttngKernelNode::setProvider(string str)
{
    vector<string> tokens;
    tokenize(str, tokens, ":");

    m_providerName = tokens[0];
}

void LttngKernelNode::setPayload(string str)
{
    if(str.size() < 8)
        return;

    string temp = str.substr(2, str.size()-5);

    vector<string> tokens, tokens2;
    tokenize(temp, tokens, ",");

    for(unsigned int i=0; i<tokens.size(); i++)
    {
        string sub = tokens[i];

        if(sub.find('=') != string::npos)
        {
            string left, right;
            dividePair(sub, left, right, "=");

            deleteChars(right, "\" ");
            addPayloadMap(left, right);
        }
    }
}

string LttngKernelNode::toString()
{
    string temp;

    temp = "ts(";
    temp += m_timeStamp;
    temp += ") host(" + m_hostName;
    temp += ") provider(" + m_providerName;
    temp += ") cpu(" + m_cpuId;
    temp += ") proc(" + m_processName;
    temp += ") pid(" + m_processId;
    temp += ") tid(" + m_threadId;
    temp += ")\n";

    map<string, string>::iterator iter;
    for(iter = m_payloadDataMap.begin(); iter != m_payloadDataMap.end(); iter++)
        temp += iter->first + "(" + iter->second + ") ";
    temp += "\n";

    return temp;
}

