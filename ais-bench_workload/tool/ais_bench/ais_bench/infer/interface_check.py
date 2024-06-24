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
from ais_bench.infer.common.path_security_check import FileStat


def check_model_path_legality(value):
    if not value:
        raise RuntimeError("empty model path!")
    try:
        file_stat = FileStat(value)
    except Exception as err:
        raise RuntimeError(f"om path:{value} is illegal. Please check.") from err
    if not file_stat.is_basically_legal('read'):
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
    if not file_stat.is_basically_legal('read'):
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
    if not file_stat.is_basically_legal('write'):
        raise RuntimeError(f"output path:{value} is illegal. Please check.")