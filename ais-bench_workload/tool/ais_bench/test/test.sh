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

# define error code
declare -i ret_ok=0
declare -i ret_failed=1
declare -i ret_invalid_args=1

CUR_PATH=$(dirname $(readlink -f "$0"))

. $CUR_PATH/utils.sh # 导入通用函数
source $CUR_PATH/test_config.sh # 导入DT配置

set -x # 打印执行命令
# set -e # 任何一行命令失败shell脚本都会退出

# 其他全局变量
MSAME_PATH=$CUR_PATH/msame
SOC_VERSION=""
UT_LIST=()
ST_LIST=()

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

function env_set()
{
    export PYTHONPATH=$CUR_PATH:$PYTHONPATH
    export MSAME_BIN_PATH=$MSAME_PATH
}

function data_generate()
{ # all generated data in $CUR_PATH/testdata
    bash -x $CUR_PATH/get_pth_resnet50_data.sh $SOC_VERSION
    bash -x $CUR_PATH/get_add_model_data.sh
    $PYTHON_COMMAND $CUR_PATH/generate_pipeline_datasets.py
}

function get_dt_list()
{
    mode=$1
    if [ $mode == "full" ];then
        echo "run DT in full mode"
        UT_LIST=(${full_ut_script_list[@]})
        ST_LIST=(${full_st_script_list[@]})
    elif [ $mode == "simple" ];then
        echo "run DT in simple mode"
        UT_LIST=(${simple_ut_script_list[@]})
        ST_LIST=(${simple_st_script_list[@]})
    else
        echo "unrecoginized mode: $mode, use default simple mode"
        UT_LIST=$simple_ut_script_list
        ST_LIST=$simple_st_script_list
    fi
}

function run_dt_only()
{
    # run selected ut list
    for scripts in ${UT_LIST[@]}
    do
        $PYTHON_COMMAND -m pytest -v -s $CUR_PATH/UT/$scripts
    done

    # run selected st list
    for scripts in ${ST_LIST[@]}
    do
        $PYTHON_COMMAND -m pytest -v -s $CUR_PATH/ST/$scripts
    done
}

main() {
    # self func
    get_npu_type
    # self func
    chmod_file_data
    # utils.sh func
    get_msame_file $MSAME_PATH || { echo "get msame bin file failed";return $ret_failed; }
    # utils.sh func
    check_python_package_is_install $PYTHON_COMMAND "aclruntime" || {
        echo "aclruntime package install failed please install or source set_env.sh"
        return $ret_invalid_args
    }
    # self func
    env_set
    # self func
    data_generate
    # self func
    get_dt_list $AISBENCH_INFER_DT_MODE

    if [ "$PYTEST_RUN_MODE" == "run_only" ];then
        # self func
        run_dt_only
    fi

    return $ret_ok
}

main "$@"
exit $?
