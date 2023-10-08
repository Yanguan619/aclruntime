import unittest
from unittest.mock import patch, MagicMock
import numpy as np
from ptdbg_ascend.parse_tool.lib import utils

class TestUtils(unittest.TestCase):
    def setUp(self):
        self.util = utils.Util()

    def test_execute_command(self):
        with patch('subprocess.run') as mocked_run:
            mocked_run.return_value.returncode = 0
            result = self.util.execute_command('echo hello')
            self.assertEqual(result, 0)

    def test_print_panel(self):
        with patch('rich.print') as mocked_print:
            self.util.print_panel('Hello')
            mocked_print.assert_called_once()

    def test_check_msaccucmp(self):
        with patch('subprocess.run') as mocked_run:
            mocked_run.return_value.returncode = 0
            result = self.util.check_msaccucmp('msaccucmp.py')
            self.assertEqual(result, 'msaccucmp.py')

    def test_gen_npy_info_txt(self):
        data = np.array([1, 2, 3])
        result = self.util.gen_npy_info_txt(data)
        self.assertEqual(result, '[Shape: (3,)] [Dtype: int64] [Max: 3] [Min: 1] [Mean: 2.0]')

    def test_save_npy_to_txt(self):
        data = np.array([1, 2, 3])
        with patch('numpy.savetxt') as mocked_savetxt:
            self.util.save_npy_to_txt(data, 'test.txt')
            mocked_savetxt.assert_called_once()

    def test_list_convert_files(self):
        with patch('os.walk') as mocked_walk:
            mocked_walk.return_value = [('dir_path', 'dir_names', ['file1', 'file2'])]
            result = self.util.list_convert_files('path')
            self.assertEqual(result, {})

    def test_list_numpy_files(self):
        with patch('os.walk') as mocked_walk:
            mocked_walk.return_value = [('dir_path', 'dir_names', ['file1.npy', 'file2.npy'])]
            result = self.util.list_numpy_files('path')
            self.assertEqual(result, {})

    def test_create_columns(self):
        with patch('rich.columns.Columns') as mocked_columns:
            self.util.create_columns('content')
            mocked_columns.assert_called_once()

    def test_create_table(self):
        with patch('rich.table.Table') as mocked_table:
            self.util.create_table('title', ['column1', 'column2'])
            mocked_table.assert_called_once()

    def test_check_path_valid(self):
        with patch('os.path.exists') as mocked_exists:
            mocked_exists.return_value = True
            self.util.check_path_valid('valid_path')

    def test_npy_info(self):
        data = np.array([1, 2, 3])
        result = self.util.npy_info(data)
        self.assertEqual(result, ((3,), np.dtype('int64'), 3, 1, 2.0))

    def test_check_path_format(self):
        with patch('os.path.isfile') as mocked_isfile:
            mocked_isfile.return_value = True
            self.util.check_path_format('file.txt', '.txt')

