import os
import unittest
from unittest.mock import patch

from ais_bench.net_test.security.other_checker import (check_linux_file_stat_string_from_shell,
    is_disk_space_enough, check_positive_integer_str, is_memory_enough)
from ais_bench.net_test.common.consts import LENGTH_LIMIT
from ais_bench.net_test.security.standard_consts import STAT_STRING_IDX, FileSizeLimit

class TestCheckFuncUtils(unittest.TestCase):
    def setUp(self):
        pass

    def tearDown(self):
        pass

    @patch("shutils.disk_usage", return_value = (0, 0, 1000))
    def test_is_disk_space_enough(self, mock_dick):
        self.assertFalse(is_disk_space_enough(1001))
        self.assertTrue(is_disk_space_enough(999))

    @patch("os.sysconf", return_value = 100)
    def test_is_memory_enough(self, monk_sysconf):
        self.assertFalse(is_memory_enough(100 * 100 + 1))
        self.assertTrue(is_memory_enough(100 * 100))

    def test_check_positive_integer_str(self):
        check_positive_integer_str(None)
        check_positive_integer_str("")

        with self.assertRaisesRegex(ValueError, "is not a string"):
            check_positive_integer_str(["a"])

        long_str = "1" * (LENGTH_LIMIT.MAX_BYTES_STR_LENGTH + 1)
        with self.assertRaisesRegex(ValueError, "is over length limit"):
            check_positive_integer_str(long_str)

        with self.assertRaisesRegex(ValueError, "is an invalid positive int value"):
            check_positive_integer_str("abc")

        with self.assertRaisesRegex(ValueError, "is not positive"):
            check_positive_integer_str("0")

    def test_check_linux_file_stat_string_from_shell(self):
        user = "a"
        size = 10000
        standard_str_list = ["-rwxr-xr-x", "1", user, user, size]

        short_list = ["a" for i in range(STAT_STRING_IDX.SIZE)]
        with self.assertRaisesRegex(ValueError, "path is not a file"):
            check_linux_file_stat_string_from_shell(short_list, user)

        with self.assertRaisesRegex(ValueError, "is not the owner of file"):
            check_linux_file_stat_string_from_shell(standard_str_list, "b")

        softlink_str_list = ["lrwxr-xr-x", "1", user, user, size]
        with self.assertRaisesRegex(ValueError, "file is a softlink"):
            check_linux_file_stat_string_from_shell(softlink_str_list, user)

        permission_over_str_list = ["-rwxrwxr-x", "1", user, user, size]
        with self.assertRaisesRegex(ValueError, "could be write by group/other user"):
            check_linux_file_stat_string_from_shell(permission_over_str_list, user)

        over_size_str_list = ["-rwxr-xr-x", "1", user, user, f"{FileSizeLimit.NORMAL_EXEC_FILE + 1}"]
        with self.assertRaisesRegex(ValueError, "should not be over"):
            check_linux_file_stat_string_from_shell(over_size_str_list, user)

if __name__ == '__main__':
    unittest.main()







