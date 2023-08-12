#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
# Copyright (C) 2019-2020. Huawei Technologies Co., Ltd. All rights reserved.
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
import argparse
import os

from .config import Const
from .utils import Util
from .compare import Compare
from .visualization import Visualization
from .parse_exception import catch_exception


class ParseTool:
    def __init__(self):
        self.util = Util()
        self.compare = Compare()
        self.visual = Visualization()

    @catch_exception
    def prepare(self):
        self.util.create_dir(Const.DATA_ROOT_DIR)

    @catch_exception
    def do_vector_compare(self, argv=None):
        parser = argparse.ArgumentParser()
        parser.add_argument(
            "-m", "--my_dump_path", dest="my_dump_path", default=None,
            help="<Required> my dump path, the data compared with golden data",
            required=True
        )
        parser.add_argument(
            "-g", "--golden_dump_path", dest="golden_dump_path", default=None,
            help="<Required> the golden dump data path",
            required=True
        )
        parser.add_argument(
            "-out", "--output_path", dest="output_path", default=None,
            help="<Optional> the output path",
            required=False
        )
        args = parser.parse_args(argv)
        if not args.output_path:
            result_dir = os.path.join(Const.COMPARE_DIR)
        else:
            result_dir = args.output_path
        my_dump_path = args.my_dump_path
        golden_dump_path = args.golden_dump_path
        self.compare.npu_vs_npu_compare(my_dump_path, golden_dump_path, result_dir)

    @catch_exception
    def do_convert_dump(self, argv=None):
        parser = argparse.ArgumentParser()
        parser.add_argument(
            '-n', '--name', dest='path', default=None, required=True, help='dump file or dump file directory')
        parser.add_argument(
            '-f', '--format', dest='format', default=None, required=False, help='target format')
        parser.add_argument(
            '-out', '--output_path', dest='output_path', required=False, default=None, help='output path')
        args = parser.parse_args(argv)
        self.util.check_path_valid(args.path)
        self.compare.convert_dump_to_npy(args.path, args.format, args.output_path)

    @catch_exception
    def do_print_data(self, argv=None):
        """print tensor data"""
        parser = argparse.ArgumentParser()
        parser.add_argument('-n', '--name', dest='path', default=None, required=True, help='File name')
        args = parser.parse_args(argv)
        self.visual.print_npy_data(args.path)

    @catch_exception
    def do_parse_pkl(self, argv=None):
        parser = argparse.ArgumentParser()
        parser.add_argument(
            '-f', '--file', dest='file_name', default=None,  required=True, help='PKL file path')
        parser.add_argument(
            '-n', '--name', dest='api_name', default=None,  required=True, help='API name')
        args = parser.parse_args(argv)
        self.visual.parse_pkl(args.file_name, args.api_name)


















