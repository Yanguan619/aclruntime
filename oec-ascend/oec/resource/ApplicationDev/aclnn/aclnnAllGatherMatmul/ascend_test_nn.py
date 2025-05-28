#encoding: utf-8
import oec

oec.TestCase(
    group= ("应用开发","基础功能"),
    
    name = "ACLNN_ALLGATHERMATMUL",
    cmd = "./run.sh",
    optional=True,
    expect=0,
    exclude=['failed','Failed','FAILED','error','ERROR','Error']
    )


