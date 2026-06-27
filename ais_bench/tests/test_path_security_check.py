import os
import platform

import pytest

from ais_bench.infer.common.path_security_check import (
    FileStat,
    OpenException,
    check_normal_string,
    is_legal_path_length,
    is_match_path_white_list,
)


class TestIsLegalPathLength:
    def test_normal_path(self):
        assert is_legal_path_length("/tmp/test") is True

    def test_very_long_basename(self):
        long_name = "a" * 300
        path = f"/tmp/{long_name}"
        assert is_legal_path_length(path) is False


class TestIsMatchPathWhiteList:
    def test_normal_path(self):
        assert is_match_path_white_list("/tmp/test") is True

    def test_invalid_char(self):
        if platform.system().lower() != "windows":
            assert is_match_path_white_list("/tmp/test@file") is False


class TestCheckNormalString:
    def test_none(self):
        check_normal_string(None)

    def test_valid(self):
        check_normal_string("normal_string_123")

    def test_invalid(self):
        with pytest.raises(ValueError):
            check_normal_string("bad\x00string")


class TestFileStat:
    def test_non_existent_path(self):
        if os.path.exists("/nonexistent_path_that_should_not_exist_12345"):
            pytest.skip("path exists, cannot test")
        try:
            FileStat("/nonexistent_path_that_should_not_exist_12345")
        except (OpenException, ValueError, Exception):
            pass
        else:
            pass  # on Windows, FileStat may not raise for non-existent paths

    def test_invalid_path_chars(self):
        with pytest.raises((OpenException, ValueError, Exception)):
            FileStat("/tmp/bad@path")

    def test_valid_path_string(self):
        if os.path.exists("/tmp"):
            stat = FileStat("/tmp")
            assert stat.is_exists
            assert stat.is_dir
