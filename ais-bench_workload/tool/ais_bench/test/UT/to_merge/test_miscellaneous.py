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
from ais_bench.infer.interface import InferSession
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
)
from test_common import TestCommonClass

logging.basicConfig(
    stream=sys.stdout, level=logging.INFO, format="[%(levelname)s] %(message)s"
)
logger = logging.getLogger(__name__)

DUMP_STR = "dump"

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

    def init(self):
        self.args = TestCommonClass.get_legal_args()

    def test_version_check_with_cur_version(self):
        version_check(self.args)
        assert self.args.run_mode != "tensor"

    def test_version_check_with_old_version(self, monkeypatch):
        def mock_get_version(prompt):
            return "0.0.1"
        monkeypatch.setattr(
            "ais_bench.infer.common.miscellaneous.get_modules_version",
            mock_get_version
        )
        version_check(self.args)
        assert self.args.run_mode == "tensor"

    def test_get_version_not_found(self, monkeypatch):
        def mock_get_version(prompt):
            raise Exception
        monkeypatch.setattr(
            "ais_bench.infer.common.miscellaneous.get_modules_version",
            mock_get_version
        )
        version_check(self.args)
        assert self.args.run_mode == "tensor"

    def test_check_valid_acl_json_for_dump(self):
        with open(self.args.acl_json_path, "r") as file:
            data = json.load(file)
        data[DUMP_STR] = {}
        data[DUMP_STR]["dump_list"] = [{"model_name": get_model_name(self.args.model)}]
        data[DUMP_STR]["dump_path"] = self.args.output
        with open(self.args.acl_json_path, "w+") as file:
            json.dump(data, file, indent=4)
        os.chmod(self.args.acl_json_path, 0o750)
        check_valid_acl_json_for_dump(self.args.acl_json_path, self.args.model)

    def test_acl_json_content_wrong_model_name(self):
        with open(self.args.acl_json_path, "r") as file:
            data = json.load(file)
        data[DUMP_STR] = {}
        data[DUMP_STR]["dump_list"] = [{"model_name": "invalid_model_name"}]
        data[DUMP_STR]["dump_path"] = self.args.output
        with open(self.args.acl_json_path, "w+") as file:
            json.dump(data, file, indent=4)
        os.chmod(self.args.acl_json_path, 0o750)
        with pytest.raises(ValueError) as e:
            check_valid_acl_json_for_dump(self.args.acl_json_path, self.args.model)
            if not "'model_name' is not set or set" in str(e):
                pytest.fail("do not catch expected err!")

    def test_acl_json_content_missing_dump_list(self):
        with open(self.args.acl_json_path, "r") as file:
            data = json.load(file)
        data[DUMP_STR] = {}
        data[DUMP_STR]["dump_path"] = self.args.output
        with open(self.args.acl_json_path, "w+") as file:
            json.dump(data, file, indent=4)
        os.chmod(self.args.acl_json_path, 0o750)
        with pytest.raises(KeyError) as e:
            check_valid_acl_json_for_dump(self.args.acl_json_path, self.args.model)
            if not "need to set 'dump_list' attribute" in str(e):
                pytest.fail("do not catch expected err!")

    def test_acl_json_content_dump_path_illegal(self, monkeypatch):
        with open(self.args.acl_json_path, "r") as file:
            data = json.load(file)
        data[DUMP_STR] = {}
        data[DUMP_STR]["dump_list"] = [{"model_name": get_model_name(self.args.model)}]
        data[DUMP_STR]["dump_path"] = self.args.output
        with open(self.args.acl_json_path, "w+") as file:
            json.dump(data, file, indent=4)
        os.chmod(self.args.acl_json_path, 0o750)
        monkeypatch.setattr("os.access", lambda path, perm: False)
        with pytest.raises(ValueError) as e:
            check_valid_acl_json_for_dump(self.args.acl_json_path, self.args.model)
            if not "has no read/write permission" in str(e):
                pytest.fail("do not catch expected err!")

    def test_acl_json_content_dump_op_switch_illegal(self):
        with open(self.args.acl_json_path, "r") as file:
            data = json.load(file)
        data[DUMP_STR] = {}
        data[DUMP_STR]["dump_list"] = [{"model_name": "invalid_model_name"}]
        data[DUMP_STR]["dump_path"] = self.args.output
        data[DUMP_STR]["dump_op_switch"] = "none"
        with open(self.args.acl_json_path, "w+") as file:
            json.dump(data, file, indent=4)
        os.chmod(self.args.acl_json_path, 0o750)
        with pytest.raises(ValueError) as e:
            check_valid_acl_json_for_dump(self.args.acl_json_path, self.args.model)
            if not "'dump_op_switch' need to be" in str(e):
                pytest.fail("do not catch expected err!")

    def test_acl_json_content_dump_mode_illegal(self):
        with open(self.args.acl_json_path, "r") as file:
            data = json.load(file)
        data[DUMP_STR] = {}
        data[DUMP_STR]["dump_list"] = [{"model_name": "invalid_model_name"}]
        data[DUMP_STR]["dump_path"] = self.args.output
        data[DUMP_STR]["dump_mode"] = "none"
        with open(self.args.acl_json_path, "w+") as file:
            json.dump(data, file, indent=4)
        os.chmod(self.args.acl_json_path, 0o750)
        with pytest.raises(ValueError) as e:
            check_valid_acl_json_for_dump(self.args.acl_json_path, self.args.model)
            if not "'dump_mode' need to be set" in str(e):
                pytest.fail("do not catch expected err!")

    def test_get_acl_json_path_normal(self, monkeypatch):
        monkeypatch.setattr(
            "ais_bench.infer.common.miscellaneous.check_valid_acl_json_for_dump",
            lambda x, y: None
        )
        assert get_acl_json_path(self.args) == self.args.acl_json_path

    def test_get_acl_json_path_with_profiler(self):
        self.args.acl_json_path = None
        self.args.profiler = True
        self.args.dump = False
        profiler_dir = os.path.join(self.args.output, "profiler")
        if os.path.exists(profiler_dir):
            shutil.rmtree(profiler_dir)
        get_acl_json_path(self.args)
        assert os.path.exists(profiler_dir)

    def test_get_acl_json_path_with_dump(self):
        self.args.acl_json_path = None
        self.args.profiler = False
        self.args.dump = True
        dump_dir = os.path.join(self.args.output, "dump")
        if os.path.exists(dump_dir):
            shutil.rmtree(dump_dir)
        get_acl_json_path(self.args)
        assert os.path.exists(dump_dir)

    def test_get_batchsize_auto(self, monkeypatch):
        fake_shape = [1, 3, 4]
        monkeypatch.setattr("ais_bench.infer.interface.InferSession.__init__", lambda *args: None)
        monkeypatch.setattr("ais_bench.infer.interface.InferSession.get_inputs", lambda: fake_shape)
        session = InferSession(self.args.model)
        self.args.dym_batch = 0
        self.args.dym_dims = None
        self.args.dym_shape = None
        assert get_batchsize(session, self.args) == fake_shape[0]

    def test_get_batchsize_dym_batch(self, monkeypatch):
        fake_shape = [1, 3, 4]
        def mock_get_inputs(prompt):
            return fake_shape
        monkeypatch.setattr(
            "ais_bench.infer.interface.InferSession.get_inputs",
            mock_get_inputs
        )
        def mock_init_session(prompt):
            return
        monkeypatch.setattr(
            "ais_bench.infer.interface.InferSession.__init__",
            mock_init_session
        )
        session = InferSession(self.args.model)
        self.args.dym_batch = 2
        self.args.dym_dims = None
        self.args.dym_shape = None
        assert get_batchsize(session, self.args) == self.args.dym_batch

    def test_get_batchsize_dym_dims(self, monkeypatch):
        fake_shape = [1, 3, 4]
        def mock_get_inputs(prompt):
            return fake_shape
        monkeypatch.setattr(
            "ais_bench.infer.interface.InferSession.get_inputs",
            mock_get_inputs
        )
        def mock_init_session(prompt):
            return
        monkeypatch.setattr(
            "ais_bench.infer.interface.InferSession.__init__",
            mock_init_session
        )

        session = InferSession(self.args.model)
        bs = 3
        self.args.dym_batch = 0
        self.args.dym_dims = f"data:{bs},600"
        self.args.dym_shape = None
        assert get_batchsize(session, self.args) == bs

    def test_get_batchsize_dym_shapes(self, monkeypatch):
        fake_shape = [1, 3, 4]
        def mock_get_inputs(prompt):
            return fake_shape
        monkeypatch.setattr(
            "ais_bench.infer.InferSession.get_inputs",
            mock_get_inputs
        )
        def mock_init_session(prompt):
            return
        monkeypatch.setattr(
            "ais_bench.infer.InferSession.__init__",
            mock_init_session
        )

        session = InferSession(self.args.model)
        bs = 3
        self.args.dym_batch = 0
        self.args.dym_dims = None
        self.args.dym_shape = f"data:{bs},600"
        assert get_batchsize(session, self.args) == bs

    def test_get_range_list(self):
        ranges = "a:1-3,5;b:2,4~6"
        range_list = get_range_list(ranges)
        assert len(range_list) == 6

    def test_get_throughtput_from_log(self):
        out_log = "throughput 1.01\n"
        ret, throughput = get_throughtput_from_log(out_log)
        assert ret == "OK"
        assert abs(throughput - 1.01) < 1e-5

    def test_regenerate_dymshape_cmd(self):
        cmd_list = regenerate_dymshape_cmd(self.args, self.args.dym_shape)
        assert "--dymShape_range" not in cmd_list


if __name__ == "__main__":
    pytest.main([__file__, "-vs"])