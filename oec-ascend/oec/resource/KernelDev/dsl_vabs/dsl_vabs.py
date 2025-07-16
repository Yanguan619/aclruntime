from tbe import tvm
from tbe import dsl
import argparse
parser = argparse.ArgumentParser(
        prog="dsl-test",
    )
    
parser.add_argument(
    "output_dir",
    default=".",
    help="The path where the kernel mate data is saved, which is the current directory by default",
)
args = parser.parse_args()


shape = (28,28)
dtype = "float16"
# 定义输入占位符
data = tvm.placeholder(shape, name="data", dtype=dtype)
with tvm.target.cce():
    # 描述算子计算过程
    res = dsl.vabs(data)
    # 生成schedule对象
    sch = dsl.auto_schedule(res)
# 定义build配置参数
config = {"print_ir" : True,
        "need_build" : True,
        "name" : "abs_28_28_float16",
        "tensor_list" : [data,res],
        "kernel_meta_parent_dir": args.output_dir
    }
# build算子
dsl.build(sch, config)