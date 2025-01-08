# Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import ast


def file_context_verify(file_path):
    with open(file_path, 'r') as file:
        content = file.read()

    try:
        ast.parse(content)
    except SyntaxError as e:
        raise ValueError(f"The file contains syntax errors:{e}")
    return content


cur_path = os.path.dirname(os.path.realpath(__file__))
file_path = os.path.join(cur_path, "net_test/__main__.py")
verified_content = file_context_verify(file_path)
exec(verified_content)