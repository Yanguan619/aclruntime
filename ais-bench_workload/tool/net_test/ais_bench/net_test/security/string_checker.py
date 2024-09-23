# Copyright (c) 2024-2024 Huawei Technologies Co., Ltd.
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

import os
import re
from ais_bench.net_test.security.utils import get_platform
from ais_bench.net_test.security.standard_consts import (PlatformSupport, PathLengthLimit, StrWhitePattern,
                                                         StrBlackPattern, CommandBlackList)


class StringChecker:
    def __init__(self, this_str: str):
        self.this_str = this_str

    def is_legal_path(self):
        '''
        校验路径字符串形式规范。
        Paramters:
            None
        Returns:
            bool (True:路径规范, False:路径不规范)
        '''
        if get_platform() == PlatformSupport.LINUX:
            return self._is_legal_path_linux
        if get_platform() == PlatformSupport.WINDOWS:
            raise RuntimeError('Not supported for windows yet!')
        return False

    def is_length_legal(self, max_length: int):
        '''
        校验字符长度合法。
        Paramters:
            max_length: 字符串许可的最长长度
        Returns:
            bool (True:字符串长度合法, False:字符串长度非法)
        '''
        if len(self.this_str) > max_length:
            return False
        return True

    def is_black_pattern_check_ok(self, pattern=StrBlackPattern.NORMAL_STR):
        '''
        校验字符串是否含黑名单字符。
        Paramters:
            pattern: 字符黑名单的正则表达式
        Returns:
            bool (True:字符串不含黑名单字符, False:字符串含黑名单字符)
        '''
        if re.compile(pattern).search(self.this_str):
            return False
        return True

    def is_white_pattern_check_ok(self, pattern=StrWhitePattern.NORMAL_STR):
        '''
        校验字符串是否含白名单以外字符。
        Paramters:
            pattern: 字符白名单的正则表达式
        Returns:
            bool (True:字符串不含白名单以外字符, False:字符串含白名单以外字符)
        '''
        if re.compile(pattern).search(self.this_str):
            return False
        return True

    def is_full_match_pattern(self, pattern):
        '''
        校验字符串是否完全满足正则表达式。
        Paramters:
            pattern: 指定的正则表达式
        Returns:
            bool (True:字符串满足正则表达式, False:字符串不满足正则表达式)
        '''
        if not re.fullmatch(pattern, self.this_str):
            return False
        return True

    def is_cmd_meet_black_list(self, black_list=CommandBlackList.NORMAL_LINUX):
        '''
        校验命令字符串是否包含危险命令。
        Paramters:
            black_list: 危险命令黑名单列表
        Returns:
            bool (True:字符串不含黑名单命令, False:字符串含黑名单命令)
        '''
        sub_cmds = self.this_str.split(" ")
        for cmd in sub_cmds:
            if cmd in black_list:
                return False
        return True

    def _is_legal_path_linux(self):
        abs_path = os.path.abspath(self.this_str)
        if len(abs_path) > PathLengthLimit.LINUX_TOTAL_LENGTH:
            return False
        if re.compile(StrWhitePattern.ABS_PATH_LINUX).search(abs_path):
            return False
        sub_names = abs_path.split("/")
        for name in sub_names:
            if len(name) > PathLengthLimit.SINGLE_NAME_LENGTH:
                return False
        return True
