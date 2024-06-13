# Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
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

import logging
import os
import aclruntime
import numpy as np
import pytest
from ais_bench.infer.dym_aipp_manager import DymAippManager
from test_common import TestCommonClass

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
    def get_input_tensor_name(cls):
        return "actual_input_1"

# 各种模型
    @classmethod
    def get_without_dymaipp_om_path(cls):
        return os.path.join(
            TestCommonClass.get_basepath(), cls.model_name, "model", "pth_resnet50_bs4.om"
        )

    @classmethod
    def get_dymaipp_staticshape_om_path(cls):
        return os.path.join(
            TestCommonClass.get_basepath(), cls.model_name, "model", "pth_resnet50_bs4_dymaipp_stcbatch.om"
        )

    @classmethod
    def get_dymaipp_dymbatch_om_path(cls):
        return os.path.join(
            TestCommonClass.get_basepath(), cls.model_name, "model", "pth_resnet50_dymaipp_dymbatch.om"
        )

    @classmethod
    def get_dymaipp_dymwh_om_path(cls):
        return os.path.join(
            TestCommonClass.get_basepath(), cls.model_name, "model", "pth_resnet50_dymaipp_dymwh.om"
        )

    @classmethod
    def get_multi_dymaipp_om_path(cls):
        return os.path.join(TestCommonClass.get_basepath(), cls.model_name, "model", "multi_dym_aipp_model.om")

    # 各种输入的aipp具体参数配置文件
    @classmethod
    def get_actual_aipp_config(cls):
        return os.path.join(
            os.path.dirname(__file__), "../", "aipp_config_files", "actual_aipp_cfg.config"
        )

    @classmethod
    def get_aipp_config_param_overflowed(cls):
        return os.path.join(
            os.path.dirname(__file__), "../", "aipp_config_files", "actual_aipp_cfg_param_overflowed.config"
        )

    @classmethod
    def get_aipp_config_lack_param(cls):
        return os.path.join(
            os.path.dirname(__file__), "../", "aipp_config_files", "actual_aipp_cfg_lack_param.config"
        )

    @classmethod
    def get_aipp_config_multi_input(cls):
        return os.path.join(
            os.path.dirname(__file__), "../", "aipp_config_files", "actual_aipp_cfg_multi_input.config"
        )

    @classmethod
    def get_aipp_config_lack_title(cls):
        return os.path.join(
            os.path.dirname(__file__), "../", "aipp_config_files", "actual_aipp_cfg_lack_title.config"
        )

    @classmethod
    def get_aipp_config_all_params(cls):
        return os.path.join(
            os.path.dirname(__file__), "../", "aipp_config_files", "actual_aipp_cfg_all_params.config"
        )

    def init(self):
        self.model_name = "resnet50"

    # 各种测试场景
    def test_infer_dymaipp_staticshape(self):
        device_id = 0
        options = aclruntime.session_options()
        model_path = self.get_dymaipp_staticshape_om_path()
        session = aclruntime.InferenceSession(model_path, device_id, options)
        session.set_staticbatch()
        # only need call this functon compare infer_simple
        aipp_manager = DymAippManager(session, self.get_actual_aipp_config(), 4)
        aipp_manager.load_aipp_config_content()
        session.set_dym_aipp_info_set()
        session.check_dym_aipp_input_exist()

        # create new numpy data according inputs info
        barray = bytearray(session.get_inputs()[0].realsize)
        ndata = np.frombuffer(barray)
        # convert numpy to pytensors in device
        tensor = aclruntime.Tensor(ndata)
        tensor.to_device(device_id)

        outnames = [session.get_outputs()[0].name]
        feeds = {session.get_inputs()[0].name: tensor}

        outputs = session.run(outnames, feeds)
        logger.info("outputs:{}".format(outputs))

        for out in outputs:
            out.to_host()
        logger.info(session.sumary())

    def test_infer_dymaipp_dymbatch(self):
        device_id = 0
        options = aclruntime.session_options()
        model_path = self.get_dymaipp_dymbatch_om_path()
        session = aclruntime.InferenceSession(model_path, device_id, options)
        session.set_dynamic_batchsize(2)
        # only need call this functon compare infer_simple
        aipp_manager = DymAippManager(session, self.get_actual_aipp_config(), 4)
        aipp_manager.load_aipp_config_content()
        session.set_dym_aipp_info_set()
        session.check_dym_aipp_input_exist()

        # create new numpy data according inputs info
        barray = bytearray(session.get_inputs()[0].realsize)
        ndata = np.frombuffer(barray)
        # convert numpy to pytensors in device
        tensor = aclruntime.Tensor(ndata)
        tensor.to_device(device_id)

        outnames = [session.get_outputs()[0].name]
        feeds = {session.get_inputs()[0].name: tensor}

        outputs = session.run(outnames, feeds)
        logger.info("outputs:{}".format(outputs))

        for out in outputs:
            out.to_host()
        logger.info(session.sumary())

    def test_infer_dymaipp_dymwh(self):
        device_id = 0
        options = aclruntime.session_options()
        model_path = self.get_dymaipp_dymwh_om_path()
        session = aclruntime.InferenceSession(model_path, device_id, options)
        session.set_dynamic_hw(112, 112)
        # only need call this functon compare infer_simple
        aipp_manager = DymAippManager(session, self.get_actual_aipp_config(), 1)
        aipp_manager.load_aipp_config_content()
        session.set_dym_aipp_info_set()
        session.check_dym_aipp_input_exist()

        # create new numpy data according inputs info
        barray = bytearray(session.get_inputs()[0].realsize)
        ndata = np.frombuffer(barray)
        # convert numpy to pytensors in device
        tensor = aclruntime.Tensor(ndata)
        tensor.to_device(device_id)

        outnames = [session.get_outputs()[0].name]
        feeds = {session.get_inputs()[0].name: tensor}

        outputs = session.run(outnames, feeds)
        logger.info("outputs:{}".format(outputs))

        for out in outputs:
            out.to_host()
        logger.info(session.sumary())

    # 模型没有动态aipp input
    def test_infer_no_dymaipp_input(self):
        device_id = 0
        options = aclruntime.session_options()
        model_path = self.get_without_dymaipp_om_path()
        session = aclruntime.InferenceSession(model_path, device_id, options)
        session.set_staticbatch()
        # only need call this functon compare infer_simple
        with pytest.raises(Exception) as e:
            session.check_dym_aipp_input_exist()

    # 模型有多个动态aipp input
    def test_infer_multi_dymaipp_input(self):
        device_id = 0
        options = aclruntime.session_options()
        model_path = self.get_multi_dymaipp_om_path()
        session = aclruntime.InferenceSession(model_path, device_id, options)
        # only need call this functon compare infer_simple
        aipp_manager = DymAippManager(session, self.get_actual_aipp_config(), 1)
        aipp_manager.load_aipp_config_content()
        session.set_dym_aipp_info_set()
        with pytest.raises(Exception) as e:
            session.check_dym_aipp_input_exist()
            logger.info("get --aipp model wrong")

        # create new numpy data according inputs info
        barray = bytearray(session.get_inputs()[0].realsize)
        ndata = np.frombuffer(barray)
        # convert numpy to pytensors in device
        tensor = aclruntime.Tensor(ndata)
        tensor.to_device(device_id)

        outnames = [session.get_outputs()[0].name]
        feeds = {session.get_inputs()[0].name: tensor}

        with pytest.raises(Exception) as e:
            outputs = session.run(outnames, feeds)
            logger.info("outputs:{}".format(outputs))

    # --aipp_config 包含所有属性参数
    def test_infer_aipp_cfg_all_params(self):
        device_id = 0
        options = aclruntime.session_options()
        model_path = self.get_dymaipp_staticshape_om_path()
        session = aclruntime.InferenceSession(model_path, device_id, options)
        session.set_staticbatch()
        # only need call this functon compare infer_simple
        session.check_dym_aipp_input_exist()
        aipp_manager = DymAippManager(session, self.get_aipp_config_all_params(), 4)
        aipp_manager.load_aipp_config_content()
        session.set_dym_aipp_info_set()

    # --aipp_config 缺少[aipp_op]标识
    def test_infer_aipp_cfg_lack_title(self):
        device_id = 0
        options = aclruntime.session_options()
        model_path = self.get_dymaipp_staticshape_om_path()
        session = aclruntime.InferenceSession(model_path, device_id, options)
        session.set_staticbatch()
        # only need call this functon compare infer_simple
        session.check_dym_aipp_input_exist()
        with pytest.raises(Exception) as e:
            aipp_manager = DymAippManager(session, self.get_aipp_config_lack_title(), 4)
            aipp_manager.load_aipp_config_content()
            session.set_dym_aipp_info_set()
            logger.info("get --aipp_config wrong")

    # --aipp_config 缺少 必备参数
    def test_infer_aipp_cfg_lack_param(self):
        device_id = 0
        options = aclruntime.session_options()
        model_path = self.get_dymaipp_staticshape_om_path()
        session = aclruntime.InferenceSession(model_path, device_id, options)
        session.set_staticbatch()
        # only need call this functon compare infer_simple
        session.check_dym_aipp_input_exist()
        with pytest.raises(Exception) as e:
            aipp_manager = DymAippManager(session, self.get_aipp_config_lack_title(), 4)
            aipp_manager.load_aipp_config_content()
            session.set_dym_aipp_info_set()
            logger.info("get --aipp_config wrong")

    # --aipp_config 参数超出范围限制
    def test_infer_aipp_cfg_param_overflowed(self):
        device_id = 0
        options = aclruntime.session_options()
        model_path = self.get_dymaipp_staticshape_om_path()
        session = aclruntime.InferenceSession(model_path, device_id, options)
        session.set_staticbatch()
        # only need call this functon compare infer_simple
        session.check_dym_aipp_input_exist()
        with pytest.raises(Exception) as e:
            aipp_manager = DymAippManager(session, self.get_aipp_config_param_overflowed(), 4)
            aipp_manager.load_aipp_config_content()
            session.set_dym_aipp_info_set()
            logger.info("get --aipp_config wrong")
