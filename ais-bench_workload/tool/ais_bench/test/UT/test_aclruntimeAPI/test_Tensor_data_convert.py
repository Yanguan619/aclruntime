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
from aclruntime import Tensor
from test_common import TestCommonClass

logging.basicConfig(
    stream=sys.stdout, level=logging.INFO, format="[%(levelname)s] %(message)s"
)
logger = logging.getLogger(__name__)

# test Tensor, to_device, to_host
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

    def test_initialize_Tensor(self):
        shape = [1, 3, 32, 32]
        ndata = np.full(shape, 1).astype(np.float32)
        tensor = Tensor(ndata)

        assert tensor.device == -1 # in host
        assert tensor.shape == [1, 3, 32, 32]

    def test_to_device(self):
        shape = [1, 3, 32, 32]
        ndata = np.full(shape, 1).astype(np.float32)
        tensor = Tensor(ndata)
        tensor.to_device(self.device_id)

        assert tensor.device == TestCommonClass.default_device_id # in device 
        assert tensor.shape == [1, 3, 32, 32]


    def test_to_host(self):
        shape = [1, 3, 32, 32]
        ndata = np.full(shape, 1).astype(np.float32)
        tensor = Tensor(ndata)

        tensor.to_device(self.device_id)
        assert tensor.device == TestCommonClass.default_device_id # in device 
        assert tensor.shape == [1, 3, 32, 32]

        tensor.to_host()
        assert tensor.device == -1 # in host
        assert tensor.shape == [1, 3, 32, 32]

if __name__ == '__main__':
    pytest.main([__file__, '-vs'])