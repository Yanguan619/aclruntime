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
from aclruntime import InferenceSession
from test_common import TestCommonClass

logging.basicConfig(
    stream=sys.stdout, level=logging.INFO, format="[%(levelname)s] %(message)s"
)
logger = logging.getLogger(__name__)

# test run, InferenceSession, sumary
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

    def test_static_run(self):
        options = aclruntime.session_options()
        model_path = self.get_model_path("bs1")
        session = InferenceSession(model_path, self.device_id, options)

        shape = session.get_inputs()[0].shape
        ndata = np.full(shape, 0).astype(np.float32)
        tensor = aclruntime.Tensor(ndata)
        tensor.to_device(self.device_id)

        outnames = [session.get_outputs()[0].name]
        feeds = {session.get_inputs()[0].name: tensor}

        outputs = session.run(outnames, feeds)
        logger.info("outputs:", outputs)

        for out in outputs:
            out.to_host()
        logger.info(session.sumary())

    def test_dymbatch_run(self):
        options = aclruntime.session_options()
        model_path = self.get_model_path("dymbatch")
        session = InferenceSession(model_path, self.device_id, options)

        session.set_dynamic_batchsize(1)
        barray = bytearray(session.get_inputs()[0].realsize)
        ndata = np.frombuffer(barray)
        # convert numpy to pytensors in device
        tensor = aclruntime.Tensor(ndata)
        tensor.to_device(self.device_id)

        outnames = [session.get_outputs()[0].name]
        feeds = {session.get_inputs()[0].name: tensor}

        outputs = session.run(outnames, feeds)
        logger.info("outputs:", outputs)

        for out in outputs:
            out.to_host()
        logger.info(session.sumary())

    def test_dymhw_run(self):
        options = aclruntime.session_options()
        model_path = self.get_model_path("dymwh")
        session = InferenceSession(model_path, self.device_id, options)

        session.set_dynamic_hw(112, 112)
        barray = bytearray(session.get_inputs()[0].realsize)
        ndata = np.frombuffer(barray)
        tensor = aclruntime.Tensor(ndata)
        tensor.to_device(self.device_id)

        outnames = [session.get_outputs()[0].name]
        feeds = {session.get_inputs()[0].name: tensor}

        outputs = session.run(outnames, feeds)
        logger.info("outputs:", outputs)

        for out in outputs:
            out.to_host()
        logger.info(session.sumary())

    def test_dymdim_run(self):
        options = aclruntime.session_options()
        model_path = self.get_model_path("dymdim")
        session = InferenceSession(model_path, self.device_id, options)

        session.set_dynamic_dims(self.input_tensor_name + ":1,3,224,224")
        barray = bytearray(session.get_inputs()[0].realsize)
        ndata = np.frombuffer(barray)
        tensor = aclruntime.Tensor(ndata)
        tensor.to_device(self.device_id)

        outnames = [session.get_outputs()[0].name]
        feeds = {session.get_inputs()[0].name: tensor}

        outputs = session.run(outnames, feeds)
        logger.info("outputs:", outputs)

        for out in outputs:
            out.to_host()
        logger.info(session.sumary())

    def test_dymshape_run(self):
        options = aclruntime.session_options()
        model_path = self.get_model_path("dymshape")
        session = InferenceSession(model_path, self.device_id, options)

        session.set_dynamic_shape(self.input_tensor_name + ":1,3,224,224")
        barray = bytearray(session.get_inputs()[0].realsize)
        ndata = np.frombuffer(barray)
        tensor = aclruntime.Tensor(ndata)
        tensor.to_device(self.device_id)

        outnames = [session.get_outputs()[0].name]
        feeds = {session.get_inputs()[0].name: tensor}

        outputs = session.run(outnames, feeds)
        logger.info("outputs:", outputs)

        for out in outputs:
            out.to_host()
        logger.info(session.sumary())
        

if __name__ == '__main__':
    pytest.main([__file__, '-vs'])