import oec

oec.NPUTestCase(
    group= ("算子","算子开发"),
    name = "KERNEL_DEV_ADD_NPU",
    cmd=f"bash run.sh -r npu -v <NPU> -o '{oec.Context.output_dir}/tmp/AddKernelInvocationNeo'"
    )

oec.NPUTestCase(
    group= ("算子","算子开发"),
    name = "KERNEL_BUILD_ADD_SIM",
    cmd=f"bash run.sh -r sim -v <NPU> -o '{oec.Context.output_dir}/tmp/AddKernelInvocationNeo'"
    )

oec.TestCase(
    group= ("算子","算子开发"),
    name = "KERNEL_BUILD_ADD_CPU",
    cmd=f"bash run.sh -r cpu -v Ascend910B3 -o '{oec.Context.output_dir}/tmp/AddKernelInvocationNeo'"
    )