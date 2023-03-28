#!/usr/bin/env python
# coding=utf-8
"""
Function:
This file mainly involves the common function.
Copyright Information:
Huawei Technologies Co., Ltd. All Rights Reserved © 2020
"""

import os

from ms_interface import utils
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
            input_data_list = self.collection.input_list
            output_data_list = runner.run(op_kernel, inputs=input_data_list, tiling_data=tiling_data, 
                                          block_dim=op_kernel.block_dim, tiling_key=tiling_key)

