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
import collections
import json
import logging
import os.path
import re
import sys
import textwrap

from commander import CommanderFactory
from plat_info import PlatInfoFactory


logging.basicConfig(level=logging.ERROR)
logger = logging.getLogger('PerfLogViewer')
log_entry_logger = logging.getLogger('LogEntry')

DEFAULT_PMLOG_FILE = '/var/log/messages'

class LogEntryBase(object):
    """Base class for a line of logs"""

    def __init__(self, s):
        self.raw = s
        self.proc = None
        self.type = None
        self.grp = None
        self.msgid = None
        self.clock = 0.0

        self.parse(self.raw)

    def __str__(self):
        return 'clk(%6.2f) proc(%s) type(%s) grp(%s) msgid(%s): raw(%s)' % \
            (self.clock, self.proc, self.type, self.grp, self.msgid, self.raw)

    def is_perf_log(self):
        return self.type and self.grp

    def is_contain_required_strs(self, req_strs):
        '''
        Check raw string has every strings in req_strs in order
        '''

        if not len(req_strs):
            return True

        idx = map(self.raw.find, req_strs)

        prev_idx = 0
        for i in idx:
            if i < prev_idx:
                return False
            else:
                prev_idx = i

        return True

    def get_free_text(self):
        raise NotImplementedError

    def parse(self, s):
        raise NotImplementedError


class PmlogEntry(LogEntryBase):

    """Describe each of a line

     - Format of PmLog
    UTC MONOTONICTIME LOGLEVEL PROCESS [PID] CONTEXT MSGID {...} FREETEXT
    """

    PMLOGRE = '^(?P<utc>.+) \[(?P<monotonicSec>[0-9\.]+)\] (?P<loglevel>[\w.]+) (?P<proc>.+?) \[(?P<pid>\d*?)\] (?P<ctx>.+?) (?P<msgid>.+?) (?P<rest>.*)$|^(?P<utc1>[A-z]+ [0-9]+ [0-9]+:[0-9]+:[0-9]+) (?P<host1>[A-z0-9]+) (?P<level>[A-z.]+) (?P<proc1>[A-z]+): \[.*\] \[(?P<logger1>.*)\] (?P<ctx1>.+?) (?P<msgid1>.+?) (?P<rest1>.*)$'

    def __init__(self, s):
        self.freeText = None
        self.rest = None
        super(PmlogEntry, self).__init__(s)

    def __str__(self):
        return 'Pmlog clk(%6.2f) proc(%s) type(%s) grp(%s) msgid(%s) freetxt(%s): raw(%s)' % \
            (self.clock, self.proc, self.type, self.grp,
             self.msgid, self.freeText, self.raw)

    def get_free_text(self):
        s = self.rest + ' ' + self.freeText
        return s.strip()

    def parse(self, s):
        def decode_fields(mdict):
            try:
                tmp = {}
                tmp.update({k: v for k, v in mdict.items() if k in (
                    'utc', 'loglevel', 'proc', 'ctx', 'msgid')})
                tmp['monotonicSec'] = float(mdict['monotonicSec'])
                tmp['pid'] = int(mdict['pid']) if mdict['pid'] != '' else 0
                # 'rest' consists of kvs and freeText
                rest = mdict['rest']
                return (tmp, rest)
            except:
                pass
            try:
                tmp = {}
                tmp.update({k: v for k, v in mdict.items() if k in (
                    'utc1', 'loglevel1', 'proc1', 'ctx1', 'msgid1')})
                tmp['monotonicSec'] = float(0.0)
                tmp['pid'] = int(mdict.get('pid1')) if mdict.get('pid1') else 0
                # 'rest' consists of kvs and freeText
                rest = mdict['rest1']
                return (tmp, rest)
            except:
                pass

        match = re.search(self.PMLOGRE, s)

        if not match:
            log_entry_logger.debug('Wrong log format: %s', s)
            return False

        mdict = match.groupdict()

        tmp = {}
        try:
            tmp, rest = decode_fields(mdict)

            stk = []
            idxEndKvs = 0
            for i in range(len(rest)):
                ch = rest[i]
                if ch == '{':
                    stk.append(ch)
                elif ch == '}':
                    stk.pop()
                else:
                    continue

                if not len(stk):
                    idxEndKvs = i
                    break

            kvs = rest[0:idxEndKvs + 1]
            tmp.update(json.loads(kvs))

            self.proc = tmp.get('proc') or tmp.get('proc1')

            self.type = tmp.get('PerfType')
            self.grp = tmp.get('PerfGroup')
            self.msgid = tmp.get('msgid') or tmp.get('msgid1')
            self.clock = tmp.get('CLOCK', tmp.get('monotonicSec') or 0.0)
            self.freeText = rest[idxEndKvs + 1:].strip()

            excludeKeys = ('utc', 'utc1', 'monotonicSec', 'loglevel',
                           'loglevel1', 'proc', 'proc1', 'pid', 'pid1', 'ctx',
                           'ctx1', 'msgid', 'msgid1', 'freeText', 'PerfType',
                           'PerfGroup', 'CLOCK', 'app_id')
            self.rest = ''

            pvs = [(k, str(tmp[k]))
                   for k in tmp.keys() if k not in excludeKeys]
            if pvs:
                for i in pvs:
                    self.rest += ':'.join(i) + ' '

        except:
            log_entry_logger.debug('Wrong log contents: %s', s)
            return False

        return True


