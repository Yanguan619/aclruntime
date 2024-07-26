#!/bin/bash

# Copyright (c) 2023-2024 Huawei Technologies Co., Ltd.
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

export PYTHON_COMMAND="python3"
export AISBENCH_INFER_DT_DEVICE_ID=7
export AISBENCH_INFER_DT_MODE="debug" # "full" "simple" "debug"
export PYTEST_RUN_MODE="show_coverage" # "run_only" "csv_report" "html_report" "show_coverage"

debug_st_script_list=( \
)

debug_ut_script_list=( \
    "utils_file/test_abnormal_cmd_args_check.py" \
    "utils_file/test_cmd_args_check.py" \
    "utils_file/test_backend_trtexec.py" \
    "utils_file/test_miscellaneous.py" \
    "utils_file/test_summary.py" \
    "utils_file/test_path_security_check.py" \
    "test_argparser.py" \
)

full_st_script_list=( \
    "test_acl_json_profiling.py" \
    "test_args_st.py" \
    "test_infer_resnet50_dymshape.py" \
    "test_infer_resnet50_normal.py" \
    "test_result.py" \
)

full_ut_script_list=( \
    "utils_file/test_abnormal_cmd_args_check.py" \
    "utils_file/test_cmd_args_check.py" \
    "utils_file/test_backend_trtexec.py" \
    "utils_file/test_miscellaneous.py" \
    "utils_file/test_summary.py" \
    "utils_file/test_path_security_check.py" \
    "test_argparser.py" \
    "test_args_ut.py" \
    "test_dymaipp.py" \
    "test_dymshape.py" \
    "test_inference.py" \
    "test_interface_multidevice_api_dymshape.py" \
    "test_interface_multidevice_api_normal.py" \
    "test_interface_single_session_api_dymshape.py" \
    "test_interface_single_session_api_normal.py" \
    "test_json_convert.py" \
    "test_pipeline_interface.py" \
    "test_pipeline_run_dymshape.py" \
    "test_pipeline_run_normal.py" \
)

simple_st_script_list=( \
    "test_acl_json_profiling.py" \
    "test_args_st.py" \
    "test_infer_resnet50_normal.py" \
    "test_result.py" \
)

simple_ut_script_list=( \
    "utils_file/test_abnormal_cmd_args_check.py" \
    "utils_file/test_cmd_args_check.py" \
    "utils_file/test_backend_trtexec.py" \
    "utils_file/test_miscellaneous.py" \
    "utils_file/test_summary.py" \
    "utils_file/test_path_security_check.py" \
    "test_argparser.py" \
    "test_args_ut.py" \
    "test_dymaipp.py" \
    "test_inference.py" \
    "test_interface_multidevice_api_normal.py" \
    "test_interface_single_session_api_normal.py" \
    "test_json_convert.py" \
    "test_pipeline_interface.py" \
    "test_pipeline_run_normal.py" \
)