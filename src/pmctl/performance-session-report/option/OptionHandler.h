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

#ifndef _OPTION_HANDLER_
#define _OPTION_HANDLER_

#include <iostream>
#include <getopt.h>
#include <cstring>
#include <unistd.h>
using namespace std;

class OptionHandler
{
public:
    OptionHandler(int, char **);
    ~OptionHandler();

    void checkDebug();
    bool handleOptions();

    string getConfig() { return m_config; }
    string getInput() { return m_input; }
    string getOutput() { return m_output; }
    string getProcess() { return m_process; }
    long getMeasure() { return m_measure; }
    string toString();

    bool enableUserView() { return m_optUserView; }
    bool enableGroupView() { return m_optGroupView; }
    bool enableCpuView() { return m_optCpuView; }

    bool hasHelp() { return m_hasHelp; }
    bool hasDebug() { return m_hasDebug; }
    bool hasConfig() { return m_hasConfig; }
    bool hasInput() { return m_hasInput; }
    bool hasOutput() { return m_hasOutput; }
    bool hasView() { return m_hasView; }
    bool hasProcess() { return m_hasProcess; }
    bool hasMeasure() { return m_hasMeasure; }

private:
    bool setConfig(const char *);
    bool setInput(const char *);
    bool setOutput(const char *);
    bool setView(const char *);
    bool setProcess(const char *);
    bool setMeasure(const char *);

private:
    int m_argc;
    char ** m_argv;
    string m_config;
    string m_input;
    string m_output;
    string m_view;
    string m_process;
    long m_measure;

    bool m_optUserView;
    bool m_optGroupView;
    bool m_optCpuView;

    bool m_hasHelp;
    bool m_hasDebug;
    bool m_hasConfig;
    bool m_hasInput;
    bool m_hasOutput;
    bool m_hasView;
    bool m_hasProcess;
    bool m_hasMeasure;
};

#endif

