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
import socket
from multiprocessing import Pool
from ais_bench.backend.net_test.common.consts import RET
from ais_bench.net_test.security.file_checker import check_linux_executable_file
from ais_bench.backend.net_test.common.logger import logger, console_origin

CUR_DIR_ABS_PATH = os.path.dirname(os.path.abspath(__file__))


def run_hccl_test_exec_command(cmd_list):
    p = subprocess.Popen(cmd_list, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    # 获取实时输出并处理
    for line in iter(p.stdout.readline, b''):
        if line:
            console_origin(line)

    try:
        _, stderr = p.communicate(timeout=10)
    except subprocess.TimeoutExpired as e:
        p.kill()
        raise TimeoutError(f"exec cmd {cmd_list} timeout!") from e

    # 等待命令执行完成
    return_code = p.wait()
    if return_code != RET.SUCCESS:
        raise RuntimeError(f"Cmd {cmd_list} failed! error log: {stderr.decode('utf-8')}")


def check_root_port_free(args):
    if args.node_id != 0:
        return RET.SUCCESS
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        try:
            s.bind((args.server_ip, args.server_port))
            return RET.SUCCESS
        except socket.error as e:
            logger.error(f"port: {args.server_port} is occupied!")
            return RET.FAILED


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
    p = Pool(process_count)
    task_failed = RET.SUCCESS

    def _callback(value):
        logger.error(f"subprocess run failed, error: {value}")
        p.terminate()
        task_failed = RET.FAILED

    for _, command_list in enumerate(command_lists):
        p.apply_async(run_hccl_test_exec_command, args=(command_list,), error_callback=_callback)
    p.close()
    p.join()

    return task_failed


def launch_run_node(args):
    if check_root_port_free(args) != RET.SUCCESS:
        return RET.FAILED
    command_lists = construct_command_lists(args)
    ret = multiprocess_run(args.npus, command_lists)
    return ret