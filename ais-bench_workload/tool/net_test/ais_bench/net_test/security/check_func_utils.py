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

from ais_bench.net_test.common.consts import LENGTH_LIMIT, INT_LIMIT, STRING_PATTERN, OTHERS
from ais_bench.net_test.security.file_checker import check_linux_executable_file
from ais_bench.net_test.security.standard_consts import FileSizeLimite, PermForbid


def _check_str_length(s: str, MIN_LEN: int = 0, MAX_LEN: int = LENGTH_LIMIT.MAX_UINT64_STR_LENGTH):
    if len(s) < MIN_LEN or len(s) > MAX_LEN:
        raise ValueError('The length of input string is not between [{}, {}]'.format(MIN_LEN, MAX_LEN))
    return s


def check_int_string(x: str, X_MIN: int = 0, X_MAX: int = INT_LIMIT.UINT64_MAX):
    _check_str_length(x, MIN_LEN=1, MAX_LEN=LENGTH_LIMIT.MAX_UINT64_STR_LENGTH)
    if not x.isdigit():
        raise ValueError(f"Input x is an invalid positive int value")

    x_int = int(x)
    if x_int < X_MIN or x_int > X_MAX:
        raise ValueError('The value of x:{} is not between [{}, {}]'.format(x, X_MIN, X_MAX))
    return x_int


def check_positive_int_string(x: str):
    return check_int_string(x, X_MIN=1, X_MAX=INT_LIMIT.UINT64_MAX)


def check_exe_path(path: str):
    return check_linux_executable_file(path)


def is_regex_fullmatch(string: str, pattern: str):
    if not re.fullmatch(pattern, string):
        return False
    return True


def check_ipv4_string(value: str):
    if len(value) > LENGTH_LIMIT.MAX_IPV4_LENGTH:
        raise ValueError(
            f"The length of ipv4_string is over MAX_IPV4_LENGTH {LENGTH_LIMIT.MAX_IPV4_LENGTH}!"
        )
    if not is_regex_fullmatch(value, STRING_PATTERN.LEGAL_IPV4_PATTERN):
        raise ValueError("The format of ipv4_string is illegal!")
    return value


def check_bytes_format(value: str):
    if len(value) > LENGTH_LIMIT.MAX_BYTES_STR_LENGTH:
        raise ValueError(
            f"The length of bytes_string is over MAX_BYTES_STR_LENGTH {LENGTH_LIMIT.MAX_BYTES_STR_LENGTH}!"
        )
    if not is_regex_fullmatch(value, STRING_PATTERN.LEGAL_BYTES_FORMAT_PATTERN):
        raise ValueError("The format of bytes_string is illegal!")

    value_int = value[:-1]
    check_positive_int_string(value_int)
    return value_int


def check_linux_username(value: str):
    if len(value) > LENGTH_LIMIT.MAX_LINUX_USERNAME_LENGTH or len(value) <= 0:
        raise ValueError(
            'The linux username length must be in the range [1, {}]!'.format(LENGTH_LIMIT.MAX_LINUX_USERNAME_LENGTH))
    if not is_regex_fullmatch(value, STRING_PATTERN.LEGAL_LINUX_USERNAME_PATTERN):
        raise ValueError('The linux username format is not valid!'.format(value))
    return value


def transform_hostfile_line(line: str):
    """ 解析并提取 hostfile 的行字段值 """
    info_list = line.split(":")
    if len(info_list) < OTHERS.NODE_INFO_MIN_COUNT:
        raise ValueError(f"node_info line: {info_list} missing enough params!")
    if len(info_list) > OTHERS.NODE_INFO_MAX_COUNT:
        raise ValueError(f"node_info line: {info_list} too many params!")
    while len(info_list) < OTHERS.NODE_INFO_MAX_COUNT:
        info_list.append("")
    ipv4_str, device_count, user, port = info_list
    check_ipv4_string(ipv4_str)  # ipv4 str
    check_positive_int_string(device_count)  # device_count, empty is legal
    device_count = int(device_count)  # device_count,
    user = user if user else "root"  # user, default is root
    check_linux_username(user)
    # port, default is 22
    port = check_int_string(port, X_MIN=1, X_MAX=INT_LIMIT.PORT_MAX) if port else 22
    return ipv4_str, device_count, user, port


def parse_hostfile(hostfile):
    with open(hostfile, 'r') as file:
        count = 0
        host_tuples = []
        for line in file:
            stripped_line = line.strip()
            if not stripped_line:
                continue  # 忽略 hostfile 文件中的空白行
            if count > OTHERS.MAX_HOST_LINE_COUNT:
                ValueError('The host file line count is over {}!'.format(OTHERS.MAX_HOST_LINE_COUNT))
            host_tuples.append(transform_hostfile_line(stripped_line))
            count += 1
    return host_tuples


def find_executable(exe_name: str):
    env_path = os.environ.get("PATH", "")
    exe_path = shutil.which(exe_name, path=env_path)
    return exe_path


def check_executable(value: str):
    try:
        check_linux_executable_file(value, max_size=FileSizeLimite.NORMAL_READ_FILE_4G,
                                    perm_forbid=PermForbid.USER_MAIN_DIR)
    except ValueError as e:
        exe_path = find_executable(value)
        print('exe_path:', exe_path)
        if exe_path is None:
            raise ValueError('Cannot find the executable file! ' + str(e))
