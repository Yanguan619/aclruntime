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
import argparse

from abc import abstractmethod, ABCMeta

from ais_bench.net_test.common.args_adapter import BaseArgsAdapter
from ais_bench.net_test.security.other_checker import check_positive_integer_str
from ais_bench.net_test.common.consts import OTHERS, DEFAULT_SSH_KEY_PATH, DEFAULT_ENV_SCRIPT_PATH
from ais_bench.net_test.common.args_check import (arg_check_hostfile_legalty, arg_check_ssh_key_path_legalty)
from ais_bench.net_test.common.utils import get_actual_device_count, get_ip_address, get_user_name, get_default_port
from ais_bench.net_test.security.file_stat import ms_open


class NodeInfo:
    def __init__(self, ip, device_count, user, port):
        self.ip = ip
        self.device_count = device_count
        self.user = user
        self.port = port


class BaseSubmodule(metaclass=ABCMeta):
    def __init__(self, name) -> None:
        self.name = name
        self.hostfile_info = {} # {node_id: NodeInfo}
        self.arg_adapter = BaseArgsAdapter()

    @staticmethod
    def _transform_hostfile_line(line):
        info_list = line.split(":")
        if len(info_list) < OTHERS.NODE_INFO_MIN_COUNT:
            raise ValueError(f"node_info line: {info_list} missing enough params!")
        while len(info_list) < OTHERS.NODE_INFO_MAX_COUNT:
            info_list.append("")
        check_positive_integer_str(info_list[1]) # device_count, empty is legal
        check_positive_integer_str(info_list[3]) # port, empty is legal
        info_list[1] = int(info_list[1]) # device_count,
        info_list[2] = info_list[2] if info_list[2] else "root" # user, default is root
        info_list[3] = int(info_list[3]) if info_list[3] else 22 # port, default is 22
        return tuple(info_list)

    @abstractmethod
    def add_sub_arguments(self, subparsers):
        pass

    def add_base_arguments(self):
        self.parser.add_argument(
            "--hostfile",
            "-f",
            type=arg_check_hostfile_legalty, # int
            help="required, npus used for one node"
        )
        self.parser.add_argument(
            "--ssh_key_path",
            "-skp",
            type=arg_check_ssh_key_path_legalty, # str
            help="optional, default /root/.ssh/id_rsa, ssh key path"
        )
        self.parser.add_argument(
            "--env_script_path",
            "-esp",
            type=str,
            default=DEFAULT_ENV_SCRIPT_PATH,
            help="optional, default /usr/local/Ascend/ascend-toolkit/set_env.sh, path of shell scripts for set env"
        )

    @abstractmethod
    def exec(self, args):
        pass

    @abstractmethod
    def _init_before_exec(self, args):
        pass

    def _get_hostfile_content(self, args):
        if not args.hostfile:
            self.hostfile_info[0] = NodeInfo(
                get_ip_address(),
                get_actual_device_count(),
                get_user_name(),
                get_default_port(),
            )
            return
        else:
            if args.ssh_key_path is None:
                args.ssh_key_path = DEFAULT_SSH_KEY_PATH
                arg_check_ssh_key_path_legalty(DEFAULT_SSH_KEY_PATH)

        with ms_open(args.hostfile, 'r') as file:
            count = 0
            ip_list = []
            for line in file:
                stripped_line = line.strip()
                node_info = NodeInfo(*self._transform_hostfile_line(stripped_line))
                if node_info.ip not in ip_list:
                    ip_list.append(node_info.ip)
                else:
                    raise ValueError(f"in hostfile, line:{count + 1}, get repeated node ip:{node_info.ip}, please check")
                self.hostfile_info[count] = node_info
                count += 1