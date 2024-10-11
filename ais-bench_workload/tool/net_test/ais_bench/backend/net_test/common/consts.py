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

class INT_LIMIT:
    PORT_MAX = 65535 # linux端口最大取值限制


class LENGTH_LIMIT:
    MAX_IPV4_LENGTH = 15 # ipv4 格式IP最长限制
    MAX_PORT_STR_LENGTH = 6 # port 字符串长度限制
    MAX_UINT64_STR_LENGTH = 20 # 2^64的数字字符串长度
    MAX_UINT32_STR_LENGTH = 10 # 2^64的数字字符串长度
    MAX_BYTES_STR_LENGTH = MAX_UINT32_STR_LENGTH + 1
    MIN_BYTES_STR_LENGTH = 2


class STRING_PATTERN:
    LEGAL_IPV4_PATTERN = r'^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$'
    GET_DEVICE_COUNT_PER_NPU_PATTERN = r'Chip Count\s*:\s*(\d+)'


class OTHERS:
    DEFAULT_DEVICE_COUNT_PER_NODE = 8
    DAVINCI_DEVICE_PATTERN = r'davinci(\d+)'

class RET:
    SUCCESS = 0
    FAILED = 1