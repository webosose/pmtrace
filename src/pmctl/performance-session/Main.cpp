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

#include "PerfSession.h"

void errorHandle(const char *);
void printHelp();

int main(int argc, char **argv)
{
    OptionHandler opt(argc, argv);
    if(!opt.handleOptions())
        errorHandle("Please check options. [Usage : # pmctl session -h]\n");

    if(opt.hasHelp())
    {
        printHelp();
        exit(0);
    }

    Logger * pLog;
    if(opt.hasDebug())
        pLog = new Logger(stdout, LogLevel_Debug);
    else
        pLog = new Logger(stdout, LogLevel_Info);

    Definition::LOG = pLog;
    Definition::LOG->LogDebug("(session main) session options > %s\n", opt.toString().c_str());

    PerfSession session;
    if(!session.initialize())
        errorHandle("[ERROR] (session main) initialize fail\n");

    if(!session.controlSession(opt))
        errorHandle("[ERROR] (session main) controlSession fail\n");

    return 0;
}

void errorHandle(const char * str)
{
    cerr << str << endl;
    exit(-1);
}

void printHelp()
{
    cout << "Usage: pmctl session <command> [option]\n\n";
    cout << "commands:\n \
       start\t\t\t\tStart a lttng session\n \
       stop\t\t\t\tStop a lttng session\n\n";
    cout << "options:\n \
       -c, --config <filename>\t\tSet config file\n \
       -e, --event <target>\t\tSet particular event(s)\n \
       \t\t\t\t<target> : user,cpu (related to config file)\n \
       \t\t\t\tdefault config file : /etc/pmtrace/session-event-conf.json\n \
       \t\t\t\tIf you use '-e' option, all enable setting will be ignored in a config file.\n \
       \t\t\t\tThe information of target event must be decribed in a config file.\n \
       -d, --debug\t\t\tEnable debug messages\n\n \
       -h, --help\t\t\tDisplay this help\n\n";
    cout << "Examples:\n";
    cout << "\tStart session\n";
    cout << "\t# pmctl session start\n\n";
    cout << "\tStart session (set config file)\n";
    cout << "\t# pmctl session -c config.json start\n\n";
    cout << "\tStart session (set cpu event)\n";
    cout << "\t# pmctl session -e cpu start\n\n";
    cout << "\tStart session (set user and cpu events)\n";
    cout << "\t# pmctl session -e user,cpu start\n\n";
    cout << "\tStart session (set config file, set user and cpu events)\n";
    cout << "\t# pmctl session -c config.json -e user,cpu start\n\n";
    cout << "\tStop session\n";
    cout << "\t# pmctl session stop\n\n";
}

