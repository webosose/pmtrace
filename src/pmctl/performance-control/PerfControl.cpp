// Copyright (c) 2016-2023 LG Electronics, Inc.
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

#include "PerfControl.h"

const string PerfControl::MODULE_PERFLOG_REPORT     = "perflog-report";
const string PerfControl::MODULE_MEMORY_PROFILE     = "memory-profile";

const string PerfControl::COMMAND_PERFLOG_REPORT    = "perf_log_viewer.py";
const string PerfControl::COMMAND_MEMORY_PROFILE    = "mem_profile.py";

PerfControl::PerfControl(int argc, char ** argv)
: m_isDebug(false),
  m_module(argv[1]),
  m_argc(argc)
{
    m_argv = new char*[argc];
    for(int i=0; i<argc; i++)
    {
        m_argv[i] = new char [strlen(argv[i])+1];
        strncpy(m_argv[i], argv[i], strlen(argv[i]));
    }

    for(int i=0; i < m_argc; i++)
    {
        if(strcmp(m_argv[i], "-d") == 0 || strcmp(m_argv[i], "--debug") == 0)
            m_isDebug = true;
    }

    if(m_isDebug)
    {
        for(int i=0; i < m_argc; i++)
            cout << "[DEBUG] (PerfControl) m_argv[" << i << "] : " << m_argv[i] << endl;
    }
}

PerfControl::~PerfControl()
{
    for(int i=0; i<m_argc; i++)
        delete [] m_argv[i];
    delete [] m_argv;
}

bool PerfControl::execModule()
{
    if(m_module == PerfControl::MODULE_PERFLOG_REPORT)
    {
        if(!runModule(PerfControl::COMMAND_PERFLOG_REPORT))
        {
            cerr << "[ERROR] fail to run perflog-report\n";
            return false;
        }
    }
    else if(m_module == PerfControl::MODULE_MEMORY_PROFILE)
    {
        if(!runModule(PerfControl::COMMAND_MEMORY_PROFILE))
        {
            cerr << "[ERROR] fail to run profile\n";
            return false;
        }
    }
    else
    {
        cerr << "[ERROR] wrong module. m_module : " << m_module << "\n";
        return false;
    }

    return true;
}

bool PerfControl::runModule(string cmd)
{
    string command = std::move(cmd);

    for(int i=2; i < m_argc; i++)
    {
        command += " ";
        command += m_argv[i];
    }

    if(m_isDebug)
        cout << "[DEBUG] command : " << command << endl;

    int ret = system(command.c_str());

    if(ret == 127 || ret == -1)
    {
        cerr << "[ERROR] system function \n";
        return false;
    }

    return true;
}

