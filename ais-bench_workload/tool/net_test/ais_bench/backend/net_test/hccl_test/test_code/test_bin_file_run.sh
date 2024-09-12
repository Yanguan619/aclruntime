#!/bin/bash
declare -i ret_ok=0
declare -i ret_failed=1

CUR_DIR=$(dirname $(readlink -f "$0"))
server_ip=$1
port=$2

error_handler() {
    echo "\033[31mFAILED\033[0m test_case failed! $1"
    exit $ret_failed
}

function parallel_run()
{
    exec_file=$1
    device_count=$2

    cd $CUR_DIR/../bin
    if [ ! -f $exec_file ];then
        echo "${exec_file} not exist!"
        cd $CUR_DIR
        return $ret_failed
    fi
    test_case="op task:${exec_file}, device_count:${device_count} start running ..."
    echo "[test case] ${test_case}"
    for ((i=0; i<device_count; i++)); do
        set -e
        trap 'error_handler ${test_case}' ERR
        ./${exec_file} \
            --server_ip ${server_ip} \
            --server_port ${port} \
            --rank_size $device_count \
            --rank_id $i \
            -p $device_count \
            -b 8K \
            -e 8M \
            -f 2 \
            -d fp32 &
    done
    wait
    cd $CUR_DIR
    echo "\033[31mPASS\033[0m"
}

function test_different_device_count_run()
{
    device_count_list=(1 2 4 8)
    for count in ${device_count_list[@]}; do
        parallel_run "all_reduce_test" $count
    done
}

function test_different_op_task_run()
{
    op_task_list=("all_gather_test" "all_reduce_test" "alltoall_test" "alltoallv_test" \
        "broadcast_test" "reduce_scatter_test" "reduce_test")

    for op_task in ${op_task_list[@]}; do
        parallel_run $op_task 2
    done
}

main()
{
    test_different_device_count_run
    test_different_op_task_run
    return $ret_ok
}

main "$@"
exit $?


