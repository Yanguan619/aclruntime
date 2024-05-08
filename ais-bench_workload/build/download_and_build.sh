##!/bin/bash

CURDIR=$(dirname $(readlink -f $0))

function check_command_exist()
{
    command=$1
    if type $command >/dev/null 2>&1; then
        return 0
    else
        return 1
    fi
}

check_file_valid()
{
    if [ ! -f "$1" ]; then
        return 1
    fi
    return 0
}

function try_download_stubs_packet(){
    #mkdir -p $INFER_BASE_PATH/opensource/gflags/src/
    #cmd="wget $1 --no-check-certificate -O $2"
    cmd="curl -k -o $2 $1"
    timeout 60 $cmd #>/dev/null 2>&1
    ret=$?
    if [ "$ret" == 0 -a -s "$2" ];then
        echo "download cmd:$cmd targetfile:$2 OK"
    else
        echo "downlaod targetfile by $cmd Failed please check network or manual download to target file"
        return 1
    fi
}

# unrar_files()
# {
# 	local target_file_=$1
# 	local tmp_dir_=$2
# 	check_command_exist start && { start winrar e $target_file_ $tmp_dir_;return 0; }
# 	check_command_exist unrar && { unrar e $target_file_ $tmp_dir_ ;return 0; }
# 	return 1;
# }

main()
{
    local version="$1"
    local type="$2"
    [ "$2" != "modelarts" ] && type=""

    stubs_packet_x86_64_url="https://www.aisbench.com/assets/public/article_attachment/1/file/93fa30f8-ffac-4b99-9da6-78caa62d3df7.gz"
    stubs_bak_x86_64_url="https://aisbenchtest.obs.cn-north-4.myhuaweicloud.com/stubs_package/Ais-Benchmark-Stubs-x86_64-1.0.tar.gz" # obs link
    stubs_x86_64_pkg_name="Ais-Benchmark-Stubs-x86_64-1.0.tar.gz"
    stubs_packet_aarch64_url="https://www.aisbench.com/assets/public/article_attachment/1/file/0796bd4e-136d-45fe-b863-24661679a283.gz"
    stubs_bak_aarch64_url="https://aisbenchtest.obs.cn-north-4.myhuaweicloud.com/stubs_package/Ais-Benchmark-Stubs-aarch64-1.0.tar.gz" # obs link
    stubs_aarch64_pkg_name="Ais-Benchmark-Stubs-aarch64-1.0.tar.gz"

    tmp_dir="$CURDIR/buildtmpstubs"
    [ -d $tmp_dir ] && rm -rf $tmp_dir
    mkdir -p $tmp_dir

    check_command_exist git || { echo "git cmd not valid"; return -1; }
    check_command_exist tar || { echo "tar cmd not valid"; return -1; }

    target_file="$tmp_dir/${stubs_x86_64_pkg_name}"
    try_download_stubs_packet $stubs_packet_x86_64_url $target_file || {
        echo "donwload x86_64 stubs failed, using bak_url ${stubs_bak_x86_64_url}";
        try_download_stubs_packet $stubs_bak_x86_64_url $target_file || {  echo "donwload x86_64 stubs failed again!";return 1; }
    }
    target_file="$tmp_dir/${stubs_aarch64_pkg_name}"
    try_download_stubs_packet $stubs_packet_aarch64_url $target_file || {
        echo "donwload aarch64 stubs failed, using bak_url ${stubs_bak_aarch64_url}";
        try_download_stubs_packet $stubs_bak_aarch64_url $target_file || {  echo "donwload aarch64 stubs failed again!";return 1; }
    }

    sleep 5
    x86_stubs="$tmp_dir/Ais-Benchmark-Stubs-x86_64-1.0.tar.gz"
    check_file_valid "$x86_stubs" || { echo "x86_stubs:${x86_stubs} not valid path" ; return 1; }

    arm_stubs="$tmp_dir/Ais-Benchmark-Stubs-aarch64-1.0.tar.gz"
    check_file_valid "$arm_stubs" || { echo "arm_stubs:${arm_stubs} not valid path" ; return 1; }

    bash -x $CURDIR/build.sh  $x86_stubs train huawei train_mindspore_resnet $version $type
    bash -x $CURDIR/build.sh  $x86_stubs train huawei train_mindspore_bert $version $type
    bash -x $CURDIR/build.sh  $arm_stubs train huawei train_mindspore_resnet $version $type
    bash -x $CURDIR/build.sh  $arm_stubs train huawei train_mindspore_bert $version $type
}

main "$@"
exit $?