class JournallogEntry(LogEntryBase):

    """
    A entry of journal log from journalctl.
    Extracted by a command below:

    $ journalctl -o json

    Output example:
    {
      "__CURSOR":"s=71e4ee26a56344e5bfafe2a0a5ced026;i=363;b=d6bb4bcfe6c54658986cd0286bc1e20c;m=22b2fdf21;t=5394970c92293;x=e85ab4a96a999cb",
      "__REALTIME_TIMESTAMP":"1470362471178899",
      "__MONOTONIC_TIMESTAMP":"9314492193",
      "_BOOT_ID":"d6bb4bcfe6c54658986cd0286bc1e20c",
      "PRIORITY":"6",
      "_UID":"0",
      "_GID":"0",
      "_MACHINE_ID":"da38358c33c74627ab397750a9b9bb0a",
      "_HOSTNAME":"imx6qsabreauto",
      "_CAP_EFFECTIVE":"1fffffffff",
      "SYSLOG_FACILITY":"1",
      "_TRANSPORT":"syslog",
      "_SYSTEMD_CGROUP":"/system.slice/system-serial\\x2dgetty.slice/serial-getty@ttymxc3.service",
      "_SYSTEMD_UNIT":"serial-getty@ttymxc3.service",
      "_SYSTEMD_SLICE":"system-serial\\x2dgetty.slice",
      "SYSLOG_IDENTIFIER":"pmtrace_example",
      "MESSAGE":"{\"ctx\":\"libpmtrace_sample\", \"CLOCK\":\"9314.489\", \"msgid\":\"sample_msgid\", \"PerfType\":\"TestType\", \"PerfGroup\":\"TestGroup\", \"arg1\":\"ARG1\", \"desc\":\"Hello!!\"}",
      "_PID":"622",
      "_COMM":"pmtrace_example",
      "_EXE":"/home/root/pmtrace-ex/pmtrace_example",
      "_CMDLINE":"./pmtrace_example",
      "_SOURCE_REALTIME_TIMESTAMP":"1470362471176462"
    }
    """

    def __init__(self, s):
        super(JournallogEntry, self).__init__(s)

    def __str__(self):
        return 'Journallog clk(%6.2f) proc(%s) type(%s) grp(%s) msgid(%s): raw(%s)' % \
            (self.clock, self.proc, self.type, self.grp, self.msgid, self.raw)

    def parse(self, s):
        try:
            ent = json.loads(s)
        except ValueError:
            log_entry_logger.debug('Ignore an invalid log: (%s)' % s)
            return False

        try:
            msg = json.loads(ent.get('MESSAGE', ''))
        except ValueError:
            log_entry_logger.debug('Ignore an invalid log: (%s)' % s)
            return False

        self.proc = ent.get('_COMM')
        self.type = msg.get('PerfType')
        self.grp = msg.get('PerfGroup')
        self.msgid = msg.get('msgid')
        self.clock = msg.get('CLOCK', ent.get('__MONOTONIC_TIMESTAMP', 0.0))

        return True


