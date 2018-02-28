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

#include "Util.h"

bool isInteger(double val)
{
    double integer;

    if(modf(val, &integer) == 0)
        return true;
    else
        return false;
}

string deleteChars(string& str, string target)
{
    string::iterator iter;
    int find = 0;

    for(unsigned int i=0; i<target.size(); i++)
    {
        while((find = str.find(target[i])) != string::npos)
        {
            str.erase(find,1);
        }
    }

    return str;
}

int countChar(string& str, char target)
{
    string::iterator iter;
    int cnt = 0;

    for(unsigned int i=0; i < str.size(); i++)
    {
        if(str[i] == target)
            cnt++;
    }

    return cnt;
}

string replaceString(const string& str, const string& pattern, const string& replace)
{
    string temp = str;
    string::size_type pos = 0;
    string::size_type offset = 0;

    while((pos = temp.find(pattern, offset)) != string::npos)
    {
        temp.replace(temp.begin() + pos, temp.begin() + pos + pattern.size(), replace);
        offset = pos + replace.size();
    }

    return temp;
}

void tokenize(const string& str, vector<string>& tokens, const string& delimiters)
{
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    string::size_type pos = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        lastPos = str.find_first_not_of(delimiters, pos);
        pos = str.find_first_of(delimiters, lastPos);
    }
}

void parseTabs(const string& str, vector<string>& tokens)
{
    string buf;
    vector<string> localTokens;
    tokenize(str, localTokens, "\t");

    for(unsigned int i=0; i < localTokens.size(); i++)
    {
        if(i==0)
        {
            stringstream ss(localTokens[i]);

            while(ss >> buf)
                tokens.push_back(buf);
        }
        else
            tokens.push_back(localTokens[i]);
    }
}

string getDeviceName()
{
    FILE *fp;
    char buff[1024];
    string cmd = "nyx-cmd DeviceInfo query --format=json 2>/dev/null";
    vector<string> tokens;
    string temp;
    string devName;

    fp = popen(cmd.c_str(), "r");
    if(fp == NULL)
    {
        cerr << "[ERROR] (UTIL-getDeviceName popen error\n";
        return "";
    }

    while(fgets(buff, sizeof(buff), fp))
    {
        temp = buff;

        if(temp.find("device_name") != string::npos)
        {
            tokenize(temp, tokens, ":");
            devName = tokens[1];
            break;
        }
    }

    pclose(fp);
    deleteChars(devName, "\", \n");

    return devName;
}

int getCoreNum()
{
    FILE *fp;
    char buff[80];
    string cmd = "cat /sys/devices/system/cpu/possible";
    vector<string> tokens;
    string temp;
    string coreNum;

    fp = popen(cmd.c_str(), "r");
    if(fp == NULL)
    {
        cerr << "[ERROR] (UTIL-getCoreNum popen error\n";
        return -1;
    }

    fgets(buff, sizeof(buff), fp);
    temp = buff;

    if(temp.find("-") != string::npos)
    {
        tokenize(temp, tokens, "-");
        coreNum = tokens[1];
    }
    else
    {
        coreNum = temp;
    }

    pclose(fp);

    return atoi(coreNum.c_str()) + 1;
}

long long getBoottime()
{
    FILE *fp;
    char buff[80];
    string cmd = "cat /proc/stat | grep btime | awk '{split($0, split_line, \" \");print split_line[2]}'";

    fp = popen(cmd.c_str(), "r");
    if(fp == NULL)
    {
        cerr << "[ERROR] (UTIL-getBoottime popen error\n";
        return -1;
    }

    int readCnt = fread(buff, sizeof(char), 100, fp);
    if(readCnt < 1)
    {
        cerr << "[ERROR] (UTIL-getBoottime fread error\n";
        pclose(fp);
        return -1;
    }
    buff[readCnt - 1] = '\0';
    pclose(fp);

    return stol(buff);
}

string getFullPath(string target)
{
    FILE *fp;
    char buff[1024];
    string strCommand;
    string path;

    strCommand = "readlink -f ";
    strCommand += target;

    fp = popen(strCommand.c_str(), "r");
    if(fp == NULL)
    {
        cerr << "[ERROR] (UTIL-getFullPath) popen error\n";
        return "";
    }

    fgets(buff, sizeof(buff), fp);
    path = buff;
    if(path.size())
        path.resize(path.size() -1);

    pclose(fp);
    return path;
}

pbnjson::JValue parseFile(const char *file)
{
    return JDomParser::fromFile(file);
}
