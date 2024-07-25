# Copyright (c) 2024-2024 Huawei Technologies Co., Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#lzy
import sys
import logging
import pytest
import os
from ais_bench.infer.summary import ListInfo, Result, Summary
from test_common import TestCommonClass

logging.basicConfig(
    stream=sys.stdout, level=logging.INFO, format="[%(levelname)s] %(message)s"
)
logger = logging.getLogger(__name__)


class TestClass:
    @classmethod
    def setup_class(cls):
        """
        class level setup_class
        """
        cls.init(TestClass)

    @classmethod
    def teardown_class(cls):
        logger.info('\n ---class level teardown_class')

    def init(self):
        self.summary = Summary()
        self.result = Result()
        self.list_info = ListInfo()
        self.result.d2h_latency = self.listInfo
        self.result.npu_compute_time = self.listInfo
        self.result.h2d_latency = self.listInfo

    def test_get_list_info_merged(self):
        intervals_list = [[0, 1.0], [1.0, 2.0], [3.0, 5.0], [4.0, 6.0], [8.0, 9.0]]
        list_info = self.summary.get_list_info(
            work_list=intervals_list,
            percentile_scale=90,
            merge=True
        )
        assert abs(list_info.mean - 1.2) <= 1e-5

    def test_get_list_info_not_merged(self):

        work_list = [1.0, 2.0, 3.0, 4.0, 5.0]
        list_info = self.summary.get_list_info(
            work_list=work_list,
            percentile_scale=60,
            merge=True
        )
        assert abs(list_info.min - 1.0) <= 1e-5
        assert abs(list_info.max - 5.0) <= 1e-5
        assert abs(list_info.median - 3.0) <= 1e-5
        assert abs(list_info.mean - 3.0) <= 1e-5
        assert abs(list_info.percentile - 3.0) <= 1e-5

    def test_add_batchsize(self):
        bs = 4
        self.summary.reset()
        self.summary.add_batchsize(bs)
        assert self.summary._batchsizes[-1] == bs

    def test_add_sample_id_infiles(self):
        self.summary.infodict = {"filesinfo": {}}
        sample_id = 0
        infiles = ["infile1", "infile2"]
        self.summary.add_sample_id_infiles(sample_id, infiles)
        assert self.summary.infodict["filesinfo"][sample_id].get("infiles")[0] == infiles[0]
        assert self.summary.infodict["filesinfo"][sample_id].get("infiles")[1] == infiles[1]

    def test_add_sample_id_outfiles(self):
        self.summary.infodict = {"filesinfo": {}}
        sample_id = 0
        outfile = "outfile1"
        self.summary.append_sample_id_outfile(sample_id, outfile)
        assert self.summary.infodict["filesinfo"][sample_id].get("outfiles")[0] == outfile

    def test_add_args(self):
        fake_args = "arg1"
        self.summary.add_args(args=fake_args)
        assert self.summary.infodict.get("args") == fake_args

    def test_record_normal(self):
        multi_threads = True
        self.summary.record(self.result, multi_threads)
        multi_threads = False
        self.summary.record(self.result, multi_threads)

    def test_record_normal(self):
        target_time_list_size = 6
        self.summary.reset()
        self._init_result_normal()
        self.summary.record(self.result, multi_threads=False)
        assert len(self.summary.infodict.get('NPU_compute_time')) == target_time_list_size
        assert len(self.summary.infodict.get('H2D_latency')) == target_time_list_size
        assert len(self.summary.infodict.get('D2H_latency')) == target_time_list_size
        assert self.summary.infodict.get('npu_compute_time_list') == self.summary.npu_compute_time_interval_list
        assert self.summary.infodict.get('throughput') is not None
        assert self.summary.infodict.get('pid') is not None

    def test_record_multi_thread(self):
        target_time_list_size = 2
        self.summary.reset()
        self._init_result_multi_threads()
        self.summary.record(self.result, multi_threads=True)
        assert len(self.summary.infodict.get('NPU_compute_time')) == target_time_list_size
        assert len(self.summary.infodict.get('H2D_latency')) == target_time_list_size
        assert len(self.summary.infodict.get('D2H_latency')) == target_time_list_size
        assert self.summary.infodict.get('npu_compute_time_list') == self.summary.npu_compute_time_interval_list

    def test_display(self):
        self.summary.reset()
        self._init_result_normal()
        self.summary.display(self.result, diaplay_all_summary=False, multi_threads=False)
        self.summary.display(self.result, diaplay_all_summary=True, multi_threads=False)
        self._init_result_multi_threads()
        self.summary.display(self.result, diaplay_all_summary=False, multi_threads=True)
        self.summary.display(self.result, diaplay_all_summary=True, multi_threads=True)

    def test_report(self, monkeypatch):
        self._init_summary()
        self._init_list_info()
        monkeypatch.setattr(
            "ais_bench.infer.summary.Summary.get_list_info",
            lambda x, y: self.list_info
        )
        monkeypatch.setattr(
            "ais_bench.infer.summary.Summary.display",
            lambda x, y, z: None
        )
        output_prefix = TestCommonClass.base_path + "/test"
        bs = 3

        # expect Exception,pipeline format list and normal list can't exist both
        with pytest.raises(Exception):
            self.summary.report(batchsize=bs, output_prefix=output_prefix)

        self.summary.npu_compute_time_interval_list = []
        out_summary_json_path = output_prefix + "_summary.json"
        if os.path.exists(out_summary_json_path):
            os.remove(out_summary_json_path)
        self.summary.report(batchsize=bs, output_prefix=output_prefix)
        assert os.path.exists(out_summary_json_path)

    def _init_result_normal(self):
        time_list = [1, 2, 3]
        scale = 100
        self.result.npu_compute_time = self.summary.get_list_info(time_list, scale)
        self.result.h2d_latency = self.summary.get_list_info(time_list, scale)
        self.result.d2h_latency = self.summary.get_list_info(time_list, scale)
        self.throughput = 10.0
        self.batchsize = 1

    def _init_result_multi_threads(self):
        time_list = [[0, 1], [2, 3], [3, 4]]
        scale = 100
        self.result.npu_compute_time = self.summary.get_list_info(time_list, scale, True)
        self.result.h2d_latency = self.summary.get_list_info(time_list, scale, True)
        self.result.d2h_latency = self.summary.get_list_info(time_list, scale, True)

    def _init_summary(self):
        self.summary.h2d_latency_list = [1]
        self.summary.d2h_latency_list = [1]
        self.summary.npu_compute_time_list = [1]
        self.summary.npu_compute_time_interval_list = [1]
        self.summary._batchsizes = [1]

    def _init_list_info(self):
        self.list_info.min = 1.0
        self.list_info.max = 5.0
        self.list_info.mean = 3.0
        self.list_info.median = 3.0
        self.list_info.percentile = 3.0
if __name__ == "__main__":
    pytest.main([__file__, "-vs"])