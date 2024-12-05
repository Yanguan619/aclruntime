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
from ais_bench.infer.interface_check import (
    check_model_path_legality,
    check_acl_json_path_legality,
    check_device_range_valid,
    check_output_dir_legality,
    check_positive_integer,
    check_in_out_list,
    check_list,
    check_dict,
    check_custom_size,
    check_loop_size,
    check_bool_value,
    check_dym_hw_list
)
from ais_bench.infer.common.path_security_check import check_path_legality

logging.basicConfig(
    stream=sys.stdout, level=logging.INFO, format="[%(levelname)s] %(message)s"
)

logger = logging.getLogger(__name__)

class FakeFile:
    NOT_READABLE_OM_MODEL = "not_read_om_model.om"
    SUFFIX_WRONG_OM_MODEL = "model.test_om"
    NOT_EXIST_OM_MODEL = "not_exist_modle.om"
    NOT_READABLE_ACL_JSON = "not_read_acl.json"
    SUFFIX_WRONG_ACL_JSON = "acl.test_json"
    NOT_EXIST_ACL_JSON = "not_exist_acl.json"
    NOT_READABLE_OUTPUT_DIR = "not_read_output"


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

    def init(self):
        self.cur_dir = os.path.dirname(os.path.abspath(__file__))

    def test_check_model_path_legality(self):
        with pytest.raises(RuntimeError):
            check_model_path_legality("")

        self.check_illegal_fake_path_case(
            func_to_test=check_model_path_legality,
            fake_value=FakeFile.NOT_READABLE_OM_MODEL,
            permission=0o100
        )

        self.check_illegal_fake_path_case(
            func_to_test=check_model_path_legality,
            fake_value=FakeFile.SUFFIX_WRONG_OM_MODEL,
            permission=0o750
        )

        self.check_illegal_fake_path_case(
            func_to_test=check_model_path_legality,
            fake_value=FakeFile.NOT_EXIST_OM_MODEL,
            is_exist=False
        )

    def test_check_acl_json_path_legality(self):
        assert check_acl_json_path_legality("") == None

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

    def test_check_device_range_valid(self):
        DEVICE_COUNT_MAX = 256
        value = "a"
        with pytest.raises(TypeError):
            check_device_range_valid(value)
        value = 256
        with pytest.raises(ValueError):
            check_device_range_valid(value)
        value = -1
        with pytest.raises(ValueError):
            check_device_range_valid(value)

    def test_check_output_dir_legality(self, monkeypatch):
        def mock_check():
            return True
        fake_value="output"
        monkeypatch.setattr("check_path_legality", mock_check)
        result = check_output_dir_legality(fake_value)
        assert result == True

    def test_check_positive_integer(self):
        value = "abc"
        with pytest.raises(TypeError):
            check_positive_integer(value)

        value = 0
        with pytest.raises(ValueError):
            check_positive_integer(value)

        value = 2147483648
        with pytest.raises(ValueError):
            check_positive_integer(value)

    def test_check_in_out_list(self):
        in_out_list = [1, 2, 3]
        inputs = [1, 2]
        outputs = [1, 2, 3]
        with pytest.raises(RuntimeError):
            check_in_out_list(in_out_list, inputs, outputs)

        in_out_list = [1, 2, "a"]
        inputs = [1, 2, 3]
        outputs = [1, 2, 3]
        with pytest.raises(TypeError):
            check_in_out_list(in_out_list, inputs, outputs)

        in_out_list = [1, -2, 3]
        inputs = [1, 2, 3]
        outputs = [1, 2, 3]
        with pytest.raises(IndexError):
            check_in_out_list(in_out_list, inputs, outputs)

        in_out_list = [1, 2, 3]
        inputs = [1, 2, 3]
        outputs = [1, 2, 3]
        with pytest.raises(IndexError):
            check_in_out_list(in_out_list, inputs, outputs)
        
    def test_check_list(self):
        list_check = 1
        max_len = 2
        with pytest.raises(ValueError):
            check_list(list_check, max_len)

        list_check = []
        with pytest.raises(ValueError):
            check_list(list_check, max_len, False)

        list_check = [1, 2, 3]
        with pytest.raises(ValueError):
            check_list(list_check, max_len)

        list_check = [1, 2, 3]
        max_len = 4
        with pytest.raises(ValueError):
            check_list(list_check, max_len, True, str)

    def test_check_dict(self):
        dict_check = 1
        max_len = 1
        with pytest.raises(ValueError):
            check_dict(dict_check, max_len)

        dict_check = {
            "key1" : "value1",
            "key2" : "value2"
        }
        with pytest.raises(ValueError):
            check_dict(dict_check, max_len)

        dict_check = {}
        with pytest.raises(ValueError):
            check_dict(dict_check, max_len, False)

    def test_check_custom_size(self):
        CUSTOME_SIZE_MAX_SIZE = 16 * 1024 * 1024 * 1024
        value = 1
        mode="xxx"
        with pytest.raises(ValueError):
            check_custom_size(value, mode)
        
        value = None
        mode = "dymshape"
        with pytest.raises(ValueError):
            check_custom_size(value, mode)
        
        value = [0, 2]
        with pytest.raises(ValueError):
            check_custom_size(value, mode)
        
        value = [1, CUSTOME_SIZE_MAX_SIZE + 1]
        with pytest.raises(ValueError):
            check_custom_size(value, mode)

        value = 0
        with pytest.raises(ValueError):
            check_custom_size(value, mode)

        value = CUSTOME_SIZE_MAX_SIZE + 1
        with pytest.raises(ValueError):
            check_custom_size(value, mode)

        value = "abc"
        with pytest.raises(TypeError):
            check_custom_size(value, mode)
        
    def test_check_loop_size(self):
        LOOP_MAX_SIZE = 100000
        loop = "abc"
        with pytest.raises(TypeError):
            check_loop_size(loop)
        
        loop = 0
        with pytest.raises(ValueError):
            check_loop_size(loop)

        loop = LOOP_MAX_SIZE + 1
        with pytest.raises(ValueError):
            check_loop_size(loop)

    def test_check_bool_value(self):
        value = 10
        with pytest.raises(TypeError):
            check_bool_value(value)

    def test_check_dym_hw_list(self):
        CPP_INT_MAX_SIZE = 2147483647
        hw_list = [1]
        with pytest.raises(ValueError):
            check_dym_hw_list(hw_list)
        
        hw_list = ["a", 1]
        with pytest.raises(ValueError):
            check_dym_hw_list(hw_list)
        
        hw_list = [0, 1]
        with pytest.raises(ValueError):
            check_dym_hw_list(hw_list)
        
        hw_list = [CPP_INT_MAX_SIZE + 1, 1]
        with pytest.raises(ValueError):
            check_dym_hw_list(hw_list)

        hw_list = [1, 0]
        with pytest.raises(ValueError):
            check_dym_hw_list(hw_list)
        
        hw_list = [1, CPP_INT_MAX_SIZE + 1]
        with pytest.raises(ValueError):
            check_dym_hw_list(hw_list)


if __name__ == "__main__":
    pytest.main([__file__, "-vs"])
