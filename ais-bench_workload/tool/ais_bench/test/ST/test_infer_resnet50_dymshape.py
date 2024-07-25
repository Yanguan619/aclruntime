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
import json
import math
import os
import shutil
import sys
import logging
import stat
import torch
import numpy as np
import pytest
from test_common import TestCommonClass
from ais_bench.infer.interface import InferSession

logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='[%(levelname)s] %(message)s')
logger = logging.getLogger(__name__)

OPEN_FLAGS = os.O_WRONLY | os.O_CREAT | os.O_TRUNC
OPEN_MODES = stat.S_IWUSR | stat.S_IRUSR
MSPROF_SWITCH = 'AIT_NO_MSPROF_MODE'


class TestClass:
    @staticmethod
    def get_dynamic_shape_range_mode_inference_result_info(log_path):
        run_count = 0
        result_ok_num = 0
        shape_status = dict()
        with open(log_path) as f:
            for line in f:
                if 'run_count' in line:
                    str_list = line.split()
                    tmp_str = str_list[1]
                    num_str = tmp_str[(tmp_str.rfind(':') + 1) :]
                    num_str = num_str.replace('\n', '')
                    run_count = int(num_str)
                if "result:OK throughput" in line:
                    result_ok_num += 1
                    str_list = line.split()
                    tmp_str = str_list[2]
                    shape_str = tmp_str[(tmp_str.find(':') + 1) :]
                    shape_str = shape_str.replace('\n', '')
                    shape_status[shape_str] = True

        return run_count, result_ok_num, shape_status

    @staticmethod
    def get_dynamic_shape_om_file_size(shape):
        """ "
        dym_shape = "actual_input_1:1,3,224,224"
        """
        if len(shape) == 0:
            return 0

        sub_str = shape[(shape.rfind(':') + 1) :]
        sub_str = sub_str.replace('\n', '')
        num_arr = sub_str.split(',')
        fix_num = 4
        size = int(num_arr[0]) * int(num_arr[1]) * int(num_arr[2]) * int(num_arr[3]) * fix_num
        return size

    @staticmethod
    def create_npy_files_in_auto_set_dymshape_mode_input(dirname, shapes):
        if os.path.exists(dirname):
            shutil.rmtree(dirname)

        os.makedirs(dirname, 0o750)

        i = 1
        for shape in shapes:
            x = np.zeros(shape, dtype=np.int32)
            file_name = 'input_shape_{}'.format(i)
            file = os.path.join(dirname, "{}.npy".format(file_name))
            np.save(file, x)
            os.chmod(file, 0o640)
            i += 1

    @classmethod
    def setup_class(cls):
        """
        class level setup_class
        """
        cls.init(TestClass)

    @classmethod
    def teardown_class(cls):
        logger.info('\n ---class level teardown_class')

    def get_dynamic_shape_om_path(self):
        return os.path.join(self.model_base_path, "model", "pth_resnet50_dymshape.om")

    def init(self):
        self.model_name = "resnet50"
        self.model_base_path = self.get_model_base_path(self)
        self.output_file_num = 5

    def get_model_base_path(self):
        """
        supported model names as resnet50, resnet101,...。folder struct as follows
        testdata
         └── resnet50   # model base
            ├── input
            ├── model
            └── output
        """
        return os.path.join(TestCommonClass.get_basepath(), self.model_name)

    def test_pure_inference_normal_dynamic_shape(self):
        dym_shape = "actual_input_1:1,3,224,224"
        output_size = 10000
        model_path = self.get_dynamic_shape_om_path()
        cmd = "{} --model {} --device {} --outputSize {} --dymShape {} ".format(
            TestCommonClass.cmd_prefix, model_path, TestCommonClass.default_device_id, output_size, dym_shape
        )
        logger.info("run cmd:{}".format(cmd))
        ret = os.system(cmd)
        assert ret == 0

    def test_inference_normal_dynamic_shape_auto_set_dymshape_mode(self):
        """ "
        multiple npy input files or a npy folder as input parameter
        """
        shapes = [[1, 3, 224, 224], [1, 3, 300, 300], [1, 3, 200, 200]]
        auto_set_dymshape_mode_input_dir_path = os.path.join(
            self.model_base_path, "input", "auto_set_dymshape_mode_input"
        )
        self.create_npy_files_in_auto_set_dymshape_mode_input(auto_set_dymshape_mode_input_dir_path, shapes)

        output_size = 10000
        model_path = self.get_dynamic_shape_om_path()
        filelist = os.listdir(auto_set_dymshape_mode_input_dir_path)
        num_shape = len(filelist)
        file_paths_list = []
        for file in filelist:
            file_paths_list.append(os.path.join(auto_set_dymshape_mode_input_dir_path, file))
        file_paths = ",".join(file_paths_list)
        output_parent_path = os.path.join(self.model_base_path, "output")
        output_dirname = "auto_set_dymshape_mode_output"
        output_path = os.path.join(output_parent_path, output_dirname)
        if os.path.exists(output_path):
            shutil.rmtree(output_path)
        os.makedirs(output_path, 0o750)
        cmd = "{} --model {} --device {} --outputSize {} --auto_set_dymshape_mode true --input {} --output {} \
            --output_dirname {} ".format(
            TestCommonClass.cmd_prefix,
            model_path,
            TestCommonClass.default_device_id,
            output_size,
            file_paths,
            output_parent_path,
            output_dirname,
        )

        ret = os.system(cmd)
        assert ret == 0

        cmd = "find {} -name '*.bin'|wc -l".format(output_path)
        try:
            bin_num = os.popen(cmd).read()
        except Exception as e:
            raise Exception("raise an exception: {}".format(e)) from e

        assert int(bin_num) == num_shape
        shutil.rmtree(output_path)
        os.makedirs(output_path, 0o750)
        # check input parameter is a folder
        cmd = "{} --model {} --device {} --outputSize {} --auto_set_dymshape_mode true --input {} --output {} \
            --output_dirname {} ".format(
            TestCommonClass.cmd_prefix,
            model_path,
            TestCommonClass.default_device_id,
            output_size,
            auto_set_dymshape_mode_input_dir_path,
            output_parent_path,
            output_dirname,
        )

        ret2 = os.system(cmd)
        assert ret2 == 0

        cmd = "find {} -name '*.bin'|wc -l".format(output_path)
        try:
            bin_num2 = os.popen(cmd).read()
        except Exception as e:
            raise Exception("raise an exception: {}".format(e)) from e

        assert int(bin_num2) == num_shape
        shutil.rmtree(output_path)

    def test_general_inference_prformance_comparison_with_msame_dynamic_shape(self):
        dym_shape = "actual_input_1:1,3,224,224"
        input_file_num = 100
        output_size = 100000

        model_path = self.get_dynamic_shape_om_path()
        input_size = self.get_dynamic_shape_om_file_size(dym_shape)

        input_path = TestCommonClass.get_inputs_path(
            input_size, os.path.join(self.model_base_path, "input"), input_file_num
        )
        output_path = os.path.join(self.model_base_path, "output")
        output_dir_name = "ais_bench_dym_output"
        output_dir_path = os.path.join(output_path, output_dir_name)
        if os.path.exists(output_dir_path):
            shutil.rmtree(output_dir_path)
        os.makedirs(output_dir_path, 0o750)
        summary_json_path = os.path.join(output_path, "{}_summary.json".format(output_dir_name))

        cmd = "{} --model {}  --outputSize {} --dymShape {} --input {} --output {} --output_dirname {}".format(
            TestCommonClass.cmd_prefix, model_path, output_size, dym_shape, input_path, output_path, output_dir_name
        )
        logger.info("run cmd:{}".format(cmd))
        ret = os.system(cmd)
        assert ret == 0

        with open(summary_json_path, 'r', encoding='utf8') as fp:
            json_data = json.load(fp)
            ais_bench_inference_time_ms = json_data["NPU_compute_time"]["mean"]

        assert math.fabs(ais_bench_inference_time_ms) > TestCommonClass.EPSILON
        # get msame inference  average time without first time
        msame_infer_log_path = os.path.join(output_path, output_dir_name, "msame_infer.log")
        cmd = "{} --model {} --outputSize {} --dymShape {} --input {} --output {} > {}".format(
            TestCommonClass.msame_bin_path,
            model_path,
            output_size,
            dym_shape,
            input_path,
            output_path,
            msame_infer_log_path,
        )
        logger.info("run cmd:{}".format(cmd))
        ret = os.system(cmd)
        assert ret == 0
        assert os.path.exists(msame_infer_log_path)

        msame_inference_time_ms = 0.0
        with open(msame_infer_log_path) as f:
            for line in f:
                if "Inference average time without first time" not in line:
                    continue

                sub_str = line[(line.rfind(':') + 1) :]
                sub_str = sub_str.replace('ms\n', '')
                msame_inference_time_ms = float(sub_str)

        assert math.fabs(msame_inference_time_ms) > TestCommonClass.EPSILON
        # compare
        allowable_performance_deviation = 0.2 # 0.04 -> 0.2 连续跑多用例性能差距大
        if msame_inference_time_ms != 0.0:
            reference_deviation = (ais_bench_inference_time_ms - msame_inference_time_ms) / msame_inference_time_ms
            logger.info(
                "dymshape msame time:{} ais time:{} ref:{}".format(
                    msame_inference_time_ms, ais_bench_inference_time_ms, reference_deviation
                )
            )
            assert reference_deviation < allowable_performance_deviation
        else:
            logger.warning("zero divisoin!")
        os.remove(msame_infer_log_path)
        shutil.rmtree(output_dir_path)

    def test_pure_inference_batchsize_is_none_normal_dynamic_shape(self):
        dym_shapes = [
            "actual_input_1:1,3,224,224",
            "actual_input_1:1,3,300,300",
            "actual_input_1:2,3,224,224",
            "actual_input_1:2,3,300,300",
            "actual_input_1:4,3,224,224",
            "actual_input_1:4,3,300,300",
            "actual_input_1:8,3,300,300",
            "actual_input_1:8,3,300,300",
            "actual_input_1:16,3,224,224",
            "actual_input_1:16,3,300,300",
        ]
        batchsizes = [1, 1, 2, 2, 4, 4, 8, 8, 16, 16]
        output_size = 100000
        model_path = self.get_dynamic_shape_om_path()
        output_parent_path = os.path.join(self.model_base_path, "output")
        output_paths = []
        summary_paths = []
        for i, dym_shape in enumerate(dym_shapes):
            output_dirname = "dynamic_shape_{}".format(i)
            output_path = os.path.join(output_parent_path, output_dirname)
            log_path = os.path.join(output_path, "log.txt")
            if os.path.exists(output_path):
                shutil.rmtree(output_path)
            os.makedirs(output_path, 0o750)
            summary_json_path = os.path.join(output_parent_path, "{}_summary.json".format(output_dirname))
            cmd = "{} --model {} --device {} --outputSize {} --dymShape {} --output {} --output_dirname {} > {}".format(
                TestCommonClass.cmd_prefix,
                model_path,
                TestCommonClass.default_device_id,
                output_size,
                dym_shape,
                output_parent_path,
                output_dirname,
                log_path,
            )
            logger.info("run cmd:{}".format(cmd))
            output_paths.append(output_path)
            summary_paths.append(summary_json_path)
            ret = os.system(cmd)
            assert ret == 0

            with open(log_path) as f:
                for line in f:
                    if "1000*batchsize" not in line:
                        continue

                    sub_str = line.split('/')[0].split('(')[1].strip(')')
                    cur_batchsize = int(sub_str)
                    assert batchsizes[i] == cur_batchsize
                    break

        for output_path in output_paths:
            shutil.rmtree(output_path)
        for summary_path in summary_paths:
            os.remove(summary_path)

    def test_pure_inference_normal_dynamic_shape_range_mode(self):
        dymshape_range = "actual_input_1:1,3,224,224~226"
        dymshapes = ["actual_input_1:1,3,224,224", "actual_input_1:1,3,224,225", "actual_input_1:1,3,224,226"]
        model_path = self.get_dynamic_shape_om_path()
        output_size = 100000
        output_parent_path = os.path.join(self.model_base_path, "output")
        output_dirname = "dynamic_shape_range"
        output_path = os.path.join(output_parent_path, output_dirname)
        if os.path.exists(output_path):
            shutil.rmtree(output_path)
        os.makedirs(output_path, 0o750)
        summary_json_path = os.path.join(output_parent_path, "{}_summary.json".format(output_dirname))
        log_path = os.path.join(output_path, "log.txt")

        cmd = "{} --model {} --outputSize {} --dymShape_range {} --output {} --output_dirname {} > \
            {}".format(
            TestCommonClass.cmd_prefix,
            model_path,
            output_size,
            dymshape_range,
            output_parent_path,
            output_dirname,
            log_path,
        )
        logger.info("run cmd:{}".format(cmd))
        ret = os.system(cmd)
        assert ret == 0

        run_count, result_ok_num, shape_status = self.get_dynamic_shape_range_mode_inference_result_info(log_path)
        assert run_count == len(dymshapes)
        assert run_count == result_ok_num
        assert len(dymshapes) == len(shape_status.keys())
        for k, v in shape_status.items():
            assert k in dymshapes
            assert v is True

        shutil.rmtree(output_path)
        os.remove(summary_json_path)

    def test_pure_inference_normal_dynamic_shape_range_mode_2(self):
        dymshape_range = "actual_input_1:1~2,3,224-300,224-300"
        dymshapes = [
            "actual_input_1:1,3,224,224",
            "actual_input_1:1,3,224,300",
            "actual_input_1:1,3,300,224",
            "actual_input_1:1,3,300,300",
            "actual_input_1:2,3,224,224",
            "actual_input_1:2,3,224,300",
            "actual_input_1:2,3,300,224",
            "actual_input_1:2,3,300,300",
        ]
        model_path = self.get_dynamic_shape_om_path()
        output_size = 100000
        output_parent_path = os.path.join(self.model_base_path, "output")
        output_dirname = "dynamic_shape_range"
        output_path = os.path.join(output_parent_path, output_dirname)
        if os.path.exists(output_path):
            shutil.rmtree(output_path)
        os.makedirs(output_path, 0o750)
        summary_json_path = os.path.join(output_parent_path, "{}_summary.json".format(output_dirname))
        log_path = os.path.join(output_path, "log.txt")

        cmd = "{} --model {} --outputSize {} --dymShape_range {} --output {} --output_dirname {} > \
            {}".format(
            TestCommonClass.cmd_prefix,
            model_path,
            output_size,
            dymshape_range,
            output_parent_path,
            output_dirname,
            log_path,
        )
        logger.info("run cmd:{}".format(cmd))
        ret = os.system(cmd)
        assert ret == 0

        run_count, result_ok_num, shape_status = self.get_dynamic_shape_range_mode_inference_result_info(log_path)
        assert run_count == len(dymshapes)
        assert run_count == result_ok_num
        assert len(dymshapes) == len(shape_status.keys())
        for k, v in shape_status.items():
            assert k in dymshapes
            assert v is True

        shutil.rmtree(output_path)
        os.remove(summary_json_path)

    def test_pure_inference_normal_dynamic_shape_range_mode_3(self):
        range_file_parent_path = os.path.join(self.model_base_path, "input")
        dymshape_range_file = os.path.join(range_file_parent_path, "dymshape_range.info")
        with os.fdopen(os.open(dymshape_range_file, OPEN_FLAGS, OPEN_MODES), 'w') as f:
            f.write("actual_input_1:1,3,224-300,224-225\n")
            f.write("actual_input_1:8-9,3,224-300,260-300")

        dymshapes = [
            "actual_input_1:1,3,224,224",
            "actual_input_1:1,3,224,225",
            "actual_input_1:1,3,300,224",
            "actual_input_1:1,3,300,225",
            "actual_input_1:8,3,224,260",
            "actual_input_1:8,3,224,300",
            "actual_input_1:8,3,300,260",
            "actual_input_1:8,3,300,300",
            "actual_input_1:9,3,224,260",
            "actual_input_1:9,3,224,300",
            "actual_input_1:9,3,300,260",
            "actual_input_1:9,3,300,300",
        ]
        model_path = self.get_dynamic_shape_om_path()
        output_size = 100000
        output_parent_path = os.path.join(self.model_base_path, "output")
        output_dirname = "dynamic_shape_range"
        output_path = os.path.join(output_parent_path, output_dirname)
        if os.path.exists(output_path):
            shutil.rmtree(output_path)
        os.makedirs(output_path, 0o750)
        summary_json_path = os.path.join(output_parent_path, "{}_summary.json".format(output_dirname))
        log_path = os.path.join(output_path, "log.txt")

        cmd = "{} --model {} --outputSize {} --dymShape_range {} --output {} --output_dirname {} > \
            {}".format(
            TestCommonClass.cmd_prefix,
            model_path,
            output_size,
            dymshape_range_file,
            output_parent_path,
            output_dirname,
            log_path,
        )
        logger.info("run cmd:{}".format(cmd))
        ret = os.system(cmd)
        assert ret == 0

        run_count, result_ok_num, shape_status = self.get_dynamic_shape_range_mode_inference_result_info(log_path)
        assert run_count == len(dymshapes)
        assert run_count == result_ok_num
        assert len(dymshapes) == len(shape_status.keys())

        for k, v in shape_status.items():
            assert k in dymshapes
            assert v is True

        shutil.rmtree(output_path)
        os.remove(summary_json_path)
        os.remove(dymshape_range_file)

    def test_pure_inference_abnormal_dynamic_shape_range_mode(self):
        dymshape_range = "actual_input_1:1,3~4,224-300,224"
        dymshapes = [
            "actual_input_1:1,3,224,224",
            "actual_input_1:1,3,300,224",
            "actual_input_1:1,4,224,224",
            "actual_input_1:1,4,300,224",
        ]
        model_path = self.get_dynamic_shape_om_path()
        output_size = 100000
        output_parent_path = os.path.join(self.model_base_path, "output")
        output_dirname = "dynamic_shape_range"
        output_path = os.path.join(output_parent_path, output_dirname)
        if os.path.exists(output_path):
            shutil.rmtree(output_path)
        os.makedirs(output_path, 0o750)
        summary_json_path = os.path.join(output_parent_path, "{}_summary.json".format(output_dirname))
        log_path = os.path.join(output_path, "log.txt")
        cmd = "{} --model {} --outputSize {} --dymShape_range {} --output {} --output_dirname {} > \
                {}".format(
            TestCommonClass.cmd_prefix,
            model_path,
            output_size,
            dymshape_range,
            output_parent_path,
            output_dirname,
            log_path,
        )
        logger.info("run cmd:{}".format(cmd))
        ret = os.system(cmd)
        try:
            assert ret != 0
        except Exception as e:
            logger.info("some case run failure")

        run_count, result_ok_num, shape_status = self.get_dynamic_shape_range_mode_inference_result_info(log_path)
        assert run_count == len(dymshapes)
        assert 2 == result_ok_num

        shutil.rmtree(output_path)
        os.remove(summary_json_path)

    def test_general_inference_interface_dynamicshape(self):
        model_path = self.get_dynamic_shape_om_path()
        output_size = 100000
        custom_sizes_list = [100000, [100000]]
        for custom_sizes in custom_sizes_list:
            # interface
            session = InferSession(TestCommonClass.default_device_id, model_path)
            ndata = np.zeros([1, 3, 224, 224], dtype=np.float32)
            mode = "dymshape"
            outputs = session.infer([ndata], mode, custom_sizes=custom_sizes)

            outarray = []
            for out in outputs:
                outarray.append(np.array(out))

            # cmd
            infer_dynamicshape_output_path = os.path.join(
                self.model_base_path, "output", "infer_dynamicshape_output.bin"
            )
            out = np.array(outarray)
            out.tofile(infer_dynamicshape_output_path)

            dym_shape = "actual_input_1:1,3,224,224"
            output_parent_path = os.path.join(self.model_base_path, "output")
            output_dirname = "interface_dynamicshape"
            output_path = os.path.join(output_parent_path, output_dirname)
            summary_path = os.path.join(output_parent_path, "{}_summary.json".format(output_dirname))
            if os.path.exists(output_path):
                shutil.rmtree(output_path)
            os.makedirs(output_path, 0o750)
            cmd = "{} --model {} --outputSize {} --dymShape {} --output {} --output_dirname {} --outfmt BIN".format(
                TestCommonClass.cmd_prefix, model_path, output_size, dym_shape, output_parent_path, output_dirname
            )
            logger.info("run cmd:{}".format(cmd))
            ret = os.system(cmd)
            assert ret == 0
            output_bin_file_path = os.path.join(output_path, "pure_infer_data_0.bin")

            # compare bin file
            assert filecmp.cmp(infer_dynamicshape_output_path, output_bin_file_path)

            shutil.rmtree(output_path)
            os.remove(summary_path)
            os.remove(infer_dynamicshape_output_path)

    def test_general_inference_interface_dyshape_compare_tensor_npy(self):
        model_path = self.get_dynamic_shape_om_path()
        output_size = 100000

        # tonsor interface
        session = InferSession(TestCommonClass.default_device_id, model_path)
        ndata = torch.rand([1, 3, 224, 224], out=None, dtype=torch.float32)
        mode = "dymshape"
        tensor_outputs = session.infer([ndata], mode, custom_sizes=output_size)
        output_parent_path = os.path.join(self.model_base_path, "output")
        tensor_infer_result_file_path = os.path.join(output_parent_path, "tensor_infer_result.npy")
        np.save(tensor_infer_result_file_path, tensor_outputs)

        # numpy interface
        ndata = ndata.numpy()
        outputs = session.infer([ndata], mode, custom_sizes=output_size)
        npy_infer_result_file_path = os.path.join(output_parent_path, "npy_infer_result.npy")
        np.save(npy_infer_result_file_path, tensor_outputs)
        # compare bin file
        assert filecmp.cmp(tensor_infer_result_file_path, npy_infer_result_file_path)

        os.remove(tensor_infer_result_file_path)
        os.remove(npy_infer_result_file_path)

    def test_general_inference_interface_dyshape_compare_tensor_discontinuous_tensor_continues(self):
        model_path = self.get_dynamic_shape_om_path()
        output_size = 100000

        output_parent_path = os.path.join(self.model_base_path, "output")

        mode = "dymshape"
        session = InferSession(TestCommonClass.default_device_id, model_path)
        ndata = torch.rand([1, 224, 3, 224], out=None, dtype=torch.float32)

        ndata_discontinue = ndata.permute(0, 2, 1, 3)
        ndata_continue = ndata_discontinue.contiguous()

        tensor_outputs = session.infer([ndata_continue], mode, custom_sizes=output_size)
        tensor_infer_result1_path = os.path.join(output_parent_path, "first_tensor_infer_result.npy")
        np.save(tensor_infer_result1_path, tensor_outputs)

        npy_outputs = session.infer([ndata_discontinue], mode, custom_sizes=output_size)
        tensor_infer_result2_path = os.path.join(output_parent_path, "second_tensor_infer_result.npy")
        np.save(tensor_infer_result2_path, npy_outputs)

        # compare infer result
        assert filecmp.cmp(tensor_infer_result1_path, tensor_infer_result2_path)

        os.remove(tensor_infer_result1_path)
        os.remove(tensor_infer_result2_path)


if __name__ == '__main__':
    pytest.main([__file__, '-vs'])

