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
import sys
import pytest
import logging
import aclruntime
import numpy as np
from aclruntime import get_inputs, get_outputs
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

    @classmethod
    def get_model_path(cls, kind: str):
        return os.path.join(TestCommonClass.base_path, cls.model_name, "model", f"{cls.model_name}_{kind}.om")

    def init(self):
        self.model_name = "add_model"
        self.model_kind = "bs1"
        self.TensorDataType = [-1,0,1,2,3,4,6,7,8,9,10,11]
        self.device_id = TestCommonClass.default_device_id
        self.options = aclruntime.session_options()
        self.session = aclruntime.InferenceSession(self.get_model_path(self.model_kind), self.device_id, self.options)

    # 问题：具体format、shape、size、realsize初始化是多少
    def test_get_inputs(self):
        inTensorDescList = self.session.get_inputs()
        
        # TensorDesc content
        # std::string name;
        # TensorDataType datatype;
        # size_t format;
        # std::vector<int64_t> shape;
        # size_t size;
        # size_t realsize;    // 针对动态shape 动态分档场景 实际需要的大小

        assert type(inTensorDescList) == list
        assert len(inTensorDescList) == 2
        for i, inTensorDesc in enumerate(inTensorDescList):
            assert inTensorDescList[i].name == f"inputs{i}"
            assert inTensorDesc.datatype in self.TensorDataType
            assert inTensorDesc.format == 0
            assert inTensorDesc.shape == [1, 3, 32, 32]
            assert inTensorDesc.size == 3072
            assert inTensorDesc.realsize == 3072

    def test_get_outputs(self):
        outTensorDescList = self.session.get_outputs()

        assert type(outTensorDescList) == list
        assert len(outTensorDescList) == 1
        for i, outTensorDesc in enumerate(outTensorDescList):
            assert outTensorDescList[i].name == f"outputs{i}"
            assert outTensorDesc.datatype in self.TensorDataType
            assert outTensorDesc.format == 0
            assert outTensorDesc.shape == [1, 3, 32, 32]
            assert outTensorDesc.size == 3072
            assert outTensorDesc.realsize == 3072

if __name__ == '__main__':
    pytest.main([__file__, '-vs'])