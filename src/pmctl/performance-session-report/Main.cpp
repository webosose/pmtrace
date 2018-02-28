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

#include "PerfSessionReport.h"

void errorHandle(const char *);
void printHelp();

int main(int argc, char **argv)
{
    OptionHandler opt(argc, argv);
    if(!opt.handleOptions())
        errorHandle("Please check options. [Usage : # pmctl session-report -h]\n");

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
    Definition::LOG->LogDebug("(main) session-report options > %s\n", opt.toString().c_str());

    PerfSessionReport sessionReport;
    if(!sessionReport.initialize())
        errorHandle("[ERROR] (PerfSessionReport main) initialize fail\n");

    if(!sessionReport.runParser(opt))
        errorHandle("[ERROR] (PerfSessionReport main) runParser fail\n");

    return 0;
}

void errorHandle(const char * str)
{
    cerr << str << endl;
    exit(-1);
}

void printHelp()
{
    cout << "Usage: pmctl session-report [option]\n\n";
    cout << "options:\n \
       -c, --config <file name>\tCatapult configuration file\n\n \
       -i, --input <file name>\t\tInsert lttng file\n \
       \t\t\t\tDefault input file : /tmp/pmtrace/lttng.txt\n\n \
       -e, --enable <view>\t\tShow selected View(s)\n \
       \t\t\t\t<view> : user,group,cpu\n\n \
       -o, --output <file name>\tOutput result file\n \
       \t\t\t\tDefault output file : /tmp/pmtrace/session-report.json\n\n \
       -p, --process <process name>\tShow selected process(es) (ex. surface-manager,WebAppMgr)\n\n \
       -m, --measure <period (ms)>\tMeasure cpu usage at an interval of period\n \
       \t\t\t\trange : 1 <= period <= 1000\n\n \
       -d, --debug\t\t\tEnable debug messages\n\n \
       -h, --help\t\t\tDisplay this help\n\n";
    cout << "Examples:\n";
    cout << "\tRun session-report (default)\n\t# pmctl session-report\n\n";
    cout << "\tRun session-report (custom configuration)\n\t# pmctl session-report -c session-report-conf.json\n\n";
    cout << "\tRun session-report (output file)\n\t# pmctl session-report -o myresult.json\n\n";
    cout << "\tRun session-report (enable view : ex. user and cpu)\n\t# pmctl session-report -e user,cpu\n\n";
    cout << "\tRun session-report (input file)\n\t# pmctl session-report -i lttng.txt\n\n";
    cout << "\tRun session-report (select process)\n\t# pmctl session-report -p surface-manager,WebAppMgr\n\n";
    cout << "\tRun session-report (measure cpu usage per 10 ms)\n\t# pmctl session-report -e cpu -m 10\n\n";
}

