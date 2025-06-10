import unittest
from unittest.mock import MagicMock, patch
import numpy as np
from ais_bench.infer.summary import Summary, logger, ms_open, Result, ListInfo  # Replace 'your_module' with the actual module name

class TestSummary(unittest.TestCase):
    def setUp(self):
        self.summary = Summary()

    def test_init(self):
        self.summary.reset()
        self.assertEqual(self.summary.h2d_latency_list, [])
        self.assertEqual(self.summary.d2h_latency_list, [])
        self.assertEqual(self.summary.npu_compute_time_list, [])
        self.assertEqual(self.summary.npu_compute_time_interval_list, [])
        self.assertEqual(self.summary._batchsizes, [])
        self.assertEqual(self.summary.infodict, {"filesinfo": {}})

    def test_merge_intervals(self):
        intervals = [(1, 3), (2, 4), (6, 8)]
        merged = Summary.merge_intervals(intervals)
        self.assertEqual(merged, [[1, 4], [6, 8]])

    def test_get_list_info(self):
        work_list = [1, 2, 3, 4, 5]
        percentile_scale = 99
        list_info = Summary.get_list_info(work_list, percentile_scale)
        self.assertEqual(list_info.min, 1.0)
        self.assertEqual(list_info.max, 5.0)
        self.assertEqual(list_info.mean, 3.0)
        self.assertEqual(list_info.median, 3.0)
        self.assertEqual(list_info.percentile, 4.96)

        work_list = []
        Summary.get_list_info(work_list, percentile_scale)
    
    def test_get_list_info1(self):
        # work_list = [1, 2, 3, 4, 5]
        percentile_scale = 99
        # list_info = Summary.get_list_info(work_list, percentile_scale)
        # self.assertEqual(list_info.min, 1.0)
        # self.assertEqual(list_info.max, 5.0)
        # self.assertEqual(list_info.mean, 3.0)
        # self.assertEqual(list_info.median, 3.0)
        # self.assertEqual(list_info.percentile, 4.96)

        work_list = [[1,2]]
        Summary.get_list_info(work_list, percentile_scale, True)

    def test_reset(self):
        self.summary.reset()
        self.assertEqual(self.summary.h2d_latency_list, [])
        self.assertEqual(self.summary.d2h_latency_list, [])
        self.assertEqual(self.summary.npu_compute_time_list, [])
        self.assertEqual(self.summary.npu_compute_time_interval_list, [])
        self.assertEqual(self.summary._batchsizes, [])

    def test_add_batchsize(self):
        self.summary.add_batchsize(32)
        self.summary.add_batchsize(64)
        self.assertEqual(self.summary._batchsizes, [32, 64])

    def test_add_sample_id_infiles(self):
        sample_id = "sample1"
        infiles = ["file1", "file2"]
        self.summary.add_sample_id_infiles(sample_id, infiles)
        self.assertEqual(self.summary.infodict["filesinfo"][sample_id]["infiles"], infiles)

    def test_append_sample_id_outfile(self):
        sample_id = "sample1"
        outfile = "output_file"
        self.summary.append_sample_id_outfile(sample_id, outfile)
        self.assertEqual(self.summary.infodict["filesinfo"][sample_id]["outfiles"], [outfile])

    def test_add_args(self):
        args = {"arg1": "value1", "arg2": "value2"}
        self.summary.add_args(args)
        self.assertEqual(self.summary.infodict["args"], args)

    @patch('ais_bench.infer.summary.logger')
    def test_record(self, mock_logger):
        result = Result()
        result.npu_compute_time = ListInfo()
        result.npu_compute_time.min = 1.0
        result.npu_compute_time.max = 5.0
        result.npu_compute_time.mean = 3.0
        result.npu_compute_time.median = 3.0
        result.npu_compute_time.percentile = 4.95
        result.h2d_latency = ListInfo()
        result.h2d_latency.min = 1.0
        result.h2d_latency.max = 5.0
        result.h2d_latency.mean = 3.0
        result.h2d_latency.median = 3.0
        result.h2d_latency.percentile = 4.95
        result.d2h_latency = ListInfo()
        result.d2h_latency.min = 1.0
        result.d2h_latency.max = 5.0
        result.d2h_latency.mean = 3.0
        result.d2h_latency.median = 3.0
        result.d2h_latency.percentile = 4.95
        result.throughput = 100.0
        result.scale = 99
        result.batchsize = 32

        self.summary.record(result, multi_threads=False)
        self.assertEqual(self.summary.infodict["NPU_compute_time"]["min"], 1.0)
        self.assertEqual(self.summary.infodict["NPU_compute_time"]["max"], 5.0)
        self.assertEqual(self.summary.infodict["NPU_compute_time"]["mean"], 3.0)
        self.assertEqual(self.summary.infodict["NPU_compute_time"]["median"], 3.0)
        self.assertEqual(self.summary.infodict["NPU_compute_time"]["percentile(99%)"], 4.95)
        self.assertEqual(self.summary.infodict["H2D_latency"]["min"], 1.0)
        self.assertEqual(self.summary.infodict["H2D_latency"]["max"], 5.0)
        self.assertEqual(self.summary.infodict["H2D_latency"]["mean"], 3.0)
        self.assertEqual(self.summary.infodict["H2D_latency"]["median"], 3.0)
        self.assertEqual(self.summary.infodict["H2D_latency"]["percentile(99%)"], 4.95)
        self.assertEqual(self.summary.infodict["D2H_latency"]["min"], 1.0)
        self.assertEqual(self.summary.infodict["D2H_latency"]["max"], 5.0)
        self.assertEqual(self.summary.infodict["D2H_latency"]["mean"], 3.0)
        self.assertEqual(self.summary.infodict["D2H_latency"]["median"], 3.0)
        self.assertEqual(self.summary.infodict["D2H_latency"]["percentile(99%)"], 4.95)
        self.assertEqual(self.summary.infodict["throughput"], 100.0)
    
    @patch('ais_bench.infer.summary.logger')
    def test_record1(self, mock_logger):
        result = Result()
        result.npu_compute_time = ListInfo()
        result.npu_compute_time.min = 1.0
        result.npu_compute_time.max = 5.0
        result.npu_compute_time.mean = 3.0
        result.npu_compute_time.median = 3.0
        result.npu_compute_time.percentile = 4.95
        result.h2d_latency = ListInfo()
        result.h2d_latency.min = 1.0
        result.h2d_latency.max = 5.0
        result.h2d_latency.mean = 3.0
        result.h2d_latency.median = 3.0
        result.h2d_latency.percentile = 4.95
        result.d2h_latency = ListInfo()
        result.d2h_latency.min = 1.0
        result.d2h_latency.max = 5.0
        result.d2h_latency.mean = 3.0
        result.d2h_latency.median = 3.0
        result.d2h_latency.percentile = 4.95
        result.throughput = 100.0
        result.scale = 99
        result.batchsize = 32

        self.summary.record(result, multi_threads=True)
        self.assertEqual(self.summary.infodict["H2D_latency"]["mean"], 3.0)

    @patch('ais_bench.infer.summary.logger')
    def test_display(self, mock_logger):
        result = Result()
        result.npu_compute_time = ListInfo()
        result.npu_compute_time.min = 1.0
        result.npu_compute_time.max = 5.0
        result.npu_compute_time.mean = 3.0
        result.npu_compute_time.median = 3.0
        result.npu_compute_time.percentile = 4.95
        result.h2d_latency = ListInfo()
        result.h2d_latency.min = 1.0
        result.h2d_latency.max = 5.0
        result.h2d_latency.mean = 3.0
        result.h2d_latency.median = 3.0
        result.h2d_latency.percentile = 4.95
        result.d2h_latency = ListInfo()
        result.d2h_latency.min = 1.0
        result.d2h_latency.max = 5.0
        result.d2h_latency.mean = 3.0
        result.d2h_latency.median = 3.0
        result.d2h_latency.percentile = 4.95
        result.throughput = 100.0
        result.scale = 99
        result.batchsize = 32

        self.summary.display(result, display_all_summary=True, multi_threads=False)
        mock_logger.info.assert_called_with("------------------------------------------------------")
    
    @patch('ais_bench.infer.summary.logger')
    def test_display1(self, mock_logger):
        result = Result()
        result.npu_compute_time = ListInfo()
        result.npu_compute_time.min = 1.0
        result.npu_compute_time.max = 5.0
        result.npu_compute_time.mean = 3.0
        result.npu_compute_time.median = 3.0
        result.npu_compute_time.percentile = 4.95
        result.h2d_latency = ListInfo()
        result.h2d_latency.min = 1.0
        result.h2d_latency.max = 5.0
        result.h2d_latency.mean = 3.0
        result.h2d_latency.median = 3.0
        result.h2d_latency.percentile = 4.95
        result.d2h_latency = ListInfo()
        result.d2h_latency.min = 1.0
        result.d2h_latency.max = 5.0
        result.d2h_latency.mean = 3.0
        result.d2h_latency.median = 3.0
        result.d2h_latency.percentile = 4.95
        result.throughput = 100.0
        result.scale = 99
        result.batchsize = 32

        self.summary.display(result, display_all_summary=True, multi_threads=True)
        mock_logger.info.assert_called()

    @patch('ais_bench.infer.summary.ms_open')
    @patch('json.dump')
    @patch('ais_bench.infer.summary.Summary.get_list_info')
    def test_report(self, mock_get_list_info, mock_json_dump, mock_ms_open):
        mock_get_list_info.return_value = MagicMock(mean=33)
        result = Result()
        result.npu_compute_time = ListInfo()
        result.npu_compute_time.min = 1.0
        result.npu_compute_time.max = 5.0
        result.npu_compute_time.mean = 3.0
        result.npu_compute_time.median = 3.0
        result.npu_compute_time.percentile = 4.95
        result.h2d_latency = ListInfo()
        result.h2d_latency.min = 1.0
        result.h2d_latency.max = 5.0
        result.h2d_latency.mean = 3.0
        result.h2d_latency.median = 3.0
        result.h2d_latency.percentile = 4.95
        result.d2h_latency = ListInfo()
        result.d2h_latency.min = 1.0
        result.d2h_latency.max = 5.0
        result.d2h_latency.mean = 3.0
        result.d2h_latency.median = 3.0
        result.d2h_latency.percentile = 4.95
        result.throughput = 100.0
        result.scale = 99
        result.batchsize = 32

        self.summary.report(batchsize=32, output_prefix="test", display_all_summary=True, multi_threads=False)
        mock_ms_open.assert_called_with("test_summary.json", mode="w")
        mock_json_dump.assert_called_once()

if __name__ == "__main__":
    unittest.main()