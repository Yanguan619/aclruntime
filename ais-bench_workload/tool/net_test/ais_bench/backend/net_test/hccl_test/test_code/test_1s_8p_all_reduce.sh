#!/bin/bash
CUR_DIR=$(dirname $(readlink -f "$0"))

server_ip=$1
port=$2
exec_file="all_reduce_test"

cd $CUR_DIR/../bin
if [ -f $exec_file ];then
    echo "${exec_file} exist!"
else
    echo "${exec_file} not exist!"
    exit
fi

for i in {0..7}; do
    echo "rank: $i started"
    ./${exec_file} \
        --server_ip ${server_ip} \
        --server_port ${port} \
        --rank_size 8 \
        --rank_id $i \
        -p 8 \
        -b 8K \
        -e 64M \
        -f 2 \
        -d fp32 \
        -o sum &
done

wait
cd $CUR_DIR
echo "test_1s_8p_all_reduce finished"