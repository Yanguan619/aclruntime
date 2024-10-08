import unittest
import re
import os
from ais_bench.net_test.security.string_checker import StringChecker


class TestIsLegalPathLinux(unittest.TestCase):
    def setUp(self):
        self.path_checker = StringChecker("")

    def test_legal_path(self):
        self.path_checker.this_str = "/usr/bin/legal_path"
        self.assertTrue(self.path_checker._is_legal_path_linux())

    def test_long_path(self):
        long_path = "/" + "a" * 4097
        self.path_checker.this_str = long_path
        self.assertFalse(self.path_checker._is_legal_path_linux())

    def test_illegal_characters(self):
        illegal_path = "/usr/bin/illegal@path"
        self.path_checker.this_str = illegal_path
        self.assertFalse(self.path_checker._is_legal_path_linux())

    def test_long_filename(self):
        long_filename_path = "/usr/bin/" + "a" * 256
        self.path_checker.this_str = long_filename_path
        self.assertFalse(self.path_checker._is_legal_path_linux())


if __name__ == '__main__':
    unittest.main()
