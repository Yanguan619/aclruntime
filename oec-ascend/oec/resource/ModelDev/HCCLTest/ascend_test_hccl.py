import oec

oec.TestCase(
    group= ("模型开发","集合通信"),
    name = "MODEL_HCCL_AIS_BENCH_CHECK",
    cmd=f"python3 check_package_version.py",
    exclude=None
    )

oec.TestCase(
    group= ("模型开发","集合通信"),
    name = "MODEL_HCCL_ALL_REDUCE_TEST",
    cmd=f"python3 -m ais_bench -n 8 all_reduce_test -p 8 -b 8K -e 64M -f 2 -d fp32 -o sum",
    exclude=None
    )

oec.TestCase(
    group= ("模型开发","集合通信"),
    name = "MODEL_HCCL_ALL_GATHER_TEST",
    cmd=f"python3 -m ais_bench -n 8 all_gather_test -p 8 -b 8K -e 64M -f 2 -d fp32",
    exclude=None
    )

oec.TestCase(
    group= ("模型开发","集合通信"),
    name = "MODEL_HCCL_BROADCAST_TEST",
    cmd=f"python3 -m ais_bench -n 8 broadcast_test -p 8 -b 8K -e 64M -f 2 -d fp32",
    exclude=None
    )

