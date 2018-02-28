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

#include "PerfControl.h"

void printHelp();

int main(int argc, char **argv)
{
    if(argc < 2)
    {
        printHelp();
        exit(-1);
    }

    PerfControl ctl(argc, argv);

    if(!ctl.execModule())
    {
        cerr << "[ERROR] (pmctl main) fail to execute module\n";
        printHelp();
        exit(-1);
    }

    return 0;
}

void printHelp()
{
    cout << "Usage: pmctl <module> [option]\n\n";
    cout << "modules:\n \
        session\t\tControl a performance-session\n \
        session-report\t\tControl a performance-session-report\n \
        perflog-report\t\tControl a performance-log-viewer\n \
        memory-profile\t\tControl a memory-profile\n\n";
    cout << "\nExamples:\n \
        pmctl session -h\n \
        pmctl session-report -h\n \
        pmctl perflog-report -h\n \
        pmctl memory-profile -h\n\n";
}

