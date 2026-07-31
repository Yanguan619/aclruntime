#!/bin/bash

# Copyright (c) 2025-2025 Huawei Technologies Co., Ltd. All rights reserved.
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

set -e
script=$(readlink -f "$0") # test/scripts/run_ut_and_gen_cov.sh
route=$(dirname "$script") # test/scripts/
test_root=$(dirname "$route") # test/

cd ${test_root}/../
CUR_DIR=$(pwd)

if pip show aclruntime >/dev/null 2>&1; then
    pip uninstall -y aclruntime
fi

if pip show ais_bench >/dev/null 2>&1; then
    pip uninstall -y ais_bench
fi

rm -f ${CUR_DIR}/aclruntime-0.0.2-*.whl
rm -f ${CUR_DIR}/ais_bench-0.0.2-py3-none-any.whl

bash -x aclruntime/build_aclruntime.sh pip3

pip3 install ${CUR_DIR}/aclruntime-0.0.2-*.whl
pip3 install -e ./
export PYTHONPATH=${CUR_DIR}:$PYTHONPATH

cd ${test_root}
code_dir=${test_root}/../ais_bench/

function clean() {
    rm -rf ${test_root}/.coverage ${test_root}/report
    mkdir -p ${test_root}/report
    coverage erase
}

ut_script_list=( \
    "utils_file/test_abnormal_cmd_args_check.py" \
    "utils_file/test_interface_check.py" \
    "utils_file/test_backend_trtexec.py" \
    "utils_file/test_io_operations.py" \
    "utils_file/test_miscellaneous.py" \
    "utils_file/test_path_security_check.py" \
    "utils_file/test_utils.py" \
    "utils_file/test_dym_aipp_manager.py" \
    "utils_file/test_summary.py" \
    "utils_file/test_infer_process.py" \
    "test_argparser.py" \
    "test_json_convert.py" \
    "test_aclruntimeAPI/test_session_options.py"\
)

SELECTED_TEST_DIR=${test_root}/test_to_run_and_gen_cov

if [ -d $SELECTED_TEST_DIR ];then
    rm -rf $SELECTED_TEST_DIR
fi
mkdir -p $SELECTED_TEST_DIR/ut

cp -r ${test_root}/aipp_config_files $SELECTED_TEST_DIR
for scripts in ${ut_script_list[@]}
do
    cp ${test_root}/UT/$scripts $SELECTED_TEST_DIR/ut
done

clean

ret=0
echo "run test cases and generate coverage report"
coverage3 run --branch --source=${code_dir} -m pytest $SELECTED_TEST_DIR/ut/ --junitxml="${test_root}/report/final.xml" -W ignore::DeprecationWarning || ret=1

coverage3 xml -o ${test_root}/report/coverage.xml

exit ${ret}
