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
import stat
import sys
import subprocess
from multiprocessing import Pool
from ais_bench.backend.net_test.common.consts import RET
from ais_bench.net_test.security.file_checker import check_linux_executable_file
from ais_bench.backend.net_test.common.logger import logger

CUR_DIR_ABS_PATH = os.path.dirname(os.path.abspath(__file__))


def run_hccl_test_exec_command(cmd_list):
    p = subprocess.Popen(cmd_list, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    # 获取实时输出并处理
    for line in iter(p.stdout.readline, b''):
        if line:
            sys.stdout.write(line.decode('utf-8'))
            sys.stdout.flush()

    _, stderr = p.communicate()

    # 等待命令执行完成
    return_code = p.wait()
    if return_code != RET.SUCCESS:
        return RET.FAILED, f"Cmd {cmd_list} failed! error log: {stderr.decode('utf-8')}"
    return RET.SUCCESS, ""


def generate_rank_id_list(args):
    rank_id_list = []
    for i in range(args.npus):
        rank_id_list.append(args.node_id * args.npus + i)
    return rank_id_list


def get_rank_related_cmd_list(args):
    sub_cmd_list = []
    for op_cmd, value in args.get_rank_related_args_dict().items():
        if (op_cmd == "--stepbytes" and value == 0):
            continue
        if (op_cmd == "--op" and value == "na"):
            continue
        sub_cmd_list.append(op_cmd)
        sub_cmd_list.append(f"{value}")
    return sub_cmd_list


def construct_command_lists(args):
    exec_file_path = os.path.join(CUR_DIR_ABS_PATH, f"hccl_test/bin/{args.op_task}")
    check_linux_executable_file(exec_file_path, perm_forbid=stat.S_IWGRP | stat.S_IWOTH)  # 暂时想不到如何限制安装后的bin文件权限，只能先755了
    rank_id_list = generate_rank_id_list(args)
    cmd_lists = []
    for rank_id in rank_id_list:
        cmd_list = [exec_file_path, "--rank_id", f"{rank_id}"]
        cmd_list.extend(get_rank_related_cmd_list(args))
        cmd_lists.append(cmd_list)
    return cmd_lists


def multiprocess_run(process_count, command_lists):
    # 创建一个进程池
    with Pool(processes=process_count) as pool:  # 可以指定进程数量，默认为CPU核心数
        # 使用进程池映射命令到 run_command 函数
        results = pool.map(run_hccl_test_exec_command, command_lists)
    return results


def parse_result(results, args):
    rank_id_list = generate_rank_id_list(args)
    for device_id, result in enumerate(results):
        if (result[0] != RET.SUCCESS):
            logger.error(f"rank_id:{rank_id_list[device_id]}, device id:{device_id}, run failed! error info:{result[1]}")
            return result[0]
        if result[1]:
            print(result[1])
    return RET.SUCCESS

def launch_run_node(args):
    command_lists = construct_command_lists(args)
    results = multiprocess_run(args.npus, command_lists)
    ret = parse_result(results, args)
    return ret