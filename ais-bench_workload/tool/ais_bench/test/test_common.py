# Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
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

import filecmp
import math
import os
import random
import shutil
import sys
import json
from fileinput import filename
from ais_bench.infer.args_adapter import AISBenchInferArgsAdapter

import aclruntime
import numpy as np


class TestCommonClass:
    default_device_id = int(os.getenv("AISBENCH_INFER_DT_DEVICE_ID", 0))
    EPSILON = 1e-6
    epsilon = 1e-6
    cmd_prefix = (
        sys.executable + " " + os.path.join(os.path.dirname(os.path.realpath(__file__)), "../ais_bench/__main__.py")
    )
    base_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), "../test/testdata")
    msame_bin_path = os.getenv('MSAME_BIN_PATH')

    @staticmethod
    def get_cmd_prefix():
        _current_dir = os.path.dirname(os.path.realpath(__file__))
        return sys.executable + " " + os.path.join(_current_dir, "../ais_bench/__main__.py")

    @staticmethod
    def get_basepath():
        """
        test/testdata
        """
        data_path = os.getenv("AIT_BENCHMARK_DT_DATA_PATH")
        if not data_path:
            _current_dir = os.path.dirname(os.path.realpath(__file__))
            return os.path.join(_current_dir, "../test/testdata")
        else:
            return os.path.realpath(data_path)

    @staticmethod
    def create_inputs_file(input_path, size, pure_data_type=random):
        file_path = os.path.join(input_path, "{}.bin".format(size))
        if pure_data_type == "zero":
            lst = [0 for _ in range(size)]
        else:
            lst = [random.randrange(0, 256) for _ in range(size)]
        barray = bytearray(lst)
        ndata = np.frombuffer(barray, dtype=np.uint8)
        ndata.tofile(file_path)
        return file_path

    @staticmethod
    def prepare_dir(target_folder_path):
        if os.path.exists(target_folder_path):
            shutil.rmtree(target_folder_path)
        os.makedirs(target_folder_path, 0o750)

    @staticmethod
    def get_model_inputs_size(model_path):
        options = aclruntime.session_options()
        session = aclruntime.InferenceSession(model_path, TestCommonClass.default_device_id, options)
        return [meta.realsize for meta in session.get_inputs()]

    @staticmethod
    def get_inference_execute_num(log_path):
        if not os.path.exists(log_path) and not os.path.isfile(log_path):
            return 0

        cmd = "cat {} |grep 'model aclExec cost :' | wc -l".format(log_path)
        try:
            outval = os.popen(cmd).read()
        except Exception as e:
            raise Exception("grep action raises raise an exception: {}".format(e)) from e

        return int(outval.replace('\n', ''))

    @classmethod
    def get_inputs_path(cls, size, input_path, input_file_num, pure_data_type=random):
        """generate input files
        folder structure as follows.
        input_path
                |_ 196608           # size_path
                    |- 196608.bin   # base_size_file
                    |_ 5            # input_file_num_folder_path

        """
        size_path = os.path.join(input_path, str(size))
        if not os.path.exists(size_path):
            os.makedirs(size_path, 0o750)

        base_size_file_path = os.path.join(size_path, "{}.bin".format(size))
        if not os.path.exists(base_size_file_path):
            cls.create_inputs_file(size_path, size, pure_data_type)

        input_file_num_folder_path = os.path.join(size_path, str(input_file_num))

        if os.path.exists(input_file_num_folder_path):
            if len(os.listdir(input_file_num_folder_path)) == input_file_num:
                return input_file_num_folder_path
            else:
                shutil.rmtree(input_file_num_folder_path)

        if not os.path.exists(input_file_num_folder_path):
            os.makedirs(input_file_num_folder_path, 0o750)

        strs = []
        # create soft link to base_size_file
        for i in range(input_file_num):
            file_name = "{}-{}.bin".format(size, i)
            file_path = os.path.join(input_file_num_folder_path, file_name)
            strs.append("cp {} {}".format(base_size_file_path, file_path))
            strs.append(f"chmod 750 {file_path}")

        cmd = ';'.join(strs)
        os.system(cmd)

        return input_file_num_folder_path

    @classmethod
    def get_model_static_om_path(cls, batchsize, modelname):
        base_path = cls.get_basepath()
        return os.path.join(base_path, "{}/model".format(modelname), "pth_{}_bs{}.om".format(modelname, batchsize))

    @classmethod
    def get_legal_args(cls):
        current_directory = cls.get_basepath()
        model = os.path.join(
            current_directory, "resnet50/model/pth_resnet50_bs1.om"
        )
        input_path = os.path.join(current_directory, "resnet50/input")
        output = os.path.join(current_directory, "resnet50/output")
        output_dirname = os.path.join(current_directory, "resnet50/output")
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
        acl_json_path = os.path.join(current_directory, "acl.json")
        json_data = {"test": {"data": "data"}}
        with open(acl_json_path, "w") as file:
            json.dump(json_data, file)
        output_batchsize_axis = 1
        run_mode = "array"
        display_all_summary = False
        warmup_count = 1
        dym_shape_range = "data:1,600~700;img_info:1,600-700"
        aipp_config = os.path.join(current_directory, "test_aipp_conf.config")
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
