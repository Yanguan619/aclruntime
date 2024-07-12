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

logging.basicConfig(
    stream=sys.stdout, level=logging.INFO, format="[%(levelname)s] %(message)s"
)
logger = logging.getLogger(__name__)


class TestClass:

    summary = Summary()
    result = Result()
    listInfo = ListInfo()
    result.d2h_latency = listInfo
    result.npu_compute_time = listInfo
    result.h2d_latency = listInfo

    def test_get_list_info(self):
        work_list = [[1, 2], [1, 3], [3, 4], [5, 6], [6, 7]]
        percentile_scale = 90
        merge = True
        self.summary.get_list_info(work_list, percentile_scale, merge)
        merge = False
        self.summary.get_list_info(work_list, percentile_scale, merge)

    def test_add_batchsize(self):
        self.summary.add_batchsize(1)

    def test_add_sample_id_infiles(self):
        sample_id = 0
        infiles = ["infile1", "infile2", "infile3"]
        self.summary.add_sample_id_infiles(sample_id, infiles)

    def test_add_sample_id_outfiles(self):
        sample_id = 0
        outfiles = "outfile1"
        self.summary.append_sample_id_outfile(sample_id, outfiles)

    def test_add_args(self):
        args = "args"
        self.summary.add_args(args)

    def test_record(self):
        multi_threads = True
        self.summary.record(self.result, multi_threads)
        multi_threads = False
        self.summary.record(self.result, multi_threads)

    def test_display(self):
        diaplay_all_summary = True
        multi_threads = True
        self.summary.display(self.result, diaplay_all_summary, multi_threads)
        diaplay_all_summary = False
        multi_threads = False
        self.summary.display(self.result, diaplay_all_summary, multi_threads)

    def test_report(self):
        batchsize = 1
        work_list = [[1, 2], [1, 3], [3, 4], [5, 6], [6, 7]]
        current_directory = os.getcwd()
        output_prefix = os.path.join(current_directory, "testdata/")
        self.summary.npu_compute_time_list = work_list
        self.summary.npu_compute_time_interval_list = work_list
        display_all_summary = False
        multi_threads = False
        with pytest.raises(Exception):
            self.summary.report(
                batchsize, output_prefix, display_all_summary, multi_threads
            )
        self.summary.npu_compute_time_list = []
        self.summary.npu_compute_time_interval_list = work_list
        self.summary.report(
            batchsize, output_prefix, display_all_summary, multi_threads
        )


if __name__ == "__main__":
    pytest.main([__file__, "-vs"])