class LogList(list):

    def __init__(self, *args):
        self.repr_type = None
        self.repr_grp = None

        super(LogList, self).__init__(*args)

    def clock_begin(self):
        if not self:
            return None

        return self[0].clock

    def clock_end(self):
        if not self:
            return None

        return self[-1].clock

    def sort_by_time(self):
        self.sort(key=lambda e: e.clock)

    def set_represent_type(self, type_name=None):
        if type_name:
            self.repr_type = type_name
        else:
            type_cnt = collections.Counter([x.type for x in self])
            logger.debug('set_represent_type(): %s' % type_cnt)

            common = type_cnt.most_common(1)
            if len(common):
                t, c = common[0]
                self.repr_type = t

    def set_represent_group(self, group_name=None):
        if group_name:
            self.repr_grp = group_name
        else:
            grp_cnt = collections.Counter([x.grp for x in self])
            logger.debug('set_represent_grp(): %s' % grp_cnt)

            common = grp_cnt.most_common(1)
            if len(common):
                g, c = common[0]
                self.repr_grp = g


def load_pmlog(comm, config, pmlogs):
    log_entries = LogList()

    if not pmlogs:
        return log_entries

    try:
        for i in pmlogs:
            with comm.read_file(i) as f:
                for raw_line in f.readlines():
                    ent = PmlogEntry(raw_line.strip())

                    if ent.is_perf_log() or config.is_in_conditions(ent):
                        log_entries.append(ent)
                    else:
                        log_entry_logger.debug('Ignore: %s' % ent)
    except IOError as e:
        print('Error while loading pmlog', e)
        raise e

    log_entry_logger.info('Load pmlog: cnt(%d) from (%s)' %
                          (len(log_entries), ' ,'.join(pmlogs)))

    return log_entries


def load_journallog(comm, config, journallogs):
    log_entries = LogList()
    cmd = ['journalctl', '-o', 'json']
    out = comm.exec_command(cmd)

    for line in out.split('\n'):
        ent = JournallogEntry(line)

        if ent.is_perf_log() or config.is_in_conditions(ent):
            log_entries.append(ent)
        else:
            log_entry_logger.debug('Ignore: %s' % ent)

    log_entry_logger.info('Loaded journallog: cnt(%d)' % len(log_entries))

    return log_entries


def load_logs(comm, plat, config, **kargs):
    log_entries = LogList()

    if plat.get_hw_name().lower().find('sabreauto') != -1:
        # VC
        log_entry_logger.info('Running platform: VC %s' % plat.get_hw_name())

        log_entries.extend(
            load_journallog(comm, config, kargs.get('journallogs')))
    else:
        # webOS or Host
        log_entry_logger.info('Running platform: %s' % plat.get_os_name())

        # Lookup log files if a user doesn't input specific log files
        if not kargs.get('pmlogs') :
            pmlog_files = comm.exec_command(
                ['ls', DEFAULT_PMLOG_FILE + '*'], True).split()
            logger.info('Pmlog files: %s' % pmlog_files)
            kargs['pmlogs'] = pmlog_files

        log_entries.extend(load_pmlog(comm, config, kargs.get('pmlogs', [])))

    log_entry_logger.info('Nr of entries: (%d)' % len(log_entries))

    return log_entries


