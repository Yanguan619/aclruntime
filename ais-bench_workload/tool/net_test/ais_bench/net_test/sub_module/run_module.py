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
import argparse
import subprocess
from abc import abstractmethod, ABCMeta
from ais_bench.net_test.sub_module.base_sub_module import BaseSubmodule
from ais_bench.net_test.common.utils import multiprocess_run
from ais_bench.net_test.common.logger import logger
from ais_bench.net_test.ssh.ssh_operation import remote_exec, remote_exec_file_check
from ais_bench.net_test.common.consts import (RUN_MODE_NAME, REMOTE_NODE_INFO_NAME,
    OP_TASK, OP_CMD_HELP_INFO, RET, DEFAULT)
from ais_bench.net_test.common.args_check import (arg_check_positive_integer, arg_check_port_range)

class BaseRunMode(metaclass=ABCMeta):
    def __init__(self) -> None:
        self.name = ""
        self.hostfile_info = {}

    @abstractmethod
    def __call__(self, args, hostfile_info):
        raise NotImplementedError


class FullRun(BaseRunMode):
    def __init__(self) -> None:
        self.name = RUN_MODE_NAME.FULL
        self.hostfile_info = {}

    def __call__(self, args, hostfile_info: dict):
        self.hostfile_info = hostfile_info
        args_dict_list = self._gen_full_args_dict_list(args)
        try:
            self._run(args, args_dict_list)
        except (KeyboardInterrupt, RuntimeError) as err:
            logger.error(f"get some error in full run mode, error detail: {err}")
            self._clean_up(args, args_dict_list)

    def _run(self, args, args_dict_list):
        if not args.hostfile: # only root node run
            self._root_node_run_py_backend_cmd(args_dict_list[0].get(REMOTE_NODE_INFO_NAME.CMD))
        else:
            multiprocess_run(self._remote_run_py_backend_cmd, args_dict_list)

    def _clean_up(self, args, args_dict_list):
        clean_up_cmd = f"pkill -9 {args.op_cmds[0]}"
        if not args.hostfile: # only root node run
            self._root_node_run_py_backend_cmd(cmd=clean_up_cmd)
        else:
            for i, _ in enumerate(args_dict_list):
                args_dict_list[i][REMOTE_NODE_INFO_NAME.CMD] = f"pkill -9 {args.op_cmds[0]}"
            multiprocess_run(self._remote_run_py_backend_cmd, args_dict_list)

    def _gen_full_args_dict_list(self, args):
        args_dict_list = []
        for node_id, node_info in self.hostfile_info.items():
            args_dict = {
                REMOTE_NODE_INFO_NAME.NODE_ID: node_id,
                REMOTE_NODE_INFO_NAME.NODE_INFO: node_info,
                REMOTE_NODE_INFO_NAME.CMD: self._gen_cmd_per_node(node_id, args),
                REMOTE_NODE_INFO_NAME.SSH_KEY_PATH: args.ssh_key_path,
            }
            args_dict_list.append(args_dict)
        return args_dict_list

    def _refresh_npus_for_node(self, node_id, args):
        for i, arg in enumerate(args.op_cmds):
            if arg == "--npus":
                if i == len(args.op_cmds) - 1:
                    return
                if args.op_cmds[i + 1].isdigit():
                    args.op_cmds[i + 1] = self.hostfile_info.get(node_id).device_count
                else:
                    args.op_cmds.insert(i + 1, self.hostfile_info.get(node_id).device_count)

    def _gen_cmd_per_node(self, node_id, args) -> str:
        root_rank_node_ip = self.hostfile_info.get(0).ip
        self._refresh_npus_for_node(node_id, args) # 单个节点使用的npu以device_count为准
        cmd = f"{args.python} -m ais_bench.backend.net_test --server_ip {root_rank_node_ip} " + \
            f" --server_port {args.link_port} --rank_size {args.rank_size} --node_id {node_id} --op_task"
        for value in args.op_cmds:
            cmd = cmd + " " + value
        cmd = f"source {args.env_script_path};" + cmd # activate CANN dependency of nodes
        return cmd

    def _remote_run_py_backend_cmd(self, args_dict):
        """
            args_dict: (node_id, node_info, cmd, ssh_key_path),
        """
        logger.info(f"node id:{args_dict[REMOTE_NODE_INFO_NAME.NODE_ID]}, " + \
            f"server ip:{args_dict[REMOTE_NODE_INFO_NAME.NODE_INFO].ip} start running...")
        logger.debug(f"All node related info: {args_dict}")
        env_path = args_dict.get(REMOTE_NODE_INFO_NAME.CMD).split(";")[0].split()[1]
        remote_exec_file_check(
            env_path,
            args_dict.get(REMOTE_NODE_INFO_NAME.NODE_INFO),
            args_dict.get(REMOTE_NODE_INFO_NAME.SSH_KEY_PATH)
        )
        remote_exec(
            args_dict[REMOTE_NODE_INFO_NAME.NODE_ID],
            args_dict[REMOTE_NODE_INFO_NAME.NODE_INFO],
            args_dict[REMOTE_NODE_INFO_NAME.CMD],
            args_dict[REMOTE_NODE_INFO_NAME.SSH_KEY_PATH]
        )

    def _root_node_run_py_backend_cmd(self, cmd: str):
        cmd_list = cmd.split(";")[-1].split()
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
            raise RuntimeError(f"exec cmd {cmd_list} failed! error log: {stderr.decode('utf-8')}")


