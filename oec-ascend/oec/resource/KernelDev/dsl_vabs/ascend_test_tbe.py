#encoding: utf-8
import oec

oec.TestCase(
    group= ("算子","算子编译"),
    name = "KERNEL_BUILD_DSL_VABS",
    cmd=f"python3 dsl_vabs.py {oec.Context.output_dir}/tmp/dsl_vabs"
    )