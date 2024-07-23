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

# lzy
import json
import sys
import logging
import os
import pytest
from ais_bench.infer.args_adapter import AISBenchInferArgsAdapter
from ais_bench.infer.args_check import (
    check_dym_string,
    check_dym_range_string,
    check_number_list,
    str2bool,
    check_batchsize_valid,
    check_nonnegative_integer,
    check_npu_id_range_vaild,
    check_device_range_valid,
    check_om_path_legality,
    check_input_path_legality,
    check_output_path_legality,
    check_acl_json_path_legality,
    check_aipp_config_path_legality,
)

logging.basicConfig(
    stream=sys.stdout, level=logging.INFO, format="[%(levelname)s] %(message)s"
)


class TestClass:

    @classmethod
    def get_args(cls):
        current_directory = os.getcwd()
        model = os.path.join(
            current_directory, "testdata/resnet50/model/pth_resnet50_bs1.om"
        )
        input_path = os.path.join(current_directory, "testdata/resnet50/input")
        output = os.path.join(current_directory, "testdata/resnet50/output")
        output_dirname = os.path.join(current_directory, "testdata/resnet50/output")
        outfmt = "BIN"
        loop = 1
        debug = False
        device = "100"
        dym_batch = 2
        dym_hw = "300,500"
        dym_dims = "data:1,600;img_info:1,600"
        dym_shape = "data:1,600:img_info:1,600"
        output_size = "10"
        auto_set_dymshape_mode = False
        auto_set_dymdims_mode = False
        batchsize = 10
        pure_data_type = "zero"
        profiler = False
        dump = False
        acl_json_path = os.path.join(current_directory, "testdata/acl.json")
        json_data = {"test": {"data": "data"}}
        with open(acl_json_path, "w") as file:
            json.dump(json_data, file)
        output_batchsize_axis = 1
        run_mode = "array"
        display_all_summary = False
        warmup_count = 1
        dym_shape_range = "data:1,600~700;img_info:1,600-700"
        aipp_config = os.path.join(current_directory, "testdata/test_aipp_conf.config")
        with open(aipp_config, "w"):
            pass
        energy_consumption = False
        npu_id = "1,2,3"
        backend = "trtexec"
        perf = False
        pipeline = False
        profiler_rename = True
        dump_npy = False
        divide_input = False
        threads = 1

        args = AISBenchInferArgsAdapter(
            model,
            input_path,
            output,
            output_dirname,
            outfmt,
            loop,
            debug,
            device,
            dym_batch,
            dym_hw,
            dym_dims,
            dym_shape,
            output_size,
            auto_set_dymshape_mode,
            auto_set_dymdims_mode,
            batchsize,
            pure_data_type,
            profiler,
            dump,
            acl_json_path,
            output_batchsize_axis,
            run_mode,
            display_all_summary,
            warmup_count,
            dym_shape_range,
            aipp_config,
            energy_consumption,
            npu_id,
            backend,
            perf,
            pipeline,
            profiler_rename,
            dump_npy,
            divide_input,
            threads,
        )
        return args

    def test_args_check(self):
        args = self.get_args()
        args_dict = args.get_all_args_dict()
        assert check_dym_string(args.dym_dims) == args.dym_dims
        assert check_dym_range_string(args.dym_shape_range) == args.dym_shape_range
        assert check_number_list(args.output_size) == args.output_size
        assert str2bool(args.auto_set_dymdims_mode) == False
        assert check_batchsize_valid(args.batchsize) == args.batchsize
        assert check_nonnegative_integer(args.output_batchsize_axis) == args.output_batchsize_axis
        assert check_npu_id_range_vaild(args.npu_id) == args.npu_id
        check_device_range_valid(args.device) # 没返回值
        assert check_om_path_legality(args.model) == args.model
        assert check_input_path_legality(args.input) == args.input
        assert check_output_path_legality(args.output) == args.output
        assert check_acl_json_path_legality(args.acl_json_path) == args.acl_json_path
        assert check_aipp_config_path_legality(args.aipp_config) == args.aipp_config


if __name__ == "__main__":
    pytest.main([__file__, "-vs"])