import unittest
import torch
import numpy as np
from ptdbg_ascend.online_dispatch.dump_compare import DispatchRunParam, TimeStatistics, get_compare_result, save_summery, support_basic_type, dump_data, compare_data, save_temp_summery, dispatch_workflow, get_torch_func, dispatch_multiprocess, save_csv
from unittest.mock import patch, MagicMock

class TestDumpCompare(unittest.TestCase):

    def setUp(self):
        self.run_param = DispatchRunParam(True, 0, './npu', './cpu', 1)
        self.run_param.aten_api = 'aten_api'
        self.run_param.single_api_index = 0

    def test_get_compare_result(self):
        npu_data = torch.tensor([1.0, 2.0, 3.0])
        cpu_data = torch.tensor([1.0, 2.0, 3.0])
        result = get_compare_result(npu_data, cpu_data)
        self.assertEqual(result, (1.0, 0.0, 0.0, ''))

    def test_save_summery(self):
        npu_data = torch.tensor([1.0, 2.0, 3.0])
        cpu_data = torch.tensor([1.0, 2.0, 3.0])
        prefix = 'test'
        summery_list = []
        compute_flag = True
        result = save_summery(self.run_param, npu_data, cpu_data, prefix, summery_list, compute_flag)
        self.assertTrue(result)

    def test_support_basic_type(self):
        data = torch.tensor([1.0, 2.0, 3.0])
        result = support_basic_type(data)
        self.assertTrue(result)

    @patch('ptdbg_ascend.online_dispatch.dump_compare.np_save_data')
    def test_dump_data(self, mock_np_save_data):
        data = torch.tensor([1.0, 2.0, 3.0])
        prefix = 'test'
        dump_path = './'
        dump_data(data, prefix, dump_path)
        mock_np_save_data.assert_called_once()

    def test_compare_data(self):
        npu_data = torch.tensor([1.0, 2.0, 3.0])
        cpu_data = torch.tensor([1.0, 2.0, 3.0])
        prefix = 'test'
        summery_list = []
        compute_flag = True
        result = compare_data(self.run_param, npu_data, cpu_data, prefix, summery_list, compute_flag)
        self.assertTrue(result)

    @patch('ptdbg_ascend.online_dispatch.dump_compare.json.dump')
    def test_save_temp_summery(self, mock_json_dump):
        api_index = 0
        single_api_summery = []
        path = './'
        lock = MagicMock()
        save_temp_summery(api_index, single_api_summery, path, lock)
        mock_json_dump.assert_called_once()

    @patch('ptdbg_ascend.online_dispatch.dump_compare.compare_data')
    @patch('ptdbg_ascend.online_dispatch.dump_compare.dump_data')
    def test_dispatch_workflow(self, mock_dump_data, mock_compare_data):
        cpu_args = (torch.tensor([1.0, 2.0, 3.0]),)
        cpu_kwargs = {}
        all_summery = []
        func = torch.sum
        npu_out_cpu = torch.tensor([6.0])
        lock = MagicMock()
        dispatch_workflow(self.run_param, cpu_args, cpu_kwargs, all_summery, func, npu_out_cpu, lock)
        mock_dump_data.assert_called()
        mock_compare_data.assert_called()

    def test_get_torch_func(self):
        self.run_param.func_namespace = 'torch'
        self.run_param.aten_api = 'sum'
        self.run_param.aten_api_overload_name = 'sum'
        result = get_torch_func(self.run_param)
        self.assertEqual(result, torch.sum)

    @patch('ptdbg_ascend.online_dispatch.dump_compare.dispatch_workflow')
    def test_dispatch_multiprocess(self, mock_dispatch_workflow):
        cpu_args = (torch.tensor([1.0, 2.0, 3.0]),)
        cpu_kwargs = {}
        all_summery = []
        npu_out_cpu = torch.tensor([6.0])
        lock = MagicMock()
        self.run_param.func_namespace = 'torch'
        self.run_param.aten_api = 'sum'
        self.run_param.aten_api_overload_name = 'sum'
        dispatch_multiprocess(self.run_param, cpu_args, cpu_kwargs, all_summery, npu_out_cpu, lock)
        mock_dispatch_workflow.assert_called_once()

    @patch('ptdbg_ascend.online_dispatch.dump_compare.pd.DataFrame')
    def test_save_csv(self, mock_df):
        all_summery = [[{'NPU_NAME': 'test', 'BENCH_NAME': 'test', 'NPU_DTYPE': 'float32', 'BENCH_DTYPE': 'float32', 'NPU_SHAPE': '[3]', 'BENCH_SHAPE': '[3]', 'NPU_MAX': [3.0], 'NPU_MIN': [1.0], 'NPU_MEAN': [2.0], 'BENCH_MAX': [3.0], 'BENCH_MIN': [1.0], 'BENCH_MEAN': [2.0], 'COSINE': 1.0, 'MAX_ABS_ERR': 0.0, 'MAX_RELATIVE_ERR': 0.0, 'ACCURACY': 'YES', 'ERROR_MESSAGE': ''}]]
        call_stack_list = ['test']
        csv_path = './test.csv'
        save_csv(all_summery, call_stack_list, csv_path)
        mock_df.assert_called()

