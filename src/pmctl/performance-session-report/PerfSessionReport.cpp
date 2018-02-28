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

PerfSessionReport::PerfSessionReport()
: m_parser(NULL)
{
    Definition::LOG->LogDebug("(PerfSessionReport) module start\n");
}

PerfSessionReport::~PerfSessionReport()
{
    Definition::LOG->LogDebug("(PerfSessionReport) module exit\n");
    if(m_parser != NULL)
        delete m_parser;
}

bool PerfSessionReport::initialize()
{
    Definition::LOG->LogDebug("(PerfSessionReport) initialize\n");

    if(mkdir(Definition::RAW_DATA_PATH.c_str(), S_IFDIR) != 0 && errno != EEXIST)
    {
        Definition::LOG->LogError("(PerfSessionReport) raw data path is wrong! error msg : %s\n", strerror(errno));
        return false;
    }
    return true;
}

bool PerfSessionReport::runParser(OptionHandler& opt)
{
    Definition::LOG->LogDebug("(PerfSessionReport) runParser function\n");

    m_parser = new LttngParser(opt);

    if(!m_parser->initialize(opt))
    {
        Definition::LOG->LogError("(PerfSessionReport) LttngParser initialize fail\n");
        return false;
    }

    if(!m_parser->runParser())
    {
        Definition::LOG->LogError("(PerfSessionReport) LttngParser runParser fail\n");
        return true;
    }

    return true;
}

