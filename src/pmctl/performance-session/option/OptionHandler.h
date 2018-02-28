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

    bool handleOptions();

    string getCmd() { return m_cmd; }
    string getConfig() { return m_config; }
    string getEvent() { return m_event; }
    string toString() { return string("event : " + m_event); }

    bool hasConfig() { return m_hasConfig; }
    bool hasEvent() { return m_hasEvent; }
    bool hasDebug() { return m_hasDebug; }
    bool hasHelp() { return m_hasHelp; }

private:
    bool setConfig(const char *);
    bool setEvent(const char *);
    bool checkCommand();

private:
    int m_argc;
    char ** m_argv;
    string m_config;
    string m_event;
    string m_cmd;

    bool m_hasConfig;
    bool m_hasEvent;
    bool m_hasDebug;
    bool m_hasHelp;
};

#endif

