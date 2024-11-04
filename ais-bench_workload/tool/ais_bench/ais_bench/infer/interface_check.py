# Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
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

import re
from ais_bench.infer.args_check import (OM_MODEL_MAX_SIZE, ACL_JSON_MAX_SIZE, LOOP_MAX_SIZE,
    CPP_INT_MAX_SIZE, INPUT_LIST_MAX_SIZE, INPUT_NAME_LENGTH_MAX)
from ais_bench.infer.common.path_security_check import FileStat, FILE_PERM_CHOICE, check_path_legality

CUSTOME_SIZE_MAX_SIZE = 16 * 1024 * 1024 * 1024 # 16 GB
CUSTOM_MAX_COUNT = 256
MODEL_INPUT_TENSOR_COUNT_MAX = 1024
DYM_INFO_PATTERN = "[1-9][0-9]{0,4}(\,[1-9][0-9]{0,4}){0,6}"

def check_model_path_legality(value):
    if not value:
        raise RuntimeError("empty model path!")
    check_path_legality(value, FILE_PERM_CHOICE.READ, max_size=OM_MODEL_MAX_SIZE, suffix=["om"])


def check_acl_json_path_legality(value):
    if not value:
        return
    check_path_legality(value, FILE_PERM_CHOICE.READ, max_size=ACL_JSON_MAX_SIZE, suffix=["json"])


def check_device_range_valid(value):
    # if contain , split to int list
    min_value = 0
    max_value = 255
    # default as single int value
    if not isinstance(value, int):
        raise TypeError(f"device:{value} is not a integer!")
    if value < min_value or value > max_value:
        raise ValueError(f"device:{value} is illegal. legal value range is [{min_value}, {max_value}]")


def check_output_dir_legality(value):
    check_path_legality(value, FILE_PERM_CHOICE.READ, is_file=False)


def check_positive_integer(value):
    if not isinstance(value, int):
        raise TypeError(f"value:{value} is not a integer!")
    if value <= 0 or value > CPP_INT_MAX_SIZE:
        raise ValueError(f"input value:{value} is out of range. Please check.")


def check_in_out_list(in_out_list, inputs, outputs):
    if len(in_out_list) != len(inputs):
        raise RuntimeError(f"inputs' amount and length of in_out_list not matched!")
    for _, reused_index in enumerate(in_out_list):
        if not isinstance(reused_index, int):
            raise TypeError(f"in_out_list reused_index:{reused_index} is not a integer!")
        if reused_index < -1 or reused_index >= len(outputs):
            raise IndexError(f"in_out_list[{in_out_list}] out of range, length range is (-1, {len(outputs)})")


def check_all_list(list_to_check: list, max_len: int, allow_empty: bool = True, data_type: type = int):
    if not isinstance(list_to_check, list):
        raise ValueError("the list be checked is not a list!")
    if not allow_empty and len(list_to_check) == 0:
        raise ValueError("the list is empty")
    if len(list_to_check) > max_len:
        raise ValueError(f"the list's length is over {max_len}!")
    if not data_type:
        return
    for value in list_to_check:
        if not isinstance(value, data_type):
            raise ValueError(f"some value in list is not the expected type: {data_type}")


def check_custom_size(value, mode="dymshape"):
    if mode not in ["static", "dymbatch", "dymhw", "dymdims", "dymshape"]:
        raise ValueError(f"infer mode is illegal, Please check.")
    if mode in ["static", "dymbatch", "dymhw", "dymdims"] and value is None:
        return
    if mode == "dymshape" and value is None:
        raise ValueError(f"custom_size:{value} dismatch with mode. Please check.")

    if isinstance(value, list):
        check_all_list(value, max_len=CUSTOM_MAX_COUNT, allow_empty=False)
        for data in value:
            if data <= 0 or data > CUSTOME_SIZE_MAX_SIZE:
                raise ValueError(f"value:{value} in custom size list is out of range. Please check.")

    elif isinstance(value, int):
        if value <= 0 or value > CUSTOME_SIZE_MAX_SIZE:
            raise ValueError(f"custom size value:{value} is out of range. Please check.")
    else:
        raise TypeError(f"value:{value} is not a integer!")


def check_loop_size(value):
    if not isinstance(value, int):
        raise TypeError(f"value:{value} is not a integer!")
    if value <= 0 or value > LOOP_MAX_SIZE:
        raise ValueError(f"input value:{value} is out of range. Please check.")


def check_bool_value(value):
    if not isinstance(value, bool):
        raise TypeError(f"value:{value} is not a bool!")


def check_dym_hw_list(hw_list:list):
    if len(hw_list) != 2:
        raise ValueError("int count not match, legal format of dymHW string is \"int,int\"")
    try :
        h = int(hw_list[0])
        w = int(hw_list[1])
    except ValueError as err:
        raise ValueError("data type not match, legal format of dymHW string is \"int,int\"")
    if h < 1 or h > CPP_INT_MAX_SIZE:
        raise ValueError(f"height of dym_hw string is out of range [1, {CPP_INT_MAX_SIZE}]")
    if w < 1 or w > CPP_INT_MAX_SIZE:
        raise ValueError(f"width of dym_hw string is out of range [1, {CPP_INT_MAX_SIZE}]")


def check_dym_str_format(shapes_str: str):
    input_info_list = shapes_str.split(";")
    if len(input_info_list) > INPUT_LIST_MAX_SIZE:
        raise ValueError(f"dymshape range string's format is illegal! input count over {INPUT_LIST_MAX_SIZE}")
    for input_info_str in input_info_list:
        input_name_and_value = input_info_str.split(":")
        if len(input_name_and_value) != 2:
            raise ValueError("dymshape range string's format is illegal! input info format wrong!")
        if len(input_name_and_value[0]) < 0 or len(input_name_and_value[0]) > INPUT_NAME_LENGTH_MAX:
            raise ValueError("dymshape range string's format is illegal! " + \
                f"input name len is output of [1, {INPUT_LIST_MAX_SIZE}]")
        if re.compile(r"[^_A-Za-z0-9/.-]").search(input_name_and_value[0]):
            raise ValueError("dymshape range string's format is illegal! input name contain illegal char!")
        if not re.fullmatch(DYM_INFO_PATTERN, input_name_and_value[1]):
            raise ValueError("dymshape range string's format is illegal! range string's format is illegal!")
