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
        self.standard_file_path = os.path.join(TestCommonClass.base_path, "resnet50/fake_model.json")
        if os.path.exists(self.standard_file_path):
            os.remove(self.standard_file_path)
        with open(self.standard_file_path, "w") as file:
            json.dump({"key": "value"}, file)
        os.chmod(self.standard_file_path, 0o600)
        self.end_label = "end"

    def test_is_legal_path_length_linux(self, monkeypatch):
        monkeypatch.setattr("ais_bench.infer.common.path_security_check.is_platform", lambda *arg: False)
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
        monkeypatch.setattr("ais_bench.infer.common.path_security_check.is_platform", lambda *arg: True)
        path = ""
        for _ in range(261):
            path = path + "x"
        assert is_legal_path_length(path) == False

    def test_is_match_path_white_list_linux(self, monkeypatch):
        monkeypatch.setattr("ais_bench.infer.common.path_security_check.is_platform", lambda *arg: False)
        path = "/home&*/"
        assert is_match_path_white_list(path) == False

    def test_is_match_path_white_list_windows(self, monkeypatch):
        monkeypatch.setattr("ais_bench.infer.common.path_security_check.is_platform", lambda *arg: True)
        path = "D:\home&"
        assert is_match_path_white_list(path) == False
        path = "D:\home"
        assert is_match_path_white_list(path) == True

    def test_is_legal_args_path_string(self, monkeypatch):
        assert is_legal_args_path_string("") == True

        monkeypatch.setattr("ais_bench.infer.common.path_security_check.is_legal_path_length", lambda *arg: False)
        assert is_legal_args_path_string("/home") == False
        monkeypatch.undo()

        monkeypatch.setattr("ais_bench.infer.common.path_security_check.is_match_path_white_list", lambda *arg: False)
        assert is_legal_args_path_string("/home") == False
        monkeypatch.undo()

        assert is_legal_args_path_string("/home") == True

    def test_file_stat_exception(self, monkeypatch):
        monkeypatch.setattr("ais_bench.infer.common.path_security_check.is_legal_path_length", lambda *arg: False)
        monkeypatch.setattr("ais_bench.infer.common.path_security_check.is_match_path_white_list", lambda *arg: False)
        with pytest.raises(Exception) as e:
            FileStat(self.standard_file_path)
            if not "create FileStat failed" in str(e):
                pytest.fail(f"Do not catch expected err! Actual error is {str(e)}")

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
        monkeypatch.setattr("ais_bench.infer.common.path_security_check.is_platform", lambda *arg: True)
        monkeypatch.setattr(
            "ais_bench.infer.common.path_security_check.FileStat.check_windows_permission",
            lambda *arg: True
        )
        assert file_stat.is_basically_legal()
        monkeypatch.undo()

        monkeypatch.setattr("ais_bench.infer.common.path_security_check.is_platform", lambda *arg: False)
        monkeypatch.setattr(
            "ais_bench.infer.common.path_security_check.FileStat.check_windows_permission",
            lambda *arg: True
        )
        assert file_stat.is_basically_legal()

    def test_check_linux_permission(self, monkeypatch):
        file_stat = FileStat(self.standard_file_path)
        file_stat.is_file_exist = False
        assert not file_stat.check_linux_permission(perm="read")

        file_stat = FileStat(self.standard_file_path)
        monkeypatch.setattr("os.path.islink", lambda *arg: True)
        assert not file_stat.check_linux_permission(perm="write")
        monkeypatch.undo()

        monkeypatch.setattr("stat.S_IMODE", lambda *arg: 0o770)
        assert not file_stat.check_linux_permission(perm="read")
        monkeypatch.undo()

        monkeypatch.setattr("os.access", lambda *arg: False)
        assert not file_stat.check_linux_permission(perm="read")
        monkeypatch.undo()

        monkeypatch.setattr("stat.S_IMODE", lambda *arg: 0o755)
        assert not file_stat.check_linux_permission(perm="write")
        monkeypatch.undo()

        monkeypatch.setattr("os.access", lambda *arg: False)
        assert not file_stat.check_linux_permission(perm="write")
        monkeypatch.undo()

        assert file_stat.check_windows_permission(perm="read")

    def test_check_windows_permission(self, monkeypatch):
        file_stat = FileStat(self.standard_file_path)
        file_stat.is_file_exist = False
        assert not file_stat.check_windows_permission(perm="read")

        file_stat = FileStat(self.standard_file_path)
        monkeypatch.setattr("os.path.islink", lambda *arg: True)
        assert not file_stat.check_windows_permission(perm="write")
        monkeypatch.undo()

        assert file_stat.check_windows_permission(perm="read")

    def test_is_legal_file_size(self, monkeypatch):
        file_stat = FileStat(self.standard_file_path)
        monkeypatch.setattr("stat.S_ISREG", lambda *arg: False)
        assert not file_stat.is_legal_file_size(1)
        monkeypatch.undo()

        assert not file_stat.is_legal_file_size(1)

        size_big_enough = 50* 1024 * 1024 * 1024
        assert file_stat.is_legal_file_size(size_big_enough)

    def test_is_legal_file_type(self, monkeypatch):
        file_stat = FileStat(self.standard_file_path)
        monkeypatch.setattr("stat.S_ISREG", lambda *arg: False)
        assert not file_stat.is_legal_file_type([])
        monkeypatch.undo()

        assert file_stat.is_legal_file_type(["json"])

        assert file_stat.is_legal_file_type(["invalid"])

    def test_ms_open_exist_dir(self, monkeypatch):
        monkeypatch.setattr("ais_bench.infer.common.path_security_check.FileStat.is_exists", lambda *arg: True)
        monkeypatch.setattr("ais_bench.infer.common.path_security_check.FileStat.is_dir", lambda *arg: True)
        with pytest.raises(Exception) as e:
            ms_open(self.standard_file_path)
            if not "but it's a folder" in str(e):
                pytest.fail(f"Do not catch expected err! Actual error is {str(e)}")

    def test_ms_open_softlink(self, monkeypatch):
        monkeypatch.setattr("ais_bench.infer.common.path_security_check.FileStat.is_softlink", lambda *arg: True)
        with pytest.raises(Exception) as e:
            ms_open(self.standard_file_path, mode="r")
            if not "Softlink is not allowed" in str(e):
                pytest.fail(f"Do not catch expected err! Actual error is {str(e)}")

    def test_ms_open_read(self, monkeypatch):
        monkeypatch.setattr("ais_bench.infer.common.path_security_check.FileStat.is_exists", lambda *arg: False)
        with pytest.raises(Exception) as e:
            ms_open(self.standard_file_path, mode="r")
            if not "No such file or directory" in str(e):
                pytest.fail(f"Do not catch expected err! Actual error is {str(e)}")
        monkeypatch.undo()

        with pytest.raises(Exception) as e:
            ms_open(self.standard_file_path, mode="r")
            if not "must have a size limit" in str(e):
                pytest.fail(f"Do not catch expected err! Actual error is {str(e)}")

        with pytest.raises(Exception) as e:
            ms_open(self.standard_file_path, mode="r", max_size=1)
            if not "The file size has exceeded" in str(e):
                pytest.fail(f"Do not catch expected err! Actual error is {str(e)}")

    def test_ms_open_write(self, monkeypatch):
        monkeypatch.setattr("ais_bench.infer.common.path_security_check.FileStat.is_exists", lambda *arg: True)
        monkeypatch.setattr("ais_bench.infer.common.path_security_check.FileStat.is_owner", lambda *arg: False)
        with pytest.raises(Exception) as e:
            ms_open(self.standard_file_path, mode="w")
            if not "file owner is inconsistent" in str(e):
                pytest.fail(f"Do not catch expected err! Actual error is {str(e)}")
        monkeypatch.undo()

        # monkeypatch.setattr("os.open", lambda *arg: None)
        # monkeypatch.setattr("os.fdopen", lambda *arg, **kwargs: self.end_label)
        # monkeypatch.setattr("os.remove", lambda *arg: None)
        # assert ms_open(self.standard_file_path, mode="w") ==self.end_label

    def test_ms_open_add(self, monkeypatch):
        monkeypatch.setattr("os.open", lambda *arg: None)
        monkeypatch.setattr("os.fdopen", lambda *arg, **kwargs: self.end_label)
        monkeypatch.setattr("ais_bench.infer.common.path_security_check.FileStat.is_owner", lambda *arg: False)
        with pytest.raises(Exception) as e:
            ms_open(self.standard_file_path, mode="a")
            if not "file owner is inconsistent" in str(e):
                pytest.fail(f"Do not catch expected err! Actual error is {str(e)}")
        monkeypatch.undo()

        # monkeypatch.setattr("os.fdopen", lambda *arg, **kwargs: self.end_label)
        # monkeypatch.setattr("os.open", lambda *arg: None)
        # monkeypatch.setattr("os.chmod", lambda *arg: None)
        # monkeypatch.setattr("ais_bench.infer.common.path_security_check.FileStat.permission", lambda *arg: 0o100)
        # assert ms_open(self.standard_file_path, mode="a") == self.end_label

    def test_ms_open_normal(self, monkeypatch):
        monkeypatch.setattr("os.fdopen", lambda *arg, **kwargs: self.end_label)
        monkeypatch.setattr("os.open", lambda *arg: None)
        monkeypatch.setattr("os.chmod", lambda *arg: None)
        monkeypatch.setattr("os.remove", lambda *arg: None)

        # assert ms_open(self.standard_file_path, mode="+") == self.end_label
        # assert ms_open(self.standard_file_path, mode="w") == self.end_label
        # assert ms_open(self.standard_file_path, mode="a") == self.end_label





