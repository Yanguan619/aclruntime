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

    def test_parse_log(self):
        assert self.backend_trtexec.parse_log("Throughput: 100.1\n") == 100.1
        self.backend_trtexec.parse_log("H2D Latency= 1 H2D Latency= 1 H2D Latency= 1 H2D Latency= 1 H2D Latency= 1\n")
        self.backend_trtexec.parse_log("GPU Compute Time= 1 GPU Compute Time= 1 GPU Compute Time= 1 GPU Compute Time= 1 GPU Compute Time= 1")
        self.backend_trtexec.parse_log("D2H Latency= 1 D2H Latency= 1 D2H Latency= 1 D2H Latency= 1 D2H Latency= 1\n")
        self.backend_trtexec.parse_log("Total Host Walltime: 100\n")

    def test_warm_up(self):
        self.backend_trtexec.warm_up([], 100)

    def test_predict(self):
        self.backend_trtexec.predict([])

    def test_build(self):
        self.backend_trtexec.build()

    def test_get_perf(self):
        self.backend_trtexec.get_perf()

    def test_run(self):
        self.backend_trtexec.run()


if __name__ == "__main__":
    pytest.main([__file__, "-vs"])