#!/bin/bash

# Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
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

declare -i ret_ok=0
declare -i ret_failed=1
declare -i ret_invalid_args=1
CUR_PATH=$(dirname $(readlink -f "$0"))
. $CUR_PATH/utils.sh
set -x
set -e
MSAME_PATH=$CUR_PATH/msame
SOC_VERSION=""

function get_npu_type()
{
    SOC_VERSION=`python3 -c 'import acl; print(acl.get_soc_name())'` || { return $ret_failed; }
    echo "npu is $SOC_VERSION"
    return $ret_ok
}

function chmod_file_data()
{
    chmod 750 $CUR_PATH/json_for_arg_test.json
    chmod -R 750 $CUR_PATH/aipp_config_files
}

function data_generate()
{ # all generated data in $CUR_PATH/testdata
    py_cmd=$1
    bash -x $CUR_PATH/get_pth_resnet50_data.sh $SOC_VERSION $py_cmd $AISBENCH_INFER_DT_MODE
    bash -x $CUR_PATH/get_add_model_data.sh
    ${py_cmd} $CUR_PATH/generate_pipeline_datasets.py
}

main() {
    chmod_file_data
    # utils.sh func
    get_msame_file $MSAME_PATH || { echo "get msame bin file failed";return $ret_failed; }
    [ -f $MSAME_PATH ] || { echo "not find msame:$MSAME_PATH please check"; return $ret_invalid_args; }
    chmod 750 $MSAME_PATH

    if [ $# -lt 2 ]; then
        echo "at least one parameter. for example: bash test.sh Ascend310P3 python3"
        return $ret_invalid_args
    fi

    export PYTHON_COMMAND=${1:-"python3"}
    export AISBENCH_INFER_DT_MODE=${2:-"simple"}
    export PYTHONPATH=$CUR_PATH:$PYTHONPATH
    export MSAME_BIN_PATH=$MSAME_PATH

    # utils.sh func
    check_python_package_is_install $PYTHON_COMMAND "aclruntime" || {
        echo "aclruntime package install failed please install or source set_env.sh"
        return $ret_invalid_args
    }

    data_generate $PYTHON_COMMAND

    if [ $BENCKMARK_DT_MODE == "full" ];then
        echo "run DT in full mode"
        ${PYTHON_COMMAND} -m pytest -s $CUR_PATH/UT/
        ${PYTHON_COMMAND} -m pytest -s $CUR_PATH/ST/
    else
        echo "run DT in simple mode"
        ${PYTHON_COMMAND} -m pytest -x $CUR_PATH/UT_SIMPLE/ || { return $ret_failed; }
        ${PYTHON_COMMAND} -m pytest -x $CUR_PATH/ST_SIMPLE/ || { return $ret_failed; }
    fi

    return $ret_ok
}

main "$@"
exit $?
