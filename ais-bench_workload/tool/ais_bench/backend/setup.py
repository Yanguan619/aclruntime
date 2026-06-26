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

import os
import re
import subprocess
import platform
import logging
import sys

from pybind11 import get_cmake_dir

# Available at setup time due to pyproject.toml
from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup

logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='[%(levelname)s] %(message)s')
logger = logging.getLogger(__name__)

STATIC_VERSION = "0.0.2"
PATH_WHITE_LIST_REGEX = re.compile(r"[^_A-Za-z0-9/.-]")


def is_legal_path_length(path):
    if len(path) > 4096:
        logger.error(f"file total path length out of range (4096)")
        return False
    dirnames = path.split("/")
    for dirname in dirnames:
        if len(dirname) > 255:
            logger.error(f"file name length out of range (255)")
            return False
    return True


def is_match_path_white_list(path):
    if PATH_WHITE_LIST_REGEX.search(path):
        logger.error(f"path:{path} contains illegal char")
        return False
    return True


def is_legal_args_path_string(path):
    # only check path string
    if not path:
        return True
    if not is_legal_path_length(path):
        return False
    if not is_match_path_white_list(path):
        return False
    return True


class BuildExt(build_ext):
    def build_extensions(self):
        if '-Wstrict-prototypes' in self.compiler.compiler_so:
            self.compiler.compiler_so.remove('-Wstrict-prototypes')
        super().build_extensions()


cann_base_path = None
cann_lib_path = None


def get_cann_path():
    global cann_base_path
    global cann_lib_path
    global cann_include_path

    # get CANN path from env
    set_env_path = os.getenv("CANN_PATH", "")
    if not set_env_path:
        set_env_path = os.environ.get("ASCEND_TOOLKIT_HOME")

    if set_env_path:
        if not is_legal_args_path_string(set_env_path):
            raise TypeError(f"env CANN_PATH:{set_env_path} is illegal")

    other_possible_cann_paths = [
        "/usr/local/Ascend/cann/", #
        "/usr/local/Ascend/nnae/latest/", # atlas_nnae_path old
        "/usr/local/Ascend/ascend-toolkit/latest/", #  atlas_toolkit_path old
        "/usr/local/Ascend/", # hisi_fwk_path old
    ]

    if platform.machine() == "x86_64":
        sub_paths = [
            {"lib": "runtime/lib64/stub/x86_64/", "include": "runtime/include/"}, # old cann structure
            {"lib": "x86_64-linux/lib64/", "include": "x86_64-linux/include/"}, # new cann structure
        ]
    elif platform.machine() == "aarch64":
        sub_paths = [
            {"lib": "runtime/lib64/stub/aarch64/", "include": "runtime/include/"}, # old cann structure
            {"lib": "aarch64-linux/lib64/", "include": "aarch64-linux/include/"}, # new cann structure
        ]

    def is_paths_exist(base_path, sub_paths):
        for sub_path in sub_paths:
            if os.path.exists(os.path.join(base_path, sub_path["lib"])) and \
                os.path.exists(os.path.join(base_path, sub_path["include"])):
                return True, base_path, os.path.join(base_path, sub_path["lib"]), \
                    os.path.join(base_path, sub_path["include"]) # return base_path , lib_path, include_path
        return False, None, None, None # return base_path , lib_path, include_path

    is_exist = False
    cann_base_path = set_env_path
    cann_lib_path = None
    cann_include_path = None

    # get cann path from env
    if set_env_path:
        is_exist, cann_base_path, cann_lib_path, cann_include_path = is_paths_exist(set_env_path, sub_paths)

    if not is_exist:
        for other_path in other_possible_cann_paths:
            is_exist, cann_base_path, cann_lib_path, cann_include_path = is_paths_exist(other_path, sub_paths)
            if is_exist:
                break
        if not is_exist:
            raise RuntimeError(f"Fail to find cann path in {set_env_path} and {other_possible_cann_paths}")

    lib_so_to_check = "libascendcl.so"
    if not os.path.exists(os.path.join(cann_lib_path, lib_so_to_check)):
        raise RuntimeError(f"Fail to find cann lib path in {cann_lib_path}")

    logger.info("Successfully find valid cann path: %s", cann_base_path)


get_cann_path()

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

ext_modules = [
    Pybind11Extension(
        'aclruntime',
        sources=[
            'base/module/DeviceManager/DeviceManager.cpp',
            'base/module/ErrorCode/ErrorCode.cpp',
            'base/module/Log/Log.cpp',
            'base/module/MemoryHelper/MemoryHelper.cpp',
            'base/module/Tensor/TensorBase/TensorBase.cpp',
            'base/module/Tensor/TensorBuffer/TensorBuffer.cpp',
            'base/module/Tensor/TensorContext/TensorContext.cpp',
            'base/module/ModelInfer/model_process.cpp',
            'base/module/ModelInfer/WeightPool.cpp',
            'base/module/ModelInfer/utils.cpp',
            'base/module/ModelInfer/SessionOptions.cpp',
            'base/module/ModelInfer/ModelInferenceProcessor.cpp',
            'base/module/ModelInfer/DynamicAippConfig.cpp',
            'base/module/ModelInfer/cnpy.cpp',
            'base/module/ModelInfer/pipeline.cpp',
            'base/module/ModelInfer/File.cpp',
            'base/module/ModelInfer/StringChecker.cpp',
            'python/src/PyInterface/PyInterface.cpp',
            'python/src/PyTensor/PyTensor.cpp',
            'python/src/PyInferenceSession/PyInferenceSession.cpp',
        ],
        include_dirs=[
            'python/include/',
            'base/include/',
            'base/include/Base/ModelInfer/',
            f'{cann_include_path}/',
        ],
        library_dirs=[
            cann_lib_path,
        ],
        extra_compile_args=['--std=c++11', '-g3', '-fstack-protector-all'],
        extra_link_args=['-Wl,-z,relro,-z,now', '-s'],
        libraries=['ascendcl', 'acl_dvpp', 'acl_cblas'],
        language='c++',
        define_macros=[('ENABLE_DVPP_INTERFACE', 1), ('COMPILE_PYTHON_MODULE', 1)],
    ),
]

setup(
    name="aclruntime",
    version=STATIC_VERSION,
    author="ais_bench",
    author_email="aclruntime",
    url=f"gitee repo: Ascend/tools, commit id: {git_hash}, release_date: {git_date}",
    release_date=git_date,
    description="A test project using pybind11 and aclruntime",
    long_description="",
    ext_modules=ext_modules,
    cmdclass={"build_ext": BuildExt},
    zip_safe=False,
    python_requires=">=3.6",
)
