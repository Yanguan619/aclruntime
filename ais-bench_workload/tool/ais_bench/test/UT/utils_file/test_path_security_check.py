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
import json
import shutil
import pytest

from test_common import TestCommonClass
from ais_bench.infer.common.path_security_check import (
    is_legal_path_length, is_match_path_white_list, is_legal_args_path_string,
    FileStat, ms_open, check_normal_string,
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

    def init(self):
        self.standard_file_path = os.path.join(TestCommonClass.base_path, "resnet50/model/pth_resnet50_bs1.om")

    def test_is_legal_path_length_linux(self, monkeypatch):
        monkeypatch.setattr("sys.platform.startswith", lambda *arg: False)
        path = ""
        for _ in range(4097):
            path = path + "x"
        assert is_legal_path_length(path) == False

        path = "/"
        for _ in range(256):
            path = path + "x"
        assert is_legal_path_length(path) == False

        path = "xxx"
        assert is_legal_path_length(path) == True

    def test_is_legal_path_length_windows(self, monkeypatch):
        monkeypatch.setattr("sys.platform.startswith", lambda *arg: True)
        path = ""
        for _ in range(261):
            path = path + "x"
        assert is_legal_path_length(path) == False

    def test_is_match_path_white_list_linux(self, monkeypatch):
        monkeypatch.setattr("sys.platform.startswith", lambda *arg: False)
        path = "/home&*/"
        assert is_match_path_white_list(path) == False

    def test_is_match_path_white_list_windows(self, monkeypatch):
        monkeypatch.setattr("sys.platform.startswith", lambda *arg: True)
        path = "D:\home&"
        assert is_match_path_white_list(path) == False
        path = "D:\home"
        assert is_match_path_white_list(path) == True

    def test_is_legal_args_path_string(self, monkeypatch):
        assert is_legal_args_path_string("") == True

        monkeypatch.setattr("ais_bench.infer.common.path_security_check.is_legal_path_length", lambda *arg: False)
        assert is_legal_args_path_string("") == False
        monkeypatch.stopall()

        monkeypatch.setattr("ais_bench.infer.common.path_security_check.is_match_path_white_list", lambda *arg: False)
        assert is_legal_args_path_string("") == False
        monkeypatch.stopall()

        assert is_legal_args_path_string("/home") == True

    def test_file_stat_exception(self, monkeypatch):
        monkeypatch.setattr("ais_bench.infer.common.path_security_check.is_legal_path_length", lambda *arg: False)
        monkeypatch.setattr("ais_bench.infer.common.path_security_check.is_match_path_white_list", lambda *arg: False)
        with pytest.raises(KeyError) as e:
            FileStat(self.standard_file_path)
            if not "create FileStat failed" in str(e):
                pytest.fail("do not catch expected err!")

    def test_file_stat_property(self):
        file_stat = FileStat(self.standard_file_path)
        assert file_stat.is_exists
        assert not file_stat.is_softlink
        assert file_stat.is_file
        assert not file_stat.is_dir
        assert file_stat.file_size > 0
        assert file_stat.permission == 0o600
        assert file_stat.owner != -1
        assert file_stat.group_owner != -1
        assert file_stat.is_owner
        assert file_stat.is_group_owner
        assert file_stat.is_user_or_group_owner
        assert file_stat.is_user_and_group_owner

    def test_is_basically_legal(self, monkeypatch):
        file_stat = FileStat(self.standard_file_path)
        monkeypatch.setattr("sys.platform.startswith", lambda *arg: True)
        monkeypatch.setattr(
            "ais_bench.infer.common.path_security_check.FileStat.check_windows_permission",
            lambda *arg: True
        )
        assert file_stat.is_basically_legal()
        monkeypatch.stopall()

        monkeypatch.setattr("sys.platform.startswith", lambda *arg: False)
        monkeypatch.setattr(
            "ais_bench.infer.common.path_security_check.FileStat.check_windows_permission",
            lambda *arg: True
        )
        assert file_stat.is_basically_legal()
