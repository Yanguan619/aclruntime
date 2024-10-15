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
    DEVICE_COUNT_MAX
)
logging.basicConfig(
    stream=sys.stdout, level=logging.INFO, format="[%(levelname)s] %(message)s"
)
logger = logging.getLogger(__name__)

class FakeFile:
    NOT_READABLE_ACL_JSON = "not_read_acl.json"
    SUFFIX_WRONG_ACL_JSON = "acl.test_json"
    NOT_EXIST_ACL_JSON = "not_exist_acl.json"
    NOT_READABLE_AIPP_CONFIG = "not_read_test_aipp_conf.config"
    SUFFIX_WRONG_AIPP_CONFIG = "test_aipp_conf.test_config"
    NOT_EXIST_AIPP_CONFIG = "not_exist_test_aipp_conf.config"
    NOT_READABLE_INPUT_DIR = "not_read_input"
    NOT_EXIST_INPUT_DIR = "not_exist_input"
    NOT_READABLE_MODEL = "not_read_model.om"
    SUFFIX_WRONG_MODEL = "model.onnx"
    NOT_EXIST_MODEL = "not_exist_model.om"
    NOT_WRITABLE_OUTPUT_DIR = "not_write_output"


class TestClass:
    @staticmethod
    def touch_file(file_path, permission=0o750):
        with open(file_path, "w"):
            pass
        os.chmod(file_path, permission)

    @staticmethod
    def rmforce(path):
        os.system(f"rm -rf {path}") # 这里之所以不用shutil.rmtree ，os.remove之类的是考虑到服务器会询问是否删除，干脆直接rm -rf了。

    @classmethod
    def check_illegal_fake_path_case(cls, func_to_test, fake_value, permission=0o750, is_exist=True):
        fake_path = cls._get_abs_path(fake_value)
        if is_exist:
            if os.path.exists(fake_path):
                cls.rmforce(fake_path)
            if os.path.isfile(fake_path):
                cls.touch_file(fake_path, permission)
            else:
                os.mkdir(fake_path, permission)

        with pytest.raises(Exception):
            func_to_test(fake_path)

        if is_exist:
            cls.rmforce(fake_path)

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
    def _get_abs_path(cls, name):
        return os.path.join(cls.cur_dir, name)

    @classmethod
    def test_check_batchsize_valid(self):
        value = None
        assert check_batchsize_valid(value) == value
        value = -1
        with pytest.raises(Exception):
            check_batchsize_valid(value)

    def init(self):
        self.cur_dir = os.path.dirname(os.path.abspath(__file__))

    def test_check_acl_json_path_legality(self):
        assert check_acl_json_path_legality("") == ""

        self.check_illegal_fake_path_case(
            func_to_test=check_acl_json_path_legality,
            fake_value=FakeFile.NOT_READABLE_ACL_JSON,
            permission=0o100
        )

        self.check_illegal_fake_path_case(
            func_to_test=check_acl_json_path_legality,
            fake_value=FakeFile.SUFFIX_WRONG_ACL_JSON,
            permission=0o750
        )

        self.check_illegal_fake_path_case(
            func_to_test=check_acl_json_path_legality,
            fake_value=FakeFile.NOT_EXIST_ACL_JSON,
            is_exist=False
        )

    def test_check_aipp_config_path_legality(self):
        assert check_aipp_config_path_legality("") == ""

        self.check_illegal_fake_path_case(
            func_to_test=check_aipp_config_path_legality,
            fake_value=FakeFile.NOT_READABLE_AIPP_CONFIG,
            permission=0o100
        )

        self.check_illegal_fake_path_case(
            func_to_test=check_aipp_config_path_legality,
            fake_value=FakeFile.SUFFIX_WRONG_AIPP_CONFIG,
            permission=0o750
        )

        self.check_illegal_fake_path_case(
            func_to_test=check_aipp_config_path_legality,
            fake_value=FakeFile.NOT_EXIST_AIPP_CONFIG,
            is_exist=False
        )

    def test_device_range_valid(self):
        value = "1,-1"
        with pytest.raises(Exception):
            check_device_range_valid(value)
        value = "256"
        with pytest.raises(Exception):
            check_device_range_valid(value)
        value = ["1" for _ in range(DEVICE_COUNT_MAX + 1)]
        with pytest.raises(Exception):
            check_device_range_valid(value)

    def test_check_dym_range_string(self):
        assert check_dym_range_string("") == ""
        value = "**"
        with pytest.raises(Exception):
            check_dym_range_string(value)

    def test_check_dym_string(self):
        assert check_dym_string("") == ""
        value = "**"
        with pytest.raises(Exception):
            check_dym_string(value)

    def test_check_input_path_legality(self):
        assert check_input_path_legality("") == ""

        self.check_illegal_fake_path_case(
            func_to_test=check_input_path_legality,
            fake_value=FakeFile.NOT_READABLE_INPUT_DIR,
            permission=0o100
        )

        self.check_illegal_fake_path_case(
            func_to_test=check_input_path_legality,
            fake_value=FakeFile.NOT_EXIST_INPUT_DIR,
            is_exist=False
        )

    def test_check_nonnegative_integer(self):
        value = -1
        with pytest.raises(Exception):
            check_nonnegative_integer(value)

    def test_check_npu_id_range_vaild(self):
        value = "1,-1"
        with pytest.raises(Exception):
            check_npu_id_range_vaild(value)
        value = "2049"
        with pytest.raises(Exception):
            check_npu_id_range_vaild(value)

    def test_check_number_list(self):
        assert check_number_list(None) == None
        value = "**"
        with pytest.raises(Exception):
            check_number_list(value)

    def test_check_om_path_legality(self):
        self.check_illegal_fake_path_case(
            func_to_test=check_om_path_legality,
            fake_value=FakeFile.NOT_READABLE_MODEL,
            permission=0o100
        )

        self.check_illegal_fake_path_case(
            func_to_test=check_om_path_legality,
            fake_value=FakeFile.SUFFIX_WRONG_MODEL,
            permission=0o750
        )

        self.check_illegal_fake_path_case(
            func_to_test=check_om_path_legality,
            fake_value=FakeFile.NOT_EXIST_MODEL,
            is_exist=False
        )

    def test_check_output_path_legality(self):
        assert check_output_path_legality("") == ""
        self.check_illegal_fake_path_case(
            func_to_test=check_output_path_legality,
            fake_value=FakeFile.NOT_WRITABLE_OUTPUT_DIR,
            permission=0o400
        )

    def test_check_positive_integer(self):
        value = 0
        with pytest.raises(Exception):
            check_positive_integer(value)

    def test_str2bool(self):
        assert str2bool("yes") == True
        assert str2bool("no") == False
        with pytest.raises(Exception):
            str2bool("input_no_in_options")


if __name__ == "__main__":
    pytest.main([__file__, "-vs"])