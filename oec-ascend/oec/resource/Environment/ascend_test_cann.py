import oec

oec.TestCase(
    group=("运行环境","CANN安装"),
    name="INSTALL_CANN_TOOLKIT",
    cmd=f"bash ./install_cann.sh {oec.Context.cann_path} Ascend-cann-toolkit"
)

oec.TestCase(
    group=("运行环境","CANN安装"),
    name="INSTALL_CANN_KERNELS",
    cmd=f"bash ./install_cann.sh {oec.Context.cann_path} Ascend-cann-kernels"
)

oec.TestCase(
    group=("运行环境","CANN安装"),
    name="INSTALL_CANN_NNAL",
    cmd=f"bash ./install_cann.sh {oec.Context.cann_path} Ascend-cann-nnal"
)