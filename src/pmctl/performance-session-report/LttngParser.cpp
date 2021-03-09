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

#include "LttngParser.h"

LttngParser::LttngParser(OptionHandler& opt)
: m_conf(NULL),
  m_configFile(Definition::DEFAULT_CONFIG_FILE),
  m_inputOutput(Definition::DEFAULT_OUTPUT_FILE),
  m_lttngFileName(Definition::DEFAULT_INPUT_FILE),
  m_enabledProcessOpt(false),
  m_coreNum(0),
  m_bootTime(0),
  m_cpuUsageCount(NULL),
  m_cpuUsageTargetTs(0),
  m_cpuUsagePeriod(Definition::DEFAULT_CPU_USAGE_PERIOD),
  m_groupViewId(Definition::GROUP_VIEW_ID_BASE),
  m_outCount(0),
  m_userView(false),
  m_groupView(false),
  m_cpuView(false)
{
    m_isDebug = opt.hasDebug();
    m_validBoottime = !opt.hasInput();

    if(opt.hasConfig())
        m_configFile = opt.getConfig();

    if(opt.hasInput())
        m_lttngFileName = opt.getInput();

    if(opt.hasOutput())
        m_inputOutput = opt.getOutput();

    m_logFileName = Definition::SESSION_REPORT_LOG;
    m_logFileNameTemp = m_logFileName;
    m_logFileNameTemp.append(".temp");

    Definition::LOG->LogDebug("(LttngParser) input(%s), output(%s), config(%s)\n", m_lttngFileName.c_str(), m_inputOutput.c_str(), m_configFile.c_str());
}

LttngParser::~LttngParser()
{
    Definition::LOG->LogDebug("(LttngParser) module exit\n");

    if(m_conf != NULL)
        delete m_conf;
    if(m_cpuUsageCount != NULL)
        delete [] m_cpuUsageCount;
}

bool LttngParser::initialize(OptionHandler& opt)
{
    Definition::LOG->LogDebug("(LttngParser) initialize\n");

    m_conf = new Configuration(m_configFile);

    if(!m_conf->initialize())
    {
        Definition::LOG->LogError("(LttngParser) session-report conf initialize fail\n");
        return false;
    }

    if(opt.hasView()) // enable option
    {
        m_userView = opt.enableUserView();
        m_groupView = opt.enableGroupView();
        m_cpuView = opt.enableCpuView();

        m_conf->setGroupView(m_groupView);
        m_conf->setUserView(m_userView);
        m_conf->setCpuView(m_cpuView);
    }
    else // configuration file
    {
        m_userView = m_conf->isEnabledUserView();
        m_groupView = m_conf->isEnabledGroupView();
        m_cpuView = m_conf->isEnabledCpuView();
    }
    Definition::LOG->LogDebug("(LttngParser) enable view info : user(%d), group(%d), cpu(%d)\n", m_userView, m_groupView, m_cpuView);

    if(!m_userView && !m_groupView && !m_cpuView)
    {
        Definition::LOG->LogError("(LttngParser) no activated view\n");
        return false;
    }

    if(m_cpuView)
    {
        m_coreNum = getCoreNum();
        Definition::LOG->LogDebug("(LttngParser) core num : %d\n", m_coreNum);
        m_cpuUsageCount = new unsigned long long[m_coreNum];
        memset(m_cpuUsageCount, 0, sizeof(unsigned long long)*(m_coreNum));
    }

    if(opt.hasProcess()) // process selection
    {
        m_enabledProcessOpt = true;
        handleProcessOpt(opt.getProcess());
    }

    if(opt.hasMeasure()) // cpu usage period
    {
        Definition::LOG->LogDebug("(LttngParser) input period : %ld ms\n", opt.getMeasure());
        m_cpuUsagePeriod = opt.getMeasure() * 1000;
    }
    Definition::LOG->LogDebug("(LttngParser) period : %ld us\n", m_cpuUsagePeriod);

    m_bootTime = getBoottime();
    Definition::LOG->LogDebug("(LttngParser) boottime : %ld us\n", m_bootTime);

    return true;
}

bool LttngParser::runParser()
{
    Definition::LOG->StopWatch(true, NULL);
    if(!convertData())
    {
        Definition::LOG->LogError("(LttngParser) convert fail (%s)\n", m_lttngFileName.c_str());
        return false;
    }
    Definition::LOG->StopWatch(false, "Converting time");
    return true;
}

bool LttngParser::convertData()
{
    Definition::LOG->LogDebug("(LttngParser) convertData function\n");
    char buff[Definition::BUFF_SIZE];
    vector<string> tokens;
    Definition::LOG->LogInfo("Please wait...\n");

    FILE * outFile = fopen(m_inputOutput.c_str(), "w");
    if(outFile == NULL)
    {
        Definition::LOG->LogError("(LttngParser) fopen error (%s)\n", m_inputOutput.c_str());
        return false;
    }

    FILE * inFile = fopen(m_lttngFileName.c_str(), "r");
    if(inFile == NULL)
    {
        Definition::LOG->LogError("(LttngParser) fopen error (%s)\n", m_logFileName.c_str());
        fclose(outFile);
        return false;
    }

    Logger * pLog;
    if(m_isDebug)
        pLog = new Logger(m_logFileNameTemp.c_str(), LogLevel_Debug);
    else
        pLog = new Logger(m_logFileNameTemp.c_str(), LogLevel_Info);

    if(m_groupView)
        makeGroupViewInfo();

    makeJsonStart(outFile);

    while(fgets(buff, Definition::BUFF_SIZE, inFile))
    {
        string temp = buff;

        if(temp[1] == ' ')
            temp.erase(1,1);

        if(temp.find(Definition::TRACE_TYPE_BIN_INFO) != string::npos
                || temp.find(Definition::TRACE_TYPE_BUILD_ID) != string::npos
                || temp.find(Definition::TRACE_TYPE_DEBUG_LINK) != string ::npos)
            continue;

        parseTabs(temp, tokens);

        if(m_cpuView && isKernelMsg(temp))
        {
            if(!makeKernelEvent(tokens, pLog, outFile))
            {
                Definition::LOG->LogError("(LttngParser) invalid input file.\n");
                return false;
            }
        }
        else
        {
            if(isNewPmtraceMsg(temp))
            {
                if(!makeUserNewEvent(tokens, pLog, outFile))
                {
                    Definition::LOG->LogError("(LttngParser) invalid input file.\n");
                    return false;
                }
            }
            else
            {
                if(!makeUserEvent(tokens, pLog, outFile))
                {
                    Definition::LOG->LogError("(LttngParser) invalid input file.\n");
                    return false;
                }
            }
        }

        tokens.clear();
    }

    makeJsonMeta(outFile);
    fclose(inFile);
    fclose(outFile);
    delete pLog;
    makeJsonEnd();

    Definition::LOG->LogInfo("\nConverting finished!!\nCreate report file : %s\n", m_inputOutput.c_str());

    if(m_isDebug)
    {
        printUserList();
        printKernelList();
        printMetaMap();
    }

    return true;
}

