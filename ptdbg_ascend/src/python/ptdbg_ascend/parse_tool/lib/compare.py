#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
# Copyright (C) 2022-2023. Huawei Technologies Co., Ltd. All rights reserved.
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
"""

import os
from .utils import Util
from .config import Const


class Compare:
    def __init__(self):
        self.util = Util()
        self.log = self.util.log
        self.vector_compare_result = {}
        self.msaccucmp = None

    @property
    def call_msaccucmp(self):
        if not self.msaccucmp:
            self.msaccucmp = self.util.check_msaccucmp(Const.MS_ACCU_CMP_PATH)
        return self.msaccucmp

    def npu_vs_npu_compare(self, my_dump_path, golden_dump_path, result_dir):
        self.log.info("Start Compare ...............")
        self.compare_vector(my_dump_path, golden_dump_path, result_dir)
        self.log.info("Compare finished!!")

    def compare_vector(self, my_dump_path, golden_dump_path, result_dir):
        self.util.create_dir(result_dir)
        cmd = '%s %s compare -m %s -g %s -out %s' % (
            self.util.python, self.call_msaccucmp, my_dump_path, golden_dump_path, result_dir
        )
        return self.util.execute_command(cmd)

    def convert_dump_to_npy(self, dump_file, data_format, output):
        file_name = ""
        if os.path.isfile(dump_file):
            self.log.info("Covert file is: %s", dump_file)
            file_name = os.path.basename(dump_file)
        elif os.path.isdir(dump_file):
            self.log.info("Convert all files in path: %s", dump_file)
            file_name = ""
        output = output if output else Const.DUMP_CONVERT_DIR
        convert = self.convert(dump_file, data_format, output)
        if convert == 0:
            convert_files = self.util.list_convert_files(output, file_name)

            summary_txt = ["SrcFile: %s" % dump_file]
            for convert_file in convert_files.values():
                summary_txt.append(" - %s" % convert_file.file_name)
            self.util.print_panel("\n".join(summary_txt))

    def convert(self, dump_file, data_format, output):
        self.util.create_dir(output)
        if data_format:
            cmd = '%s %s convert -d %s -out %s -f %s' % (
                self.util.python, self.call_msaccucmp, dump_file, output, data_format
            )
        else:
            cmd = '%s %s convert -d %s -out %s' % (
                self.util.python, self.call_msaccucmp, dump_file, output
            )
        return self.util.execute_command(cmd)


