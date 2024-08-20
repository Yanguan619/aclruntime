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
from aclruntime import session_options
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

    def init(self):
        pass

    def test_default_session_options():
        options = session_options()
        assert options.log_level == 2
        assert options.loop == 1
        assert options.acl_json_path == ""

    def test_custom_session_options():
        options = session_options()
        options.log_level = 1
        options.loop = 10
        options.acl_json_path = "/path/to/acl.json"

        assert options.log_level == 1
        assert options.loop == 10
        assert options.acl_json_path == "/path/to/acl.json"

if __name__ == '__main__':
    pytest.main([__file__, '-vs'])