bool LttngParser::makeUserEvent(vector<string>& tokens, Logger * pLog, FILE * outFile)
{
    if(tokens.size() < Definition::MSG_INDEX_MINIMUM_USER)
    {
        Definition::LOG->LogError("(LttngParser) invalid lttng format.\n");
        return false;
    }

    LttngUserNode node;
    string temp;
    string sub;
    int tokenIndex = 0;

    for(unsigned int i=0; i < tokens.size(); i++)
    {
        if(i < Definition::MSG_INDEX_MINIMUM_USER)
        {
            switch(i)
            {
                case Definition::MSG_INDEX_TIMESTAMP:
                    node.setTimeStamp(tokens[i], m_bootTime, m_validBoottime);
                    break;
                case Definition::MSG_INDEX_HOSTNAME:
                    node.setHostName(tokens[i]);
                    break;
                case Definition::MSG_INDEX_PROVIDER_AND_EVENT:
                    node.setProviderAndEvent(tokens[i]);
                    break;
                case Definition::MSG_INDEX_CPU_ID:
                    node.setCpuId(tokens[i]);
                    break;
                case Definition::MSG_INDEX_PROCESS_NAME:
                    node.setProcessName(tokens[i]);
                    break;
                case Definition::MSG_INDEX_PROCESS_ID:
                    node.setProcessId(tokens[i]);
                    break;
                case Definition::MSG_INDEX_THREAD_ID:
                    node.setThreadId(tokens[i]);
                    break;
                default:
                    break;
            }
            pLog->LogDebug("[tok %d] : %s\n", tokenIndex, tokens[i].c_str());
            tokenIndex++;
        }
        else
        {
            temp += tokens[i] + " ";
        }
    }

    node.setPayload(temp);
    pLog->LogDebug("[tok %d] : %s\n[tok cnt : %d, %d]\n\n", tokenIndex, temp.c_str(), tokens.size(), tokenIndex);

    if(m_conf->isValidEvent(node.getProcessName(), node.getProvider()))
    {
        m_userNodeList.push_back(node);
        makeMetaMap(node.getProcessName(), node.getProcessId(), node.getThreadId());
        if(m_userView)
            makeJsonAllEvent(outFile, node);
        if(m_groupView)
            makeJsonGroupEvent(outFile, node);
        cout << '\r' << m_outCount++;
    }
    return true;
}

bool LttngParser::makeUserNewEvent(vector<string>& tokens, Logger * pLog, FILE * outFile)
{
    if(tokens.size() < Definition::MSG_INDEX_MINIMUM_USER)
    {
        Definition::LOG->LogError("(LttngParser) invalid lttng format.\n");
        return false;
    }

    LttngUserNewNode node;
    string temp;
    string sub;
    int tokenIndex = 0;

    for(unsigned int i=0; i < tokens.size(); i++)
    {
        if(i < Definition::MSG_INDEX_MINIMUM_USER)
        {
            switch(i)
            {
                case Definition::MSG_INDEX_TIMESTAMP:
                    node.setTimeStamp(tokens[i], m_bootTime, m_validBoottime);
                    break;
                case Definition::MSG_INDEX_HOSTNAME:
                    node.setHostName(tokens[i]);
                    break;
                case Definition::MSG_INDEX_PROVIDER_AND_EVENT:
                    node.setProviderAndEvent(tokens[i]);
                    break;
                case Definition::MSG_INDEX_CPU_ID:
                    node.setCpuId(tokens[i]);
                    break;
                case Definition::MSG_INDEX_PROCESS_NAME:
                    node.setProcessName(tokens[i]);
                    break;
                case Definition::MSG_INDEX_PROCESS_ID:
                    node.setProcessId(tokens[i]);
                    break;
                case Definition::MSG_INDEX_THREAD_ID:
                    node.setThreadId(tokens[i]);
                    break;
                default:
                    break;
            }
            pLog->LogDebug("[tok %d] : %s\n", tokenIndex, tokens[i].c_str());
            tokenIndex++;
        }
        else
        {
            temp += tokens[i] + " ";
        }
    }

    node.setPayload(temp);
    pLog->LogDebug("[tok %d] : %s\n[tok cnt : %d, %d]\n\n", tokenIndex, temp.c_str(), tokens.size(), tokenIndex);

    if(m_conf->isValidEvent(node.getProcessName(), node.getProvider()))
    {
        m_userNewNodeList.push_back(node);
        makeMetaMap(node.getProcessName(), node.getProcessId(), node.getThreadId());
        if(m_userView)
            makeJsonAllNewEvent(outFile, node);
        if(m_groupView)
            makeJsonGroupNewEvent(outFile, node);
        cout << '\r' << m_outCount++;
    }
    return true;
}

