import unittest
from unittest.mock import patch
from src.python.ptdbg_ascend.compare.distributed_compare import compare_distributed

class TestDistributedCompare(unittest.TestCase):

    @patch('src.python.ptdbg_ascend.compare.distributed_compare.compare_distributed.check_and_return_dir_contents')
    @patch('src.python.ptdbg_ascend.compare.distributed_compare.compare_distributed.extract_pkl_and_data_dir')
    def test_compare_distributed(self, mock_extract, mock_check):
        mock_check.return_value = ['rank1', 'rank2']
        mock_extract.return_value = ('pkl_path', 'dump_data_dir')
        npu_dump_dir = 'npu_dump_dir'
        bench_dump_dir = 'bench_dump_dir'
        output_path = 'output_path'
        compare_distributed(npu_dump_dir, bench_dump_dir, output_path)
        mock_check.assert_called()
        mock_extract.assert_called()

