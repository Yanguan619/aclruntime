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
from shutil import which

from ais_bench.net_test.common.consts import LENGTH_LIMIT, INT_LIMIT, STRING_PATTERN, OTHERS
from ais_bench.net_test.security.file_checker import check_linux_executable_file, check_linux_readable_file
from ais_bench.net_test.security.standard_consts import FileSizeLimite, PermForbid, PermNeed


def check_str_length(s: str, MIN_LEN: int = 0, MAX_LEN: int = LENGTH_LIMIT.MAX_UINT64_STR_LENGTH):
    if len(s) < MIN_LEN or len(s) > MAX_LEN:
        raise ValueError('The length of string:{} is not between [{}, {}]'.format(s, MIN_LEN, MAX_LEN))


def check_int_string(x: str, X_MIN: int = 0, X_MAX: int = INT_LIMIT.UINT64_MAX):
    check_str_length(x, MIN_LEN=1, MAX_LEN=LENGTH_LIMIT.MAX_UINT64_STR_LENGTH)
    if not x.isdigit():
        raise ValueError(f"{x} is an invalid positive int value")

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
        raise (
            f"The length of ipv4_string is over MAX_IPV4_LENGTH {LENGTH_LIMIT.MAX_IPV4_LENGTH}!"
        )
    if not is_regex_fullmatch(value, STRING_PATTERN.LEGAL_IPV4_PATTERN):
        raise ValueError(f"The format of ipv4_string:{value} is illegal!")
    return value


def check_bytes_format(value: str):
    if len(value) > LENGTH_LIMIT.MAX_BYTES_STR_LENGTH:
        raise (
            f"The length of bytes_string is over MAX_BYTES_STR_LENGTH {LENGTH_LIMIT.MAX_BYTES_STR_LENGTH}!"
        )
    if not is_regex_fullmatch(value, STRING_PATTERN.LEGAL_BYTES_FORMAT_PATTERN):
        raise ValueError(f"The format of bytes_string:{value} is illegal!")

    value_int = value[:-1]
    check_positive_int_string(value_int)
    return value_int


def check_linux_username(value: str):
    if len(value) > LENGTH_LIMIT.MAX_LINUX_USERNAME_LENGTH or len(value) <= 0:
        raise ValueError(
            'username: {} length must be in the range [1, {}]!'.format(value, LENGTH_LIMIT.MAX_LINUX_USERNAME_LENGTH))
    if not is_regex_fullmatch(value, STRING_PATTERN.LEGAL_LINUX_USERNAME_PATTERN):
        raise ValueError('username: {} is not valid!'.format(value))
    return value


def transform_hostfile_line(line: str):
    # todo, 是否需要改成正则匹配
    info_list = line.split(":")
    if len(info_list) < OTHERS.NODE_INFO_MIN_COUNT:
        raise ValueError(f"node_info line: {info_list} missing enough params!")
    if len(info_list) > OTHERS.NODE_INFO_MAX_COUNT:
        raise ValueError(f"node_info line: {info_list} too many params!")
    while len(info_list) < OTHERS.NODE_INFO_MAX_COUNT:
        info_list.append("")
    check_ipv4_string(info_list[0])  # ipv4 str
    check_positive_int_string(info_list[1])  # device_count, empty is legal
    info_list[1] = int(info_list[1])  # device_count,
    info_list[2] = info_list[2] if info_list[2] else "root"  # user, default is root
    check_linux_username(info_list[2])
    info_list[3] = check_positive_int_string(info_list[3]) if info_list[3] else 22  # port, default is 22
    return tuple(info_list)


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
