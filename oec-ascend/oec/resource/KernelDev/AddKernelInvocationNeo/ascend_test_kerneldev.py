import oec

oec.NPUTestCase(
    group= ("算子","算子开发"),
    name = "KERNEL_DEV_ADD_NPU",
    tags = [oec.kernel_dev],
    cmd=f"bash run.sh -r npu -v <NPU> -o '{oec.Context.output_dir}/tmp/AddKernelInvocationNeo_npu'",
    include="test pass",
    exclude=[r"\bfailed\b", r"\bFailed\b", r"\bFAILED\b",
             r"\bERROR\b", r"\bError\b"]
    )

oec.NPUTestCase(
    group= ("算子","算子开发"),
    name = "KERNEL_DEV_ADD_SIM",
    tags = [oec.kernel_dev],
    cmd=f"bash run.sh -r sim -v <NPU> -o '{oec.Context.output_dir}/tmp/AddKernelInvocationNeo_sim'",
    include="test pass",
    exclude=[r"\bfailed\b", r"\bFailed\b", r"\bFAILED\b",
             r"\bERROR\b", r"\bError\b"]
    )

oec.TestCase(
    group= ("算子","算子开发"),
    name = "KERNEL_DEV_ADD_CPU",
    tags = [oec.kernel_dev],
    cmd=f"bash run.sh -r cpu -v Ascend910B3 -o '{oec.Context.output_dir}/tmp/AddKernelInvocationNeo_cpu'",
    include="test pass",
    exclude=[r"\bfailed\b", r"\bFailed\b", r"\bFAILED\b",
             r"\bERROR\b", r"\bError\b"]
    )