bool LttngParser::makeKernelEvent(vector<string>& tokens, Logger * pLog, FILE * outFile)
{
    if(tokens.size() < Definition::MSG_INDEX_MINIMUM_KERNEL)
    {
        Definition::LOG->LogError("(LttngParser) invalid lttng format.\n");
        return false;
    }

    LttngKernelNode node;
    string temp;
    string sub;
    int tokenIndex = 0;

    for(unsigned int i=0; i < tokens.size(); i++)
    {
        if(i < Definition::MSG_INDEX_MINIMUM_KERNEL)
        {
            switch(i)
            {
                case Definition::MSG_INDEX_TIMESTAMP:
                    node.setTimeStamp(tokens[i], m_bootTime, m_validBoottime);
                    break;
                case Definition::MSG_INDEX_HOSTNAME:
                    node.setHostName(tokens[i]);
                    break;
                case Definition::MSG_INDEX_PROVIDER_AND_EVENT:
                    node.setProvider(tokens[i]);
                    break;
                case Definition::MSG_INDEX_CPU_ID:
                    node.setCpuId(tokens[i]);
                    break;
                case Definition::MSG_INDEX_PROCESS_NAME:
                    node.setProcessName(tokens[i]);
                    break;
                case Definition::MSG_INDEX_PROCESS_ID:
                    node.setThreadId(tokens[i]);
                    node.setProcessId(tokens[i]);
                    break;
                default:
                    break;
            }
            pLog->LogDebug("[tok %d] : %s\n", tokenIndex, tokens[i].c_str());
            tokenIndex++;
        }
        else
        {
            temp += tokens[i] + " ";
        }
    }

    node.setPayload(temp);
    pLog->LogDebug("[tok %d] : %s\n[tok cnt : %d, %d]\n\n", tokenIndex, temp.c_str(), tokens.size(), tokenIndex);

    m_kernelNodeList.push_back(node);
    makeMetaMap(node.getProcessName(), node.getProcessId(), node.getThreadId());
    makeJsonCpuEvent(outFile, node);

    return true;
}

void LttngParser::printUserList()
{
    string temp = m_logFileName;
    temp.append(".user.dat");

    Logger log(temp.c_str(), LogLevel_Info);
    list<LttngUserNode>::iterator itor;

    for(itor = m_userNodeList.begin(); itor != m_userNodeList.end(); itor++)
    {
        log.LogInfo("%s\n\n", itor->toString().c_str());
    }
}

void LttngParser::printKernelList()
{
    string temp = m_logFileName;
    temp.append(".kernel.dat");
    
    Logger log(temp.c_str(), LogLevel_Info);
    list<LttngKernelNode>::iterator itor;

    for(itor = m_kernelNodeList.begin(); itor != m_kernelNodeList.end(); itor++)
    {
        log.LogInfo("%s\n\n", itor->toString().c_str());
    }
}

void LttngParser::printMetaMap()
{
    string temp = m_logFileName;
    temp.append(".meta.pid");
    FILE * outPidMap = fopen(temp.c_str(), "w");

    string temp2 = m_logFileName;
    temp2.append(".meta.tid");
    FILE * outTidMap = fopen(temp2.c_str(), "w");

    string temp3 = m_logFileName;
    temp3.append(".meta.tidpid");
    FILE * outTidPidMap = fopen(temp3.c_str(), "w");

    map<int, string>::iterator iter;
    map<int, int>::iterator iter2;
    for(iter = m_metaPidMap.begin(); iter != m_metaPidMap.end(); iter++)
        fprintf(outPidMap, "pid : %d, procname : %s\n", iter->first, iter->second.c_str());
    for(iter = m_metaTidMap.begin(); iter != m_metaTidMap.end(); iter++)
        fprintf(outTidMap, "tid : %d, procname : %s\n", iter->first, iter->second.c_str());
    for(iter2 = m_metaTidPidMap.begin(); iter2 != m_metaTidPidMap.end(); iter2++)
        fprintf(outTidPidMap, "tid : %d, pid : %d\n", iter2->first, iter2->second);

    fclose(outPidMap);
    fclose(outTidMap);
    fclose(outTidPidMap);
}

int LttngParser::getGroupId(string name)
{
    map<string, int>::iterator iter;

    iter = m_catapultGroupMap.find(name);

    if(iter != m_catapultGroupMap.end())
        return iter->second;
    else
        return Definition::RETURN_ERROR;
}

int LttngParser::getGroupTid(string name, string proc, string physical)
{
    for(unsigned int i=0; i < m_catapultGroupEvents.size(); i++)
    {
        if(m_catapultGroupEvents[i].m_groupName == name &&
           m_catapultGroupEvents[i].m_process == proc &&
           m_catapultGroupEvents[i].m_physical == physical)
            return m_catapultGroupEvents[i].m_tid;
    }
    return -1;
}

bool LttngParser::isKernelMsg(string msg)
{
    if(msg.find("sched_switch") != string::npos)
        return true;
    else
        return false;
}

bool LttngParser::isNewPmtraceMsg(string msg)
{
    if(msg.find("pmtrace") != string::npos
       && msg.find("cat") != string::npos
       && msg.find("payload") != string::npos)
        return true;
    else
        return false;
}

int LttngParser::getSwapperTid(string name)
{
    vector<string> tokens;
    tokenize(name, tokens, "/");
    return Definition::CPU_CORE_BASE_NUMBER - atoi(tokens[1].c_str());
}

int LttngParser::getCoreId(string str)
{
    vector<string> tokens;
    tokenize(str, tokens, "/");
    return atoi(tokens[1].c_str());
}

void LttngParser::setCpuUsageInfo(unsigned long long ts)
{
    m_cpuUsageTargetTs = ts + m_cpuUsagePeriod;
    memset(m_cpuUsageCount, 0, sizeof(unsigned long long)*(m_coreNum));
}

