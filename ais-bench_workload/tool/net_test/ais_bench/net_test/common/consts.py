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
import sys
import platform

DEFAULT_ENV_SCRIPT_PATH = "/usr/local/Ascend/ascend-toolkit/set_env.sh"
DEFAULT_SSH_KEY_PATH = "/root/.ssh/id_rsa"

class PACKAGE_INFO:
    version = "1.0.0"


class SUB_MODULE_NAME:
    RUN = "run"  # 正常运行模式
    INSTALL = "install"  # 分布式安装模式


class RUN_MODE_NAME:
    FULL = "full"


class REMOTE_NODE_INFO_NAME:
    NODE_ID = "node_id"
    NODE_INFO = "node_info"
    CMD = "cmd"
    SRC_PATH = "src_path"
    DST_PATH = "dst_path"
    SSH_KEY_PATH = "ssh_key_path"


class INT_LIMIT:
    PORT_MAX = 65535  # linux端口最大取值限制
    UINT64_MAX = (1 << 64) - 1


class LENGTH_LIMIT:
    MAX_IPV4_LENGTH = 15  # ipv4 格式IP最长限制
    MAX_PORT_STR_LENGTH = 6  # port 字符串长度限制
    MAX_UINT64_STR_LENGTH = 20  # 2^64的数字字符串长度
    MAX_UINT32_STR_LENGTH = 10  # 2^64的数字字符串长度
    MAX_BYTES_STR_LENGTH = MAX_UINT32_STR_LENGTH + 1
    MIN_UINT_STR_LENGTH = 1
    MIN_BYTES_STR_LENGTH = 2
    MAX_LINUX_USERNAME_LENGTH = 30  # linux 用户名长度限制


class STRING_PATTERN:
    LEGAL_IPV4_PATTERN = r'^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$'
    GET_DEVICE_COUNT_PER_NPU_PATTERN = r'Chip Count\s*:\s*(\d+)'
    LEGAL_LINUX_USERNAME_PATTERN = r'^[a-zA-Z_]([a-zA-Z0-9_-]*[a-zA-Z0-9_]+)*$'
    LEGAL_BYTES_FORMAT_PATTERN = r'^[0-9]{1,30}(k|m|g|K|M|G)$'


class OTHERS:
    DEFAULT_DEVICE_COUNT_PER_NODE = 8
    NODE_INFO_MIN_COUNT = 2
    NODE_INFO_MAX_COUNT = 4
    DAVINCI_DEVICE_PATTERN = r'davinci(\d+)'
    MAX_HOST_LINE_COUNT = 1000_0000


class RET:
    SUCCESS = 0
    FAILED = 1


class TIME_OUT:
    NORMAL_SSH_EXEC_TIMEOUT = 6000


class DEFAULT:
    NPUS = 8


OP_TASK = [
    "all_reduce_test", "all_gather_test", "alltoall_test", "alltoallv_test",
    "broadcast_test", "reduce_scatter_test", "reduce_test", "scatter_test",
]

OP_CMD_HELP_INFO = "[-b,--minbytes <min size in bytes>] \n\t" + \
                   "[-e,--maxbytes <max size in bytes>] \n\t" + \
                   "[-i,--stepbytes <increment size>] \n\t" + \
                   "[-f,--stepfactor <increment factor>] \n\t" + \
                   "[-n,--iters <iteration count>] \n\t" + \
                   "[-o,--op <sum/prod/min/max>] \n\t" + \
                   "[-d,--datatype <int8/int16/int/fp16/fp32/int64/uint64/uint8/uint16/uint32/fp64/bfp16>] \n\t" + \
                   "[-r,--root <root>] \n\t" + \
                   "[-w,--warmup_iters <warmup iteration count>] \n\t" + \
                   "[-c,--check <result verification> 0:disabled 1:enabled.] \n\t" + \
                   "[-p,--npus <npus used for one node>] \n\t"

DEFAULT_WHL_PATH = f"ais_bench_net_test-{PACKAGE_INFO.version}-py3-none-{sys.platform}_{platform.machine()}.whl"