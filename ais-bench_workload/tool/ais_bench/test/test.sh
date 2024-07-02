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
SELECTED_TEST_DIR=$CUR_PATH/test_script_to_run
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
    elif [ $mode == "debug" ];then
        echo "run DT in simple mode"
        UT_LIST=(${debug_ut_script_list[@]})
        ST_LIST=(${debug_st_script_list[@]})
    else
        echo "unrecoginized AISBENCH_INFER_DT_MODE: $mode, use default simple mode"
        UT_LIST=$simple_ut_script_list
        ST_LIST=$simple_st_script_list
    fi

    if [ -d $SELECTED_TEST_DIR ];then
        rm -r $SELECTED_TEST_DIR
    fi
    mkdir -p $SELECTED_TEST_DIR/ut
    mkdir -p $SELECTED_TEST_DIR/st

    cp -r $CUR_PATH/aipp_config_files $SELECTED_TEST_DIR
    # copy selected ut list
    for scripts in ${UT_LIST[@]}
    do
        cp $CUR_PATH/UT/$scripts $SELECTED_TEST_DIR/ut
    done
    # copy selected st list
    for scripts in ${ST_LIST[@]}
    do
        cp $CUR_PATH/ST/$scripts $SELECTED_TEST_DIR/st
    done
}

function run_dt()
{
    if [ "$PYTEST_RUN_MODE" == "run_only" ];then
        # self func
        run_dt_only
    elif [ "$PYTEST_RUN_MODE" == "csv_report" ];then
        # self func
        run_dt_with_csv_report
    elif [ "$PYTEST_RUN_MODE" == "html_report" ];then
        # self func
        run_dt_with_html_report
    else
        echo "unrecoginized PYTEST_RUN_MODE: $PYTEST_RUN_MODE, use default run_only"
    fi
}

function run_dt_only()
{
    $PYTHON_COMMAND -m pytest -s $SELECTED_TEST_DIR
}

function run_dt_with_csv_report()
{
    csv_path=$CUR_PATH/report.csv
    if [ -f $csv_path ];then
        rm $csv_path
    fi
    $PYTHON_COMMAND -m pytest --csv $csv_path  -s $SELECTED_TEST_DIR
}

function run_dt_with_html_report()
{
    html_path=$CUR_PATH/report.html
    if [ -f $html_path ];then
        rm $html_path
    fi
    $PYTHON_COMMAND -m pytest --html $html_path -s $SELECTED_TEST_DIR
}

main() {
    # self func
    install_pypi_requirements
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
    # self func
    run_dt

    return $ret_ok
}

main "$@"
exit $?
