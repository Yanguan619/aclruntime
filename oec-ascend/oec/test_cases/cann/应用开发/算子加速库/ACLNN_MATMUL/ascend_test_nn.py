#encoding: utf-8
import oec

oec.TestCase(
    group= ("应用开发","算子加速库"),
    
    name = "ACLNN_MATMUL",
    products = oec.EXCLUDE_A200_A500,
    tags = [oec.app_dev, oec.aclnn],
    cmd=f"./run.sh {oec.Context.output_dir}/tmp/aclnn"
    )


