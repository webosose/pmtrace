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

import os, json, abc, subprocess

class PlatInfo(object):
    ''' Class that abstracts a platform information '''
    __metaclass__ = abc.ABCMeta
    @abc.abstractmethod
    def get_hw_name(self):
        ''' Get hardware name; e.g.: 'm16' '''
        return

    @abc.abstractmethod
    def get_os_name(self):
        ''' Get operating system name; e.g.: 'webOS TV Reference' '''
        return

    @abc.abstractmethod
    def get_build_info(self):
        ''' Get build information; e.g.: '100' '''
        return

    @abc.abstractmethod
    def get_code_name(self):
        ''' Get platform code name; e.g.: 'flattop-master' '''
        return

    @abc.abstractmethod
    def get_model_name(self):
        ''' Get platform model name; e.g.: 'starfish-atsc-flash' '''
        return

class PlatInfoFactory(object):
    ''' Get platform information instance via factory method pattern '''
    @staticmethod
    def makePlatInfo(comm):
        try:
            out = comm.exec_command(['which', 'nyx-cmd', '>', '/dev/null', '2>&1'], shell=True)
        except subprocess.CalledProcessError as e:
            # nyx-cmd not found; VC platform
            return VCPlatInfo(comm)
        # webOS platform information
        return webOSPlatInfo(comm)

class webOSPlatInfo(PlatInfo):
    def __init__(self, comm):
        '''
        webOS Platform Information
        @param comm Commander instance
        '''
        self._plat_info = {}
        for k in ['OSInfo', 'DeviceInfo']:
            out = comm.exec_command(['nyx-cmd', k, 'query', '--format=json', '2>/dev/null'], shell=True)
            self._plat_info.update(json.loads(out))

    def get_hw_name(self):
        return self._plat_info.get('device_name', '')

    def get_os_name(self):
        return self._plat_info.get('webos_name', '')

    def get_build_info(self):
        return self._plat_info.get('webos_build_id', '')

    def get_code_name(self):
        return self._plat_info.get('webos_release_codename', '')

    def get_model_name(self):
        return self._plat_info.get('webos_imagename', '')

    def __repr__(self):
        return str(self._plat_info)

class VCPlatInfo(PlatInfo):
    def __init__(self, comm):
        '''
        VC Platform Information
        @param comm Commander instance
        '''
        self._plat_info = {}
        out = comm.exec_command(['uname', '-a'])
        s = out.split()
        if len(s) >= 2:
            self._plat_info = {
                'hwName': s[1],
                'osName': s[0],
            }
        out = comm.exec_command(['cat', '/etc/os-release'])
        for line in out.splitlines():
            key, value = line.partition("=")[::2]
            key = key.strip()
            value = value.strip('\n').strip('"').strip()
            if key == 'ID':
                self._plat_info['modelName'] = value
            elif key == 'VERSION_ID':
                self._plat_info['buildInfo'] = value
            elif key == 'PRETTY_NAME':
                self._plat_info['codeName'] = value

    def get_hw_name(self):
        return self._plat_info.get('hwName', '')

    def get_os_name(self):
        return self._plat_info.get('osName', '')

    def get_build_info(self):
        return self._plat_info.get('buildInfo', '')

    def get_code_name(self):
        return self._plat_info.get('codeName', '')

    def get_model_name(self):
        return self._plat_info.get('modelName', '')

    def __repr__(self):
        return str(self._plat_info)
