#!/bin/bash
declare -i ret_ok=0
declare -i ret_failed=1

CUR_DIR=$(dirname $(readlink -f "$0"))
server_ip=$1
port=$2

function parallel_run()
{
    exec_file=$1
    device_count=$2

    cd $CUR_DIR/../bin
    if [ -f $exec_file ];then
        echo "${exec_file} exist!"
    else
        echo "${exec_file} not exist!"
        cd $CUR_DIR
        return $ret_failed
    fi
    rank_id_max=$(($device_count - 1))

    for i in {0..$rank_id_max}; do
        echo "rank: $i started"
        ./${exec_file} \
            --server_ip ${server_ip} \
            --server_port ${port} \
            --rank_size $device_count \
            --rank_id $i \
            -p $device_count \
            -b 8K \
            -e 16M \
            -f 2 \
            -d fp32 \
            -o sum &
    done
    wait
    cd $CUR_DIR
}

function test_different_device_count_run()
{
    parallel_run "all_reduce_test" 8
}

function test_different_op_task_run()
{
    echo "test_1s_8p_all_reduce finished"
}


main()
{
    test_different_device_count_run
    test_different_op_task_run
    return $ret_ok
}

main "$@"
exit $?


