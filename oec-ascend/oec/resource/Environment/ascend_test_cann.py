import oec

oec.TestCase(
    group=("运行环境","CANN安装卸载"),
    name="INSTALL_CANN",
    cmd=f"bash ./install_and_check_cann.sh '{oec.Context.data_path}' '{oec.Context.output_dir}/Ascend'"
)    