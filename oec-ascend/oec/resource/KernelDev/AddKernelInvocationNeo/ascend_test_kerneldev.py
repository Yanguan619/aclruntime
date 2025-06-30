import oec

oec.NPUTestCase(
    group= ("算子","算子编译"),
    name = "KERNEL_BUILD_BISHENG_DEMO",
    cmd=f"bash run.sh -r npu -v <NPU> -i '$ASCEND_HOME_PATH' -o '{oec.Context.output_dir}/tmp/AddKernelInvocationNeo'"
    )