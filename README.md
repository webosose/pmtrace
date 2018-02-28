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

Unless otherwise specified, all content, including all source code files and
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
