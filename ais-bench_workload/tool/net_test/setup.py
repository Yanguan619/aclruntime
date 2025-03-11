# Copyright (c) 2024-2025 Huawei Technologies Co., Ltd.
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
import platform
import subprocess
from setuptools import setup, find_packages  # type: ignore
from setuptools.command.install_scripts import install_scripts
from ais_bench.net_test.common.consts import PackageInfo

CUR_DIR_ABS_PATH = os.path.dirname(os.path.abspath(__file__))

OP_TASK_LIST = [
    "all_gather_test",
    "all_reduce_test",
    "alltoall_test",
    "alltoallv_test",
    "broadcast_test",
    "reduce_scatter_test",
    "reduce_test",
    "scatter_test",
]


class InstallScripts(install_scripts):
    def run(self):
        install_scripts.run(self)
        for op_bin in OP_TASK_LIST:
            bin_abs_path = os.path.join(CUR_DIR_ABS_PATH, f"ais_bench/backend/net_test/hccl_test/bin/{op_bin}")
            os.chmod(bin_abs_path, 0o500)


def make_hccl_test_bin():
    cann_path = os.getenv("ASCEND_TOOLKIT_HOME", "/usr/local/Ascend/ascend-toolkit/latest")
    work_dir = "./ais_bench/backend/net_test/hccl_test"
    make_cmd_list = [
        "make", "-f", os.path.join(work_dir, "Makefile"), f"WORK_DIR={work_dir}", f"ASCEND_DIR={cann_path}"
    ]

    try:
        subprocess.run(make_cmd_list, check=True, text=True, capture_output=True)
    except subprocess.CalledProcessError as e:
        raise RuntimeError(f"make hccl test failed with return code {e.returncode}: {e.stderr}") from e


def check_all_op_task_bin_exist():
    for op_task in OP_TASK_LIST:
        if not os.path.exists(f"./ais_bench/backend/net_test/hccl_test/bin/{op_task}"):
            return False
    return True

with open('requirements.txt', encoding='utf-8') as f:
    required = f.read().splitlines()

with open('README.md', encoding='utf-8') as f:
    long_description = f.read()

if not check_all_op_task_bin_exist():
    make_hccl_test_bin()

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
    name='ais_bench_net_test',
    version=PackageInfo.version,
    description='ais_bench net test tools',
    long_description=long_description,
    url=f"gitee repo: Ascend/tools, commit id: {git_hash}, release_date: {git_date}",
    packages=find_packages(),
    include_package_data=True,
    keywords='ais_bench tool',
    install_requires=required,
    python_requires='>=3.7',
    cmdclass={
        "install_scripts": InstallScripts
    },
    options={
        "bdist_wheel": {
            "plat_name": f"{sys.platform}-{platform.machine()}"
        }
    }
)