void LttngParser::calculateCpuUsage(FILE * outFile, unsigned long long current)
{
    long temp = 0;
    float value = 0;
    int cpuId = 0;
    unsigned long long ts = 0;
    map<int, CpuUsage>::iterator iter;

    for(iter = m_cpuUsageMap.begin(); iter != m_cpuUsageMap.end(); iter++)
    {
        cpuId = iter->second.id;
        ts = iter->second.ts;
        m_cpuUsageCount[cpuId] += current - ts;
        iter->second.ts = m_cpuUsageTargetTs;
    }

    for(int i=0; i < m_coreNum; i++)
    {
        temp += m_cpuUsageCount[i];
        value = m_cpuUsageCount[i] * 100.0 / m_cpuUsagePeriod;
        // The max value is 100, but sometimes it is more than 100 (ex. 100.21, 102.33 etc) because of sampling rate.
        value = (value > 100) ? 100.0 : value;
        makeCpuUsageEvent(outFile, i, value);
    }
    value = temp * 100.0 / m_cpuUsagePeriod / m_coreNum;
    value = (value > 100) ? 100.0 : value; // same reasen as above
    makeCpuUsageEvent(outFile, m_coreNum, value);
}

bool LttngParser::makeGroupViewInfo()
{
    if(!m_conf->getCatapultGroupsInfo(m_catapultGroupMap, m_groupViewId) || !m_conf->getCatapultEventsInfo(m_catapultGroupEvents))
    {
        Definition::LOG->LogError("(LttngParser) getCatapultEventsInfo fail\n");
        return false;
    }

    string temp = m_logFileName;
    temp.append(".group.info");
    
    Logger * pLog;
    if(m_isDebug)
        pLog = new Logger(temp.c_str(), LogLevel_Debug);
    else
        pLog = new Logger(temp.c_str(), LogLevel_Info);

    map<string, int>::iterator iter;

    for(iter = m_catapultGroupMap.begin(); iter != m_catapultGroupMap.end(); iter++)
    {
        pLog->LogDebug("(LttngParser) groupKey(procname) : %s, groupValue(pid) : %d\n", iter->first.c_str(), iter->second);
    }

    for(unsigned int i=0; i < m_catapultGroupEvents.size(); i++)
    {
        pLog->LogDebug("(LttngParser) Event : tid(%d) name(%s) process(%s) physical(%s) logical(%s)\n", m_catapultGroupEvents[i].m_tid
                , m_catapultGroupEvents[i].m_groupName.c_str()
                , m_catapultGroupEvents[i].m_process.c_str()
                , m_catapultGroupEvents[i].m_physical.c_str()
                , m_catapultGroupEvents[i].m_logical.c_str());
    }

    delete pLog;
    return true;
}

bool LttngParser::isGroupMsg(string name, string process, string physical)
{
    deleteChars(physical, " ");
    for(unsigned int i=0; i < m_catapultGroupEvents.size(); i++)
    {
        if(m_catapultGroupEvents[i].m_groupName == name &&
           m_catapultGroupEvents[i].m_process == process &&
           m_catapultGroupEvents[i].m_physical == physical)
           return true;
    }
    return false;
}

void LttngParser::handleProcessOpt(string str)
{
    tokenize(str, m_processOpt, ",");

    if(m_isDebug)
    {
        for(unsigned int i=0; i < m_processOpt.size(); i++)
            Definition::LOG->LogDebug("(LttngParser) -p data : %s\n", m_processOpt[i].c_str());
    }
}

bool LttngParser::isSelectedProcess(string process)
{
    for(unsigned int i=0; i < m_processOpt.size(); i++)
        if(m_processOpt[i] == process)
            return true;
    return false;
}

void LttngParser::makeUserDetail(pbnjson::JValue& args, LttngUserNode& node)
{
    if(node.getHostName().size() != 0)
        args.put("hostname", node.getHostName());
    if(node.getProvider().size() != 0)
        args.put("provider", node.getProvider());
    if(node.getProcessName().size() != 0)
        args.put("process name", node.getProcessName());
    if(node.getProcessId().size() != 0)
        args.put("pid", node.getProcessId());
    if(node.getThreadId().size() != 0)
        args.put("tid", node.getThreadId());
    if(node.getCpuId().size() != 0)
        args.put("cpu_id", node.getCpuId());
    if(node.getSignal().size() != 0)
        args.put("signal", node.getSignal());

    map<string, string> payload = node.getPayloadMap();
    map<string, string>::iterator iter;

    for(iter = payload.begin(); iter != payload.end(); iter++)
        args.put(iter->first, iter->second);
}

void LttngParser::makeUserNewDetail(pbnjson::JValue& args, LttngUserNewNode& node)
{
    pbnjson::JValue snapshotArgs = pbnjson::Object();
    map<string, string> payload = node.getPayloadMap();
    map<string, string>::iterator iter;

    for(iter = payload.begin(); iter != payload.end(); iter++)
        if(iter->first != "cat" && iter->first != "name")
            args.put(iter->first, iter->second);
}

void LttngParser::makeUserMarkerDetail(pbnjson::JValue& args, LttngUserNewNode& node)
{
    pbnjson::JValue snapshotArgs = pbnjson::Object();
    map<string, string> payload = node.getPayloadMap();
    map<string, string>::iterator iter;

    for(iter = payload.begin(); iter != payload.end(); iter++)
        snapshotArgs.put(iter->first, iter->second);
    args.put("snapshot", snapshotArgs);
}

void LttngParser::makeKernelDetail(pbnjson::JValue& args, LttngKernelNode& node)
{
    if(node.getCpuId().size() != 0)
        args.put("cpu_id", node.getCpuId());
}

void LttngParser::makeJsonStart(FILE * outFile)
{
    fprintf(outFile, "{\"displayTimeUnit\":\"ms\",\"traceEvents\":[\n");
}

void LttngParser::makeJsonEnd()
{
    FILE * outFile = fopen(m_inputOutput.c_str(), "r+");
    fseek(outFile, -2, SEEK_END);
    fprintf(outFile, "]}\n");
    fclose(outFile);
}

