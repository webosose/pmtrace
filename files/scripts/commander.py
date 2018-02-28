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

import subprocess

class CommanderBase(object):

    def read_file(self, f):
        raise NotImplemented

    def exec_command(self, cmd, shell=False):
        raise NotImplemented


class CommanderFactory(object):
    ''' Get Command instance via factory method pattern '''
    @staticmethod
    def makeCommander(ip=None, port=None, user=None, pw=None):
        if ip and port and user:
            return RemoteCommand(hostname=ip, port=port, user=user, pw=pw)

        return LocalCommand()


class LocalCommand(CommanderBase):

    def read_file(self, f):
        if f.endswith('.gz'):
            p = subprocess.Popen(['zcat', f], stdout=subprocess.PIPE)

            fp = p.stdout
        else:
            fp = open(f)

        return fp

    def exec_command(self, cmd, shell=False):
        if shell:
            cmd = ' '.join(cmd)
        return subprocess.check_output(cmd, shell=shell)


class RemoteCommand(CommanderBase):
    SSH_ERR_MSG = '%s\nFailed to connect via ssh. Is remote %r online?'

    def __init__(self, hostname, port=22, user=None, pw=None, timeout=15.0):
        self.hostname = hostname
        self.timeout = float(timeout) # seconds
        self.port = int(port)
        self.user = user
        self.pw = pw
        # Raise ImportError if paramiko or select are not found
        import paramiko, select
        self.selector = select.select
        self.selector_timeout = 1 # second
        self.client = paramiko.SSHClient()
        self.client.load_system_host_keys()
        self.client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        try:
            self.client.connect(hostname,
                                username=self.user,
                                password=self.pw,
                                port=self.port,
                                timeout=self.timeout,
                                )
        except (paramiko.SSHException, Exception) as e:
            transport = self.client.get_transport()
            if transport:
                # If no authentication is needed try without password
                transport.auth_none(self.user)
        self.buf_size = 4096 # bytes
        transport = self.client.get_transport()
        if transport:
            self.buf_size = transport.default_window_size

    def __del__(self):
        try:
            self.client.close()
        except:
            pass

    def read_file(self, f):
        cmdline = ' '.join(['zcat' if f.endswith('.gz') else 'cat', f])
        try:
            stdin, stdout, stderr = self.client.exec_command(cmdline)
        except Exception as e:
            raise subprocess.CalledProcessError(
                255, self.SSH_ERR_MSG % (cmdline, self.hostname)
                )
        return stdout

    def exec_command(self, cmd, shell=False):
        cmdline = ' '.join(cmd)
        try:
            stdin, stdout, stderr = self.client.exec_command(cmdline)
        except Exception as e:
            raise subprocess.CalledProcessError(
                255, self.SSH_ERR_MSG % (cmdline, self.hostname)
                )
        s = ''
        while not stdout.channel.exit_status_ready():
            self.selector([stdout.channel], [], [], self.selector_timeout)
            if stdout.channel.recv_ready():
                s += stdout.channel.recv(self.buf_size)
        while stdout.channel.recv_ready():
            s += stdout.channel.recv(self.buf_size)
            self.selector([stdout.channel], [], [], self.selector_timeout)
        if stdout.channel.recv_exit_status() != 0:
            ret = stdout.channel.recv_exit_status()
            raise subprocess.CalledProcessError(
                ret, 'Command returned non-zero status: %d' % ret
                )
        return s
