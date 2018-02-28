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

Configuration::Configuration()
:m_eventFile(Definition::SESSION_EVENT_CONF),
 m_hasKernelEvent(false)
{
    Definition::LOG->LogDebug("(Configuration) event conf file[%s]\n"
            , m_eventFile.c_str());

    m_eventConf = pbnjson::Object();
}

bool Configuration::initialize(OptionHandler & opt)
{
    if(opt.hasConfig())
        m_eventFile = opt.getConfig();

    if(!initEvent(opt))
        return false;

    return true;
}

bool Configuration::getLttngEventsInfo(map<string, string> &lttngEventsMap)
{
    for(int i=0; i < m_eventConf["events"].arraySize(); i++)
    {
        pbnjson::JValue cur = m_eventConf["events"][i];
        if(find(m_events.begin(), m_events.end(), cur["name"].asString()) != m_events.end())
        {
            for(int j=0; j < cur["lttngEvents"].arraySize(); j++)
            {
                if(cur["lttngEvents"][j].asString() != "sched_switch")
                {
                    lttngEventsMap.insert(pair<string, string>(cur["lttngEvents"][j].asString(), Definition::LTTNG_EVENT_USER));
                }
                else
                {
                    lttngEventsMap.insert(pair<string, string>(cur["lttngEvents"][j].asString(), Definition::LTTNG_EVENT_KERNEL));
                    m_hasKernelEvent = true;
                }
            }
        }
    }

    return true;
}

bool Configuration::initEvent(OptionHandler & opt)
{
    if(access(m_eventFile.c_str(), R_OK) != 0)
    {
        Definition::LOG->LogError("(Configuration) Wrong session event conf path!\n");
        return false;
    }

    m_eventConf = parseFile(m_eventFile.c_str());
    Definition::LOG->LogDebug("(Configuration) session event conf file : \n%s\n", m_eventConf.stringify().c_str());

    if(!m_eventConf.hasKey("events"))
    {
        Definition::LOG->LogError("(Configuration) no events key\n");
        return false;
    }

    for(int i=0; i < m_eventConf["events"].arraySize(); i++)
    {
        pbnjson::JValue cur = m_eventConf["events"][i];

        if(!cur.hasKey("name"))
        {
            Definition::LOG->LogError("(Configuration) no name key in events\n");
            return false;
        }
        if(!cur.hasKey("enable"))
        {
            Definition::LOG->LogError("(Configuration) no enable key in events\n");
            return false;
        }
        if(!cur.hasKey("lttngEvents"))
        {
            Definition::LOG->LogError("(Configuration) no lttngEvents key in events\n");
            return false;
        }
    }

    if(opt.hasEvent())
    {
        string inputEvent = opt.getEvent();
        Definition::LOG->LogDebug("(Configuration) input event : %s\n", inputEvent.c_str());
        tokenize(inputEvent, m_events, ",");

        for(unsigned int i=0; i < m_events.size(); i++)
        {
            bool isExplained = false;

            for(int j=0; j < m_eventConf["events"].arraySize(); j++)
            {
                if(m_eventConf["events"][j]["name"].asString() == m_events[i])
                    isExplained = true;
            }

            if(!isExplained)
            {
                Definition::LOG->LogError("(Configuration) There is no information of %s event in config file.\n", m_events[i].c_str());
                return false;
            }
        }
    }
    else
    {
        for(int i=0; i < m_eventConf["events"].arraySize(); i++)
        {
            pbnjson::JValue cur = m_eventConf["events"][i];

            if(cur["enable"].asBool())
                m_events.push_back(cur["name"].asString());
        }
    }

    if(opt.hasDebug())
    {
        for(unsigned int i=0; i < m_events.size(); i++)
            Definition::LOG->LogDebug("(Configuration) event[%d] : %s\n", i, m_events[i].c_str());
    }

    return true;
}

