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
import sys
import logging
import unittest
import json
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
from test_args_check import TestClass as get_args_class


logging.basicConfig(
    stream=sys.stdout, level=logging.INFO, format="[%(levelname)s] %(message)s"
)
logger = logging.getLogger(__name__)

DUMP_STR = "dump"


class TestClass(unittest.TestCase):
    args = get_args_class.get_args()

    def test_version_check_with_correct_version(self):
        version_check(self.args)

    @patch("pkg_resources.get_distribution")
    def test_version_check_with_correct_version(self, mock_get_distribution):
        mock_distribution = Mock()
        mock_distribution.version = "0.0.1"
        mock_get_distribution.return_value = mock_distribution
        args = Mock()
        version_check(args)

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
        data[DUMP_STR]["dump_path"] = self.args.output_dirname
        with open(self.args.acl_json_path, "w+") as file:
            json.dump(data, file, indent=4)
        check_valid_acl_json_for_dump(self.args.acl_json_path, self.args.model)

    def test_get_acl_json_path(self):
        get_acl_json_path(self.args)
        self.args.profiler = True
        get_acl_json_path(self.args)
        self.args.profiler = False
        self.args.dump = True
        get_acl_json_path(self.args)

    def test_get_batchsize(self):
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