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

#lzy
import os
import sys
import logging
import unittest
import json
import shutil
from unittest.mock import Mock, patch

import pytest
from ais_bench.infer.common.miscellaneous import (
    get_modules_version,
    version_check,
    get_model_name,
    check_valid_acl_json_for_dump,
    get_acl_json_path,
    get_batchsize,
    get_range_list,
    get_dymshape_list,
    get_throughtput_from_log,
    regenerate_dymshape_cmd,
    dymshape_range_run,
)
from test_common import TestCommonClass
from test_args_check import TestClass as get_args_class


logging.basicConfig(
    stream=sys.stdout, level=logging.INFO, format="[%(levelname)s] %(message)s"
)
logger = logging.getLogger(__name__)

DUMP_STR = "dump"

class FakeModelInput:
    shape = "1,3,224,224"


class TestClass(unittest.TestCase):
    args = TestCommonClass.get_legal_args()

    def test_version_check_with_cur_version(self):
        tmp_args = self.args
        version_check(tmp_args)
        assert  tmp_args.run_mode != "tensor"

    @patch("pkg_resources.get_distribution")
    def test_version_check_with_old_version(self, mock_get_distribution):
        mock_distribution = Mock()
        mock_distribution.aclruntime_version = "0.0.1"
        mock_get_distribution.return_value = mock_distribution
        args = Mock()
        version_check(args)
        assert args.run_mode == "tensor"

    @patch("pkg_resources.get_distribution", side_effect=Exception("importerror"))
    def test_get_version_not_found(self, mock_get_distribution):
        args = Mock()
        with self.assertRaises(Exception):
            version_check(args)

    def test_check_valid_acl_json_for_dump(self):
        with open(self.args.acl_json_path, "r") as file:
            data = json.load(file)
        data[DUMP_STR] = {}
        data[DUMP_STR]["dump_list"] = [{"model": get_model_name(self.args.model)}]
        data[DUMP_STR]["dump_path"] = self.args.output
        with open(self.args.acl_json_path, "w+") as file:
            json.dump(data, file, indent=4)
        check_valid_acl_json_for_dump(self.args.acl_json_path, self.args.model)

    def test_acl_json_content_wrong_model_name(self):
        with open(self.args.acl_json_path, "r") as file:
            data = json.load(file)
        data[DUMP_STR] = {}
        data[DUMP_STR]["dump_list"] = [{"model": "invalid_model_name"}]
        data[DUMP_STR]["dump_path"] = self.args.output
        with open(self.args.acl_json_path, "w+") as file:
            json.dump(data, file, indent=4)
        with pytest.raises(ValueError) as e:
            check_valid_acl_json_for_dump(self.args.acl_json_path, self.args.model)
            assert "'model_name' is not set or set" in e

    def test_acl_json_content_missing_dump_list(self):
        with open(self.args.acl_json_path, "r") as file:
            data = json.load(file)
        data[DUMP_STR] = {}
        data[DUMP_STR]["dump_path"] = self.args.output
        with open(self.args.acl_json_path, "w+") as file:
            json.dump(data, file, indent=4)
        with pytest.raises(KeyError) as e:
            check_valid_acl_json_for_dump(self.args.acl_json_path, self.args.model)
            assert "need to set 'dump_list' attribute" in e

    def test_acl_json_content_dump_path_illegal(self):
        with open(self.args.acl_json_path, "r") as file:
            data = json.load(file)
        data[DUMP_STR] = {}
        data[DUMP_STR]["dump_list"] = [{"model": get_model_name(self.args.model)}]
        data[DUMP_STR]["dump_path"] = self.args.output
        with open(self.args.acl_json_path, "w+") as file:
            json.dump(data, file, indent=4)
        os.chmod(self.args.acl_json_path, 0o500) # r x ok, w not ok
        with pytest.raises(ValueError) as e:
            check_valid_acl_json_for_dump(self.args.acl_json_path, self.args.model)
            assert "has no read/write permission" in e

    def test_acl_json_content_dump_op_switch_illegal(self):
        with open(self.args.acl_json_path, "r") as file:
            data = json.load(file)
        data[DUMP_STR] = {}
        data[DUMP_STR]["dump_list"] = [{"model": "invalid_model_name"}]
        data[DUMP_STR]["dump_path"] = self.args.output
        data[DUMP_STR]["dump_op_switch"] = "none"
        with open(self.args.acl_json_path, "w+") as file:
            json.dump(data, file, indent=4)
        with pytest.raises(ValueError) as e:
            check_valid_acl_json_for_dump(self.args.acl_json_path, self.args.model)
            assert "'dump_op_switch' need to be" in e

    def test_acl_json_content_dump_mode_illegal(self):
        with open(self.args.acl_json_path, "r") as file:
            data = json.load(file)
        data[DUMP_STR] = {}
        data[DUMP_STR]["dump_list"] = [{"model": "invalid_model_name"}]
        data[DUMP_STR]["dump_path"] = self.args.output
        data[DUMP_STR]["dump_mode"] = "none"
        with open(self.args.acl_json_path, "w+") as file:
            json.dump(data, file, indent=4)
        with pytest.raises(ValueError) as e:
            check_valid_acl_json_for_dump(self.args.acl_json_path, self.args.model)
            assert "'dump_mode' need to be set" in e

    def test_get_acl_json_path_normal(self, monkeypatch):
        def mock_check_acl_json(prompt):
            return
        monkeypatch.setattr(
            ais_bench.infer.common.miscellaneous,
            "check_valid_acl_json_for_dump",
            mock_check_acl_json
        )
        assert get_acl_json_path(self.args) == self.args.acl_json_path

    def test_get_acl_json_path_with_profiler(self):
        tmp_args = self.args
        tmp_args.acl_json_path = None
        tmp_args.profiler = True
        profiler_dir = os.path.join(tmp_args.output, "profiler")
        shutil.rmtree(profiler_dir)
        get_acl_json_path(tmp_args)
        assert os.path.exists(profiler_dir)

    def test_get_acl_json_path_with_dump(self):
        tmp_args = self.args
        tmp_args.acl_json_path = None
        tmp_args.dump = True
        dump_dir = os.path.join(tmp_args.output, "dump")
        shutil.rmtree(dump_dir)
        get_acl_json_path(tmp_args)
        assert os.path.exists(dump_dir)

    def test_get_batchsize(self):
        mock_distribution = Mock()
        mock_distribution.intensors_desc = [FakeModelInput]
        with pytest.raises(Exception):
            get_batchsize(None, self.args)

    def test_get_range_list(self):
        ranges = "a:1-3,5;b:2,4-6"
        get_range_list(ranges)

    def test_get_dymshape_list(self):
        get_dymshape_list(self.args.input)

    def test_get_throughtput_from_log(self):
        out_log = "throughput 1\n"
        get_throughtput_from_log(out_log)

    def test_regenerate_dymshape_cmd(self):
        regenerate_dymshape_cmd(self.args, self.args.dym_shape)


if __name__ == "__main__":
    unittest.main()