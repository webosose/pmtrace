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

#ifndef _LTTNG_CONTROLLER_H_
#define _LTTNG_CONTROLLER_H_

#include "Definition.h"
#include "utils/Util.h"

class LttngController
{
public:
    LttngController();

    bool isLttngSetup() { return m_setupLttng; }

    void prepareLttng();
    void setupLttng(map<string, string>&, bool);
    void startLttng();
    void stopLttng();
    void viewLttng(string);
    void destroyLttng();

private:
    bool m_setupLttng;
};

#endif
