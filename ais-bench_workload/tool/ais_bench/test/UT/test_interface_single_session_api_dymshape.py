import os
import sys
import logging
import pytest
import numpy as np
from ais_bench.infer.interface import InferSession
from test_common import TestCommonClass


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
    def get_resnet50_static(cls, bs: int = 1):
        return os.path.join(TestCommonClass.get_basepath(), cls.resnet_name, "model", f"pth_resnet50_bs{bs}.om")

    @classmethod
    def get_resnet50_dynamic(cls, kind: str):
        return os.path.join(TestCommonClass.get_basepath(), cls.resnet_name, "model", f"pth_resnet50_{kind}.om")

    @classmethod
    def get_add_model_static(cls, bs: int = 1):
        return os.path.join(TestCommonClass.get_basepath(), cls.add_name, "model", f"add_model_bs{bs}.om")

    @classmethod
    def get_add_model_dynamic(cls, kind: str):
        return os.path.join(TestCommonClass.get_basepath(), cls.add_name, "model", f"add_model_{kind}.om")

    def init(self):
        self.resnet_name = "resnet50"
        self.add_name = "add_model"
        self.iteration_times = 1000
        self.in_out_list = [-1, 0]

    # ====================test single process infer api==========================
    def test_infer_api_dymshape(self):
        device_id = 0
        model_path = self.get_resnet50_dynamic('dymshape')
        session = InferSession(device_id, model_path)

        # create new numpy data according inputs info
        shape = [1, 3, 224, 224]
        ndata = np.full(shape, 0).astype(np.float32)

        # in is numpy list and output is numpy list
        outputs = session.infer([ndata], mode='dymshape', custom_sizes=100000)
        session.free_resource()

    # ====================test single process infer pipeline api==========================
    def test_infer_pipeline_api_dymshape(self):
        device_id = 0
        model_path = self.get_resnet50_dynamic('dymshape')
        session = InferSession(device_id, model_path)

        # create new numpy data according inputs info
        shape1 = [1, 3, 224, 224]
        shape2 = [4, 3, 225, 225]
        ndata1 = np.full(shape1, 0).astype(np.float32)
        ndata2 = np.full(shape2, 0).astype(np.float32)
        ndata_list = [[ndata1], [ndata2]]

        # in is numpy list and output is numpy list
        outputs = session.infer_pipeline(ndata_list, mode='dymshape', custom_sizes=100000)
        session.free_resource()

    # ====================test single process infer iteration api==========================
    def test_infer_iteration_api_dymshape(self):
        device_id = 0
        model_path = self.get_add_model_dynamic('dymshape')
        session = InferSession(device_id, model_path)

        # create new numpy data according inputs info
        shape = [4, 3, 32, 32]
        ndata = np.full(shape, 0).astype(np.float32)
        out_size = 4 * 3 * 32 * 32 * 4
        # in is numpy list and output is numpy list
        outputs = session.infer_iteration(
            [ndata, ndata],
            in_out_list=self.in_out_list,
            iteration_times=self.iteration_times,
            mode='dymshape',
            custom_sizes=out_size,
        )
        session.free_resource()


if __name__ == '__main__':
    pytest.main([__file__, '-vs'])
