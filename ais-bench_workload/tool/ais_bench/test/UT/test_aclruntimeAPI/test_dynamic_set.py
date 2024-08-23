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
from aclruntime import set_dynamic_batchsize, set_dynamic_hw, set_dynamic_dims, set_dynamic_shape
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
    def get_model_path(cls, kind : str):
        return os.path.join(cls.model_path, cls.model_name, "model", f"pth_resnet50_{kind}.om")

    def init(self):
        self.model_name = "resnet50"
        self.model_path = "../../testdata/"
        self.device_id = 0
        self.input_tensor_name = "actual_input_1"
        
    def test_set_dynamic_batchsize(self):
        options = aclruntime.session_options()
        model_path = self.get_model_path("dymbatch")
        session = aclruntime.InferenceSession(model_path, self.device_id, options)

        session.set_dynamic_batchsize(1)
        basesize = session.get_inputs()[0].realsize

        session.set_dynamic_batchsize(2)
        basesize1 = session.get_inputs()[0].realsize
        assert basesize1 == basesize * 2

        session.set_dynamic_batchsize(4)
        basesize2 = session.get_inputs()[0].realsize
        assert basesize2 == basesize1 * 2

        session.set_dynamic_batchsize(8)
        basesize3 = session.get_inputs()[0].realsize
        assert basesize3 == basesize2 * 2

    def test_set_dynamic_hw(self):
        options = aclruntime.session_options()
        model_path = self.get_model_path("dymwh")
        session = aclruntime.InferenceSession(model_path, self.device_id, options)

        session.set_dynamic_hw(112, 112)
        basesize = session.get_inputs()[0].realsize

        session.set_dynamic_hw(224, 224)
        basesize1 = session.get_inputs()[0].realsize
        assert basesize1 == basesize * 2

        session.set_dynamic_hw(448, 448)
        basesize2 = session.get_inputs()[0].realsize
        assert basesize2 == basesize1 * 2


    def test_set_dynamic_dims(self):
        options = aclruntime.session_options()
        model_path = self.get_model_path("dymdim")
        session = aclruntime.InferenceSession(model_path, self.device_id, options)

        session.set_dynamic_dims(self.input_tensor_name + ":1,3,112,112")
        basesize = session.get_inputs()[0].realsize

        session.set_dynamic_dims(self.input_tensor_name + ":1,3,224,224")
        basesize1 = session.get_inputs()[0].realsize
        assert basesize1 == basesize * 4

        session.set_dynamic_dims(self.input_tensor_name + ":2,3,224,224")
        basesize2 = session.get_inputs()[0].realsize
        assert basesize2 == basesize1 * 2

        session.set_dynamic_dims(self.input_tensor_name + ":8,3,224,224")
        basesize3 = session.get_inputs()[0].realsize
        assert basesize3 == basesize2 * 4


    def test_set_dynamic_shape(self):
        options = aclruntime.session_options()
        model_path = self.get_model_path("dymshape")
        session = aclruntime.InferenceSession(model_path, self.device_id, options)

        session.set_dynamic_shape(self.input_tensor_name + ":1,3,112,112")
        basesize = session.get_inputs()[0].realsize

        session.set_dynamic_shape(self.input_tensor_name + ":1,3,224,224")
        basesize1 = session.get_inputs()[0].realsize
        assert basesize1 == basesize * 4

        session.set_dynamic_shape(self.input_tensor_name + ":2,3,224,224")
        basesize2 = session.get_inputs()[0].realsize
        assert basesize2 == basesize1 * 2

        session.set_dynamic_shape(self.input_tensor_name + ":8,3,224,224")
        basesize3 = session.get_inputs()[0].realsize
        assert basesize3 == basesize2 * 4
        

if __name__ == '__main__':
    pytest.main([__file__, '-vs'])