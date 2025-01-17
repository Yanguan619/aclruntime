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


def console_origin(line):
    sys.stdout.write(line.decode('utf-8'))
    sys.stdout.flush()


MODULE_NAME = "AIS_BENCH_NET_TEST"
ENV_DEBUG_SWITCH = "AIS_BENCH_NET_TEST_DEBUG_SWITCH"

# 是否开启debug
debug_switch = os.environ.get(ENV_DEBUG_SWITCH, "0")
default_log_level = logging.INFO
if debug_switch == "1":
    default_log_level = logging.DEBUG

# 创建一个logger
logger = logging.getLogger(MODULE_NAME)
logger.setLevel(default_log_level)  # 设置日志级别

# 创建一个handler，用于将日志输出到控制台
console_handler = logging.StreamHandler()
console_handler.setLevel(default_log_level)  # 设置handler的日志级别

# 创建一个formatter，设置日志格式
formatter = logging.Formatter("[%(asctime)s][%(levelname)s][%(name)s] %(message)s", datefmt="%Y-%m-%d %H:%M:%S")
# 其中，'%(asctime)s' 是自动添加时间戳的关键

# 添加formatter到handler
console_handler.setFormatter(formatter)

# 添加handler到logger
logger.addHandler(console_handler)