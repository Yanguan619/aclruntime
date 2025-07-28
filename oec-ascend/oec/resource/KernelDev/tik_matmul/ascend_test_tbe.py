#encoding: utf-8
import oec

oec.TestCase(
    group= ("算子","算子编译"),
    name = "KERNEL_BUILD_TIK_MATMUL",
    tags = [oec.kernel_dev],
    cmd=f"python3 tik_matmul.py {oec.Context.output_dir}/tmp/tik_matmul"
    )