#!/usr/bin/env python
# coding=utf-8
"""
Function:
This file mainly involves the common function.
Copyright Information:
Huawei Technologies Co., Ltd. All Rights Reserved © 2020
"""

import os
from time import sleep
import numpy as np
from ms_interface import utils
from ms_interface.constant import Constant
from ms_interface.dump_data_parser import DumpDataParser
from ms_interface.single_op_test_frame.common.ascend_tbe_op import AscendOpKernel, AscendOpKernelRunner


class SingleOpCase:

    def __init__(self, collection) -> None:
        self.collection = collection

    def search_aicerr_log(self, path):
        for root, _, files in os.walk(path):
            for file in files:
                if file.endswith(".log"):
                    utils.print_info_log(f"The find single op log  {file}")
                    log_size=os.path.getsize(os.path.join(root, file))
                    while True:
                        sleep(0.2)
                        current_log_size=os.path.getsize(os.path.join(root, file))
                        if current_log_size == log_size:
                            break
                        else:
                            log_size = current_log_size
                    with open(os.path.join(root, file), "r") as f:
                        content = f.read()
                        if "there is an aivec error exception" in content or "there is an aicore error exception" in content or "aicore exception" in content:
                            return True
        return False

    def run(self):
        # set single op log path
        single_op_log_path=os.path.join(self.collection.output_path, "single_op_log")
        os.environ['ASCEND_PROCESS_LOG_PATH']=single_op_log_path
        utils.print_info_log(f"The single_op_log_path is {single_op_log_path}")

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
            
            runner.run(op_kernel, inputs=input_data_list, tiling_data=tiling_data, 
                                          block_dim=block_dim, tiling_key=tiling_key)
        
        return not self.search_aicerr_log(os.path.join(single_op_log_path, "debug"))
