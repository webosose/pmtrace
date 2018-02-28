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

#include "LttngUserNewNode.h"

LttngUserNewNode::LttngUserNewNode()
{
    m_logFileName = Definition::SESSION_REPORT_LOG;
    m_logFileName.append(".user.new.node");
}

void LttngUserNewNode::setProviderAndEvent(string str)
{
    vector<string> tokens;
    tokenize(str, tokens, ":");

    if(tokens.size() < 2)
        return;

    m_providerName = deleteChars(tokens[0], " ");
    m_eventType = deleteChars(tokens[1], " ");
}

void LttngUserNewNode::setPayload(string str)
{
    if(str.size() < 8)
        return;

    string temp = str.substr(2, str.size()-5);
    int length = temp.length();
    int payloadIndex = temp.find("payload");
    string etc = temp.substr(0, payloadIndex-2);
    string payload = temp.substr(payloadIndex+11, length-payloadIndex-12);

    vector<string> tokens, tokens2;
    tokenize(etc, tokens, ",");
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

    if(!payload.empty())
    {
        JDomParser parser;
        JSchemaFragment schema("{}");
        parser.parse(payload, schema);
        pbnjson::JValue payloadJson = parser.getDom();

        for(auto it = payloadJson.children().begin(); it != payloadJson.children().end(); ++it)
        {
            if((*it).second.isString())
            {
                addPayloadMap((*it).first.asString(), (*it).second.asString());
            }
            else if((*it).second.isNumber())
            {
                if(isInteger((*it).second.asNumber<double>()))
                    addPayloadMap((*it).first.asString(), to_string(((*it).second.asNumber<int>())));
                else
                    addPayloadMap((*it).first.asString(), to_string(((*it).second.asNumber<double>())));
            }
            else
            {
                Definition::LOG->LogError("(LttngUserNewNode) new payload? : str(%s), payload(%s)\n", str.c_str(), payload.c_str());
            }
        }
    }
}

string LttngUserNewNode::toString()
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
    temp += ")\n";

    map<string, string>::iterator iter;
    for(iter = m_payloadDataMap.begin(); iter != m_payloadDataMap.end(); iter++)
        temp += iter->first + "(" + iter->second + ") ";
    temp += "\n";

    return temp;
}

