Performance tools
==========================

Summary
-------
Performance tools for tracing and profiling the userspace stack in the platform

Description
-----------
The pmtrace library provides the methods of tracing and profiling in other software components.

The pmtrace-daemon can manage a Lttng session and export the result in json format
that can be loaded in Catapult tool. Catapult tool can then convert the json file to html
and visualize it graphically on Chrome Browser.

The pmtrace-daemon-control manage the pmtrace-daemon.

How to Build on Linux
=====================

## Dependencies

Below are the tools and libraries (and their minimum versions) required to build

* cmake (version required by build environment)
* gcc 4.8.0
* make (any version)
* pkg-config 0.26
* libpbnjson

## Building

Once you have downloaded the source, enter the following to build it (after
following commands):

    $ mkdir BUILD
    $ cd BUILD
    $ cmake ..
    $ make
    $ sudo make install

To see a list of the make targets that `cmake` has generated, enter:

    $ make help

## Uninstalling

From the directory where you originally ran `make install`, enter:

    $ [sudo] make uninstall

# Copyright and License Information

1) The lttng-ust-mtrace-malloc.c file under src/libmemtracker/liblttng-ust-mtrace-malloc is :

Copyright (C) 2009  Pierre-Marc Fournier
Copyright (C) 2011  Mathieu Desnoyers <mathieu.desnoyers@efficios.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA


2) The ust_mtrace_malloc.h file under src/libmemtracker/liblttng-ust-mtrace-malloc is :

Copyright (C) 2011  Mathieu Desnoyers <mathieu.desnoyers@efficios.com>
Copyright (c) 2016-2018 LG Electronics, Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

3) Unless otherwise specified, all content, including all source code files and
documentation files in this repository are:

Copyright (c) 2016-2018 LG Electronics, Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

SPDX-License-Identifier: Apache-2.0

