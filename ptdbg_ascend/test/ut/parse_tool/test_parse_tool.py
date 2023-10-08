import unittest
from unittest.mock import patch, MagicMock
from src.python.ptdbg_ascend.parse_tool.lib.parse_tool import ParseTool
# 由于ParseTool类的方法大部分都涉及到文件操作和命令行参数解析无法断言，只检查相关方法是否被正确调用。
# 使用unittest.mock.patch来模拟argparse.ArgumentParser和Util类的方法。
# 使用assert_called_once_with和assert_any_call来检查这些方法是否被正确调用
class TestParseTool(unittest.TestCase):
    def setUp(self):
        self.parse_tool = ParseTool()

    @patch('src.python.ptdbg_ascend.parse_tool.lib.parse_tool.Util')
    def test_prepare(self, mock_util):
        self.parse_tool.prepare()
        mock_util.create_dir.assert_called_once()

    @patch('src.python.ptdbg_ascend.parse_tool.lib.parse_tool.argparse.ArgumentParser')
    @patch('src.python.ptdbg_ascend.parse_tool.lib.parse_tool.Util')
    def test_do_vector_compare(self, mock_util, mock_argparse):
        mock_args = MagicMock()
        mock_args.my_dump_path = '/path/to/my_dump'
        mock_args.golden_dump_path = '/path/to/golden_dump'
        mock_args.output_path = '/path/to/output'
        mock_args.ascend_path = '/path/to/ascend'
        mock_argparse.parse_args.return_value = mock_args
        self.parse_tool.do_vector_compare()
        mock_util.check_path_valid.assert_any_call('/path/to/my_dump')
        mock_util.check_path_valid.assert_any_call('/path/to/golden_dump')
        mock_util.check_files_in_path.assert_any_call('/path/to/my_dump')
        mock_util.check_files_in_path.assert_any_call('/path/to/golden_dump')

    @patch('src.python.ptdbg_ascend.parse_tool.lib.parse_tool.argparse.ArgumentParser')
    @patch('src.python.ptdbg_ascend.parse_tool.lib.parse_tool.Util')
    def test_do_convert_dump(self, mock_util, mock_argparse):
        mock_args = MagicMock()
        mock_args.path = '/path/to/dump'
        mock_args.format = 'format'
        mock_args.output_path = '/path/to/output'
        mock_args.ascend_path = '/path/to/ascend'
        mock_argparse.parse_args.return_value = mock_args
        self.parse_tool.do_convert_dump()
        mock_util.check_path_valid.assert_called_once_with('/path/to/dump')
        mock_util.check_files_in_path.assert_called_once_with('/path/to/dump')

    @patch('src.python.ptdbg_ascend.parse_tool.lib.parse_tool.argparse.ArgumentParser')
    @patch('src.python.ptdbg_ascend.parse_tool.lib.parse_tool.Visualization')
    def test_do_print_data(self, mock_visual, mock_argparse):
        mock_args = MagicMock()
        mock_args.path = '/path/to/data'
        mock_argparse.parse_args.return_value = mock_args
        self.parse_tool.do_print_data()
        mock_visual.print_npy_data.assert_called_once_with('/path/to/data')

    @patch('src.python.ptdbg_ascend.parse_tool.lib.parse_tool.argparse.ArgumentParser')
    @patch('src.python.ptdbg_ascend.parse_tool.lib.parse_tool.Visualization')
    def test_do_parse_pkl(self, mock_visual, mock_argparse):
        mock_args = MagicMock()
        mock_args.file_name = '/path/to/file'
        mock_args.api_name = 'api_name'
        mock_argparse.parse_args.return_value = mock_args
        self.parse_tool.do_parse_pkl()
        mock_visual.parse_pkl.assert_called_once_with('/path/to/file', 'api_name')

    @patch('src.python.ptdbg_ascend.parse_tool.lib.parse_tool.argparse.ArgumentParser')
    @patch('src.python.ptdbg_ascend.parse_tool.lib.parse_tool.Util')
    def test_do_compare_data(self, mock_util, mock_argparse):
        mock_args = MagicMock()
        mock_args.my_dump_path = '/path/to/my_dump'
        mock_args.golden_dump_path = '/path/to/golden_dump'
        mock_args.count = 20
        mock_args.save = True
        mock_args.atol = 0.001
        mock_args.rtol = 0.001
        mock_argparse.parse_args.return_value = mock_args
        self.parse_tool.do_compare_data()
        mock_util.check_path_valid.assert_any_call('/path/to/my_dump')
        mock_util.check_path_valid.assert_any_call('/path/to/golden_dump')
        mock_util.check_path_format.assert_any_call('/path/to/my_dump', 'NPY_SUFFIX')
        mock_util.check_path_format.assert_any_call('/path/to/golden_dump', 'NPY_SUFFIX')

 