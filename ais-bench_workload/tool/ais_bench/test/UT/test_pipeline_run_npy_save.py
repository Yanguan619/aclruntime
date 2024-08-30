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
import logging
import glob

import aclruntime
import numpy as np
import pytest
from test_common import TestCommonClass

from ais_bench.infer.common.io_operations import (
    create_pipeline_fileslist_from_inputs_list,
    PURE_INFER_FAKE_FILE_ZERO,
    PURE_INFER_FAKE_FILE_RANDOM,
)


logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='[%(levelname)s] %(message)s')
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
    def get_resnet50_output_dir_npy(cls):
        return os.path.realpath(os.path.join(TestCommonClass.get_basepath(), "resnet50", "output", "npy_out"))
    
    @classmethod
    def get_add_model_output_dir_npy(cls):
        return os.path.realpath(os.path.join(TestCommonClass.get_basepath(), "add_model", "output", "npy_out"))

    @classmethod
    def get_resnet50_stcshape_om_path(cls):
        return os.path.join(TestCommonClass.get_basepath(), "resnet50", "model", "pth_resnet50_bs1.om")

    @classmethod
    def get_add_model_stcshape_om_path(cls):
        return os.path.join(TestCommonClass.get_basepath(), "add_model", "model", "add_model_bs1.om")

    def init(self):
        pass

    def test_resnet50_pure_infer_stc_shape_random(self):
        device_id = TestCommonClass.default_device_id
        options = aclruntime.session_options()
        model_path = self.get_resnet50_stcshape_om_path()
        session = aclruntime.InferenceSession(model_path, device_id, options)
        intensors_desc = session.get_inputs()
        infileslist = [[]]
        pure_file = PURE_INFER_FAKE_FILE_RANDOM
        for _ in intensors_desc:
            infileslist[0].append(pure_file)
        output_dir = self.get_resnet50_output_dir_npy()
        if not os.path.exists(output_dir):
            os.makedirs(output_dir, 0o755)
        infer_options = aclruntime.infer_options()
        infer_options.output_dir = output_dir
        infer_options.out_format = 'NPY'
        infer_options.pure_infer_mode = True
        extra_session = []
        session.run_pipeline(infileslist, infer_options, extra_session)
        npy_files = glob.glob(os.path.join(output_dir, "*.npy"))
        assert len(npy_files) == 1
        
    def test_add_model_pure_infer_stc_shape_random(self):
        device_id = TestCommonClass.default_device_id
        options = aclruntime.session_options()
        model_path = self.get_add_model_stcshape_om_path()
        session = aclruntime.InferenceSession(model_path, device_id, options)
        # shape = session.get_inputs()[0].shape
        # ndata = np.full(shape, 0).astype(np.float32)
        intensors_desc = session.get_inputs()
        infileslist = [[]]
        pure_file = PURE_INFER_FAKE_FILE_RANDOM
        for _ in intensors_desc:
            infileslist[0].append(pure_file)
        output_dir = self.get_add_model_output_dir_npy()
        if not os.path.exists(output_dir):
            os.makedirs(output_dir, 0o755)
        infer_options = aclruntime.infer_options()
        infer_options.output_dir = output_dir
        infer_options.out_format = 'NPY'
        infer_options.pure_infer_mode = True
        extra_session = []
        session.run_pipeline(infileslist, infer_options, extra_session)
        npy_files = glob.glob(os.path.join(output_dir, "*.npy"))
        assert len(npy_files) == 1

if __name__ == '__main__':
    pytest.main([__file__, '-vs'])