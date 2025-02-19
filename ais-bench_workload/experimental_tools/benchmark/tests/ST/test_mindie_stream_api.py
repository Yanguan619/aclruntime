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

import os
import json
import shutil
import sys
import logging
import pytest
import requests
from unittest.mock import patch
from ais_bench.benchmark.cli.main import main

GSK8K_DATA_COUNT = 1

class Response:
    def __init__(self):
        self.response = []
        for s in ["A","is","ben", "ch", "20"]:
            self.response.append(f"data: {{\"prefill_time\":100,\"decode_time\":100,\"token\":{{\"id\":0,\"text\":\"{s}\"}}}}")
    def iter_content(self,*args):
        for content in self.response:
            yield content.encode()

# def mock_path():
#     return "tests/datasets/gsm8k"
class TestClass:
    @classmethod
    def setup_class(cls):
        """
        class level setup_class
        """
        cls.init(TestClass)

    @classmethod
    def teardown_class(cls):

        print('\n ---class level teardown_class')

    def init(self):
        self.cur_dir = os.path.dirname(os.path.abspath(__file__))
        self.test_data_path = os.path.abspath(os.path.join(self.cur_dir, "../testdatas"))
        if os.path.exists(self.test_data_path):
            shutil.rmtree(self.test_data_path)
        os.makedirs(self.test_data_path)

    #mode infer
    def test_mindie_stream_api_infer(self, monkeypatch):
        fake_prediction = "Aisbench20"
        fake_time_str = "fake_time_mindie_stream_api"
        monkeypatch.setattr('sys.argv',
            ["ais_bench", "--models", "mindie_stream_api", "--datasets", "gsm8k_gen",
            "--mode", "infer", "-w", self.test_data_path])
        monkeypatch.setattr(requests, 'post', lambda *args, **kwargs: Response())
        monkeypatch.setattr("ais_bench.benchmark.utils.get_data_path", lambda *arg, **kwargs:"tests/datasets/gsm8k")
        monkeypatch.setattr("ais_bench.benchmark.cli.main.get_current_time_str", lambda *arg, **kwargs: fake_time_str)
        main()

        # check infer out
        infer_outputs_json_path = os.path.join(self.test_data_path, f"{fake_time_str}/predictions/mindie-stream-api/gsm8k.json")
        with open(infer_outputs_json_path, 'r', encoding='utf-8') as file:
            data = json.load(file)
        assert os.path.exists(infer_outputs_json_path)
        assert len(data) == GSK8K_DATA_COUNT
        assert data.get(f"{GSK8K_DATA_COUNT - 1}").get("prediction") == fake_prediction
    