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
from ais_bench.net_test.sub_module.sub_module_factory import sub_module_switch, sub_module_factory
from ais_bench.net_test.common.consts import OP_TASK, SubModuleName
from ais_bench.net_test.ssh.ssh_operation import remote_exec

SUB_MODULE_LIST = [
    SubModuleName.RUN,
    SubModuleName.INSTALL,
]


def get_separated_argv():
    origin_args = sys.argv
    if origin_args[1] not in SUB_MODULE_LIST and origin_args[1] != "-h" and origin_args[1] != "--help":
        origin_args.insert(1, SubModuleName.RUN) # default run

    for i, arg in enumerate(origin_args):
        if arg in OP_TASK:
            return origin_args[0:i], origin_args[i:]
    return origin_args, []


def get_args():
    parser = argparse.ArgumentParser(description='ais_bench net_test tool')

    subparsers = parser.add_subparsers(dest='command', help='ais_bench net_test sub module, default \"run\"')
    sub_module_instances = {}
    for name, _ in sub_module_switch.items():
        sub_module_instance = sub_module_factory.get(name)
        sub_module_instance.add_sub_arguments(subparsers)
        sub_module_instances[name] = sub_module_instance

    self_args, op_args = get_separated_argv()
    args = parser.parse_args(self_args[1:])
    args.op_cmds = op_args
    # to do: op_cmds need to check
    return args, sub_module_instances


if __name__ == '__main__':
    args, sub_module_instances = get_args()
    sub_module_instances.get(args.command).exec(args)
