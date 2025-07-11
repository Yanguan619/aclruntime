#encoding: utf-8
import oec

oec.NPUTestCase(
    group= ("应用开发","算子加速库"),
    
    name = "ACLNN_ALLGATHERMATMUL",
    cmd=f"./run.sh {oec.Context.output_dir}/tmp/aclnn <Count>"
    )


