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
import os
import sys
import logging
import pytest
import json

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
from test_common import TestCommonClass

logging.basicConfig(
    stream=sys.stdout, level=logging.INFO, format="[%(levelname)s] %(message)s"
)


class TestClass:
    def test_args_check(self):
        args = TestCommonClass.get_legal_args()
        args_dict = args.get_all_args_dict()
        assert check_dym_string(args.dym_dims) == args.dym_dims
        assert check_dym_range_string(args.dym_shape_range) == args.dym_shape_range
        assert check_number_list(args.output_size) == args.output_size
        assert str2bool(args.auto_set_dymdims_mode) == False
        assert check_batchsize_valid(args.batchsize) == args.batchsize
        assert check_nonnegative_integer(args.output_batchsize_axis) == args.output_batchsize_axis
        assert check_npu_id_range_vaild(args.npu_id) == [1, 2, 3]
        check_device_range_valid(args.device) # 没返回值
        assert check_om_path_legality(args.model) == args.model
        assert check_input_path_legality(args.input) == args.input
        assert check_output_path_legality(args.output) == args.output
        self._create_acl_json(args)
        assert check_acl_json_path_legality(args.acl_json_path) == args.acl_json_path
        assert check_aipp_config_path_legality(args.aipp_config) == args.aipp_config

    def _create_acl_json(self, args):
        if not os.path.exists(args.acl_json_path):
            with open(args.acl_json_path, "w+") as file:
                json.dump({}, file, indent=4)
        os.chmod(args.acl_json_path, 0o750)


if __name__ == "__main__":
    pytest.main([__file__, "-vs"])