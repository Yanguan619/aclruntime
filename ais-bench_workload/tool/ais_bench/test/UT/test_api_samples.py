# Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
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
import importlib

from test_common import TestCommonClass

class TestClass:
    @classmethod
    def setup_class(cls):
        """
        class level setup_class
        """
        cls.init(TestClass)

    @classmethod
    def get_aclruntime_sample_func(cls):
        folder_name = "aclruntime_api_usage"
        cls.aclruntime_dir = os.path.join(cls.sample_dir, folder_name)
        cls.aclruntime_funcs = {}
        for filename in os.listdir(cls.aclruntime_dir):
            if filename.endswith('.py') and not filename.startswith('__'):
                module_name = filename[:-3]
                module = importlib.import_module(f"{folder_name}.{module_name}")
                cls.aclruntime_funcs[module_name] = getattr(module, module_name, None)

    @classmethod
    def get_interface_sample_func(cls):
        folder_name = "interface_api_usage"
        cls.interface_dir = os.path.join(cls.sample_dir, folder_name)
        cls.interface_funcs = {}
        for sub_dir_name in os.listdir(cls.interface_dir):
            if sub_dir_name.startswith('__'):
                continue
            for filename in os.listdir(os.path.join(cls.interface_dir, sub_dir_name)):
                if filename.endswith('.py') and not filename.startswith('__'):
                    module_name = filename[:-3]
                    module = importlib.import_module(f"{folder_name}.{sub_dir_name}.{module_name}")
                    cls.interface_funcs[module_name] = getattr(module, module_name, None)

    def init(self):
        self.sample_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "../../../api_samples")
        sys.path.append(self.sample_dir)
        self.get_aclruntime_sample_func()
        self.get_interface_sample_func()

    def test_interface_api_samples(self):
        for _, func in self.interface_funcs.items():
            func()

    def test_aclruntime_api_samples(self):
        for _, func in self.aclruntime_funcs.items():
            func()

