#!/bin/bash

declare -i ret_ok=0
declare -i ret_failed=1
CUR_DIR=$(dirname $(readlink -f $0))
USED_PIP=$1

gen_sha256()
{
    whl_path=$1
    hash_path=$2
    hash_cmd="shasum -a 256 ${whl_path} > ${hash_path}"
    eval ${hash_cmd} || { echo "gen hash file of ${whl_path} failed!";return $ret_failed; }
    return $ret_ok
}

build_whl()
{
    whl_path=$1
    hash_path=$2
    if [ -f ${whl_path} ]; then
        rm -f ${whl_path}
    fi
    cmd="${USED_PIP} wheel ${CUR_DIR}/backend/ -v"
    eval $cmd || { echo "build aclruntime wheel failed!"; return $ret_failed; }
    gen_sha256 $whl_path $hash_path || { return $ret_failed; }
    return $ret_ok
}

main()
{
    pip_info=$($USED_PIP --version)
    arch_info=$(uname -i)
    if [[ "${pip_info}" =~ "3.7" ]]; then
        name_info="${CUR_DIR}/aclruntime-0.0.2-cp37-cp37m-linux_${arch_info}"
        build_whl "${name_info}.whl" "${name_info}.sha256" || { return $ret_failed; }
    elif [[ "${pip_info}" =~ "3.8" ]]; then
        name_info="${CUR_DIR}/aclruntime-0.0.2-cp38-cp38-linux_${arch_info}"
        build_whl "${name_info}.whl" "${name_info}.sha256" || { return $ret_failed; }
    elif [[ "${pip_info}" =~ "3.9" ]]; then
        name_info="${CUR_DIR}/aclruntime-0.0.2-cp39-cp39-linux_${arch_info}"
        build_whl "${name_info}.whl" "${name_info}.sha256" || { return $ret_failed; }
    elif [[ "${pip_info}" =~ "3.10" ]]; then
        name_info="${CUR_DIR}/aclruntime-0.0.2-cp310-cp310-linux_${arch_info}"
        build_whl "${name_info}.whl" "${name_info}.sha256" || { return $ret_failed; }
    fi
    return $ret_ok
}

main "$@"
exit $?