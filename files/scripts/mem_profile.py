#!/usr/bin/python
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

from plat_info import PlatInfoFactory
from commander import CommanderFactory
import os, sys, json, collections, argparse, datetime, subprocess

class MemoryProfiler(object):
    DFT_SNAPSHOT_EXT   = '.tar.gz'
    SEPARATORS = (',', ': ')
    def __init__(self, folder_to_analyze, perf_type, perf_group, perf_value):
        '''
        Class that analyzes a folder and generates APS output
        from multiple memory snapshots
        @param folder_to_analyze Folder containing the testcases data
        @param PerfType string to appear in perfmeta.json
        @param PerfGroup string to appear in perfmeta.json
        @param PerfValue string to appear in perfmeta.json
        '''
        self._folder = folder_to_analyze
        self._pt = perf_type
        self._pg = perf_group
        self._pv = perf_value
        print 'folder(%s) pt(%s) pg(%s) pv(%s)' \
            % (self._folder, self._pt, self._pg, self._pv)
        self._snapshot_exts = (MemoryProfiler.DFT_SNAPSHOT_EXT, '.tgz', '.tar', '.gz')
        # List of tuples with testcase information and memory map
        self._aps_lst = []
        # List of aps files generated
        self._aps_files = []
        # Dict containing information about all test cases
        self._all_tc_map = collections.OrderedDict()

    @staticmethod
    def capture_snapshot(comm, base_folder, test_case, scenario):
        '''
        Capture a memory snapshot and put it under specific folder structure
        and file name as:
            <base_folder>/<test_case>/<timestamp>_<scenario>.tar.gz
        @param base_folder Folder containing a subfolder for each test case
        @param test_case Folder containing snapshots captured for test case
        @param scenario Name indicating scenario when snapshot was captured
        @return String that will be passed to sys.exit() and be printed,
                otherwise os.EX_OK indicating success
        '''
        base_utils = ('smemcap', 'gzip')
        for u in base_utils:
            try:
                out = comm.exec_command(['which', u, '>', '/dev/null', '2>&1'], shell=True)
            except subprocess.CalledProcessError as e:
                if e.returncode != 255:
                    return 'Error %d: %s is not in PATH environment variable' \
                            % (e.returncode, u)
                raise e
        full_path = os.path.join(base_folder, test_case)
        try:
            os.makedirs(full_path)
        except OSError as e:
            if e.errno == 17:
                # Folder already exists
                pass
            else:
                # Failed to create folder
                return 'Exception: ' + str(e)
        cmd = ['mount', '-t', 'debugfs', 'none', '/sys/kernel/debug', '>', '/dev/null', '2>&1']
        try:
            comm.exec_command(cmd, shell=True)
        except subprocess.CalledProcessError as e:
            # Ignore error code 32 meaning it's already mounted
            if e.returncode != 32:
                return 'Error %d: Failed to mount debugfs with command %r' \
                        % (e.returncode, " ".join(cmd))
        ts = datetime.datetime.now().strftime('%Y%m%d%H%M%S')
        file_name = ts + '_' + scenario + MemoryProfiler.DFT_SNAPSHOT_EXT
        dest_file = os.path.join(full_path, file_name)
        cmd = [base_utils[0], '|', base_utils[1], '-']
        try:
            out = comm.exec_command(cmd, shell=True)
        except subprocess.CalledProcessError as e:
            if e.returncode != 255:
                return 'Error %d: Failed to create snapshot: %r' \
                        % (e.returncode, cmd)
            raise e
        with open(dest_file, 'w') as f:
            f.write(out)
        print 'Created snapshot: %s' % dest_file
        return os.EX_OK

    @staticmethod
    def remove_ext(path, exts = []):
        '''
        Remove extension from a path from a list of extensions
        @param path Path to remove its extension
        @param exts List containing the extensions to search and remove
        @return Path with extension removed
        '''
        for ext in exts:
            if path.endswith(ext):
                path = path[:-len(ext)]
                break
        return path

    def get_info_from_path(self, path):
        '''
        Parses a file path and extract metadata about memory snapshot
        @param path Path to extract test case name, timestamp and scenario
        @return Tuple containing test case name, timestamp and scenario
        '''
        rest, f = os.path.split(path)
        testcase = os.path.split(rest)[-1]
        f = self.remove_ext(f, self._snapshot_exts)
        timestamp, scenario = f.split("_")
        return (testcase, timestamp, scenario)

    def _run_smem(self, fpath, unit):
        # Expects smem.arm to be in same folder as mem_profile.py
        smem_path = os.path.join(os.path.dirname(__file__), './smem.arm')
        cmd = 'python2 %s -S %s -t %s --export aps >/dev/null 2>&1' \
              % (smem_path, fpath, unit)
        try:
            # Try processing memory snapshot asynchronously
            from multiprocessing import Process
            p = Process(target=os.system, args=(cmd,))
            p.start()
            return p
        except ImportError as e:
            # Process memory snapshot synchronously
            os.system(cmd)
            return None

    def _load_aps_files(self):
        print '\tLoading %d APS file(s):' % len(self._aps_files)
        for aps_tuple in self._aps_files:
            process, aps_path, test, timestamp, scenario = aps_tuple
            if process:
                # Wait until APS file is generated if needed
                process.join()
            print '\t\ttest(%s) timestamp(%s) scenario(%s) aps_path(%s)' \
                % (test, timestamp, scenario, aps_path)
            try:
                with open(aps_path, 'r') as aps_file:
                    aps_map = json.load(aps_file)
                    ttuple = ((test, timestamp, scenario), aps_map)
                    self._aps_lst.append(ttuple)
            except IOError as e:
                print '\t\tFile not found: ' + str(e.message)

    def _analyze_sorted_files(self, dpath, unit_kb=True):
        if not os.path.exists(dpath):
            raise Exception('%s folder does not exist! ' \
                            'Did you run \'capture\' command before?' % dpath)
        print '\tScanning: %s' % dpath
        for f in sorted(os.listdir(dpath)):
            fpath = os.path.join(dpath, f)
            if not os.path.isfile(fpath) \
                or not fpath.endswith(self._snapshot_exts):
                print '\t\tIgnoring unknown file: ' + fpath
                continue
            print '\t\t%s' % fpath
            test, timestamp, scenario = self.get_info_from_path(fpath)
            # Process snapshots and generate final merged output to APS
            unit = '' if unit_kb else '-k'
            process = self._run_smem(fpath, unit)
            aps_path = self.remove_ext(fpath, self._snapshot_exts) + '.aps'
            self._aps_files.append((process, aps_path, test, timestamp, scenario))

    def analyze_folder(self):
        '''
        Traverses folder searching for test cases to analyze looking for
        memory snapshots in sorted order and convert them to APS output
        @param unit_kb If True output in 'kB' and otherwise in 'Mb'
        '''
        if not self._pg:
            # Scan folder for subfolders and then files
            for d in sorted(os.listdir(self._folder)):
                dpath = os.path.join(self._folder, d)
                if not os.path.isdir(dpath):
                    continue
                self._analyze_sorted_files(dpath)
        else:
            # Scan folder for files
            dpath = os.path.join(self._folder, self._pg)
            self._analyze_sorted_files(dpath)
        self._load_aps_files()

    def merge_results(self):
        '''
        Merge results from multiple test cases consolidating into
        a .profile file for APS
        @return List containing the output filenames as TestCase.profile
        '''
        if not self._aps_lst:
            print "No elements in APS list"
            return
        print 'Merging %d APS file(s)' % len(self._aps_lst)
        # Specify changes in placement of some APS keys in final .profile
        scenario_key = 'Scenario'
        aps_unit_key = 'APS_Unit'
        to_be_submaps = {
            aps_unit_key      : (),
            'APS_SystemMemory': ('System', 'Memory'),
            'APS_AllProcesses': ('Process', 'AllProcesses', 'Memory')
        }
        for elem in self._aps_lst:
            test_case, ts, scenario = (elem[0][0], elem[0][1], elem[0][2])
            self._all_tc_map[test_case] = self._all_tc_map.get(test_case, {})
            tc_map = self._all_tc_map[test_case]
            tc_map[scenario_key] = tc_map.get(scenario_key, [])
            tc_map[scenario_key].append( { 'Name': scenario, 'TS': ts } )
            submap = elem[1]
            unit = submap.get(aps_unit_key, 'KB').upper()
            for k1 in submap:
                if k1 in to_be_submaps:
                    # Handle custom hierarchy for some APS keys
                    if not to_be_submaps[k1]:
                        continue
                    level = [ tc_map ]
                    for elem in to_be_submaps[k1]:
                        level[-1][elem] = level[-1].get(elem, {})
                        level.append(level[-1][elem])
                        level = [ level[-1] ]
                    inner_map = level[-1]
                else:
                    # Then it's the memory information of a specific process
                    level = tc_map['Process'] = tc_map.get('Process', {})
                    level[k1] = level.get(k1, {})
                    level[k1]['Memory'] = level[k1].get('Memory', {})
                    inner_map = level[k1]['Memory']
                # inner_map is the final level where data finally needs to be
                for k2 in submap[k1]:
                    inner_map[k2] = inner_map.get(k2, {'Unit': unit, 'Values': []})
                    inner_map[k2]['Values'].append(submap[k1][k2])
        flist = []
        for test_case in self._all_tc_map:
            fname = os.path.join(self._folder, test_case + '.profile')
            with open(fname, 'w') as f:
                json.dump(self._all_tc_map[test_case], f, sort_keys=True \
                    , indent=4, separators=self.SEPARATORS)
                flist.append(fname)
        return flist

    def generate_perfmeta_json(self, comm):
        '''
        Generate perfmeta.json report file
        @return String containing the path to perfmeta.json report file
        '''
        print 'Generating perfmeta.json'
        pinfo = PlatInfoFactory.makePlatInfo(comm)
        perfmeta = {
            'targetDevice': {
                'HWName': pinfo.get_hw_name(),
                'OSName': pinfo.get_os_name(),
                'BuildInfo': pinfo.get_build_info(),
                'CodeName': pinfo.get_code_name(),
                'ModelName': pinfo.get_model_name()
            },
            'data': []
        }
        pm_data = perfmeta['data']
        for test_case in self._all_tc_map:
            print '\ttest_case(%s)' % str(test_case)
            value = self._all_tc_map[test_case]
            # Traverse keys in ['System']['Memory']['Used_Total']['Values']
            for k in ('System', self._pt, self._pv, 'Values'):
                value = value.get(k)
                if not value:
                    break
            else:
                # All keys found, proceed
                if not value or value < 1 or not isinstance(value, list):
                    continue
                print '\t\tvalue[s] = ' + str(value)
                if len(value) > 1:
                    # Last value minus first value
                    value = value[-1] - value[0]
                else:
                    value = value[0]
                print '\t\tvalue = ' + str(value)
                data_map = {
                    'PerfType' : self._pt,
                    'PerfGroup': test_case,
                    'PerfValue': round(float(value), 1)
                }
                pm_data.append(data_map)
        fname = os.path.join(self._folder, 'perfmeta.json')
        with open(fname, 'w') as f:
            json.dump(perfmeta, f, sort_keys=True, indent=4,
                    separators=self.SEPARATORS)

