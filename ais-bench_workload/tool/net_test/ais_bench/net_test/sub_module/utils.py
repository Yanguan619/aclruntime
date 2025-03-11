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
from ais_bench.net_test.common.consts import RemoteNodeInfoName
from ais_bench.net_test.ssh.ssh_operation import remote_exec_file_check


def remote_run_env_check(args_dict):
    if (args_dict.get(RemoteNodeInfoName.CMD).split(";")[0].split()[0] == "source"):
        env_path = args_dict.get(RemoteNodeInfoName.CMD).split(";")[0].split()[1]
        remote_exec_file_check(
            env_path,
            args_dict.get(RemoteNodeInfoName.NODE_INFO),
            args_dict.get(RemoteNodeInfoName.SSH_KEY_PATH)
        )