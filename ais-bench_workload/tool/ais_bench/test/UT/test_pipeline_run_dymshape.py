# Copyright (c) 2023-2024 Huawei Technologies Co., Ltd.
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
import logging

import aclruntime
import numpy as np
from test_common import TestCommonClass

from ais_bench.infer.common.io_operations import (
    create_pipeline_fileslist_from_inputs_list,
)


logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='[%(levelname)s] %(message)s')
logger = logging.getLogger(__name__)


class TestClass:
    @staticmethod
    def get_input_tensor_name():
        return "actual_input_1"

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
    def get_input_datas_file_npy_nor(cls):
        return os.path.realpath(
            os.path.join(TestCommonClass.get_basepath(), cls.model_name, "input", "fake_dataset_npy_nor/1.npy")
        )

    @classmethod
    def get_resnet_dymshape_om_path(cls):
        return os.path.join(TestCommonClass.get_basepath(), cls.model_name, "model", "pth_resnet50_dymshape.om")

    def init(self):
        self.model_name = "resnet50"

    def test_infer_dym_shape_input_file(self):
        device_id = 0
        input_tensor_name = self.get_input_tensor_name()
        options = aclruntime.session_options()
        model_path = self.get_resnet_dymshape_om_path()
        session = aclruntime.InferenceSession(model_path, device_id, options)
        session.set_dynamic_shape(input_tensor_name + ":1,3,224,224")
        session.set_custom_outsize([10000])
        intensors_desc = session.get_inputs()
        infilespath = create_pipeline_fileslist_from_inputs_list(
            self.get_input_datas_file_npy_nor().split(','), intensors_desc
        )
        infer_options = aclruntime.infer_options()
        extra_session = []
        session.run_pipeline(infilespath, infer_options, extra_session)

    def test_infer_auto_shape_input_file(self):
        device_id = 0
        options = aclruntime.session_options()
        model_path = self.get_resnet_dymshape_om_path()
        session = aclruntime.InferenceSession(model_path, device_id, options)
        session.set_custom_outsize([10000])
        intensors_desc = session.get_inputs()
        infilespath = create_pipeline_fileslist_from_inputs_list(
            self.get_input_datas_file_npy_nor().split(','), intensors_desc
        )
        infer_options = aclruntime.infer_options()
        infer_options.auto_dym_shape = True
        extra_session = []
        session.run_pipeline(infilespath, infer_options, extra_session)
