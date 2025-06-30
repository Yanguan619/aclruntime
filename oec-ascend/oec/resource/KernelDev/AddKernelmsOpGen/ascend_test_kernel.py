import oec
oec.TestCase(
    group= ("算子","算子开发"),
    name = "KERNEL_DEV_ADD_MSOPGEN",
    cmd=f"bash run.sh -r cpu -v Ascend910B3 -o '{oec.Context.output_dir}/tmp/AddKernelmsOpGen'",
    exclude=None,
    include="test pass"
    )