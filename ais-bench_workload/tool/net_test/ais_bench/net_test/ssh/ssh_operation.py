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
import sys
import multiprocessing as mpc
import paramiko
import scp

from ais_bench.net_test.common.logger import logger
from ais_bench.net_test.common.consts import TIME_OUT, DEFAULT_ENV_SCRIPT_PATH
from ais_bench.net_test.sub_module.base_sub_module import NodeInfo
from ais_bench.net_test.security.file_checker import check_linux_path_format
from ais_bench.net_test.security.other_checker import check_linux_file_stat_string_from_shell


def console_origin(line):
    sys.stdout.write(line)
    sys.stdout.flush()


def ssh_client_connect(ssh_client, node_info: NodeInfo, ssh_key_path: str = ""):
    ssh_client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    ip = node_info.ip
    port = node_info.port
    user = node_info.user
    if ssh_key_path:
        try:
            ssh_client.connect(ip, port=int(port), username=user, key_filename=ssh_key_path)
        except Exception as err:
            ssh_client.close()
            raise RuntimeError(f"ssh connect use ssh key: {ssh_key_path}, server:{ip} port:{port} failed!") from err
    else:
        raise FileExistsError(f"ssh_key_path not offered, can not connect to nodes")


def remote_exec_file_check(file_path: str, node_info: NodeInfo, ssh_key_path: str = ""):
    ssh_client = paramiko.SSHClient()
    ssh_client_connect(ssh_client, node_info, ssh_key_path)
    actual_path = file_path.replace(" ","")

    if len(actual_path) == 0:
        raise ValueError("file path is empty!")
    check_linux_path_format(actual_path)

    get_file_info_cmd = f"ls -l {actual_path}"
    try:
        _, stdout, stderr = ssh_client.exec_command(get_file_info_cmd, bufsize=1,
            timeout=TIME_OUT.NORMAL_SSH_EXEC_TIMEOUT)
    except Exception as err:
        ssh_client.close()
        raise ValueError(f"user:{node_info.user}, server_ip:{node_info.ip}, " +
            f"port:{node_info.port} exec command:{get_file_info_cmd} failed!") from err
    error_str = stderr.read().decode("utf-8")
    if error_str:
        raise ValueError(f"remote check file failed! error log: {error_str}")

    result = stdout.readlines()
    if len(result) > 0:
        file_info = result[0].split()
        check_linux_file_stat_string_from_shell(file_info, node_info.user, file_path == DEFAULT_ENV_SCRIPT_PATH)
    ssh_client.close()


def remote_exec(node_id: int, node_info: NodeInfo, cmd: str, ssh_key_path: str = ""):
    logger.debug(f"node_id:{node_id}, server:{node_info.ip} remote_exec start")
    ssh_client = paramiko.SSHClient()
    ssh_client_connect(ssh_client, node_info, ssh_key_path)
    try:
        _, stdout, stderr = ssh_client.exec_command(cmd, bufsize=1, timeout=TIME_OUT.NORMAL_SSH_EXEC_TIMEOUT)
    except Exception as err:
        ssh_client.close()
        raise RuntimeError(f"user:{node_info.user}, server_ip:{node_info.ip}, " +
            f"port:{node_info.port} exec command:{cmd} failed!") from err
    for line in iter(lambda: stdout.readline(2048), ""):
        console_origin(line)
    error_str = stderr.read().decode("utf-8")
    if error_str:
        if "WARNING" in error_str:
            logger.warning(f"command:{cmd} exec in user:{node_info.user}, server:{node_info.ip}, " +
             f"port:{node_info.port} get some error log from node: {error_str}")
        else:
            raise RuntimeError(f"command:{cmd} exec in user:{node_info.user}, server:{node_info.ip}, " +
                f"port:{node_info.port} failed, error log from node: {error_str}")
    logger.debug(f"node_id:{node_id}, server:{node_info.ip} remote_exec end")
    ssh_client.close()


def remote_put(node_id: int, node_info: NodeInfo, src_path: str, dst_path: str, ssh_key_path: str = ""):
    logger.debug(f"node_id: {node_id} remote_put start")
    ssh_client = paramiko.SSHClient()
    ssh_client_connect(ssh_client, node_info, ssh_key_path)
    try:
        trans_client = scp.SCPClient(ssh_client.get_transport())
    except Exception as err:
        ssh_client.close()
        raise RuntimeError(f"user:{node_info.user}, server:{node_info.ip} " + \
             f"port:{node_info.port} open trans_client failed") from err
    try:
        trans_client.put(src_path, dst_path, recursive=True)
    except Exception as err:
        raise RuntimeError(f"user:{node_info.user}, server:{node_info.ip} port:{node_info.port} scp put " +
            f"src_path: {src_path} to dst_path: {dst_path} failed") from err
    finally:
        trans_client.close()
        ssh_client.close()
    logger.info(f"node_id: {node_id} single_put end")