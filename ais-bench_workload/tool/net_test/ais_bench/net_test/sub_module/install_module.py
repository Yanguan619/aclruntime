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
import platform
import argparse
from collections import namedtuple
from abc import abstractmethod, ABCMeta
from ais_bench.net_test.sub_module.base_sub_module import BaseSubmodule
from ais_bench.net_test.common.utils import multiprocess_run, get_ip_address
from ais_bench.net_test.common.logger import logger
from ais_bench.net_test.common.consts import REMOTE_NODE_INFO_NAME, DEFAULT_WHL_PATH
from ais_bench.net_test.ssh.ssh_operation import remote_put, remote_exec, remote_exec_file_check
from ais_bench.net_test.common.args_check import arg_check_whl_legalty


def remote_install_whl_pkg(args_dict):
    """
        args_dict: (node_id, node_info, cmd, ssh_key_path),
    """
    logger.info(f"node id:{args_dict[REMOTE_NODE_INFO_NAME.NODE_ID]}, " + \
        f"server ip:{args_dict[REMOTE_NODE_INFO_NAME.NODE_INFO].ip} start installing...")
    logger.debug(f"All node related info: {args_dict}")
    env_path = args_dict.get(REMOTE_NODE_INFO_NAME.CMD).split(";")[0].split()[1]
    remote_exec_file_check(
        env_path,
        args_dict.get(REMOTE_NODE_INFO_NAME.NODE_INFO),
        args_dict.get(REMOTE_NODE_INFO_NAME.SSH_KEY_PATH)
    )
    remote_exec(
        args_dict.get(REMOTE_NODE_INFO_NAME.NODE_ID),
        args_dict.get(REMOTE_NODE_INFO_NAME.NODE_INFO),
        args_dict.get(REMOTE_NODE_INFO_NAME.CMD),
        args_dict.get(REMOTE_NODE_INFO_NAME.SSH_KEY_PATH)
    )


def remote_deploy_whl_pkg(args_dict):
    """
        args_dict: (node_id, node_info, src_path, dst_path, ssh_key_path),
    """
    logger.info(f"node id:{args_dict[REMOTE_NODE_INFO_NAME.NODE_ID]}, " + \
        f"server ip:{args_dict[REMOTE_NODE_INFO_NAME.NODE_INFO].ip} start deploying...")
    logger.debug(f"All node related info: {args_dict}")
    remote_put(
        args_dict.get(REMOTE_NODE_INFO_NAME.NODE_ID),
        args_dict.get(REMOTE_NODE_INFO_NAME.NODE_INFO),
        args_dict.get(REMOTE_NODE_INFO_NAME.SRC_PATH),
        args_dict.get(REMOTE_NODE_INFO_NAME.DST_PATH),
        args_dict.get(REMOTE_NODE_INFO_NAME.SSH_KEY_PATH)
    )


class InstallModule(BaseSubmodule):
    def __init__(self, name) -> None:
        super().__init__(name)

    def add_sub_arguments(self, subparsers):
        self.parser = subparsers.add_parser(
            self.name,
            help=f"install whl pkg for other nodes.",
            usage="%(prog)s [optional arguments]",
        )
        super().add_base_arguments()
        self.parser.add_argument(
            "--pip",
            default="pip3",
            choices=["pip3", "pip"],
            help="optional, default pip3, pip interpreter"
        )
        self.parser.add_argument(
            "--whl_pkg_path",
            "-wp",
            type=arg_check_whl_legalty,
            default=DEFAULT_WHL_PATH,
            help=f"optional, default {DEFAULT_WHL_PATH}, the path of whl package"
        )
        self.parser.add_argument(
            '--force-reinstall',
            "-fr",
            action="store_true",
            help="whether --force-reinstall *.whl(do not reinstall dependencies) for nodes"
        )

    def exec(self, args):
        self._init_before_exec(args)
        self._erase_self_node_info()
        if len(self.hostfile_info) > 0:
            self._deploy(args)
            self._install(args)
            logger.info(f"install whl pkg:{args.whl_pkg_path} for all nodes success!")
        else:
            logger.warning(f"hostfile do not contain other node, won't install!")

    def _erase_self_node_info(self):
        self_ip = get_ip_address()
        for key, node_info in self.hostfile_info.items():
            if (node_info.ip == self_ip):
                self.hostfile_info.pop(key)
                return

    def _init_before_exec(self, args):
        self.arg_adapter.set_all_args_dict(args)
        self._get_hostfile_content(args)

    def _deploy(self, args):
        args_dict_list = self._gen_deploy_args_dict_list(args)
        multiprocess_run(remote_deploy_whl_pkg, args_dict_list)

    def _gen_deploy_args_dict_list(self, args):
        args_dict_list = []
        src_path = os.path.abspath(args.whl_pkg_path)
        dst_path = "./"
        for node_id, node_info in self.hostfile_info.items():
            args_dict = {
                REMOTE_NODE_INFO_NAME.NODE_ID: node_id,
                REMOTE_NODE_INFO_NAME.NODE_INFO: node_info,
                REMOTE_NODE_INFO_NAME.SRC_PATH: src_path,
                REMOTE_NODE_INFO_NAME.DST_PATH: dst_path,
                REMOTE_NODE_INFO_NAME.SSH_KEY_PATH: args.ssh_key_path,
            }
            args_dict_list.append(args_dict)
        return args_dict_list

    def _install(self, args):
        args_dict_list = self._gen_install_args_dict_list(args)
        multiprocess_run(remote_install_whl_pkg, args_dict_list)

    def _gen_install_cmd(self, args):
        pkg_name = os.path.basename(args.whl_pkg_path)
        cmd = f"{args.pip} install {pkg_name}"

        if args.force_reinstall:
            cmd = cmd + " --no-deps --force-reinstall"

        cmd = cmd + f";rm -f {pkg_name}" # delete tmp whl pkg
        cmd = "umask 0022;" + cmd # limit remote installed bin file permission
        cmd = f"source {args.env_script_path};" + cmd
        return cmd

    def _gen_install_args_dict_list(self, args):
        args_dict_list = []
        for node_id, node_info in self.hostfile_info.items():
            args_dict = {
                REMOTE_NODE_INFO_NAME.NODE_ID: node_id,
                REMOTE_NODE_INFO_NAME.NODE_INFO: node_info,
                REMOTE_NODE_INFO_NAME.CMD: self._gen_install_cmd(args),
                REMOTE_NODE_INFO_NAME.SSH_KEY_PATH: args.ssh_key_path,
            }
            args_dict_list.append(args_dict)
        return args_dict_list

