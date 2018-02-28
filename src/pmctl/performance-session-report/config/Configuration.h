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

#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

#include "../Definition.h"
#include "utils/Util.h"

typedef struct
{
    string m_groupName;
    string m_process;
    string m_physical;
    string m_logical;
    int m_tid;
} CatapultCustomEvent;

typedef struct
{
    string m_procName;
    string m_physical;
} GroupMetaInfo;

class Configuration
{
public:
    Configuration(string file);

    bool initialize();

    bool isEnabledUserView() { return m_enableUserView; }
    bool isEnabledGroupView() { return m_enableGroupView; }
    bool isEnabledCpuView() { return m_enableCpuView; }
    void setUserView(bool val) { m_enableUserView = val; }
    void setGroupView(bool val) { m_enableGroupView = val; }
    void setCpuView(bool val) { m_enableCpuView = val; }

    int isEnabledGroup(string name);
    bool isValidEvent(string process, string provider);

    bool getCatapultGroupsInfo(map<string, int> &groupMap, int &gid);
    bool getCatapultEventsInfo(vector<CatapultCustomEvent> &catapultEvents);

private:
    string m_configFile;
    pbnjson::JValue m_configObj;

    bool m_enableUserView;
    bool m_enableGroupView;
    bool m_enableCpuView;
    int m_groupTid;
};

#endif

