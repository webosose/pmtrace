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

#include "Configuration.h"

Configuration::Configuration(string file)
:m_configFile(file),
 m_enableUserView(false),
 m_enableGroupView(false),
 m_enableCpuView(false),
 m_groupTid(0)
{
    Definition::LOG->LogDebug("(Configuration) session-report conf file : (%s)\n"
            , m_configFile.c_str());

    m_configObj = pbnjson::Object();
}

bool Configuration::initialize()
{
    if(access(m_configFile.c_str(), R_OK) != 0)
    {
        Definition::LOG->LogError("(Configuration) Wrong catapult conf path!\n");
        return false;
    }

    m_configObj = parseFile(m_configFile.c_str());
    Definition::LOG->LogDebug("(Configuration) catapult conf file : %s\n%s\n", m_configFile.c_str(), m_configObj.stringify().c_str());

    if(!m_configObj.hasKey("catapultIgnoreEvents"))
    {
        Definition::LOG->LogError("(Configuration) no catapultIgnoreEvents key\n");
        return false;
    }

    if(!m_configObj.hasKey("catapultUserView"))
    {
        Definition::LOG->LogError("(Configuration) no catapultUserView key\n");
        return false;
    }

    if(!m_configObj["catapultUserView"].hasKey("enable"))
    {
        Definition::LOG->LogError("(Configuration) no enable key in catapultUserView\n");
        return false;
    }

    if(!m_configObj.hasKey("catapultGroupView"))
    {
        Definition::LOG->LogError("(Configuration) no catapultGroupView key\n");
        return false;
    }

    if(!m_configObj["catapultGroupView"].hasKey("enable"))
    {
        Definition::LOG->LogError("(Configuration) no enable key in catapultGroupView\n");
        return false;
    }

    if(!m_configObj["catapultGroupView"].hasKey("groups"))
    {
        Definition::LOG->LogError("(Configuration) no groups key in catapultGroupView\n");
        return false;
    }

    if(!m_configObj.hasKey("catapultCPUView"))
    {
        Definition::LOG->LogError("(Configuration) no catapultCPUView key\n");
        return false;
    }

    if(!m_configObj["catapultCPUView"].hasKey("enable"))
    {
        Definition::LOG->LogError("(Configuration) no enable key in catapultCPUView\n");
        return false;
    }

    m_enableUserView = m_configObj["catapultUserView"]["enable"].asBool();
    m_enableGroupView = m_configObj["catapultGroupView"]["enable"].asBool();
    m_enableCpuView = m_configObj["catapultCPUView"]["enable"].asBool();
    return true;
}

int Configuration::isEnabledGroup(string name)
{
    for(int i=0; i < m_configObj["catapultGroupView"]["groups"].arraySize(); i++)
    {
        pbnjson::JValue cur = m_configObj["catapultGroupView"]["groups"][i];

        if(!cur.hasKey("enable"))
        {
            Definition::LOG->LogError("(Configuration) no enable key in catapultGroupView\n");
            return Definition::RETURN_ERROR;
        }

        if(cur["name"].asString() == name)
        {
            if(cur["enable"].asBool())
                return Definition::RETURN_TRUE;
            else
                return Definition::RETURN_FALSE;
        }
    }
    return Definition::RETURN_FALSE;
}

bool Configuration::isValidEvent(string process, string provider)
{
    bool ret = true;
    bool hasProcess;
    bool hasProvider;

    for(int i=0; i < m_configObj["catapultIgnoreEvents"].arraySize(); i++)
    {
        pbnjson::JValue cur = m_configObj["catapultIgnoreEvents"][i];

        hasProcess = cur.hasKey("process");
        hasProvider = cur.hasKey("provider");

        if(hasProcess)
        {
            if(hasProvider)
            {
                if(cur["process"] == process && cur["provider"] == provider)
                {
                    ret = false;
                    break;
                }
            }
            else
            {
                if(cur["process"] == process)
                {
                    ret = false;
                    break;
                }
            }
        }
        else if(hasProvider)
        {
            if(cur["provider"] == provider)
            {
                ret = false;
                break;
            }
        }
    }

    return ret;
}

bool Configuration::getCatapultGroupsInfo(map<string, int> &groupMap, int &gid)
{
    for(int i=0; i < m_configObj["catapultGroupView"]["groups"].arraySize(); i++)
    {
        pbnjson::JValue cur = m_configObj["catapultGroupView"]["groups"][i];

        if(!cur.hasKey("enable"))
        {
            Definition::LOG->LogError("(Configuration) no enable key in catapultGroupView groups\n");
            return false;
        }

        if(cur["enable"].asBool())
        {
            if(!cur.hasKey("name"))
            {
                Definition::LOG->LogError("(Configuration) no name key in catapultGroupView groups\n");
                return false;
            }
            groupMap.insert(pair<string, int>(cur["name"].asString(), gid--));
        }
    }

    return true;
}

bool Configuration::getCatapultEventsInfo(vector<CatapultCustomEvent> &catapultEvents)
{
    for(int i=0; i < m_configObj["catapultGroupView"]["groups"].arraySize(); i++)
    {
        pbnjson::JValue cur = m_configObj["catapultGroupView"]["groups"][i];

        if(!cur.hasKey("enable"))
        {
            Definition::LOG->LogError("(Configuration) no enable key catapultGroupView\n");
            return false;
        }

        if(!cur.hasKey("name"))
        {
            Definition::LOG->LogError("(Configuration) no name key in catapultGroupView\n");
            return false;
        }

        if(cur["enable"].asBool())
        {
            if(!cur.hasKey("catapultEvents"))
            {
                Definition::LOG->LogError("(Configuration) no catapultEvents key in catapultGroupView\n");
                return false;
            }

            for(int j=0; j < cur["catapultEvents"].arraySize(); j++)
            {
                pbnjson::JValue curEvent = cur["catapultEvents"][j];

                CatapultCustomEvent event;
                event.m_groupName = cur["name"].asString();
                event.m_process = curEvent["process"].asString();
                event.m_physical = curEvent["physical"].asString();
                event.m_logical = curEvent["logical"].asString();
                event.m_tid = m_groupTid++;

                catapultEvents.push_back(event);
            }
        }
    }

    return true;
}

