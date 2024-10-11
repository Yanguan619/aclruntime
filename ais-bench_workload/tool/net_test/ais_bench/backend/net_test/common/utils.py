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

import re
import subprocess

from ais_bench.backend.net_test.common.consts import STRING_PATTERN, OTHERS
from ais_bench.backend.net_test.common.logger import logger


def get_actual_device_count():
    npu_smi_cmd_list = ["ls", "/dev/"]
    try:
        result = subprocess.run(npu_smi_cmd_list, capture_output=True, text=True)
    except subprocess.CalledProcessError as e:
        logger.warning(f"Command '{e.cmd}' returned non-zero exit status {e.returncode}.")
        if e.stderr:
            logger.warning(f"Standard Error Output: {e.stderr}")
            logger.warning(f"get_actual_device_count return default count: {OTHERS.DEFAULT_DEVICE_COUNT_PER_NODE}")
        return OTHERS.DEFAULT_DEVICE_COUNT_PER_NODE
    davinci_list = result.stdout.split()
    chip_counts = 0
    for davinci_part in davinci_list:
        if re.compile(OTHERS.DAVINCI_DEVICE_PATTERN).match(davinci_part):
            chip_counts += 1

    return chip_counts


def compare_bytes_string(min_bytes, max_bytes):
    suffix_dict = {
        "K": 1,
        "M": 1024,
        "G": 1024 * 1024,
    }
    num_min = int(min_bytes[:-1]) if int(min_bytes[:-1]) else 1
    num_max = int(max_bytes[:-1])
    min_suffix_k_bytes = suffix_dict.get(min_bytes[-1])
    max_suffix_k_bytes = suffix_dict.get(max_bytes[-1])

    if (num_max / num_min) * (max_suffix_k_bytes / min_suffix_k_bytes) >= 1:
        return True
    return False