def capture(args):
    return MemoryProfiler.capture_snapshot(
        args.comm, args.workdir, args.PerfGroup, args.Scenario)

def report(args):
    memp = MemoryProfiler(args.workdir, args.perftype, args.perfgroup, args.perfvalue)
    memp.analyze_folder()
    ret = memp.merge_results()
    print 'Generated output files: %r' % ret
    memp.generate_perfmeta_json(args.comm)
    return os.EX_OK

def parse_args():
    '''
    Parse program arguments using argparse.
    @param Implicit program arguments via sys.argv
    @return Tuple containing the argument parser and its parsed arguments
    '''
    dft_perf_type   = 'Memory'
    dft_work_dir    = '/tmp/pmtrace/memory-profiling'
    dft_perf_value  = 'Used_Total'
    description     = 'Capture a memory snapshot or analyze captured snapshots'
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument('-d', '--workdir', action='store',
                        help='Working directory\n(default: %s)' \
                        % dft_work_dir, default=dft_work_dir)
    # ssh options
    ssh_grp = parser.add_argument_group()
    ssh_grp.add_argument('-i', '--ip', action='store',
                         help='ip address to a device')
    ssh_grp.add_argument('--user', action='store', default='root',
                         help='username for connecting to a target device\n(default: root)')
    ssh_grp.add_argument('--pw', action='store', default=None,
                         help='password for connecting to a target device\n(default: None)')
    ssh_grp.add_argument(
        '--port', action='store', default=22, help='Port (default:22)')
    # Subparsers
    subparsers = parser.add_subparsers(help='For sub-command help, use (-h) after command',
                                       dest='command')
    # Capture
    parser_capture = subparsers.add_parser('capture', \
        help='Capture memory snapshot for test case in some scenario\n'\
             'Needs options (-g) for test case and (-s) for scenario')
    parser_capture.add_argument('PerfGroup', action='store', help='PerfGroup (test case name)')
    parser_capture.add_argument('Scenario', action='store', help='Scenario name')
    parser_capture.set_defaults(func=capture)
    # Report
    parser_report = subparsers.add_parser('report', \
        help='Generate APS report based on captured memory snapshots\n'\
             'Use option (-g) for test case, otherwise scans all')
    parser_report.add_argument('-g', '--perfgroup', action='store',
                        help='PerfGroup (test case name)')
    parser_report.add_argument('-t', '--perftype', action='store',
                        help='PerfType (default: %s)' \
                        % dft_perf_type, default=dft_perf_type)
    parser_report.add_argument('-v', '--perfvalue', action='store',
                        help='PerfValue (default: %s)' \
                        % dft_perf_value, default=dft_perf_value)
    parser_report.set_defaults(func=report)

    args = parser.parse_args()
    return (parser, args)

def main():
    ''' Main function '''
    parser, args = parse_args()
    args.comm = CommanderFactory.makeCommander(args.ip, args.port, args.user, args.pw)
    if args.command in ('capture', 'report'):
        return args.func(args)
    parser.print_help()
    return os.EX_USAGE

if __name__ == '__main__':
    sys.exit(main())
