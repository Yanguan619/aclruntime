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
import shutil

from ais_bench.net_test.common.consts import LENGTH_LIMIT, INT_LIMIT, STRING_PATTERN
from ais_bench.net_test.security.file_checker import check_linux_executable_file
from ais_bench.net_test.security.standard_consts import FileSizeLimit, PermForbid


def _check_str_length(s: str, min_len: int = 0, max_len: int = LENGTH_LIMIT.MAX_UINT64_STR_LENGTH):
    if len(s) < min_len or len(s) > max_len:
        raise ValueError('The length of input string is not between [{}, {}]'.format(min_len, max_len))
    return s


def check_int_string(x: str, x_min: int = 0, x_max: int = INT_LIMIT.UINT64_MAX):
    _check_str_length(x, min_len=LENGTH_LIMIT.MIN_UINT_STR_LENGTH, max_len=LENGTH_LIMIT.MAX_UINT64_STR_LENGTH)
    if not x.isdigit():
        raise ValueError(f"Input x is an invalid positive int value")

    x_int = int(x)
    if x_int < x_min or x_int > x_max:
        raise ValueError('The value of x:{} is not between [{}, {}]'.format(x, x_min, x_max))
    return x_int


def check_positive_int_string(x: str):
    return check_int_string(x, x_min=LENGTH_LIMIT.MIN_UINT_STR_LENGTH, x_max=INT_LIMIT.UINT64_MAX)


def _is_regex_full_match(string: str, pattern: str):
    if not re.fullmatch(pattern, string):
        return False
    return True


def check_ipv4_string(value: str):
    if len(value) > LENGTH_LIMIT.MAX_IPV4_LENGTH:
        raise ValueError(
            f"The length of ipv4_string is over MAX_IPV4_LENGTH {LENGTH_LIMIT.MAX_IPV4_LENGTH}!"
        )
    if not _is_regex_full_match(value, STRING_PATTERN.LEGAL_IPV4_PATTERN):
        raise ValueError("The format of ipv4_string is illegal!")
    return value


def check_bytes_format(value: str):
    if len(value) > LENGTH_LIMIT.MAX_BYTES_STR_LENGTH:
        raise ValueError(
            f"The length of bytes_string is over MAX_BYTES_STR_LENGTH {LENGTH_LIMIT.MAX_BYTES_STR_LENGTH}!"
        )
    if not _is_regex_full_match(value, STRING_PATTERN.LEGAL_BYTES_FORMAT_PATTERN):
        raise ValueError("The format of bytes_string is illegal!")

    value_int = value[:-1]
    check_positive_int_string(value_int)
    return value_int


def check_linux_username(value: str):
    if len(value) > LENGTH_LIMIT.MAX_LINUX_USERNAME_LENGTH or len(value) <= 0:
        raise ValueError(
            'The linux username length must be in the range [1, {}]!'.format(LENGTH_LIMIT.MAX_LINUX_USERNAME_LENGTH))
    if not _is_regex_full_match(value, STRING_PATTERN.LEGAL_LINUX_USERNAME_PATTERN):
        raise ValueError('The linux username format is not valid!'.format(value))
    return value


def _find_executable(exe_name: str):
    env_path = os.environ.get("PATH", "")
    exe_path = shutil.which(exe_name, mode=os.F_OK | os.X_OK, path=env_path)
    return exe_path


def check_executable(value: str):
    if os.path.dirname(value):
        # is executable path
        check_linux_executable_file(value, max_size=FileSizeLimit.NORMAL_READ_FILE_4G,
                                    perm_forbid=PermForbid.USER_MAIN_DIR)
        return value
    else:
        # is executable name
        exe_path = _find_executable(value)
        if exe_path is None:
            raise ValueError('Cannot find the executable file!')
        return exe_path
