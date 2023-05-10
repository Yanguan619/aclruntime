#!/usr/bin/env python
# coding=utf-8
"""
Function:
This file mainly involves the common function.
Copyright Information:
Huawei Technologies Co., Ltd. All Rights Reserved © 2020
"""

import json
import os
from time import sleep
import time
import numpy as np
from ms_interface import utils
from ms_interface.constant import Constant
from ms_interface.dump_data_parser import DumpDataParser
from ms_interface.single_op_test_frame.common.ascend_tbe_op import AscendOpKernel, AscendOpKernelRunner


class SingleOpCase:

    def __init__(self, collection) -> None:
        self.collection = collection

    @staticmethod
    def _check_file_content(content):
        error_strings = [
            "there is an aivec error exception",
            "there is an aicore error exception",
            "aicore exception"
        ]
        for s in error_strings:
            if s in content:
                return True
        return False
 
    @staticmethod
    def _wait_for_log_stabilization(log_path):
        log_size = os.path.getsize(log_path)
        while True:
            sleep(0.2)
            current_log_size = os.path.getsize(log_path)
            if current_log_size == log_size:
                break
            log_size = current_log_size

    @staticmethod
    def search_aicerr_log(path):
        for root, _, files in os.walk(path):
            for file in files:
                if not file.endswith(".log"):
                    continue
                utils.print_info_log(f"The find single op log {file}")
                log_path = os.path.join(root, file)
                SingleOpCase._wait_for_log_stabilization(log_path)
                with open(log_path, "r") as f:
                    content = f.read()
                if SingleOpCase._check_file_content(content):
                    return True
        return False

    def generate_config(self):
        config_file_list = []
        kernel_path = self.collection.collect_kernel_path
        for kernel_name in self.collection.kernel_name_list:
            config_file = os.path.join(self.collection.output_path, f"config_{kernel_name}.json")
            data = {
                "bin_path": os.path.join(kernel_path, f"{kernel_name}.o"),
                "json_path": os.path.join(kernel_path, f"{kernel_name}.json"),
                "tiling_data": self.collection.tiling_list[1].decode("utf-8"),
                "tiling_key": self.collection.tiling_list[0],
                "block_dim": self.collection.tiling_list[2],
                "input_file_list": self.collection.input_list,
                "output_file_list": self.collection.output_list
            }
            with open(config_file, "w") as json_file:
                json.dump(data, json_file, indent=4)
            config_file_list.append(config_file)
        return config_file_list

    @staticmethod
    def run(config_file):
        # set single op log path
        date_string = time.strftime("%Y%m%d%H%M%S", time.localtime(int(time.time())))
        single_op_log_path = os.path.join(os.path.dirname(config_file), f"single_op_log_{date_string}")
        os.environ['ASCEND_PROCESS_LOG_PATH'] = single_op_log_path
        utils.print_info_log(f"The single_op_log_path is {single_op_log_path}")

        with open(config_file) as f:
            data = json.load(f)
        runner = AscendOpKernelRunner()
        bin_path = data["bin_path"]
        json_path = data["json_path"]
        tiling_data = data["tiling_data"].encode("utf-8")
        tiling_key = data["tiling_key"]
        block_dim = data["block_dim"]
        input_file_list = data["input_file_list"]
        output_file_list = data["output_file_list"]

        input_data_list = []
        for data in input_file_list:
            input_data = np.load(data)
            input_data_list.append(input_data)

        output_info_list = []
        for data in output_file_list:
            output_info = {}
            np_data = np.load(data)
            output_info["size"] = np_data.nbytes
            output_info["dtype"] = str(np_data.dtype)
            output_info["shape"] = np_data.shape
            output_info_list.append(output_info)

        op_kernel = AscendOpKernel(bin_path, json_path)
        runner.run(op_kernel,
                   inputs=input_data_list,
                   tiling_data=tiling_data,
                   block_dim=block_dim,
                   tiling_key=tiling_key,
                   actual_output_info=output_info_list)

        return not SingleOpCase.search_aicerr_log(os.path.join(single_op_log_path, "debug"))
