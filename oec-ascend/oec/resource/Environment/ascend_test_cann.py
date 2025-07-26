import oec

oec.TestCase(
    group=("运行环境","CANN安装卸载"),
    name="INSTALL_CANN",
    tags = ["combo_pacakge"],
    cmd=f"bash ./install_and_check_cann.sh '{oec.Context.work_path}' '{oec.Context.output_dir}/Ascend'"
)    