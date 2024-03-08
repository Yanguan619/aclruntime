# Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
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
import subprocess
import re
from setuptools import setup, find_packages  # type: ignore


with open('requirements.txt', encoding='utf-8') as f:
    required = f.read().splitlines()

with open('README.md', encoding='utf-8') as f:
    long_description = f.read()

# 使用Git命令获取最新的提交哈希
try:
    git_hash = subprocess.check_output(['git', 'rev-parse', 'HEAD']).decode('utf-8').strip()
except Exception:
    git_hash = ""
# 使用Git命令获取最新的提交日期和时间
try:
    git_date = subprocess.check_output(['git', 'show', '-s', '--format=%cd', 'HEAD']).decode('utf-8').strip()
except Exception:
    git_date = ""

setup(
    name='ais_bench',
    version='0.0.2',
    description='ais_bench tool',
    long_description=long_description,
    url=f"https://gitee.com/ascend/tools/, commit id: {git_hash}",
    release_date = git_date,
    packages=find_packages(),
    include_package_data=True,
    keywords='ais_bench tool',
    install_requires=required,
    python_requires='>=3.7',
    entry_points={
        'benchmark_sub_task': ['benchmark=ais_bench.infer.main_cli:get_cmd_instance'],
    },

)