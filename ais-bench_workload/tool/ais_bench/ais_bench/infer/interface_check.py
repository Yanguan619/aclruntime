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

from ais_bench.infer.args_check import OM_MODEL_MAX_SIZE,  ACL_JSON_MAX_SIZE
from ais_bench.infer.common.path_security_check import FileStat, FILE_PERM_CHOICE

CPP_INT_MAX_SIZE = 2147483647 # 2^31 - 1
SIZE_T_MAX_SIZE = 4294967295 # 2^32 - 1

def check_model_path_legality(value):
    if not value:
        raise RuntimeError("empty model path!")
    try:
        file_stat = FileStat(value)
    except Exception as err:
        raise RuntimeError(f"om path:{value} is illegal. Please check.") from err
    if not file_stat.is_basically_legal(FILE_PERM_CHOICE.READ):
        raise RuntimeError(f"om path:{value} is illegal. Please check.")
    if not file_stat.is_legal_file_size(OM_MODEL_MAX_SIZE):
        raise RuntimeError(f"om path:{value} is illegal. Please check.")


def check_acl_json_path_legality(value):
    if not value:
        return
    try:
        file_stat = FileStat(value)
    except Exception as err:
        raise RuntimeError(f"acl json path:{value} is illegal. Please check.") from err
    if not file_stat.is_basically_legal(FILE_PERM_CHOICE.READ):
        raise RuntimeError(f"acl json path:{value} is illegal. Please check.")
    if not file_stat.is_legal_file_type(["json"]):
        raise RuntimeError(f"acl json path:{value} is illegal. Please check.")
    if not file_stat.is_legal_file_size(ACL_JSON_MAX_SIZE):
        raise RuntimeError(f"acl json path:{value} is illegal. Please check.")


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
    try:
        file_stat = FileStat(value)
    except Exception as err:
        raise RuntimeError(f"output path:{value} is illegal. Please check.") from err
    if not file_stat.is_basically_legal(FILE_PERM_CHOICE.READ):
        raise RuntimeError(f"output path:{value} is illegal. Please check.")
    
def check_positive_integer(value):
    if not isinstance(value, int):
        raise TypeError(f"value:{value} is not a integer!")
    if value <= 0 or value >= CPP_INT_MAX_SIZE:
        raise ValueError(f"input value:{value} is out of range. Please check.")

def check_in_out_list(in_out_list, inputs, outputs):
    if len(in_out_list) != len(inputs):
        raise RuntimeError(f"inputs' amount and length of in_out_list not matched!")
    for _, reused_index in enumerate(in_out_list):
        if not isinstance(reused_index, int):
            raise TypeError(f"in_out_list reused_index:{reused_index} is not a integer!")
        if reused_index < -1 or reused_index >= len(outputs):
            raise IndexError(f"in_out_list[{in_out_list}] out of range, length range is (-1, {len(outputs)})")

def check_custom_size(value):
    if type(value) == list:
        ivalue = value[0]
    else:
        ivalue = value
    if not isinstance(ivalue, int):
        raise TypeError(f"value:{value} is not a integer!")
    if ivalue <= 0 or ivalue >= SIZE_T_MAX_SIZE:
        raise ValueError(f"input value:{value} is out of range. Please check.")

def check_bool_value(value):
    if not isinstance(value, bool):
        raise TypeError(f"value:{value} is not a bool!")
