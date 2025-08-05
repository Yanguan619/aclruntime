
device_count=$(python3 -c "
try:
    import acl
    count,ret = acl.rt.get_device_count()
    assert ret == 0
    print(count)
except:
    print(0)
")
python3 -m ais_bench -n $device_count all_reduce_test -p $device_count -b 8K -e 64M -f 2 -d fp32 -o sum