class ViewerConfig(object):

    class Context(object):

        def __init__(self, obj):
            self.id = None
            self.desc = obj.get('description')
            self.repr_type = obj.get('PerfType')
            self.repr_grp = obj.get('PerfGroup')
            self.resp_ms = obj.get('allowedResponseMS')
            self.starts = self.load_condition(obj['startConditions'])
            self.ends = self.load_condition(obj['endConditions'])

        def __str__(self):
            out = textwrap.dedent('''\
            desc(%s) type(%s) group(%s) response_ms(%d)
            starts:\t%s
            ends:\t%s''') % (self.desc,
                             self.repr_type,
                             self.repr_grp,
                             self.resp_ms,
                             '\n\t'.join(map(str, self.starts)),
                             '\n\t'.join(map(str, self.ends)))

            return out

        def get_matched_start_cond(self, log_entry):
            for cond in self.starts:
                if cond.is_matched(log_entry):
                    return cond

            return None

        def has_matched_entry_in_start_cond(self, log_entry):
            for cond in self.starts:
                if cond.is_matched(log_entry):
                    return True

            return False

        def has_matched_entry_in_end_cond(self, log_entry):
            for cond in self.ends:
                if cond.is_matched(log_entry):
                    return True

            return False

        def has_matched_entry_in_cond(self, log_entry):
            return self.has_matched_entry_in_start_cond(log_entry) or \
                self.has_matched_entry_in_end_cond(log_entry)

        def load_condition(self, objs):
            return list(map(ViewerConfig.Condition, objs))

    class Condition(object):

        def __init__(self, obj):
            self.type = obj.get("PerfType")
            self.grp = obj.get("PerfGroup")
            self.msgid = obj.get("msgid")
            self.req_strs = obj.get('requiredStrings', [])

        def __str__(self):
            return 'type(%s) grp(%s) msgid(%s) required_str(%s)' % \
                (self.type, self.grp, self.msgid, ' '.join(self.req_strs))

        def is_matched(self, log_entry):
            if self.type != '*' and self.type != log_entry.type:
                return False

            if self.grp != '*' and self.grp != log_entry.grp:
                return False

            if self.msgid != '*' and self.msgid != log_entry.msgid:
                return False

            if len(self.req_strs) and not log_entry.is_contain_required_strs(self.req_strs):
                return False

            return True

    def __init__(self, f):
        self.ctxs = []

        self.load(f)

    def __str__(self):
        return 'contexts:\n%s' % '\n\n'.join(self.ctxs)

    def iter_contexts(self):
        for ctx in self.ctxs:
            yield ctx

    def get_matching_contexts_in_start_cond(self, log_entry):
        ctxs = []

        for ctx in self.iter_contexts():
            if ctx.has_matched_entry_in_start_cond(log_entry):
                ctxs.append(ctx)

        return ctxs

    def __iter_all_conditions(self):
        for ctx in self.iter_contexts():
            for s in ctx.starts:
                yield s

            for e in ctx.ends:
                yield e

    def is_in_conditions(self, entry):
        '''
        @entry: LogEntryBase
        '''
        for cond in self.__iter_all_conditions():
            if cond.is_matched(entry):
                return True

        return False

    def load(self, f):
        with open(f) as fp:
            obj = json.load(fp)
            self.ctxs = list(map(ViewerConfig.Context, obj.get('contexts')))

        logger.debug('Context list')
        for i in range(len(self.ctxs)):
            self.ctxs[i].id = i
            logger.debug(self.ctxs[i])


