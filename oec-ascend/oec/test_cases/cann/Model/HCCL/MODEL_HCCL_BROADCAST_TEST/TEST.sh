unsupported=(A300)
for product in "${unsupported[@]}"; do
    if [[ "$product" == "$OEC_PRODUCT" ]]; then
        exit 192
    fi
done
device_count=$(python3 -c "
try:
    import acl
    count,ret = acl.rt.get_device_count()
    assert ret == 0
    print(count)
except:
    print(0)
")

# 执行命令并捕获所有输出
output=$(timeout -k 1s 60s python3 -m ais_bench -n $device_count broadcast_test -p $device_count -b 8K -e 64M -f 2 -d fp32 2>&1)
exit_code=$?

# 将输出打印到终端
echo "$output"

# 检查输出中是否包含[ERROR]
if echo "$output" | grep -q "check result failed"; then
    exit 1
fi

# 如果没有错误，返回原始命令的退出码
exit $exit_code