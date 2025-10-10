set -e
output_path="$OEC_OUTPUT_PATH"
device_num=$(python3 -c "
try:
    import acl
    count,ret = acl.rt.get_device_count()
    assert ret == 0
    print(count)
except:
    print('unknow')
")
echo device_num = $device_num 
src_path=$(pwd)
mkdir -p "$output_path" 
cd "${output_path}"
#device_num 是环境上npu的数量，当前测试在A2上需要为2,4,8时才能运行成功
cmake "$src_path" -DCMAKE_CXX_COMPILER=g++ -DCMAKE_SKIP_RPATH=TRUE -DDEV_NUM=${device_num}
make

cd bin
./testcase
exit $?