def analyze(conf, log_entries):
    analyzed_list = []

    cur = 0
    start_cond = None

    while cur < len(log_entries):
        cur_ent = log_entries[cur]
        match_ctxs = conf.get_matching_contexts_in_start_cond(cur_ent)

        if not match_ctxs:
            cur += 1
            continue

        start_ent = log_entries[cur]
        start_cond = match_ctxs[0].get_matched_start_cond(start_ent)
        logger.debug('Found new start: s(%d) cond(%s)' % (
            cur, start_cond))

        for ctx in match_ctxs:
            possible_window = [e for e in log_entries[cur:]
                               if e.clock < start_ent.clock + ctx.resp_ms / 1000]

            for i in reversed(range(len(possible_window))):
                if ctx.has_matched_entry_in_end_cond(possible_window[i]):
                    del possible_window[i + 1:]
                    break
            else:
                logger.info('Ignore: No end cond')
                continue

            if any(map(lambda e: start_cond.is_matched(e), possible_window[1:])):
                logger.debug('Ignore: duplicated start cond')
                continue

            found_ents = LogList(possible_window)
            found_ents.set_represent_type(ctx.repr_type)
            found_ents.set_represent_group(ctx.repr_grp)

            logger.info('Found: nr(%d) type(%s) grp(%s) cond(%s)' % (
                len(found_ents),
                found_ents.repr_type,
                found_ents.repr_grp,
                start_cond))
            analyzed_list.append(found_ents)

            cur += len(possible_window) - 1

        cur += 1

    return analyzed_list


def export_2_text(fp, ent_grps, export_filter={}, fmt_opt='text'):
    assert fp, 'Need to specify fp for an output file'

    print_items = ('Process', 'MsgID', 'Time(s)', 'Diff(s)', 'Extra')

    print_fmt = '%s,%s,%s,+%s,%s\n' if fmt_opt == 'csv' else \
        '%-30s %-25s %-8s +%-8s %-s\n'

    for ent_grp in ent_grps:

        if not len(ent_grp):
            logger.warning('export: no data')
            continue

        if export_filter.get('type') and \
                ent_grp.repr_type not in export_filter.get('type'):
            logger.info('Filtered out: type(%s)' %
                        export_filter.get('type'))
            continue

        if export_filter.get('group') and \
                ent_grp.reprgroup not in export_filter.get('group'):
            logger.info('Filtered out: group(%s)' %
                        export_filter.get('group'))
            continue

        begin = ent_grp.clock_begin()
        end = ent_grp.clock_end()

        fp.write('Type: %s\nGroup: %s\nStart time: %4.2f\n' %
                 (ent_grp.repr_type, ent_grp.repr_grp, begin))
        fp.write(print_fmt % print_items)

        prev_clock = begin

        for ent in ent_grp:
            rel_time = str(round(ent.clock - begin, 3))
            diff = round(ent.clock - prev_clock, 3)
            prev_clock = ent.clock

            fp.write(print_fmt % (ent.proc,
                                  ent.msgid,
                                  rel_time,
                                  diff,
                                  ent.get_free_text()))

        fp.write('Elapsed time (s) : %2.3f\n\n' % (end - begin))


def export_2_json(fp, ent_grps, plat_info, export_filter={}):
    assert fp, 'Need to specify fp for an output file'

    out = {
        'targetDevice': {
            'HWName': plat_info.get_hw_name(),
            'OSName': plat_info.get_os_name(),
            'BuildInfo': plat_info.get_build_info(),
            'CodeName': plat_info.get_code_name(),
            'ModelName': plat_info.get_model_name()
        },
        'data': []
    }

    for ent_grp in ent_grps:
        if not len(ent_grp):
            logger.warning('export: no data')
            continue

        if export_filter.get('type') and \
                ent_grp.repr_type not in export_filter.get('type'):
            logger.info('Filtered out: type(%s)' %
                        export_filter.get('type'))
            continue

        if export_filter.get('group') and \
                ent_grp.repr_grp not in export_filter.get('group'):
            logger.info('Filtered out: group(%s)' %
                        export_filter.get('group'))
            continue

        begin = ent_grp.clock_begin()
        end = ent_grp.clock_end()

        out['data'].append({'PerfType': ent_grp.repr_type,
                            'PerfGroup': ent_grp.repr_grp,
                            'PerfValue': round(end - begin, 3)})

    json.dump(out, fp, indent=True)


