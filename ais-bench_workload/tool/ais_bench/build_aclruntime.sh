#!/bin/bash

declare -i ret_ok=0
declare -i ret_failed=1
CUR_DIR=$(dirname $(readlink -f $0))
USED_PIP=$1
WHL_NAME="ais_bench-0.0.2-py3-none-any"
WHL_PATH="${CUR_DIR}/${WHL_NAME}.whl"
HASH_FILE_PATH="${CUR_DIR}/${WHL_NAME}.sha256"

gen_sha256()
{
    whl_path=$1
    hash_path=$2
    hash_cmd="shasum -a 256 ${whl_path} > ${hash_path}"
    eval ${hash_cmd} || { echo "gen hash file of ${WHL_PATH} failed!";return $ret_failed; }
    return $ret_ok
}

build_whl()
{
    whl_path=$1
    hash_path=$2
    if [ -f ${WHL_PATH} ]; then
        rm -f ${WHL_PATH}
    fi
    cmd="${USED_PIP} wheel ${CUR_DIR}/backend/ -v"
    eval $cmd || { echo "build aclruntime wheel failed!"; return $ret_failed; }
    gen_sha256 || { return $ret_failed; }
    return $ret_ok

}

main()
{

}

main "$@"
exit $?