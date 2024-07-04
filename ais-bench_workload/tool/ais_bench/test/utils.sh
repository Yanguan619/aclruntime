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
declare -i ret_ok=0
declare -i ret_failed=1

function check_python_package_is_install()
{
    local PYTHON_COMMAND=$1
    ${PYTHON_COMMAND} -c "import $2" >>/dev/null 2>&1
    ret=$?
    if [ $ret != 0 ]; then
        echo "python package:$1 not install"
        return $ret_failed
    fi
    return $ret_ok
}

function get_msame_file()
{
    get_arch=`arch`
    if [[ $get_arch =~ "x86_64" ]];then
        echo "arch x86_64"
        local convert_url="https://aisbench.obs.myhuaweicloud.com/packet/msame/x86/msame"
        wget $convert_url -O $1 --no-check-certificate || { echo "wget $convert_url failed!";return $ret_failed; }
    elif [[ $get_arch =~ "aarch64" ]];then
        echo "arch arm64"
        local convert_url="https://aisbench.obs.myhuaweicloud.com/packet/msame/arm/msame"
        wget $convert_url -O $1 --no-check-certificate || { echo "wget $convert_url failed!";return $ret_failed; }
    else
        echo "unknown!!"
        return $ret_failed;
    fi
    [ -f $1 ] || { echo "not find msame:$1 please check"; return $ret_failed; }
    chmod 750 $1
    return $ret_ok
}

function install_pypi_requirements()
{
    pip3 install onnx
    pip3 install torch torchvision
    pip3 install pytest pytest-cov pytest-csv pytest-html
}