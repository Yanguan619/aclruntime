import oec
oec.TestCase(
    group= ("算子","算子开发"),
    name = "KERNEL_DEV_ADD_MSOPGEN",
    cmd=f"bash run.sh '{oec.Context.output_dir}/tmp/AddKernelmsOpGen'",
    exclude=None,
    include="successfully created"
    )