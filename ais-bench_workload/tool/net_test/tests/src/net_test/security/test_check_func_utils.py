import unittest
from unittest.mock import patch, mock_open
from ais_bench.net_test.security.check_func_utils import (
    _check_str_length, check_int_string, check_positive_int_string,
    check_exe_path, is_regex_fullmatch, check_ipv4_string,
    check_bytes_format, check_linux_username, transform_hostfile_line,
    parse_hostfile, find_executable, check_executable
)
from ais_bench.net_test.common.consts import LENGTH_LIMIT, INT_LIMIT, STRING_PATTERN, OTHERS
from ais_bench.net_test.security.file_checker import check_linux_executable_file
import pytest


class TestCheckFuncUtils(unittest.TestCase):

    def test_check_str_length(self):
        with self.assertRaisesRegex(ValueError, 'is not between'):
            _check_str_length("abc", 5, 10)
        with self.assertRaisesRegex(ValueError, 'is not between'):
            _check_str_length("abcdefghijklmno", 5, 10)
        self.assertIsNone(_check_str_length("hello", 3, 10))

    def test_check_int_string(self):
        with self.assertRaisesRegex(ValueError, 'is an invalid positive int value'):
            check_int_string("abc")
        with self.assertRaisesRegex(ValueError, 'is not between'):
            check_int_string(str(1 << 64))  # 超出范围
        self.assertEqual(check_int_string("123"), 123)

    def test_check_positive_int_string(self):
        for unvalid_value in ["0", str(1 << 64)]:
            with self.assertRaisesRegex(ValueError, 'is not between'):
                check_positive_int_string(unvalid_value)
        check_positive_int_string(str((1 << 64) - 1))
        self.assertEqual(check_positive_int_string("1"), 1)

    def test_is_regex_fullmatch(self):
        self.assertTrue(is_regex_fullmatch("192.168.1.1", STRING_PATTERN.LEGAL_IPV4_PATTERN))
        self.assertFalse(is_regex_fullmatch("256.256.256.256", STRING_PATTERN.LEGAL_IPV4_PATTERN))

    def test_check_ipv4_string(self):
        for unvalid in ["999.999.999.999", "0.0.-1.0", "乱码0.0.256.0", "0.0.0. 0"]:
            with self.assertRaisesRegex(ValueError, ''):
                check_ipv4_string(unvalid)
        for valid in ["192.168.1.1", "0.0.0.0", "255.255.255.255"]:
            self.assertEqual(check_ipv4_string(valid), valid)

    def test_check_bytes_format(self):
        self.assertEqual(check_bytes_format("123K"), "123")
        for unvalid in ["123", "423kK", "乱码123k", " 1a23k", '1' * 100 + 'k']:
            with self.assertRaisesRegex(ValueError, ''):
                check_bytes_format(unvalid)

        for val in range(1, 10000):
            for tail in "KMGkmg":
                assert check_bytes_format(str(val) + tail) == str(val)

    def test_check_linux_username(self):
        with self.assertRaisesRegex(ValueError, ''):
            check_linux_username("this_is_a_very_long_username_that_exceeds_the_limit")
        with self.assertRaisesRegex(ValueError, ''):
            check_linux_username("invalid username")
        self.assertEqual(check_linux_username("validusername"), "validusername")

    def test_transform_hostfile_line(self):
        unvalids = ["192.168.1.1:abcd:root", "192.168.1.1:0:root:22", "192.168.1.1:0", "192.168.256.1:3:root:22",
                    "192.168.1.1:2:root:65536", "192.168.1.1:2:root:65535:", ]
        for unvalid in unvalids:
            with self.assertRaisesRegex(ValueError, ''):
                transform_hostfile_line(unvalid)
        valids = ["192.168.1.1:1:a:22", "192.168.1.1:1", "192.168.1.1:2:root:65535", ]
        for valid in valids:
            transform_hostfile_line(valid)
        self.assertEqual(transform_hostfile_line("192.168.1.1:1:root:22"), ('192.168.1.1', 1, 'root', 22))

    @patch("builtins.open", new_callable=mock_open, read_data="192.168.1.1:1:root:22\n192.168.1.2:2:user:22\n")
    def test_parse_hostfile(self, mock_file):
        hostfile = "mock_hostfile.txt"
        expected_output = [
            ('192.168.1.1', 1, 'root', 22),
            ('192.168.1.2', 2, 'user', 22)
        ]
        self.assertEqual(parse_hostfile(hostfile), expected_output)

    # todo, test_find_executable, test_check_executable


if __name__ == "__main__":
    unittest.main()
