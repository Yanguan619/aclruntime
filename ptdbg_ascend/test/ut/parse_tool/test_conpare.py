import unittest
import numpy as np
from src.python.ptdbg_ascend.parse_tool.lib.compare import Compare

# - my_dump_path：要比较的第一个文件或目录的路径。
# - golden_dump_path：要比较的第二个文件或目录的路径。
# - result_dir：想要存储比较结果的目录的路径。
# - dump_file：想要转换的文件或目录的路径。
# - output：想要存储转换结果的目录的路径。
# - left和right：想要比较的两个numpy数组文件的路径。
class TestCompare(unittest.TestCase):
    def setUp(self):
        self.compare = Compare()

    def test_call_msaccucmp(self):
        result = self.compare.call_msaccucmp
        self.assertIsNotNone(result)

    def test_npu_vs_npu_compare(self):
        # 替换为实际的路径
        my_dump_path = 'path_to_my_dump'
        golden_dump_path = 'path_to_golden_dump'
        result_dir = 'path_to_result_dir'
        self.compare.npu_vs_npu_compare(my_dump_path, golden_dump_path, result_dir)

    def test_compare_vector(self):
        # 替换为实际的路径
        my_dump_path = 'path_to_my_dump'
        golden_dump_path = 'path_to_golden_dump'
        result_dir = 'path_to_result_dir'
        result = self.compare.compare_vector(my_dump_path, golden_dump_path, result_dir)
        self.assertIsNotNone(result)

    def test_convert_dump_to_npy(self):
        # 替换为实际的路径
        dump_file = 'path_to_dump_file'
        data_format = 'data_format'
        output = 'path_to_output'
        self.compare.convert_dump_to_npy(dump_file, data_format, output)

    def test_convert(self):
        # 替换为实际的路径
        dump_file = 'path_to_dump_file'
        data_format = 'data_format'
        output = 'path_to_output'
        result = self.compare.convert(dump_file, data_format, output)
        self.assertIsNotNone(result)

    def test_compare_data(self):
        # 替换为实际的路径
        left = 'path_to_left'
        right = 'path_to_right'
        self.compare.compare_data(left, right)

    def test__do_compare_data(self):
        left = np.array([1, 2, 3])
        right = np.array([1, 2, 3])
        total_cnt, all_close, cos_sim, err_percent = self.compare._do_compare_data(left, right)
        self.assertEqual(total_cnt, 3)
        self.assertTrue(all_close)
        self.assertEqual(cos_sim, 1.0)
        self.assertEqual(err_percent, 0.0)

