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

#include "OptionHandler.h"

OptionHandler::OptionHandler(int argc, char ** argv)
: m_argc(argc),
  m_hasConfig(false),
  m_hasEvent(false),
  m_hasDebug(false),
  m_hasHelp(false)
{
    m_argv = new char*[argc];
    for(int i=0; i<argc; i++)
    {
        m_argv[i] = new char [strlen(argv[i])+1];
        strcpy(m_argv[i], argv[i]);
    }
}

OptionHandler::~OptionHandler()
{
    for(int i=0; i<m_argc; i++)
        delete [] m_argv[i];
    delete [] m_argv;
}

bool OptionHandler::handleOptions()
{
    int ret;
    int optionIndex = 0;

    static struct option longOptions[] = {
        {"config",  required_argument, 0,  0 },
        {"event",   required_argument, 0,  0 },
        {"debug",   no_argument,       0,  0 },
        {"help",    no_argument,       0,  0 },
        {0,         0,                 0,  0 }
    };

    while((ret = getopt_long(m_argc, m_argv, "c:e:dh", longOptions, &optionIndex)) != -1)
    {
        switch (ret) {
            case 0:
                switch(optionIndex)
                {
                    case 0: // c
                        if(!setConfig(optarg))
                            return false;
                        break;
                    case 1: // e
                        if(!setEvent(optarg))
                            return false;
                        break;
                    case 2: // d
                        m_hasDebug = true;
                        break;
                    case 3: // h
                        m_hasHelp = true;
                        return true;
                    default:
                        break;
                }
                break;
            case 'c':
                if(!setConfig(optarg))
                    return false;
                break;
            case 'e':
                if(!setEvent(optarg))
                    return false;
                break;
            case 'd':
                m_hasDebug = true;
                break;
            case 'h':
                m_hasHelp = true;
                return true;
            default:
                break;
        }
    }

    if(!checkCommand())
        return false;

    return true;
}

bool OptionHandler::setConfig(const char * str)
{
    if(str == NULL)
    {
        cerr << "[ERROR] No argument of '-c' option.\n";
        return false;
    }

    m_config = str;
    m_hasConfig = true;

    return true;
}

bool OptionHandler::setEvent(const char * str)
{
    if(str == NULL)
    {
        cerr << "[ERROR] No argument of '-e' option.\n";
        return false;
    }

    m_event = str;
    m_hasEvent = true;

    return true;
}

bool OptionHandler::checkCommand()
{
    bool bStart = false;
    bool bStop = false;

    for(int i=0; i < m_argc; i++)
    {
        if(strcmp(m_argv[i], "start") == 0)
        {
            m_cmd = "start";
            bStart = true;
        }

        if(strcmp(m_argv[i], "stop") == 0)
        {
            m_cmd = "stop";
            bStop = true;
        }
    }

    if(!bStart && !bStop)
    {
        cerr << "no command (start or stop)\n";
        return false;
    }
    else if(bStart && bStop)
    {
        cerr << "wrong command (only one)\n";
        return false;
    }
    else
        return true;
}

