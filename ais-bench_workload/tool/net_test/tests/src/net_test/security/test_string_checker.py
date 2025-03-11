import unittest
from ais_bench.net_test.security.string_checker import StringChecker
from ais_bench.net_test.security.standard_consts import PathLengthLimit, StrBlackPattern, CommandBlackList, \
    StrWhitePattern


class TestStringChecker(unittest.TestCase):
    def setUp(self):
        self.string_checker = StringChecker("")

    # 测试路径合法性（Linux）
    def test_legal_path(self):
        self.string_checker.this_str = "/usr/bin/legal_path"
        self.assertTrue(self.string_checker._is_legal_path_linux())

    def test_long_path(self):
        long_path = "/" + "a" * (PathLengthLimit.LINUX_TOTAL_LENGTH + 1)
        self.string_checker.this_str = long_path
        self.assertFalse(self.string_checker._is_legal_path_linux())

    def test_illegal_characters_in_path(self):
        illegal_path = "/usr/bin/illegal@path"
        self.string_checker.this_str = illegal_path
        self.assertFalse(self.string_checker._is_legal_path_linux())

    def test_long_filename_in_path(self):
        long_filename_path = "/usr/bin/" + "a" * (PathLengthLimit.SINGLE_NAME_LENGTH + 1)
        self.string_checker.this_str = long_filename_path
        self.assertFalse(self.string_checker._is_legal_path_linux())

    # 测试字符串长度合法性
    def test_string_length_legal(self):
        self.string_checker.this_str = "a" * 50
        self.assertTrue(self.string_checker.is_length_legal(50))
        self.assertTrue(self.string_checker.is_length_legal(51))
        self.assertFalse(self.string_checker.is_length_legal(49))

    # 测试黑名单字符
    def test_black_pattern_check(self):
        self.string_checker.this_str = "abc@def"
        self.assertFalse(self.string_checker.is_black_pattern_check_ok(char_set=StrBlackPattern.NORMAL_STR))

    # 测试白名单字符
    def test_white_pattern_check(self):
        self.string_checker.this_str = "abcdefg"
        self.assertTrue(self.string_checker.is_white_pattern_check_ok(char_set=StrWhitePattern.NORMAL_STR))

    # 测试命令黑名单
    def test_cmd_meet_black_list(self):
        self.string_checker.this_str = "ls rm"
        self.assertFalse(self.string_checker.is_cmd_meet_black_list(black_list=CommandBlackList.NORMAL_LINUX))

    # 测试正则表达式全匹配
    def test_full_match_pattern(self):
        self.string_checker.this_str = "123-456-7890"
        pattern = r"\d{3}-\d{3}-\d{4}"
        self.assertTrue(self.string_checker.is_full_match_pattern(pattern))

        self.string_checker.this_str = "123-4567-890"
        self.assertFalse(self.string_checker.is_full_match_pattern(pattern))


if __name__ == '__main__':
    unittest.main()