RUN_MODE_SWITCH = {
    RUN_MODE_NAME.FULL: FullRun,
}


class RunModeFactory:
    @staticmethod
    def get(module_name, **kwargs):
        if RUN_MODE_SWITCH.get(module_name.strip()) is not None:
            logger.debug(f"create module: {module_name} success!")
            return RUN_MODE_SWITCH.get(module_name.strip())(**kwargs)
        else:
            raise KeyError(f"sub module {module_name} is not supported. " + \
                         f"Currently only {', '.join(list(module_name.keys()))} are supported.")


class RunModule(BaseSubmodule):
    def __init__(self, name) -> None:
        super().__init__(name)
        self.run_mode_factory = RunModeFactory()

    @staticmethod
    def _get_npus_used_per_node(op_cmd_list: str):
        for i, value in enumerate(op_cmd_list):
            if i == len(op_cmd_list) - 1:
                return DEFAULT.NPUS
            if value == "-p" or value == "--npus":
                return int(op_cmd_list[i + 1])
        return DEFAULT.NPUS

    def add_sub_arguments(self, subparsers):
        self.parser = subparsers.add_parser(
            self.name,
            help=f"run net test",
            usage='%(prog)s [optional arguments] [op task] [op cmds]',
            epilog="op task:\n" + f"{OP_TASK}   " + "op cmd:\n" + OP_CMD_HELP_INFO,
        )
        super().add_base_arguments()
        # 运行任务选择
        self.parser.add_argument(
            "--rank_size",
            "-n",
            type=arg_check_positive_integer, # str
            default=8,
            help="optional, default 8, rank_size(number of devices in all nodes)"
        )
        self.parser.add_argument(
            "--link_port",
            "-lpt",
            type=arg_check_port_range, # int
            default=21345,
            help="optional, default 21345, port to share root info"
        )
        self.parser.add_argument(
            "--python",
            "-py",
            default="python3",
            choices=["python3", "python", "python3.7", "python3.8", "python3.9", "python3.10", "python3.11"],
            help="optional, default python3, python interpreter"
        )
        self.parser.add_argument(
            "--run_mode",
            "-rm",
            type=str,
            default=RUN_MODE_NAME.FULL,
            choices=[RUN_MODE_NAME.FULL],
            help="optional, default full, run mode, 'full' means run all nodes"
        )

    def exec(self, args):
        self._init_before_exec(args)
        run_mode_instance = self.run_mode_factory.get(args.run_mode)
        self._screen_hostfile_info(args)
        run_mode_instance(args, self.hostfile_info)

    def _init_before_exec(self, args):
        self.arg_adapter.set_all_args_dict(args)
        self._get_hostfile_content(args)

    def _screen_hostfile_info(self, args):
        npus_per_node = self._get_npus_used_per_node(args.op_cmds)
        if npus_per_node <= 0:
            raise ValueError("--npus is not a positive int!")
        if args.rank_size % npus_per_node != 0:
            raise ValueError("--rank_size is not a precise multiple of --npus!")

        activate_nodes_count = int(args.rank_size / npus_per_node)
        if activate_nodes_count > len(self.hostfile_info):
            raise ValueError("--rank_size is over (nodes count in host file) * (npus per node)!")

        screened_hostfile_info = {}
        for i in range(activate_nodes_count):
            if self.hostfile_info.get(i).device_count < npus_per_node:
                raise ValueError(f"the count of npus to launch in node: {i} is over permission in hostfile!")
            screened_hostfile_info[i] = self.hostfile_info.get(i)

        self.hostfile_info = screened_hostfile_info


