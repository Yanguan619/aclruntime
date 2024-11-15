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
import shutil
from ais_bench.net_test.common.consts import LENGTH_LIMIT

def is_disk_space_enough(path, need_size):
    _, _, free_space = shutil.disk_usage(path)
    if free_space < need_size:
        return False
    return True


def is_memory_enough(need_size):
    available_memory = os.sysconf('SC_PAGE_SIZE') * os.sysconf('SC_AVPHYS_PAGES')
    return available_memory >= need_size

def check_positive_integer_str(value):
    if value is None:
        return
    if not isinstance(value, str):
        raise ValueError(f"{value} is not a string")
    if not value:
        return
    if len(value) > LENGTH_LIMIT.MAX_UINT64_STR_LENGTH:
        raise ValueError(f"{value} is an invalid positive int value")
    if not value.isdigit():
        raise ValueError(f"{value} is an invalid positive int value")
    ivalue = int(value)
    if ivalue == 0:
        raise ValueError("%s is an invalid positive int value" % value)