void LttngParser::makeMetaMap(string procname, string pid, string tid)
{
    pair<map<int, string>::iterator, bool> ret;
    pair<map<int, int>::iterator, bool> ret2;

    if(procname.find("swapper") == 0)
    {
        vector<string> tokens;
        tokenize(procname, tokens, "/");
        int var = atoi(tokens[1].c_str());
        ret = m_metaTidMap.insert(pair<int, string>(Definition::CPU_CORE_BASE_NUMBER - var, procname));

        if(ret.second != true && m_metaTidMap[Definition::CPU_CORE_BASE_NUMBER - var] != procname)
            m_metaTidMap[Definition::CPU_CORE_BASE_NUMBER - var] = procname;
    }
    else
    {
        ret = m_metaPidMap.insert(pair<int, string>(atoi(pid.c_str()), procname));
        if(ret.second != true && m_metaPidMap[atoi(pid.c_str())] != procname)
            m_metaPidMap[atoi(pid.c_str())] = procname;

        ret = m_metaTidMap.insert(pair<int, string>(atoi(tid.c_str()), procname));
        if(ret.second != true && m_metaTidMap[atoi(tid.c_str())] != procname)
            m_metaTidMap[atoi(tid.c_str())] = procname;

        ret2 = m_metaTidPidMap.insert(pair<int, int>(atoi(tid.c_str()), atoi(pid.c_str())));
        if(ret.second != true && m_metaTidPidMap[atoi(tid.c_str())] != atoi(pid.c_str()))
            m_metaTidPidMap[atoi(tid.c_str())] = atoi(pid.c_str());
    }
}

void LttngParser::makeJsonMeta(FILE * outFile)
{
    map<int, string>::iterator iter;

    if(m_userView)
    {
        // All Process metadata
        for(iter = m_metaPidMap.begin(); iter != m_metaPidMap.end(); iter++)
        {
            pbnjson::JValue psInfo = pbnjson::Object();
            psInfo.put("cat", "metadata");
            psInfo.put("pid", iter->first);
            psInfo.put("ph", "M");
            psInfo.put("name", "process_name");
            pbnjson::JValue psInfoArgs = pbnjson::Object();
            psInfoArgs.put("name", iter->second);
            psInfo.put("args", psInfoArgs);
            fprintf(outFile, "%s,\n", psInfo.stringify().c_str());
        }

        // All Thread metadata
        for(iter = m_metaTidMap.begin(); iter != m_metaTidMap.end(); iter++)
        {
            map<int, int>::iterator iterTid = m_metaTidPidMap.find(iter->first);
            if(iterTid != m_metaTidPidMap.end())
            {
                pbnjson::JValue thInfo = pbnjson::Object();
                thInfo.put("cat", "metadata");
                thInfo.put("pid", iterTid->second);
                thInfo.put("tid", iter->first);
                thInfo.put("ph", "M");
                thInfo.put("name", "thread_name");
                pbnjson::JValue thInfoArgs = pbnjson::Object();
                thInfoArgs.put("name", string(iter->second + "(" + to_string(iter->first) + ")"));
                thInfo.put("args", thInfoArgs);
                fprintf(outFile, "%s,\n", thInfo.stringify().c_str());
            }
        }
    }

    if(m_cpuView)
    {
        // CPU Core View pid metadata
        pbnjson::JValue cvpInfo = pbnjson::Object();
        cvpInfo.put("cat", "metadata");
        cvpInfo.put("pid", Definition::CPU_CORE_VIEW_ID);
        cvpInfo.put("ph", "M");
        cvpInfo.put("name", "process_name");
        pbnjson::JValue cvpInfoArgs = pbnjson::Object();
        string temp = "[CUSTOM] CPU usages/core";
        cvpInfoArgs.put("name", temp);
        cvpInfo.put("args", cvpInfoArgs);
        fprintf(outFile, "%s,\n", cvpInfo.stringify().c_str());

        // CPU Core View tid metadata
        for(int i=0; i < getCoreNum(); i++)
        {
            pbnjson::JValue cvtInfo = pbnjson::Object();
            cvtInfo.put("cat", "metadata");
            cvtInfo.put("pid", Definition::CPU_CORE_VIEW_ID);
            cvtInfo.put("tid", i);
            cvtInfo.put("ph", "M");
            cvtInfo.put("name", "thread_name");
            pbnjson::JValue cvtInfoArgs = pbnjson::Object();
            temp = "Core #" + to_string(i);
            cvtInfoArgs.put("name", temp);
            cvtInfo.put("args", cvtInfoArgs);
            fprintf(outFile, "%s,\n", cvtInfo.stringify().c_str());
        }

        // CPU Core View Total tid metadata
        pbnjson::JValue cvttInfo = pbnjson::Object();
        cvttInfo.put("cat", "metadata");
        cvttInfo.put("pid", Definition::CPU_CORE_VIEW_ID);
        cvttInfo.put("tid", m_coreNum);
        cvttInfo.put("ph", "M");
        cvttInfo.put("name", "thread_name");
        pbnjson::JValue cvttInfoArgs = pbnjson::Object();
        cvttInfoArgs.put("name", "Total");
        cvttInfo.put("args", cvttInfoArgs);
        fprintf(outFile, "%s,\n", cvttInfo.stringify().c_str());

        // CPU Core Usages Reference value
        makeCpuUsageMaxEvent(outFile);

        // CPU Process metadata
        pbnjson::JValue pvpInfo = pbnjson::Object();
        pvpInfo.put("cat", "metadata");
        pvpInfo.put("pid", Definition::CPU_PROCESS_VIEW_ID);
        pvpInfo.put("ph", "M");
        pvpInfo.put("name", "process_name");
        pbnjson::JValue pvpInfoArgs = pbnjson::Object();
        pvpInfoArgs.put("name", "[CUSTOM] CPU usages/ps");
        pvpInfo.put("args", pvpInfoArgs);
        fprintf(outFile, "%s,\n", pvpInfo.stringify().c_str());

        // CPU Thread metadata
        for(iter = m_metaTidMap.begin(); iter != m_metaTidMap.end(); iter++)
        {
            map<int, int>::iterator iterTid = m_metaTidPidMap.find(iter->first);
            if(iterTid != m_metaTidPidMap.end())
            {
                pbnjson::JValue thInfo = pbnjson::Object();
                thInfo.put("cat", "metadata");
                thInfo.put("pid", Definition::CPU_PROCESS_VIEW_ID);
                thInfo.put("tid", iter->first);
                thInfo.put("ph", "M");
                thInfo.put("name", "thread_name");
                pbnjson::JValue thInfoArgs = pbnjson::Object();
                thInfoArgs.put("name", string(iter->second + "(" + to_string(iter->first) + ")"));
                thInfo.put("args", thInfoArgs);
                fprintf(outFile, "%s,\n", thInfo.stringify().c_str());
            }
        }
    }

    if(m_groupView)
    {
        list<LttngUserNode>::iterator itor;
        map<string, int>::iterator iter;

        // Group Process metadata (group name)
        for(iter = m_catapultGroupMap.begin(); iter != m_catapultGroupMap.end(); iter++)
        {
            pbnjson::JValue cgInfo = pbnjson::Object();
            cgInfo.put("cat", "metadata");
            cgInfo.put("pid", iter->second);
            cgInfo.put("ph", "M");
            cgInfo.put("name", "process_name");
            pbnjson::JValue cgInfoArgs = pbnjson::Object();
            string temp = "[CUSTOM] ";
            temp += iter->first;
            cgInfoArgs.put("name", temp);
            cgInfo.put("args", cgInfoArgs);
            fprintf(outFile, "%s,\n", cgInfo.stringify().c_str());
        }

        // Group Thread Metadata (logical name)
        for(unsigned int i=0; i < m_catapultGroupEvents.size(); i++)
        {
            for(unsigned int j=0; j < m_groupMetaInfo.size(); j++)
            {
                string physical = m_groupMetaInfo[j].m_physical;
                deleteChars(physical, " ");
                if(m_groupMetaInfo[j].m_procName == m_catapultGroupEvents[i].m_process && physical == m_catapultGroupEvents[i].m_physical)
                {
                    pbnjson::JValue sigInfo = pbnjson::Object();
                    sigInfo.put("cat", "metadata");
                    sigInfo.put("pid", getGroupId(m_catapultGroupEvents[i].m_groupName));
                    sigInfo.put("tid", m_catapultGroupEvents[i].m_tid);
                    sigInfo.put("ph", "M");
                    sigInfo.put("name", "thread_name");
                    pbnjson::JValue sigInfoArgs = pbnjson::Object();
                    sigInfoArgs.put("name", m_catapultGroupEvents[i].m_logical);
                    sigInfo.put("args", sigInfoArgs);
                    fprintf(outFile, "%s,\n", sigInfo.stringify().c_str());
                    break;
                }
            }
        }
    }
}

