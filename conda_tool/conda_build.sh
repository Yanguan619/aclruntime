#!/bin/bash

WORK_DIR=$(pwd)
SRC_DIR=$WORK_DIR/src
HOOK_FILE=$WORK_DIR/recipe
CONDA_PACKAGE_PATH=$WORK_DIR/build/linux-aarch64

declare -a file_name

function check_build_file()
{
    if [ -z "$(ls -A ${SRC_DIR})" ];then
        echo "src package is empty exit!!!"
        exit
    fi

    if [ -z "$(ls -A ${HOOK_FILE})" ];then
        echo "build hook file is empty exit!!!"
        exit
    fi
}

function process_meta_file()
{
    file=$1
    package_name=$2
    build_name=$3
    version=$4
    sed -i "2s/name\ \= '.*'/name\ \= '${package_name}'/g" $file
    sed -i "4s/build\_name\ \= '.*'/build\_name\ \= '${build_name}'/g" $file
    sed -i "6s/version\ \= '.*'/version\ \= '${version}'/g" $file 
}

function process_pre_post_file()
{
    file=$1
    package_name=$2
    build_name=$3
    version=$4
    sed -i "3s/NAME=".*"/NAME=\"${build_name}\"/g" $file
    sed -i "4s/VERSION=".*"/VERSION=\"${version}\"/g" $file
}

###钩子脚本信息替换
function  deal_with_file()
{
    package_name=$1
    build_name=$2
    version=$3
    for file in $HOOK_FILE/*; do
        #echo "begin deal with $file  package_name:$package_name build_name:$build_name version:$version"
        if [[ "$file" == *"build.sh" ]]; then
            continue
        fi

        if [[ "$file" == *"meta.yaml" ]]; then
            process_meta_file  $file $package_name $build_name $version
        else
            process_pre_post_file  $file $package_name $build_name $version
        fi
    done
}

function conda_build()
{
    conda-build --croot ./build  ./recipe > /dev/null 2 >&1
    if [ $? != 0 ]; then
        return 1
    fi
    return 0
}

function move_conda_package()
{
    if [ ! -d  $WORK_DIR/conda_packages ];then
        mkdir -p $WORK_DIR/conda_packages
    fi

    for package in ${CONDA_PACKAGE_PATH}/*; do
        if [[ "$package" == *".conda" ]]; then
            mv $package  $WORK_DIR/conda_packages/
        fi
    done
}

function sync_conda_package_to_remote_server()
{
    for package in $WORK_DIR/conda_packages/*; do
        echo "$package"
        package_name=$(basename $package)
        echo "begin rsync $package_name"
        obsutil cp $package obs://ascend-artifcat-packages/ascend/cann/linux-aarch64/$package_name
        if [ $? != 0 ]; then
            obsutil cp $package obs://ascend-artifcat-packages/ascend/cann/linux-aarch64/$package_name
            if [ $? != 0 ]; then
                echo " rsync $package_name agagin failed !!!"
                return 1;
            fi 
        fi
        echo "rsync $package_name successful"
    done
    return 0
}

function clean_all()
{
    rm -rf $WORK_DIR/build/*
    rm -rf $WORK_DIR/conda_packages/
}

function get_run_package_name()
{
    for  file in $SRC_DIR/*;do
        if [ -f $file ];then
            name=$(basename $file)
            file_name+=("$name")
        fi
    done
}

function build_conda_package()
{
    for name in "${file_name[@]}"; do
        package_name=$(echo $name | awk -F "_" '{print $1}')
        package_name=$(echo $package_name | tr 'A-Z' 'a-z')
        #echo $package_name
        build_name=$(echo $package_name | cut -d'-' -f3- | cut -d '_' -f1)
        #echo $build_name
        version=$(echo $name | awk -F "_" '{print $2}')
        #echo $version
        deal_with_file $package_name $build_name $version
        echo "begin conda build ${package_name}_${version}"
        conda_build
        if [ $? == 0 ]; then
            echo "conda build ${package_name}_${version}.conda success"
        else
            echo "conda build ${package_name}_${version}.conda failed"
            exit 1
        fi
    done
}

function main()
{
    check_build_file
    get_run_package_name
    build_conda_package
    move_conda_package
    sync_conda_package_to_remote_server
    if [ $? == 0 ]; then
        echo "rsync all package success"
        clean_all
    fi 
}

main