import oec

oec.TestCase(
    group= ("算子","算子编译"),
    name = "KERNEL_BUILD_BISHENG_DEMO",
    tags = [oec.kernel_dev, oec.bisheng],
    cmd=f"bash build.sh Ascend910B3 {oec.Context.output_dir}/tmp/KernelDev/bisheng"
    )