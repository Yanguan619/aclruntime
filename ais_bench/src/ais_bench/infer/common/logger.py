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

import sys

import aclruntime


class SpdlogLogger:
    def debug(self, fmt, *args):
        if args:
            fmt = fmt % args
        aclruntime.log_debug(fmt)

    def info(self, fmt, *args):
        if args:
            fmt = fmt % args
        aclruntime.log_info(fmt)

    def warning(self, fmt, *args):
        if args:
            fmt = fmt % args
        aclruntime.log_warning(fmt)

    def error(self, fmt, *args):
        if args:
            fmt = fmt % args
        aclruntime.log_error(fmt)

    def setLevel(self, level):
        _PYTHON_TO_SPDLOG = {10: 0, 20: 1, 30: 2, 40: 3}
        spdlog_level = _PYTHON_TO_SPDLOG.get(level, level)
        aclruntime.set_log_level(spdlog_level)


logger = SpdlogLogger()
