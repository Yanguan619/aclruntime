import argparse

import pytest

from ais_bench.infer.args_check import (
    check_batchsize_valid,
    check_device_range_valid,
    check_dym_str_format,
    check_dym_string,
    check_loop_size,
    check_nonnegative_integer,
    check_number_list,
    check_positive_integer,
)


class TestCheckPositiveInteger:
    def test_valid(self):
        assert check_positive_integer("1") == 1
        assert check_positive_integer("100") == 100
        assert check_positive_integer("2147483647") == 2147483647

    def test_zero(self):
        with pytest.raises(argparse.ArgumentTypeError):
            check_positive_integer("0")

    def test_negative(self):
        with pytest.raises(argparse.ArgumentTypeError):
            check_positive_integer("-1")

    def test_non_numeric(self):
        with pytest.raises(argparse.ArgumentTypeError):
            check_positive_integer("abc")

    def test_overflow(self):
        with pytest.raises((argparse.ArgumentTypeError, argparse.ArgumentError)):
            check_positive_integer("2147483648")


class TestCheckLoopSize:
    def test_valid(self):
        assert check_loop_size("1") == 1
        assert check_loop_size("100000") == 100000

    def test_exceeds_max(self):
        with pytest.raises(argparse.ArgumentTypeError):
            check_loop_size("100001")


class TestCheckNonnegativeInteger:
    def test_valid(self):
        assert check_nonnegative_integer("0") == 0
        assert check_nonnegative_integer("5") == 5

    def test_negative(self):
        with pytest.raises(argparse.ArgumentTypeError):
            check_nonnegative_integer("-1")


class TestCheckNumberList:
    def test_valid(self):
        assert check_number_list("1,2,3") == "1,2,3"

    def test_invalid_char(self):
        with pytest.raises(argparse.ArgumentTypeError):
            check_number_list("1,a,3")

    def test_empty(self):
        assert check_number_list(None) is None


class TestCheckDymString:
    def test_valid(self):
        assert check_dym_string("data:1,3,224,224") == "data:1,3,224,224"

    def test_empty(self):
        assert check_dym_string(None) is None

    def test_invalid_char(self):
        with pytest.raises(argparse.ArgumentTypeError):
            check_dym_string("data:1,3,224,224\x00")


class TestCheckDymStrFormat:
    def test_valid(self):
        from ais_bench.infer.args_check import DYM_RANGE_PATTERN

        check_dym_str_format("data:1,600;img_info:1,600", DYM_RANGE_PATTERN)

    def test_wrong_format(self):
        with pytest.raises(ValueError):
            check_dym_str_format("invalid", "[1-9][0-9]{0,4}")


class TestCheckDeviceRangeValid:
    def test_single_device(self):
        assert check_device_range_valid("0") == 0
        assert check_device_range_valid("255") == 255

    def test_device_list(self):
        assert check_device_range_valid("0,1,2") == [0, 1, 2]

    def test_out_of_range(self):
        with pytest.raises(argparse.ArgumentTypeError):
            check_device_range_valid("256")

    def test_invalid_format(self):
        with pytest.raises(argparse.ArgumentTypeError):
            check_device_range_valid("abc")


class TestCheckBatchsizeValid:
    def test_none(self):
        assert check_batchsize_valid(None) is None

    def test_valid(self):
        assert check_batchsize_valid("4") == 4
