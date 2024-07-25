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
import logging

import aclruntime
import numpy as np
import pytest
from test_common import TestCommonClass
from ais_bench.infer.__main__ import get_args
from ais_bench.infer.args_adapter import AISBenchInferArgsAdapter

logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='[%(levelname)s] %(message)s')
logger = logging.getLogger(__name__)

ARGS_VALUE_DICT = {
    "--model": "xx.om",
    "--input": "xx.bin",
    "--output": "xx/",
    "--output_dirname": "xx/",
    "--outfmt": "NPY",
    "--loop": "1",
    "--debug": "y",
    "--device": "1",
    "--dymBatch": "2",
    "--dymHW": "a:1,2",
    "--dymDims": "a:1,2,3",
    "--dymShape": "a:2,3",
    "--outputSize": "1,2",
    "--auto_set_dymshape_mode": "1",
    "--auto_set_dymdims_mode": "1",
    "--batchsize": "1",
    "--pure_data_type": "zero",
    "--profiler": "1",
    "--dump": "1",
    "--acl_json_path": "xx.json",
    "--output_batchsize_axis": "2",
    "--run_mode": "tensor",
    "--display_all_summary": "1",
    "--warmup_count": "1",
    "--dymShape_range": "a:1-2,2~3",
    "--aipp_config": "xx.config",
    "--energy_consumption": "1",
    "--npu_id": "12",
    "--backend": "trtexec",
    "--perf": "1",
    "--pipeline": "1",
    "--profiler_rename": "1",
    "--dump_npy": "1",
    "--divide_input": "1",
    "--threads": "2",
}


class TestClass:
    @classmethod
    def setup_class(cls):
        """
        class level setup_class
        """
        cls.init(TestClass)

    @classmethod
    def teardown_class(cls):
        logger.info('\n ---class level teardown_class')

    def init(self, monkeypatch):
        monkeypatch.setattr("ais_bench.infer.args_check.check_dym_string", lambda value: value)
        monkeypatch.setattr("ais_bench.infer.args_check.check_dym_range_string", lambda value: value)
        monkeypatch.setattr("ais_bench.infer.args_check.check_number_list", lambda value: value)
        monkeypatch.setattr("ais_bench.infer.args_check.str2bool.check_batchsize_valid", lambda value: value)
        monkeypatch.setattr("ais_bench.infer.args_check.check_nonnegative_integer", lambda value: value)
        monkeypatch.setattr("ais_bench.infer.args_check.check_npu_id_range_vaild", lambda value: value)
        monkeypatch.setattr("ais_bench.infer.args_check.check_device_range_valid", lambda value: value)
        monkeypatch.setattr("ais_bench.infer.args_check.check_om_path_legality", lambda value: value)
        monkeypatch.setattr("ais_bench.infer.args_check.check_input_path_legality", lambda value: value)
        monkeypatch.setattr("ais_bench.infer.args_check.check_output_path_legality", lambda value: value)
        monkeypatch.setattr("ais_bench.infer.args_check.check_acl_json_path_legality", lambda value: value)
        monkeypatch.setattr("ais_bench.infer.args_check.check_aipp_config_path_legality", lambda value: value)
        monkeypatch.setattr("ais_bench.infer.args_check.check_dym_string", lambda value: value)



    def test_argparser_args(self, monkeypatch):
        for arg_name, value in ARGS_VALUE_DICT.items():
            monkeypatch.setattr('sys.argv', ["python3", arg_name, value])
            monkeypatch.setattr(f'ais_bench.infer.__main__', lambda *args, **kwargs: (args, kwargs))
            args = get_args()
            args = AISBenchInferArgsAdapter(args.model, args.input, args.output,
                args.output_dirname, args.outfmt, args.loop, args.debug, args.device,
                args.dym_batch, args.dym_hw, args.dym_dims, args.dym_shape, args.output_size,
                args.auto_set_dymshape_mode, args.auto_set_dymdims_mode, args.batchsize, args.pure_data_type,
                args.profiler, args.dump, args.acl_json_path, args.output_batchsize_axis, args.run_mode,
                args.display_all_summary, args.warmup_count, args.dym_shape_range, args.aipp_config,
                args.energy_consumption, args.npu_id, args.backend, args.perf, args.pipeline, args.profiler_rename,
                args.dump_npy, args.divide_input, args.threads)

            assert args.get_all_args_dict().get(arg_name) == value