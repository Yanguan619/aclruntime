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

# lzy
import sys
import logging
import os
import pytest
from ais_bench.infer.backends.backend_trtexec import TrtexecConfig, BackendTRTExec
from test_common import TestCommonClass

logging.basicConfig(
    stream=sys.stdout, level=logging.INFO, format="[%(levelname)s] %(message)s"
)
logger = logging.getLogger(__name__)


class Config:
    loop = 1
    warmup_count = 1
    batchsize = 1
    device = 1


class TestClass:
    trtextcconfig = TrtexecConfig()
    trtextcconfig.iteration = 1
    trtextcconfig.warmup = 1
    trtextcconfig.duration = 1
    trtextcconfig.batch = 1
    trtextcconfig.device = 1
    config = Config()
    backend_trtexec = BackendTRTExec(config)
    backend_trtexec.config = trtextcconfig
    backend_trtexec.convert_config(config)

    @staticmethod
    def rmforce(path):
        os.system(f"rm -rf {path}")

    def test_name(self):
        assert self.backend_trtexec.name == "trtexec"

    def test_model_extension(self):
        assert self.backend_trtexec.model_extension == "plan"

    def test_BackendTRTExec_load(self):
        model = os.path.join(
            TestCommonClass.base_path, "resnet50/model/pth_resnet50_bs1.om"
        )
        self.backend_trtexec.load(model)
        assert self.backend_trtexec.model == model

        not_exist_model = os.path.join(
            TestCommonClass.base_path, "resnet50/model/not_exist_model.om"
        )
        if os.path.exists(not_exist_model):
            self.rmforce(not_exist_model)
        with pytest.raises(Exception):
            self.backend_trtexec.load(not_exist_model)

    def test_parse_log_throughput(self):
        perf = self.backend_trtexec.parse_log("Throughput: 120.699 qps\n")
        assert abs(perf.throughput - 120.699) <= 1e-3

    def test_parse_log_h2d_latency(self):
        perf = self.backend_trtexec.parse_log(
            "H2D Latency: min = 1.55066 ms, max = 1.57336 ms, mean = 1.55492 ms, " +
            "median = 1.55444 ms, percentile(90%) = 1.55664 ms, " +
            "percentile(95%) = 1.55835 ms, percentile(99%) = 1.56458 ms\n"
        )
        assert abs(perf.h2d_latency.min - 1.55066) <= 1e-5
        assert abs(perf.h2d_latency.max - 1.57336) <= 1e-5
        assert abs(perf.h2d_latency.mean - 1.55492) <= 1e-5
        assert abs(perf.h2d_latency.median - 1.55444) <= 1e-5
        assert abs(perf.h2d_latency.percentile - 1.55664) <= 1e-5

    def test_parse_log_compute_time(self):
        perf = self.backend_trtexec.parse_log(
            "GPU Compute Time: min = 7.54407 ms, max = 10.1723 ms, " +
            "mean = 8.23978 ms, median = 8.19409 ms, percentile(90%) = 8.5354 ms, " +
            "percentile(95%) = 8.59131 ms, percentile(99%) = 9.90002 ms"
        )
        assert abs(perf.compute_time.min - 7.54407) <= 1e-5
        assert abs(perf.compute_time.max - 10.1723) <= 1e-5
        assert abs(perf.compute_time.mean - 8.23978) <= 1e-5
        assert abs(perf.compute_time.median - 8.19409) <= 1e-5
        assert abs(perf.compute_time.percentile - 8.5354) <= 1e-5

    def test_parse_log_d2h_latency(self):
        perf = self.backend_trtexec.parse_log(
            "D2H Latency: min = 0.0130615 ms, max = 0.0170898 ms, mean = 0.015342 ms, " +
            "median = 0.0153809 ms, percentile(90%) = 0.0162354 ms, " +
            "percentile(95%) = 0.0163574 ms, percentile(99%) = 0.0168457 ms\n"
        )
        assert abs(perf.d2h_latency.min - 0.0130615) <= 1e-5
        assert abs(perf.d2h_latency.max - 0.0170898) <= 1e-5
        assert abs(perf.d2h_latency.mean - 0.015342) <= 1e-5
        assert abs(perf.d2h_latency.median - 0.0153809) <= 1e-5
        assert abs(perf.d2h_latency.percentile - 0.0162354) <= 1e-5

    def test_parse_log_d2h_latency(self):
        perf = self.backend_trtexec.parse_log("Total Host Walltime: 3.02405 s\n")
        assert abs(perf.host_wall_time - 3.02405) <= 1e-5

    def test_warm_up(self):
        self.backend_trtexec.warm_up([], 100)

    def test_predict(self):
        self.backend_trtexec.predict([])

    def test_build(self):
        self.backend_trtexec.build()

    def test_get_perf(self):
        self.backend_trtexec.get_perf()

    def test_run(self):
        assert self.backend_trtexec.run() == []


if __name__ == "__main__":
    pytest.main([__file__, "-vs"])