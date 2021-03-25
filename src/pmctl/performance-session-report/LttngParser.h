// Copyright (c) 2016-2021 LG Electronics, Inc.
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

#ifndef _LTTNG_PARSER_H_
#define _LTTNG_PARSER_H_

#include "config/Configuration.h"
#include "option/OptionHandler.h"
#include "ds/LttngUserNode.h"
#include "ds/LttngUserNewNode.h"
#include "ds/LttngKernelNode.h"

typedef struct
{
    int id;
    unsigned long long ts;
} CpuUsage;

class LttngParser
{
public:
    LttngParser(OptionHandler&);
    ~LttngParser();

    bool initialize(OptionHandler&);
    bool runParser();

private:
    bool convertData();
    bool makeUserEvent(vector<string>& tokens, Logger * pLog, FILE * outFile);
    bool makeUserNewEvent(vector<string>& tokens, Logger * pLog, FILE * outFile);
    bool makeKernelEvent(vector<string>& tokens, Logger * pLog, FILE * outFile);

    void printUserList();
    void printKernelList();
    void printMetaMap();

    int getGroupId(string);
    int getGroupTid(string, string, string);
    bool isKernelMsg(string);
    bool isNewPmtraceMsg(string);
    int getSwapperTid(string);
    int getCoreId(string);
    void setCpuUsageInfo(unsigned long long);
    void calculateCpuUsage(FILE *, unsigned long long);
    void countCpuUsage();

    void makeMetaMap(string procname, string pid, string tid);
    bool makeGroupViewInfo();
    bool isGroupMsg(string name, string process, string physical);
    bool isSelectedProcess(string process);
    void handleProcessOpt(string str);

    void makeUserDetail(pbnjson::JValue&, LttngUserNode&);
    void makeUserNewDetail(pbnjson::JValue&, LttngUserNewNode&);
    void makeUserMarkerDetail(pbnjson::JValue&, LttngUserNewNode&);
    void makeKernelDetail(pbnjson::JValue&, LttngKernelNode&);

    void makeJsonStart(FILE *);
    void makeJsonEnd();
    void makeJsonMeta(FILE *);
    void makeJsonAllEvent(FILE *, LttngUserNode &);
    void makeJsonAllNewEvent(FILE *, LttngUserNewNode &);
    void makeJsonGroupEvent(FILE *, LttngUserNode &);
    void makeJsonGroupNewEvent(FILE *, LttngUserNewNode &);
    void makeJsonCpuEvent(FILE *, LttngKernelNode &);

    void makeCpuUsageEvent(FILE *, int, float);
    void makeCpuUsageMaxEvent(FILE *);

private:
    Configuration * m_conf;

    list<LttngUserNode> m_userNodeList;
    list<LttngUserNewNode> m_userNewNodeList;
    list<LttngKernelNode> m_kernelNodeList;

    map<int, string> m_metaPidMap;
    map<int, string> m_metaTidMap;
    map<int, int> m_metaTidPidMap;

    map<string, int> m_catapultGroupMap;
    vector<CatapultCustomEvent> m_catapultGroupEvents;
    vector<GroupMetaInfo> m_groupMetaInfo;

    string m_configFile;
    string m_inputOutput;
    string m_logFileName;
    string m_logFileNameTemp;
    string m_lttngFileName;

    bool m_userView;
    bool m_groupView;
    bool m_cpuView;

    bool m_isDebug;
    bool m_enabledProcessOpt;
    bool m_validBoottime;
    vector<string> m_processOpt;

    map<int, CpuUsage> m_cpuUsageMap;
    int m_coreNum;
    unsigned long long m_bootTime;
    unsigned long long * m_cpuUsageCount;
    unsigned long long m_cpuUsageTargetTs;
    unsigned long long m_cpuUsagePeriod;

    int m_groupViewId;
    unsigned long m_outCount;
};

#endif