void LttngParser::makeJsonAllEvent(FILE * outFile, LttngUserNode& node)
{
    if(m_enabledProcessOpt && !isSelectedProcess(node.getProcessName()))
        return;

    pbnjson::JValue event = pbnjson::Object();
    event.put("cat", "all");
    event.put("pid", node.getProcessId());
    event.put("tid", node.getThreadId());
    event.put("ts", node.getTimeStamp());
    event.put("name", node.getSignal());

    if(node.getEventType() == Definition::TRACE_TYPE_SOINFO)
        event.put("ph", "I");
    else if(node.getEventType() == Definition::TRACE_TYPE_START
            || node.getEventType() == Definition::TRACE_TYPE_END)
        return;
    else if(node.getEventType() == Definition::TRACE_TYPE_TOUCHCOORDINATE)
        event.put("ph", "I");
    else if(node.getEventType() == Definition::TRACE_TYPE_MESSAGE)
        event.put("ph", "I");
    else if(node.getEventType() == Definition::TRACE_TYPE_KEYVALUE)
        event.put("ph", "I");
    else if(node.getEventType() == Definition::TRACE_TYPE_BEFORE)
        event.put("ph", "B");
    else if(node.getEventType() == Definition::TRACE_TYPE_AFTER)
        event.put("ph", "E");
    else if(node.getEventType() == Definition::TRACE_TYPE_SCOPE_ENTRY)
        event.put("ph", "B");
    else if(node.getEventType() == Definition::TRACE_TYPE_SCOPE_EXIT)
        event.put("ph", "E");
    else if(node.getEventType() == Definition::TRACE_TYPE_FUNCTION_ENTRY)
        event.put("ph", "B");
    else if(node.getEventType() == Definition::TRACE_TYPE_FUNCTION_EXIT)
        event.put("ph", "E");
    else if(node.getEventType() == Definition::TRACE_TYPE_IM_FUNCTION_ENTRY)
        event.put("ph", "B");
    else if(node.getEventType() == Definition::TRACE_TYPE_IM_FUNCTION_EXIT)
        event.put("ph", "E");
    else
    {
        Definition::LOG->LogDebug("(LttngParser) all - New event type : %s\n", node.getEventType().c_str());
        return;
    }

    pbnjson::JValue eventArgs = pbnjson::Object();
    makeUserDetail(eventArgs, node);
    event.put("args", eventArgs);

    fprintf(outFile, "%s,\n", event.stringify().c_str());
}

