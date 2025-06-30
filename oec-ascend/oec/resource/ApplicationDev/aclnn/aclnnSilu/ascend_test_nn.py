#encoding: utf-8
import oec

oec.TestCase(
    group= ("应用开发","算子加速库"),
    
    name = "ACLNN_SILU",
    cmd=f"./run.sh {oec.Context.output_dir}/tmp",
    expect=0,
    exclude=['failed','Failed','FAILED','error','ERROR','Error']
    )


