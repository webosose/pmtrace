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
  m_hasHelp(false),
  m_hasDebug(false),
  m_hasConfig(false),
  m_hasInput(false),
  m_hasOutput(false),
  m_hasView(false),
  m_hasProcess(false),
  m_hasMeasure(false)
{
    m_argv = new char*[argc];
    for(int i=0; i<argc; i++)
    {
        m_argv[i] = new char [strlen(argv[i])+1];
        strcpy(m_argv[i], argv[i]);
    }

    checkDebug();

    if(m_hasDebug)
    {
        cout << "[DEBUG] (OptionHandler) argc : " << argc << ", m_argv : " << m_argc << "\n";
        for(int i=0; i<m_argc; i++)
        {
            cout << "[DEBUG] (OptionHandler) argv[" << i << "] : " << argv[i] << ", length : " << strlen(argv[i]) << endl;
            cout << "[DEBUG] (OptionHandler) m_argv[" << i << "] : " << m_argv[i] << ", length : " << strlen(m_argv[i]) << endl;
        }
    }
}

OptionHandler::~OptionHandler()
{
    for(int i=0; i<m_argc; i++)
        delete [] m_argv[i];
    delete [] m_argv;
}

void OptionHandler::checkDebug()
{
    for(int i=0; i < m_argc; i++)
    {
        if(strcmp(m_argv[i], "-d") == 0 || strcmp(m_argv[i], "--debug") == 0)
        {
            m_hasDebug = true;
            break;
        }
    }
}

bool OptionHandler::handleOptions()
{
    int ret;
    int optionIndex = 0;

    static struct option longOptions[] = {
        {"help",    no_argument,       0,  0 },
        {"config",  required_argument, 0,  0 },
        {"enable",  required_argument, 0,  0 },
        {"input",   required_argument, 0,  0 },
        {"output",  required_argument, 0,  0 },
        {"process", required_argument, 0,  0 },
        {"measure", required_argument, 0,  0 },
        {0,         0,                 0,  0 }
    };

    while((ret = getopt_long(m_argc, m_argv, "hc:e:i:o:p:m:d", longOptions, &optionIndex)) != -1)
    {
        switch (ret) {
            case 0:
                switch(optionIndex)
                {
                    case 0: // h
                        m_hasHelp = true;
                        return true;
                    case 1: // c
                        if(!setConfig(optarg))
                            return false;
                        break;
                    case 2: // e
                        if(!setView(optarg))
                            return false;
                        break;
                    case 3: // i
                        if(!setInput(optarg))
                            return false;
                        break;
                    case 4: // o
                        if(!setOutput(optarg))
                            return false;
                        break;
                    case 5: // p
                        if(!setProcess(optarg))
                            return false;
                        break;
                    case 6: // m
                        if(!setMeasure(optarg))
                            return false;
                        break;
                    case 7: // d
                    default:
                        break;
                }
                break;
            case 'h':
                m_hasHelp = true;
                return true;
            case 'c':
                if(!setConfig(optarg))
                    return false;
                break;
            case 'e':
                if(!setView(optarg))
                    return false;
                break;
            case 'i':
                if(!setInput(optarg))
                    return false;
                break;
            case 'o':
                if(!setOutput(optarg))
                    return false;
                break;
            case 'p':
                if(!setProcess(optarg))
                    return false;
                break;
            case 'm':
                if(!setMeasure(optarg))
                    return false;
                break;
            case 'd':
            default:
                break;
        }
    }
    return true;
}

string OptionHandler::toString()
{
    return string("config : " + m_config
        + ", input : " + m_input
        + ", output : " + m_output
        + ", view : " + m_view
        + ", process : " + m_process
        + ", measure(ms) : " + to_string(m_measure));
}

bool OptionHandler::setConfig(const char * str)
{
    if(str == NULL || access(str, R_OK) != 0)
    {
        cout << "[ERROR] Wrong config file path! str : " << str << "\n";
        return false;
    }

    if(m_hasDebug)
        cout << "[DEBUG] (OptionHandler) setConfig parameter : " << str << endl;

    m_config = str;
    m_hasConfig = true;
    return true;
}

bool OptionHandler::setInput(const char * str)
{
    if(str == NULL || access(str, R_OK) != 0)
    {
        cout << "[ERROR] Wrong input file path!\n";
        return false;
    }

    if(m_hasDebug)
        cout << "[DEBUG] (OptionHandler) setInput parameter : " << str << endl;

    m_input = str;
    m_hasInput = true;
    return true;
}

bool OptionHandler::setOutput(const char * str)
{
    if(str == NULL)
    {
        cout << "[ERROR] No argument of '-o' option.\n";
        return false;
    }

    string temp = str;
    FILE * fp = fopen(str, "w");

    if(fp == NULL)
    {
        cout << "[ERROR] Wrong output directory path! (" << str << ")\n";
        return false;
    }

    if(m_hasDebug)
        cout << "[DEBUG] (OptionHandler) setOutput parameter : " << str << endl;

    m_output = temp;
    m_hasOutput = true;
    fclose(fp);
    return true;
}

bool OptionHandler::setView(const char * str)
{
    if(str == NULL)
    {
        cout << "[ERROR] No argument of '-e' option.\n";
        return false;
    }

    if(m_hasDebug)
        cout << "[DEBUG] (OptionHandler) setView parameter : " << str << endl;

    m_view = str;
    m_hasView = true;
    m_optUserView = (strstr(str, "user") != NULL) ? true : false;
    m_optGroupView = (strstr(str, "group") != NULL) ? true : false;
    m_optCpuView = (strstr(str, "cpu") != NULL) ? true : false;

    if(!m_optUserView && !m_optGroupView && !m_optCpuView)
    {
        cout << "[ERROR] Wrong argument of '-e' option. (list : user,group,cpu)\n";
        return false;
    }
    else
        return true;
}

bool OptionHandler::setProcess(const char * str)
{
    if(str == NULL)
    {
        cout << "[ERROR] No argument of '-p' option.\n";
        return false;
    }

    if(m_hasDebug)
        cout << "[DEBUG] (OptionHandler) setProcess parameter : " << str << endl;

    m_process = str;
    m_hasProcess = true;
    return true;
}

bool OptionHandler::setMeasure(const char * str)
{
    if(str == NULL)
    {
        cout << "[ERROR] No argument of '-m' option.\n";
        return false;
    }

    long temp = atol(str);
    if(temp < 1 || 1000 < temp)
    {
        cout << "[ERROR] Invalid measure value. (1 <= value <= 1000) : " << temp << "\n";
        return false;
    }

    if(m_hasDebug)
        cout << "[DEBUG] (OptionHandler) setMeasure parameter : " << str << endl;

    m_measure = temp;
    m_hasMeasure = true;

    return true;
}

