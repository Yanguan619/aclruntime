import oec

oec.TestCase(
    group=("运行环境","CANN安装卸载"),
    name="INSTALL_CANN",
    cmd=f"bash ./install_cann.sh {oec.Context.data_path}"
)    