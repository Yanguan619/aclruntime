#encoding: utf-8
import oec

oec.NPUTestCase(
    group= ("应用开发","算子加速库"),
    
    name = "ACLNN_ALLGATHERMATMUL",
    tags = [oec.app_dev, oec.aclnn],
    cmd=f"./run.sh {oec.Context.output_dir}/tmp/aclnn <Count>",
    exclude=[r"\berror\b", r"\bERROR\b", r"\bError\b"]
    )


