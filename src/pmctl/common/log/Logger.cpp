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

#include "Logger.h"

Logger::Logger()
{
    m_logFp = stdout;
    m_logLevel = LogLevel_Info;
    m_onStopWatch = false;
}

Logger::Logger(FILE * fp, LogLevel log)
{
    m_logFp = fp;
    m_logLevel = log;
    m_onStopWatch = false;
}

Logger::Logger(const char * file, LogLevel log)
{
    m_logFp = fopen(file, "w");
    m_logLevel = log;
    m_onStopWatch = false;
}

Logger::~Logger()
{
    fclose(m_logFp);
}

void Logger::LogInfo(const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    vfprintf(m_logFp, string("[INFO] " + string(format)).c_str(), ap);
    va_end(ap);
}

void Logger::LogDebug(const char * format, ...)
{
    if(m_logLevel != LogLevel_Debug)
        return;

    va_list ap;
    va_start(ap, format);
    vfprintf(m_logFp, string("[DEBUG] " + string(format)).c_str(), ap);
    va_end(ap);
}

void Logger::LogError(const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, string("[ERROR] " + string(format)).c_str(), ap);
    va_end(ap);
}

void Logger::printLogLevel()
{
    switch(m_logLevel)
    {
        case LogLevel_Info:
            LogInfo("Log level : info\n");
            break;
        case LogLevel_Debug:
            LogInfo("Log level : debug\n");
            break;
        case LogLevel_Error:
            LogInfo("Log level : error\n");
            break;
    }
}

void Logger::StopWatch(bool onOff, const char * msg)
{
    static time_t timeStamp;

    if(onOff)
    {
        timeStamp = time(NULL);
        m_onStopWatch = true;
    }
    else if(!onOff && m_onStopWatch)
    {
        m_onStopWatch = false;

        time_t secs = time(NULL) - timeStamp;
        time_t hour, min, sec;
        sec = secs % 60;
        min = secs /60 % 60;
        hour = secs / 3600;
        LogInfo("%s : %02d:%02d:%02d\n", msg, (int)hour, (int)min, (int)sec);
    }
}