void LttngParser::makeJsonAllNewEvent(FILE * outFile, LttngUserNewNode& node)
{
    if(m_enabledProcessOpt && !isSelectedProcess(node.getProcessName()))
        return;

    if(node.getEventType() == Definition::TRACE_TYPE_MARKER)
    {
        pbnjson::JValue eventN = pbnjson::Object();
        eventN.put("cat", node.getPayload("cat"));
        eventN.put("pid", node.getProcessId());
        eventN.put("tid", node.getThreadId());
        eventN.put("ts", node.getTimeStamp());
        eventN.put("ph", "N");
        eventN.put("id", node.getThreadId());
        eventN.put("name", node.getPayload("name"));

        pbnjson::JValue eventO = pbnjson::Object();
        eventO.put("cat", node.getPayload("cat"));
        eventO.put("pid", node.getProcessId());
        eventO.put("tid", node.getThreadId());
        eventO.put("ts", node.getTimeStamp());
        eventO.put("ph", "O");
        eventO.put("id", node.getThreadId());
        eventO.put("name", node.getPayload("name"));
        pbnjson::JValue eventArgs = pbnjson::Object();
        makeUserMarkerDetail(eventArgs, node);
        eventO.put("args", eventArgs);

        pbnjson::JValue eventD = pbnjson::Object();
        eventD.put("cat", node.getPayload("cat"));
        eventD.put("pid", node.getProcessId());
        eventD.put("tid", node.getThreadId());
        eventD.put("ts", node.getTimeStamp());
        eventD.put("ph", "D");
        eventD.put("id", node.getThreadId());
        eventD.put("name", node.getPayload("name"));

        pbnjson::JValue eventLine = pbnjson::Object();
        eventLine.put("cat", node.getPayload("cat"));
        eventLine.put("pid", node.getProcessId());
        eventLine.put("tid", node.getThreadId());
        eventLine.put("ts", node.getTimeStamp());
        eventLine.put("ph", "I");
        eventLine.put("s", "g");
        eventLine.put("name", node.getPayload("name"));

        fprintf(outFile, "%s,\n%s,\n%s,\n%s,\n", eventN.stringify().c_str()
                , eventO.stringify().c_str()
                , eventD.stringify().c_str()
                , eventLine.stringify().c_str());
    }
    else
    {
        pbnjson::JValue event = pbnjson::Object();
        event.put("cat", node.getPayload("cat"));
        event.put("pid", node.getProcessId());
        event.put("tid", node.getThreadId());
        event.put("ts", node.getTimeStamp());
        
        if(node.getEventType() == Definition::TRACE_TYPE_BLOCK_ENTRY)
        {
            event.put("ph", "B");
            event.put("name", node.getPayload("name"));
        }
        else if(node.getEventType() == Definition::TRACE_TYPE_BLOCK_EXIT)
        {
            event.put("ph", "E");
            event.put("name", node.getPayload("name"));
        }
        else if(node.getEventType() == Definition::TRACE_TYPE_LOG)
        {
            event.put("ph", "I");
            event.put("name", node.getPayload("cat"));
        }
        else
        {
            Definition::LOG->LogDebug("(LttngParser) new pmtrace - New event type : %s\n", node.getEventType().c_str());
            return;
        }

        pbnjson::JValue eventArgs = pbnjson::Object();
        makeUserNewDetail(eventArgs, node);
        event.put("args", eventArgs);

        fprintf(outFile, "%s,\n", event.stringify().c_str());
    }
}

void LttngParser::makeJsonGroupEvent(FILE * outFile, LttngUserNode& node)
{
    if(m_enabledProcessOpt && !isSelectedProcess(node.getProcessName()))
        return;

    map<string, int>::iterator iter;

    for(iter = m_catapultGroupMap.begin(); iter != m_catapultGroupMap.end(); iter++)
    {
        if(isGroupMsg(iter->first, node.getProcessName(), node.getSignal()))
        {
            pbnjson::JValue event = pbnjson::Object();
            event.put("cat", "group");
            event.put("pid", iter->second);
            event.put("tid", getGroupTid(iter->first, node.getProcessName(), node.getSignal()));
            event.put("ts", node.getTimeStamp());

            if(node.getEventType() == Definition::TRACE_TYPE_SOINFO)
                event.put("ph", "I");
            else if(node.getEventType() == Definition::TRACE_TYPE_TOUCHCOORDINATE)
                event.put("ph", "I");
            else if(node.getEventType() == Definition::TRACE_TYPE_MESSAGE)
                event.put("ph", "I");
            else if(node.getEventType() == Definition::TRACE_TYPE_KEYVALUE)
                event.put("ph", "I");
            else if(node.getEventType() == Definition::TRACE_TYPE_BEFORE)
                event.put("ph", "B");
            else if(node.getEventType() == Definition::TRACE_TYPE_AFTER)
                event.put("ph", "E");
            else if(node.getEventType() == Definition::TRACE_TYPE_SCOPE_ENTRY)
                event.put("ph", "B");
            else if(node.getEventType() == Definition::TRACE_TYPE_SCOPE_EXIT)
                event.put("ph", "E");
            else if(node.getEventType() == Definition::TRACE_TYPE_FUNCTION_ENTRY)
                event.put("ph", "B");
            else if(node.getEventType() == Definition::TRACE_TYPE_FUNCTION_EXIT)
                event.put("ph", "E");
            else if(node.getEventType() == Definition::TRACE_TYPE_IM_FUNCTION_ENTRY)
                event.put("ph", "B");
            else if(node.getEventType() == Definition::TRACE_TYPE_IM_FUNCTION_EXIT)
                event.put("ph", "E");
            else
            {
                Definition::LOG->LogDebug("(LttngParser) groupView - new event type : %s\n", node.getEventType().c_str());
                continue;
            }

            event.put("name", node.getSignal());
            pbnjson::JValue eventArgs = pbnjson::Object();
            makeUserDetail(eventArgs, node);
            event.put("args", eventArgs);
            fprintf(outFile, "%s,\n", event.stringify().c_str());

            GroupMetaInfo metaInfo;
            metaInfo.m_procName = node.getProcessName();
            metaInfo.m_physical = node.getSignal();
            m_groupMetaInfo.push_back(metaInfo);
        }
    }
}

