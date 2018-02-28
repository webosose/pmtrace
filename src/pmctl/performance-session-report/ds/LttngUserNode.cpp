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

#include "LttngUserNode.h"

LttngUserNode::LttngUserNode()
{
    m_logFileName = Definition::SESSION_REPORT_LOG;
    m_logFileName.append(".user.node");
}

void LttngUserNode::setProviderAndEvent(string str)
{
    vector<string> tokens;
    tokenize(str, tokens, ":");

    if(tokens.size() < 2)
        return;

    m_providerName = deleteChars(tokens[0], " ");
    m_eventType = deleteChars(tokens[1], " ");

    if(m_eventType == Definition::TRACE_TYPE_SOINFO ||
            m_eventType == Definition::TRACE_TYPE_TOUCHCOORDINATE ||
            m_eventType == Definition::TRACE_TYPE_MESSAGE ||
            m_eventType == Definition::TRACE_TYPE_KEYVALUE)
    {
        m_signal = m_eventType;
    }
}

void LttngUserNode::setPayload(string str)
{
    if(str.size() < 8)
        return;

    string temp = str.substr(2, str.size()-5);

    if(countChar(temp, '=') == 1)
    {
        string left, right;
        dividePair(temp, left, right, "=");

        if(left == "scope")
        {
            deleteChars(right, "\"");
            addPayloadMap(left, right.substr(1, right.size()));

            if(right.find("::") != string::npos)
            {
                m_signal = makeSignal(right);
            }
            else
            {
                m_signal = right;
            }
        }
        else
        {
            Definition::LOG->LogDebug("(LttngUserNode) new scope %s / %s\n", left.c_str(), right.c_str());
        }
    }
    else
    {
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
}

string LttngUserNode::toString()
{
    string temp;

    temp = "ts(";
    temp += m_timeStamp;
    temp += ") host(" + m_hostName;
    temp += ") provider(" + m_providerName;
    temp += ") event(" + m_eventType;
    temp += ") cpu(" + m_cpuId;
    temp += ") ps(" + m_processName;
    temp += ") pid(" + m_processId;
    temp += ") tid(" + m_threadId;
    temp += ") signal(" + m_signal;
    temp += ")\n";

    map<string, string>::iterator iter;
    for(iter = m_payloadDataMap.begin(); iter != m_payloadDataMap.end(); iter++)
        temp += iter->first + "(" + iter->second + ") ";
    temp += "\n";

    return temp;
}

string LttngUserNode::makeSignal(string& str)
{
    string temp;
    vector<string> tokens;
    tokenize(str, tokens, "(");
    temp = tokens[0];
    tokens.clear();

    tokenize(temp, tokens, " ");

    for(unsigned int i=0; i<tokens.size(); i++)
    {
        if(tokens[i].find("::") != string::npos)
            temp = tokens[i];
    }

    return replaceString(temp, "::", "_");
}

