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

import os
import sys
import logging
import pytest
from ais_bench.infer.args_check import (
    check_batchsize_valid,
    check_acl_json_path_legality,
    check_aipp_config_path_legality,
    check_device_range_valid,
    check_dym_range_string,
    check_dym_string,
    check_input_path_legality,
    check_nonnegative_integer,
    check_npu_id_range_vaild,
    check_number_list,
    check_om_path_legality,
    check_output_path_legality,
    check_positive_integer,
    str2bool,
)
logging.basicConfig(
    stream=sys.stdout, level=logging.INFO, format="[%(levelname)s] %(message)s"
)
logger = logging.getLogger(__name__)


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

    @classmethod
    def test_check_batchsize_valid(cls):
        value = None
        check_batchsize_valid(value)
        value = -1
        with pytest.raises(Exception):
            check_batchsize_valid(value)

    @classmethod
    def test_check_acl_json_path_legality(cls):
        value = ""
        check_acl_json_path_legality(value)
        current_directory = os.getcwd()
        value = os.path.join(current_directory, "testdata/not_read_acl.json")
        with open(value, "w"):
            pass
        os.chmod(value, 0)
        with pytest.raises(Exception):
            check_acl_json_path_legality(value)
        os.remove(value)
        value = os.path.join(current_directory, "testdata/acl.test_json")
        with open(value, "w"):
            pass
        os.chmod(value, 0o777)
        with pytest.raises(Exception):
            check_acl_json_path_legality(value)
        os.remove(value)
        value = os.path.join(current_directory, "testdata/not_exist_acl.json")
        with pytest.raises(Exception):
            check_acl_json_path_legality(value)

    @classmethod
    def test_check_aipp_config_path_legality(cls):
        value = ""
        check_aipp_config_path_legality(value)
        current_directory = os.getcwd()
        value = os.path.join(
            current_directory, "testdata/not_read_test_aipp_conf.config"
        )
        with open(value, "w"):
            pass
        os.chmod(value, 0)
        with pytest.raises(Exception):
            check_aipp_config_path_legality(value)
        os.remove(value)
        value = os.path.join(current_directory, "testdata/test_aipp_conf.test_config")
        with open(value, "w"):
            pass
        os.chmod(value, 0o777)
        with pytest.raises(Exception):
            check_aipp_config_path_legality(value)
        os.remove(value)
        value = os.path.join(
            current_directory, "testdata/not_exist_test_aipp_conf.config"
        )
        with pytest.raises(Exception):
            check_aipp_config_path_legality(value)

    @classmethod
    def test_device_range_valid(cls):
        value = "1,-1"
        with pytest.raises(Exception):
            check_device_range_valid(value)
        value = "3000"
        with pytest.raises(Exception):
            check_device_range_valid(value)

    @classmethod
    def test_check_dym_range_string(cls):
        value = None
        check_dym_range_string(value)
        value = "**"
        with pytest.raises(Exception):
            check_dym_range_string(value)

    @classmethod
    def test_check_dym_string(cls):
        value = None
        check_dym_string(value)
        value = "**"
        with pytest.raises(Exception):
            check_dym_string(value)

    @classmethod
    def test_check_input_path_legality(cls):
        value = ""
        check_input_path_legality(value)
        current_directory = os.getcwd()
        value = os.path.join(current_directory, "/testdata/resnet50/not_read_input")
        os.makedirs(value)
        os.chmod(value, 0)
        with pytest.raises(Exception):
            check_input_path_legality(value)
        os.rmdir(value)
        value = os.path.join(
            current_directory, "/testdata/resnet50/model/not_exist_input"
        )
        with pytest.raises(Exception):
            check_input_path_legality(value)

    @classmethod
    def test_check_nonnegative_integer(cls):
        value = -1
        with pytest.raises(Exception):
            check_nonnegative_integer(value)

    @classmethod
    def test_check_npu_id_range_vaild(cls):
        value = "1,-1"
        with pytest.raises(Exception):
            check_npu_id_range_vaild(value)
        value = "3000"
        with pytest.raises(Exception):
            check_npu_id_range_vaild(value)

    @classmethod
    def test_check_number_list(cls):
        value = None
        check_number_list(value)
        value = "**"
        with pytest.raises(Exception):
            check_number_list(value)

    @classmethod
    def test_check_om_path_legality(cls):
        current_directory = os.getcwd()
        value = os.path.join(
            current_directory, "testdata/resnet50/model/not_read_pth_resnet50_bs1.om"
        )
        with open(value, "w"):
            pass
        os.chmod(value, 0)
        with pytest.raises(Exception):
            check_om_path_legality(value)
        os.remove(value)
        value = os.path.join(
            current_directory, "testdata/resnet50/model/pth_resnet50_bs1.onnx"
        )
        with pytest.raises(Exception):
            check_om_path_legality(value)
        value = os.path.join(
            current_directory, "testdata/resnet50/model/not_exist_pth_resnet50_bs1.om"
        )
        with pytest.raises(Exception):
            check_om_path_legality(value)

    @classmethod
    def test_check_output_path_legality(cls):
        value = ""
        check_output_path_legality(value)
        current_directory = os.getcwd()
        value = os.path.join(current_directory, "testdata/resnet50/not_write_output")
        if not os.path.exists(value):
            os.mkdir(value)
        os.chmod(value, 0)
        check_output_path_legality(value)
        os.rmdir(value)
        value = os.path.join(current_directory, "testdata/resnet50/not_exist_path")
        check_output_path_legality(value)

    @classmethod
    def test_check_positive_integer(cls):
        value = -1
        with pytest.raises(Exception):
            check_positive_integer(value)

    @classmethod
    def test_str2bool(cls):
        value = "yes"
        str2bool(value)
        value = "no"
        str2bool(value)
        value = "a test"
        with pytest.raises(Exception):
            str2bool(value)

    def init(self):
        self.model_name = "resnet50"





if __name__ == "__main__":
    pytest.main([__file__, "-vs"])