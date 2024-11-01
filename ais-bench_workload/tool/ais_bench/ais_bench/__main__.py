# Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


import os
from ais_bench.infer.utils import get_args, check_int_args_max_limit
from ais_bench.infer.infer_process import infer_process
from ais_bench.infer.args_adapter import AISBenchInferArgsAdapter


if __name__ == "__main__":
    args = get_args()
    check_int_args_max_limit(args)

    args = AISBenchInferArgsAdapter(args.model, args.input, args.output,
                args.output_dirname, args.outfmt, args.loop, args.debug, args.device,
                args.dym_batch, args.dym_hw, args.dym_dims, args.dym_shape, args.output_size,
                args.auto_set_dymshape_mode, args.auto_set_dymdims_mode, args.batchsize, args.pure_data_type,
                args.profiler, args.dump, args.acl_json_path, args.output_batchsize_axis, args.run_mode,
                args.display_all_summary, args.warmup_count, args.dym_shape_range, args.aipp_config,
                args.energy_consumption, args.npu_id, args.backend, args.perf, args.pipeline, args.profiler_rename,
                args.dump_npy, args.divide_input, args.threads)
    ret = infer_process(args)
    exit(ret)