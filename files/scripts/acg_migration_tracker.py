#!/usr/bin/env python3
# Copyright (c) 2018 LG Electronics, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0

import argparse
import sys
import os
import json
import pprint
import commands
import re
from StringIO import StringIO

from plat_info import PlatInfoFactory
from commander import CommanderFactory

class ACGMigrationTracker(object):
    SEPARATORS = (',', ': ')
    PERF_TYPE_MIGRATION = 'LS2 ACG Migration'
    PERF_TYPE_SUMMARY = 'LS2 ACG Summary'
    PERF_TYPE_CHECK = 'LS2 ACG Check'

    def __init__(self, base_dir, is_json):
        self._is_json = is_json
        self._base_dir = base_dir
        self._all_services = set()
        self._old_model_services = set()
        self._mixed_model_services = set()
        self._acg_model_services = set()
        self._acg_missing_apis = set()

    def parse_dir(self):
        self._parse_single_dir(os.path.join(self._base_dir, 'luna-service2/roles.d'),
                               self._acg_model_services)
        self._parse_single_dir(os.path.join(self._base_dir, 'luna-service2/services.d'),
                               self._acg_model_services)
        self._parse_single_dir(os.path.join(self._base_dir, 'ls2/roles/prv'),
                               self._old_model_services)
        self._parse_single_dir(os.path.join(self._base_dir, 'ls2/roles/pub'),
                               self._old_model_services)
        self._parse_single_dir(os.path.join(self._base_dir, 'dbus-1/services'),
                               self._old_model_services)
        self._parse_single_dir(os.path.join(self._base_dir, 'dbus-1/system-services'),
                               self._old_model_services)
        self._all_services = self._old_model_services | self._acg_model_services;
        self._mixed_model_services = self._old_model_services & self._acg_model_services;
        self._old_model_services = self._old_model_services - self._mixed_model_services;
        self._acg_model_services = self._acg_model_services - self._mixed_model_services;

    def print_result(self):
        if self._is_json:
            self._print_json_result();
        else:
            self._print_text_result();

    def _add_item(self, perfdata_array, perftype, perfgroup, perfvalue):
        perfdata = {
            'PerfType' : perftype,
            'PerfGroup': perfgroup,
            'PerfValue': perfvalue
        }
        perfdata_array.append(perfdata);

    def _parse_single_dir(self, dirname, services):
        if os.path.isdir(dirname) == False:
            return
        filenames = os.listdir(dirname)
        for filename in filenames:
            name, extension = os.path.splitext(filename);
            tmp = name[-5:];
            if tmp == '.role':
                name = name[:-5];
            services.add(name);

    def _print_text_result(self):
        pprint.pprint("====== AllServices (%d) ======" % len(self._all_services));
        pprint.pprint(self._all_services);
        pprint.pprint("====== MixedModelServices (%d) ======" % len(self._mixed_model_services));
        pprint.pprint(self._mixed_model_services);
        pprint.pprint("====== OldModelServices (%d) ======" % len(self._old_model_services));
        pprint.pprint(self._old_model_services);
        pprint.pprint("====== ACGModelServices (%d) ======" % len(self._acg_model_services));
        pprint.pprint(self._acg_model_services);
        pprint.pprint("====== ACG Missing APIs (%d) ======" % len(self._acg_missing_apis));
        pprint.pprint(self._acg_missing_apis);

    def _print_json_result(self):
        # Don't support remote command yet
        comm = CommanderFactory.makeCommander()
        pinfo = PlatInfoFactory.makePlatInfo(comm)

        payload = {
            'targetDevice': {
                'HWName': pinfo.get_hw_name(),
                'OSName': pinfo.get_os_name(),
                'BuildInfo': pinfo.get_build_info(),
                'CodeName': pinfo.get_code_name(),
                'ModelName': pinfo.get_model_name()
            },
            'data': []
        }

        self._add_item(payload['data'], self.PERF_TYPE_SUMMARY, 'OLD', len(self._old_model_services))
        self._add_item(payload['data'], self.PERF_TYPE_SUMMARY, 'MIXED', len(self._mixed_model_services))
        self._add_item(payload['data'], self.PERF_TYPE_SUMMARY, 'ACG', len(self._acg_model_services))
        self._add_item(payload['data'], self.PERF_TYPE_SUMMARY, 'ACG MISSING APIs', len(self._acg_missing_apis))

        for service in self._old_model_services:
            self._add_item(payload['data'], self.PERF_TYPE_MIGRATION, service, 0)

        for service in self._mixed_model_services:
            self._add_item(payload['data'], self.PERF_TYPE_MIGRATION, service, 1)

        for service in self._acg_model_services:
            self._add_item(payload['data'], self.PERF_TYPE_MIGRATION, service, 2)

        for apis in self._acg_missing_apis:
            self._add_item(payload['data'], self.PERF_TYPE_CHECK, apis, 1)

        print json.dumps(payload, sort_keys=True, indent=4, separators=self.SEPARATORS)

    def get_missing_apis(self):
        cmdlsmonl = 'ls-monitor -l'
        lsmonlresult = commands.getstatusoutput("/usr/sbin/ls-monitor -l | cut -d'/' -f1 | cut -c12-")
        lsmonl = lsmonlresult[1].split("\n",2)[2];
        lsmonlitr = lsmonl.split("\n")
        lsmonlitr = lsmonlitr[:-1]
        countSerName  = 0
        while countSerName < len(lsmonlitr):
            filename = lsmonlitr[countSerName]
            category = filename.split(".")
            if '(null)' not in filename:
                cmd = "/usr/sbin/ls-monitor -i " + filename
                result = commands.getstatusoutput(cmd)
                if result[1]:
                    if "not registered at hub" not in result[1] and "error" not in result[1]:
                        poststr = result[1].split("\n",2)[2];
                        e1 = re.sub(r'\{.*?\}', '', poststr)
                        res1 = e1.split(":")
                        res2 = poststr.split("\n")
                        finalres = []
                        for r in res2:
                            r = r.strip()
                            if r and "\"/\"" not in r:
                                finalres.append(r)
                        categor = ""
                        for r in finalres:
                             if "/" in r:
                                 categor  = r
                                 categor = categor.lstrip('"/')
                                 categor = categor.rstrip('":')
                             else:
                                 tempStr = r
                                 apiStr= re.sub(r'\{.*?\}', '', tempStr).split(":")
                                 apiString = apiStr[0].lstrip('"')
                                 apiString = apiString.rstrip('"')
                                 if categor:
                                     finalCheck  = ''.join([categor.strip(),  "/" , apiString])
                                 else:
                                     finalCheck  = apiString
                                 matchesAccess = re.search(r'\[.*?\]', r)
                                 accessStr  = matchesAccess.group(0) + '\n'
                                 grepStr = ''.join([category[len(category) - 1].strip(), "/" , finalCheck])
                                 cmdcat = "cat /usr/share/luna-service2/api-permissions.d/compat.api.json | grep -w " + grepStr
                                 catresult = commands.getstatusoutput(cmdcat)
                                 if len(catresult[1]) <= 0:
                                     self._acg_missing_apis.add(filename.strip() + "," + finalCheck + "," + accessStr.strip())
            countSerName +=1

def parse_args():
    '''
    Parse program arguments using argparse.
    @param Implicit program arguments via sys.argv
    @return Tuple containing the argument parser and its parsed arguments
    '''
    dft_base_dir      = '/usr/share'
    dft_is_json       = False
    description       = 'Capture ACG Migartion Status'
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument('-d', '--dir',
                        dest='base_dir',
                        action='store',
                        help='Target directory\n(default: %s)' % dft_base_dir,
                        default=dft_base_dir)
    parser.add_argument('-j', '--json',
                        dest='is_json',
                        action='store_true',
                        help='Print JSON format\n(default: %s)' % dft_is_json,
                        default=dft_is_json)
    args = parser.parse_args()
    return (parser, args)

def main():
    ''' Main function '''
    parser, args = parse_args()

    if os.path.isdir(args.base_dir):
        tracker = ACGMigrationTracker(args.base_dir, args.is_json)
        tracker.parse_dir()
        tracker.get_missing_apis()
        tracker.print_result()
        return os.EX_OK;

    parser.print_help()
    return os.EX_USAGE

if __name__ == '__main__':
    sys.exit(main())