void LttngParser::makeJsonGroupNewEvent(FILE * outFile, LttngUserNewNode& node)
{
    if(m_enabledProcessOpt && !isSelectedProcess(node.getProcessName()))
        return;

    map<string, int>::iterator iter;

    for(iter = m_catapultGroupMap.begin(); iter != m_catapultGroupMap.end(); iter++)
    {
        if(isGroupMsg(iter->first, node.getProcessName(), node.getPayload("name")))
        {
            pbnjson::JValue event = pbnjson::Object();
            event.put("cat", "group");
            event.put("pid", iter->second);
            event.put("tid", getGroupTid(iter->first, node.getProcessName(), node.getPayload("name")));
            event.put("ts", node.getTimeStamp());

            if(node.getEventType() == Definition::TRACE_TYPE_BLOCK_ENTRY)
            {
                event.put("ph", "B");
                event.put("name", node.getPayload("name"));
            }
            else if(node.getEventType() == Definition::TRACE_TYPE_BLOCK_EXIT)
            {
                event.put("ph", "E");
                event.put("name", node.getPayload("name"));
            }
            else if(node.getEventType() == Definition::TRACE_TYPE_LOG)
                continue;
            else if(node.getEventType() == Definition::TRACE_TYPE_MARKER)
                continue;
            else
            {
                Definition::LOG->LogDebug("(LttngParser) groupView - new event type : %s\n", node.getEventType().c_str());
                continue;
            }

            pbnjson::JValue eventArgs = pbnjson::Object();
            makeUserNewDetail(eventArgs, node);
            event.put("args", eventArgs);
            fprintf(outFile, "%s,\n", event.stringify().c_str());

            GroupMetaInfo metaInfo;
            metaInfo.m_procName = node.getProcessName();
            metaInfo.m_physical = node.getPayload("name");
            m_groupMetaInfo.push_back(metaInfo);
        }
    }
}

void LttngParser::makeJsonCpuEvent(FILE * outFile, LttngKernelNode& node)
{
    if(node.getProvider() != Definition::TRACE_KERNEL_SCHED_SWITCH)
        return;

    if(m_cpuUsageTargetTs == 0)
        setCpuUsageInfo(node.getTimeStampl());

    bool endSwapper = false;
    bool startSwapper = false;

    // CPU Process view
    pbnjson::JValue cpuProcessEnd = pbnjson::Object();
    cpuProcessEnd.put("cat", "cpu");
    cpuProcessEnd.put("pid", Definition::CPU_PROCESS_VIEW_ID);
    cpuProcessEnd.put("ts", node.getTimeStamp());
    string prevName = node.getProcessName();
    if(prevName.find("swapper") == 0)
    {
        cpuProcessEnd.put("tid", getSwapperTid(prevName));
        endSwapper = true;
    }
    else
    {
        cpuProcessEnd.put("tid", (atoi(node.getThreadId().c_str())));
    }
    cpuProcessEnd.put("name", prevName);
    cpuProcessEnd.put("ph", "E");
    pbnjson::JValue cpuProcessEndArgs = pbnjson::Object();
    makeKernelDetail(cpuProcessEndArgs, node);
    cpuProcessEnd.put("args", cpuProcessEndArgs);
    if(!endSwapper)
    {
        if(m_enabledProcessOpt)
        {
            if(isSelectedProcess(prevName))
                fprintf(outFile, "%s,\n", cpuProcessEnd.stringify().c_str());
        }
        else
            fprintf(outFile, "%s,\n", cpuProcessEnd.stringify().c_str());

        // CPU usage
        map<int, CpuUsage>::iterator iter;
        iter = m_cpuUsageMap.find(atoi(node.getThreadId().c_str()));
        if(iter != m_cpuUsageMap.end())
        {
            m_cpuUsageCount[iter->second.id] += (node.getTimeStampl() - iter->second.ts);
            m_cpuUsageMap.erase(iter);
        }
    }

    pbnjson::JValue cpuProcessStart = pbnjson::Object();
    cpuProcessStart.put("cat", "cpu");
    cpuProcessStart.put("pid", Definition::CPU_PROCESS_VIEW_ID);
    cpuProcessStart.put("ts", node.getTimeStamp());
    string nextName = node.getPayload("next_comm");
    if(nextName.find("swapper") == 0)
    {
        cpuProcessStart.put("tid", getSwapperTid(nextName));
        startSwapper = true;
    }
    else
        cpuProcessStart.put("tid", (atoi(node.getPayload("next_tid").c_str())));
    cpuProcessStart.put("name", nextName);
    cpuProcessStart.put("ph", "B");
    pbnjson::JValue cpuProcessStartArgs = pbnjson::Object();
    makeKernelDetail(cpuProcessStartArgs, node);
    cpuProcessStart.put("args", cpuProcessStartArgs);
    if(!startSwapper)
    {
        if(m_enabledProcessOpt)
        {
            if(isSelectedProcess(nextName))
                fprintf(outFile, "%s,\n", cpuProcessStart.stringify().c_str());
        }
        else
            fprintf(outFile, "%s,\n", cpuProcessStart.stringify().c_str());

        //CPU usage
        CpuUsage cuValue = {node.getCpuIdi(), node.getTimeStampl()};
        m_cpuUsageMap.insert(pair<int, CpuUsage>(atoi(node.getPayload("next_tid").c_str()), cuValue));
    }

    if(m_cpuUsageTargetTs <= node.getTimeStampl())
    {
        calculateCpuUsage(outFile, node.getTimeStampl());
        setCpuUsageInfo(node.getTimeStampl());
    }
}

void LttngParser::makeCpuUsageEvent(FILE * outFile, int core, float value)
{
    pbnjson::JValue cpuUsage = pbnjson::Object();
    cpuUsage.put("cat", "usage");
    cpuUsage.put("pid", Definition::CPU_CORE_VIEW_ID);
    cpuUsage.put("tid", core);
    cpuUsage.put("ts", to_string(m_cpuUsageTargetTs));
    if(core == m_coreNum)
        cpuUsage.put("name", "Total");
    else
        cpuUsage.put("name", string("Core #" + to_string(core)));
    cpuUsage.put("ph", "C");
    pbnjson::JValue usageVal = pbnjson::Object();
    usageVal.put("value", value);
    cpuUsage.put("args", usageVal);
    fprintf(outFile, "%s,\n", cpuUsage.stringify().c_str());
}

void LttngParser::makeCpuUsageMaxEvent(FILE * outFile)
{
    for(int i=0; i < m_coreNum; i++)
        makeCpuUsageEvent(outFile, i, Definition::CPU_USAGE_CORE_MAX);

    makeCpuUsageEvent(outFile, m_coreNum, Definition::CPU_USAGE_TOTAL_MAX);
}

