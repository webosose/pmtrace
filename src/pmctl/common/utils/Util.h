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

#ifndef _UTIL_H_
#define _UTIL_H_

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <list>
#include <map>
#include <algorithm>
#include <pbnjson.hpp>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
using namespace std;
using namespace pbnjson;

bool isInteger(double val);
string deleteChars(string& str, string target);
int countChar(string& str, char target);
string replaceString(const string& str, const string& pattern, const string& replace);
void tokenize(const string& str, vector<string>& tokens, const string& delimiters);
void parseTabs(const string& str, vector<string>& tokens);
string getDeviceName();
int getCoreNum();
long long getBoottime();
string getFullPath(string target);
pbnjson::JValue parseFile(const char *file);

#endif
