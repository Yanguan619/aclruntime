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

# lzy
import sys
import logging
import pytest
from ais_bench.infer.args_check import check_batchsize_valid

logging.basicConfig(
    stream=sys.stdout, level=logging.INFO, format="[%(levelname)s] %(message)s"
)
logger = logging.getLogger(__name__)


class TestClass:
    @classmethod
    def test_check_batchsize_valid(cls):
        value = None
        check_batchsize_valid(value)
        value = -1
        with pytest.raises(Exception):
            check_batchsize_valid(value)


if __name__ == "__main__":
    pytest.main([__file__, "-vs"])