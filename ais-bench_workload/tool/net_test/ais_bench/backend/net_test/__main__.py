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

from ais_bench.backend.net_test.common.args_adapter import ArgsAdapter
from ais_bench.backend.net_test.common.get_args import get_args
from ais_bench.backend.net_test.lanuch_run_node import lanuch_run_node


if __name__ == "__main__":
    args = get_args()
    args = ArgsAdapter(args.server_ip, args.server_port, args.rank_size, args.node_id, args.op_task, args.npus,
            args.minbytes, args.maxbytes, args.stepbytes, args.stepfactor, args.op, args.root, args.datatype,
            args.iters, args.warmup_iters, args.check)
    ret = lanuch_run_node(args)
    exit(ret)