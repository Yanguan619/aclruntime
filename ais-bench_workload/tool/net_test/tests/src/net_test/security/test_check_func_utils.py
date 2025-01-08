import os.path
import unittest
from unittest.mock import patch
from ais_bench.net_test.security.check_func_utils import (
    _check_str_length, check_int_string, check_positive_int_string,
    _is_regex_full_match, check_ipv4_string,
    check_bytes_format, check_linux_username,
    _find_executable, check_executable
)
from ais_bench.net_test.common.consts import StringPattern
from tests.utils import FileCreationForTesting


class TestCheckFuncUtils(unittest.TestCase):
    def setUp(self):
        self.file_creator = FileCreationForTesting()

    def tearDown(self):
        self.file_creator.clean_up()

    def test_check_str_length(self):
        with self.assertRaisesRegex(ValueError, 'is not between'):
            _check_str_length("abc", 5, 10)
        with self.assertRaisesRegex(ValueError, 'is not between'):
            _check_str_length("abcdefghijklmno", 5, 10)
        self.assertEqual(_check_str_length(s := "hello", 3, 10), s)

    def test_check_int_string(self):
        with self.assertRaisesRegex(ValueError, 'is an invalid positive int value'):
            check_int_string("abc")
        with self.assertRaisesRegex(ValueError, 'is not between'):
            check_int_string(str(1 << 64))  # 超出范围
        self.assertEqual(check_int_string("123"), 123)

    def test_check_positive_int_string(self):
        for invalid_value in ["0", str(1 << 64)]:
            with self.assertRaisesRegex(ValueError, 'is not between'):
                check_positive_int_string(invalid_value)
        check_positive_int_string(str((1 << 64) - 1))
        self.assertEqual(check_positive_int_string("1"), 1)

    def test_is_regex_full_match(self):
        self.assertTrue(_is_regex_full_match("192.168.1.1", StringPattern.LEGAL_IPV4_PATTERN))
        self.assertFalse(_is_regex_full_match("256.256.256.256", StringPattern.LEGAL_IPV4_PATTERN))

    def test_check_ipv4_string(self):
        for invalid in ["999.999.999.999", "0.0.-1.0", "乱码0.0.256.0", "0.0.0. 0"]:
            with self.assertRaisesRegex(ValueError, ''):
                check_ipv4_string(invalid)
        for valid in ["192.168.1.1", "0.0.0.0", "255.255.255.255"]:
            self.assertEqual(check_ipv4_string(valid), valid)

    def test_check_bytes_format(self):
        self.assertEqual(check_bytes_format("123K"), "123")
        for invalid in ["123", "423kK", "乱码123k", " 1a23k", '1' * 100 + 'k']:
            with self.assertRaisesRegex(ValueError, ''):
                check_bytes_format(invalid)

        for val in range(1, 10000):
            for tail in "KMGkmg":
                self.assertEqual(check_bytes_format(str(val) + tail), str(val))

    def test_check_linux_username(self):
        with self.assertRaisesRegex(ValueError, ''):
            check_linux_username("this_is_a_very_long_username_that_exceeds_the_limit")
        with self.assertRaisesRegex(ValueError, ''):
            check_linux_username("invalid username")
        self.assertEqual(check_linux_username("valid_username"), "valid_username")

    def test_find_executable(self):
        # with self.assertRaisesRegex(ValueError, ''):
        self.assertIsNotNone(_find_executable("bash"))
        self.assertEqual(_find_executable(""), None)
        self.assertEqual(_find_executable("/home/this_is_a_not_exists_path"), None)
        self.assertEqual(_find_executable("this_is_a_not_exists_name"), None)

    @patch('os.environ')
    def test_check_executable(self, mock_environ):
        mock_environ.return_value = {"PATH": "/tmp/this_is_a_very_long_username"}

        file_path = self.file_creator.create_file('test_exe', permission=0o740)
        check_executable(file_path)

        mock_environ.return_value = {"PATH": "other_path_str:{}".format(os.path.dirname(file_path))}
        check_executable(file_path)

        with self.assertRaisesRegex(ValueError, "path's permission is illegal."):
            file_path = self.file_creator.create_file('test_exe2', permission=0o640)
            check_executable(file_path)

        with self.assertRaisesRegex(ValueError, 'path not exist.'):
            check_executable('/not/exists/exe_path')

        with self.assertRaisesRegex(ValueError, 'Cannot find the executable file!'):
            check_executable('not_exists_exe_name')


if __name__ == "__main__":
    unittest.main()
