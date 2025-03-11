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

class ArgsAdapter():
    def __init__(self, server_ip, server_port, rank_size, node_id, op_task, npus,
            minbytes, maxbytes, stepbytes, stepfactor, op, root, datatype,
            iters, warmup_iters, check):
        self.server_ip = server_ip
        self.server_port = server_port
        self.rank_size = rank_size
        self.node_id = node_id
        self.op_task = op_task
        self.npus = npus
        self.minbytes = minbytes
        self.maxbytes = maxbytes
        self.stepbytes = stepbytes
        self.stepfactor = stepfactor
        self.op = op
        self.root = root
        self.datatype = datatype
        self.iters = iters
        self.warmup_iters = warmup_iters
        self.check = check
        self.all_args_fields = [
            '--server_ip', '--server_port', '--rank_size', '--node_id', 
            '--op_task', '--npus', '--minbytes', '--maxbytes', 
            '--stepbytes', '--stepfactor', '--op', '--root', 
            '--datatype', '--iters', '--warmup_iters', '--check'
        ]
        self.rank_related_fields = [
            '--server_ip', '--server_port', '--rank_size', 
            '--npus', '--minbytes', '--maxbytes', 
            '--stepbytes', '--stepfactor', '--op', 
            '--root', '--datatype', '--iters', 
            '--warmup_iters', '--check'
        ]

    def get_all_args_dict(self):
        return self._build_args_dict(self.all_args_fields) 

    def get_rank_related_args_dict(self):
        return self._build_args_dict(self.rank_related_fields)

    def _build_args_dict(self, fields):
        args_dict = {}
        for field in fields:
            args_dict[field] = getattr(self, field[2:])
        return args_dict