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

import argparse
from ais_bench.backend.net_test.common.args_check import (
    arg_check_ipv4_string, arg_check_port_range, arg_check_positive_integer,
    arg_check_nonnegative_integer, arg_check_bytes_format, arg_check_device_count_range,
    arg_check_device_id_range, combination_args_check
)
from ais_bench.net_test.common.consts import OP_TASK


def get_args():
    parser = argparse.ArgumentParser()

    # 集群信息
    parser.add_argument(
        "--server_ip",
        "-sip",
        type=arg_check_ipv4_string, # str
        required=True,
        help="server ip"
    )
    parser.add_argument(
        "--server_port",
        "-spt",
        type=arg_check_port_range, # int
        required=True,
        default=21345,
        help="server port"
    )
    parser.add_argument(
        "--rank_size",
        "-rs",
        type=arg_check_positive_integer, # int
        required=True,
        default=8,
        help="rank size"
    )
    parser.add_argument(
        "--node_id",
        "-nid",
        type=arg_check_nonnegative_integer,
        required=True,
        default=0,
        help="node id"
    )

    # 通信算子任务：
    parser.add_argument(
        "--op_task",
        "-otk",
        type=str,
        default="all_reduce_test",
        choices=OP_TASK,
        help="op task, support \"all_reduce_test\", \"all_gather_test\", \"alltoall_test\", \"alltoallv_test\", " +
            "\"broadcast_test\", \"reduce_scatter_test\", \"reduce_test\""
    )

    # 集合通信性能测试命令支持的参数
    parser.add_argument(
        "--npus",
        "-p",
        type=arg_check_device_count_range, # int
        required=True,
        help="npus used for one node"
    )
    parser.add_argument(
        "--minbytes",
        "-b",
        type=arg_check_bytes_format, # str
        default="64M",
        help="min size in bytes"
    )
    parser.add_argument(
        "--maxbytes",
        "-e",
        type=arg_check_bytes_format, # str
        default="64M",
        help="max size in bytes"
    )
    parser.add_argument(
        "--stepbytes",
        "-i",
        default=0,
        type=arg_check_nonnegative_integer, # int
        help="increment size"
    )
    parser.add_argument(
        "--stepfactor",
        "-f",
        type=arg_check_positive_integer, # int
        default=2,
        help="increment factor"
    )

    # HCCL操作参数
    parser.add_argument(
        "--op",
        "-o",
        type=str,
        default="na",
        choices=["na", "sum", "prod", "min", "max"],
        help="choose from sum/prod/min/max"
    )
    parser.add_argument(
        "--root",
        "-r",
        type=arg_check_device_id_range, # int
        default=0,
        help="device id of root node"
    )
    parser.add_argument(
        "--datatype",
        "-d",
        type=str,
        default="fp32",
        choices=["int8", "int16", "int", "fp16", "fp32", "int64", "uint64",
            "uint8", "uint16", "uint32", "fp64", "bfp16"],
        help="choose from int8/int16/int/fp16/fp32/int64/uint64/uint8/uint16/uint32/fp64/bfp16"
    )

    # 性能测试参数
    parser.add_argument(
        "--iters",
        "-n",
        type=arg_check_positive_integer, # int
        default=20,
        help="iteration count"
    )
    parser.add_argument(
        "--warmup_iters",
        "-w",
        type=arg_check_nonnegative_integer, # int
        default=5,
        help="warmup iteration count"
    )

    # 结果校验参数
    parser.add_argument(
        "--check",
        "-c",
        type=int,
        default=1,
        choices=[0, 1],
        help="result verification, 0:disabled 1:enabled."
    )
    args = parser.parse_args()
    # 参数中有部分耦合的关系需要校验
    combination_args_check(args)
    return args