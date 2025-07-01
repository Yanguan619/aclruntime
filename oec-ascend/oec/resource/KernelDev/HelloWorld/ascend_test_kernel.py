import oec

oec.NPUTestCase(
    group= ("算子","算子开发"),
    name = "KERNEL_DEV_HELLO_WORLD",
    cmd=f"bash run.sh <NPU> {oec.Context.output_dir}/tmp/KernelDev dev"
    )

oec.TestCase(
    group= ("算子","算子编译"),
    name = "KERNEL_BUILD_HELLO_WORLD",
    cmd=f"bash run.sh Ascend910B3 {oec.Context.output_dir}/tmp/KernelBuild build"
    )