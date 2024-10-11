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

    def get_all_args_dict(self):
        args_dict = {}
        args_dict.update({'--server_ip': self.server_ip})
        args_dict.update({'--server_port': self.server_port})
        args_dict.update({'--rank_size':self.rank_size})
        args_dict.update({'--node_id': self.node_id})
        args_dict.update({'--op_task': self.op_task})
        args_dict.update({'--npus': self.npus})
        args_dict.update({'--minbytes': self.minbytes})
        args_dict.update({'--maxbytes': self.maxbytes})
        args_dict.update({'--stepbytes': self.stepbytes})
        args_dict.update({'--stepfactor': self.stepfactor})
        args_dict.update({'--op': self.op})
        args_dict.update({'--root': self.root})
        args_dict.update({'--datatype': self.datatype})
        args_dict.update({'--iters': self.iters})
        args_dict.update({'--warmup_iters': self.warmup_iters})
        args_dict.update({'--check': self.check})
        return args_dict

    def get_rank_related_args_dict(self):
        args_dict = {}
        args_dict.update({'--server_ip': self.server_ip})
        args_dict.update({'--server_port': self.server_port})
        args_dict.update({'--rank_size': self.rank_size})
        args_dict.update({'--npus': self.npus})
        args_dict.update({'--minbytes': self.minbytes})
        args_dict.update({'--maxbytes': self.maxbytes})
        args_dict.update({'--stepbytes': self.stepbytes})
        args_dict.update({'--stepfactor': self.stepfactor})
        args_dict.update({'--op': self.op})
        args_dict.update({'--root': self.root})
        args_dict.update({'--datatype': self.datatype})
        args_dict.update({'--iters': self.iters})
        args_dict.update({'--warmup_iters': self.warmup_iters})
        args_dict.update({'--check': self.check})
        return args_dict