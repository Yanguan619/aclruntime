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
from test_common import TestCommonClass

logging.basicConfig(
    stream=sys.stdout, level=logging.INFO, format="[%(levelname)s] %(message)s"
)
logger = logging.getLogger(__name__)

# test get_inputs, get_outputs
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
    def get_resnet50_model_path(cls, kind: str):
        return os.path.join(TestCommonClass.base_path, "resnet50", "model", f"pth_resnet50_{kind}.om")
    
    @classmethod
    def get_add_model_path(cls, kind: str):
        return os.path.join(TestCommonClass.base_path, "add_model", "model", f"add_model_{kind}.om")

    def init(self):
        self.TensorDataType = [-1,0,1,2,3,4,6,7,8,9,10,11]
        self.device_id = TestCommonClass.default_device_id

    def test_resnet50_get_inputs(self):
        options = aclruntime.session_options()
        session = aclruntime.InferenceSession(self.get_resnet50_model_path("bs1"), self.device_id, options)
        inTensorDescList = session.get_inputs()
        
        assert type(inTensorDescList) == list
        assert len(inTensorDescList) == 1
        for i, inTensorDesc in enumerate(inTensorDescList):
            assert inTensorDescList[i].name == f"actual_input_{i+1}"
            assert inTensorDesc.datatype in self.TensorDataType
            assert inTensorDesc.format == 1
            assert inTensorDesc.shape == [1, 256, 256, 3]
            assert inTensorDesc.size == 196608
            assert inTensorDesc.realsize == 196608

    def test_resnet50_get_outputs(self):
        options = aclruntime.session_options()
        session = aclruntime.InferenceSession(self.get_resnet50_model_path("bs1"), self.device_id, options)
        outTensorDescList = session.get_outputs()

        assert type(outTensorDescList) == list
        assert len(outTensorDescList) == 1
        for i, outTensorDesc in enumerate(outTensorDescList):
            assert outTensorDescList[i].name[-7:] == f"output{i+1}"
            assert outTensorDesc.datatype in self.TensorDataType
            assert outTensorDesc.format == 2
            assert outTensorDesc.shape == [1, 1000]
            assert outTensorDesc.size == 4000
            assert outTensorDesc.realsize == 4000

    def test_add_model_get_inputs(self):
        options = aclruntime.session_options()
        session = aclruntime.InferenceSession(self.get_add_model_path("bs1"), self.device_id, options)
        inTensorDescList = session.get_inputs()
        
        assert type(inTensorDescList) == list
        assert len(inTensorDescList) == 2
        for i, inTensorDesc in enumerate(inTensorDescList):
            assert inTensorDescList[i].name == f"input{i+1}"
            assert inTensorDesc.datatype in self.TensorDataType
            assert inTensorDesc.format == 0
            assert inTensorDesc.shape == [1, 3, 32, 32]
            assert inTensorDesc.size == 12288
            assert inTensorDesc.realsize == 12288

    def test_add_model_get_outputs(self):
        options = aclruntime.session_options()
        session = aclruntime.InferenceSession(self.get_add_model_path("bs1"), self.device_id, options)
        outTensorDescList = session.get_outputs()

        assert type(outTensorDescList) == list
        assert len(outTensorDescList) == 1
        for i, outTensorDesc in enumerate(outTensorDescList):
            assert outTensorDescList[i].name[-6:] == f"output"
            assert outTensorDesc.datatype in self.TensorDataType
            assert outTensorDesc.format == 0
            assert outTensorDesc.shape == [1, 3, 32, 32]
            assert outTensorDesc.size == 12288
            assert outTensorDesc.realsize == 12288

if __name__ == '__main__':
    pytest.main([__file__, '-vs'])