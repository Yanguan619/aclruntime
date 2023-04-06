#!/usr/bin/env python
# coding=utf-8
"""
Function:
This file mainly involves the common function.
Copyright Information:
Huawei Technologies Co., Ltd. All Rights Reserved © 2020
"""

import os
import numpy as np

from ms_interface import utils
from ms_interface.constant import Constant
from ms_interface.dump_data_parser import DumpDataParser
from ms_interface.single_op_test_frame.common.ascend_tbe_op import AscendOpKernel, AscendOpKernelRunner


class SingleOpCase:

    def __init__(self, collection) -> None:
        self.collection = collection

    def run(self):
        runner = AscendOpKernelRunner()

        kernel_path = self.collection.collect_kernel_path
        for kernel_name in self.collection.kernel_name_list:
            bin_path = os.path.join(kernel_path, f"{kernel_name}.o")
            json_path = os.path.join(kernel_path, f"{kernel_name}.json")
            op_kernel = AscendOpKernel(bin_path, json_path)
            tiling_data = self.collection.tiling_list[1]
            tiling_key = self.collection.tiling_list[0]
            block_dim = self.collection.tiling_list[2]
            input_data_list = []
            for (_, tensor) in enumerate(self.collection.input_list):
                if tensor.data_type not in DumpDataParser.DATA_TYPE_TO_DTYPE_MAP:
                    utils.print_error_log(f"The output data type({tensor.data_type}) does not support.")
                    raise utils.AicErrException(Constant.MS_AICERR_INVALID_DUMP_DATA_ERROR)
                dtype = DumpDataParser.DATA_TYPE_TO_DTYPE_MAP.get(tensor.data_type).get(Constant.DTYPE)
                array = np.frombuffer(tensor.data, dtype=dtype)
                input_data_list.append(array)
            output_data_list = runner.run(op_kernel, inputs=input_data_list, tiling_data=tiling_data, 
                                          block_dim=block_dim, tiling_key=tiling_key)