def export(ent_grps, plat_info, fmt, out_file=None, export_filter={}):
    '''
    @fmt: Select fmt. Support 'text, csv, json'
    @out_file: A filename to export a result into file. stdout if it's None
    @export_filter: a dict to filter in. e.g. {'type':[], 'group': []}
    '''
    if out_file:
        try:
            ofp = open(out_file, 'w')
        except IOError as e:
            logger.error('Cannot open %s: %s' % (out_file, e))
            return False
    else:
        ofp = sys.stdout

    if fmt == 'json':
        export_2_json(ofp, ent_grps, plat_info, export_filter)
    else:
        # text or csv
        export_2_text(ofp, ent_grps, export_filter, fmt)

    return True


if __name__ == "__main__":

    desc = '''
    Performance Log Viewer

    Simple usages:
    ./perf_log_viewer.py
    > Read '/var/log/messages' and show a result

    ./perf_log_viewer.py -p <PmLog file>
    > Read logs from manually specified options

    ./perf_log_viewer.py --ip <IP Address>
    > Read from a target device.

    ./perf_log_viewer.py -c <Config file>
    > Use another configuration file

    For more information, please see below links
    https://wiki.lgsvl.com/display/webOSDocs/Performance+Log+Viewer
    https://wiki.lgsvl.com/pages/viewpage.action?pageId=115933443
    '''

    arg_parser = argparse.ArgumentParser(formatter_class=argparse.RawDescriptionHelpFormatter,
                                         description=desc)
    arg_parser.add_argument('-c', '--config', action='store',
                            help='Load a config file (default: config.json or /etc/pmtrace/perf-log-viewer-conf.json')
    arg_parser.add_argument('-t', '--type', action='append',
                            help='Specify performance type to filter out')
    arg_parser.add_argument('-g', '--group', action='append',
                            help='Specify performance group to filter out')
    arg_parser.add_argument('--format', action='store', default='text',
                            choices=['text', 'csv', 'json'], help='Set output format')
    arg_parser.add_argument(
        '-o', '--output', action='store', help='Output file name')

    ssh_grp = arg_parser.add_argument_group()
    ssh_grp.add_argument('-i', '--ip', action='store',
                         help='ip address to a device')
    ssh_grp.add_argument('--user', action='store', default='root',
                         help='A username for connecting to a target device (default: root)')
    ssh_grp.add_argument('--pw', action='store', default=None,
                         help='password for connecting to a target device\n(default: None)')
    ssh_grp.add_argument(
        '--port', action='store', default=22, help='Port (default:22)')

    file_grp = arg_parser.add_argument_group()
    file_grp.add_argument(
        '-p', '--PmlogFile', action='append', help='PmLog file')
    file_grp.add_argument('--logpath', action='store',
                          help='Set a log path. Load whole messages')

    args = vars(arg_parser.parse_args())
    logger.info('Argparse conf: %s' % str(args))

    # Load configuration file
    default_configs = ('./config.json',
                       '/etc/pmtrace/perf-log-viewer-conf.json')
    config_file = args['config']
    if not config_file:
        for i in default_configs:
            if os.path.exists(i):
                config_file = i
                break

    if not config_file:
        print "Cannot find a config file"
        exit(1)

    try:
        viewer_conf = ViewerConfig(config_file)
    except Exception as e:
        print('Failed to load a config(%s): %s' % (config_file, e))
        exit(1)

    comm = CommanderFactory.makeCommander(
        args['ip'], args['port'], args['user'], args['pw'])
    plat_info = PlatInfoFactory.makePlatInfo(comm)

    # Load several logs
    ent_grp = load_logs(comm,
                        plat_info,
                        viewer_conf,
                        pmlogs=args.get('PmlogFile'))
    ent_grp.sort_by_time()

    analyzed_ent_grps = analyze(viewer_conf, ent_grp)

    filter_opts = {k: v for k, v in args.items() if k in ('type', 'group')}
    export(analyzed_ent_grps,
           plat_info,
           args['format'],
           args['output'],
           filter_opts)
