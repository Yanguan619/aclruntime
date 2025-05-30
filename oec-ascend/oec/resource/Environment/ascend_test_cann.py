import oec

oec.TestCase(
    group=("运行环境","CANN安装"),
    name="INSTALL_BACKUP_CANN_INSTALL_INFO",
    cmd=f"[ -f /etc/Ascend/ascend_cann_install.info ] && mv -v /etc/Ascend/ascend_cann_install.info /etc/Ascend/ascend_cann_install.info.backup",
    optional=True
)

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

oec.TestCase(
    group=("运行环境","CANN卸载"),
    name="UNINSTALL_CANN_NNAL",
    cmd=f"bash ./uninstall_cann.sh {oec.Context.cann_path} Ascend-cann-nnal"
)

oec.TestCase(
    group=("运行环境","CANN卸载"),
    name="UNINSTALL_CANN_KERNELS",
    cmd=f"bash ./uninstall_cann.sh {oec.Context.cann_path} Ascend-cann-kernels"
)

oec.TestCase(
    group=("运行环境","CANN卸载"),
    name="UNINSTALL_CANN_TOOLKIT",
    cmd=f"bash ./uninstall_cann.sh {oec.Context.cann_path} Ascend-cann-toolkit"
)

oec.TestCase(
    group=("运行环境","CANN卸载"),
    name="UNINSTALL_RESTORE_CANN_INSTALL_INFO",
    cmd=f"[ -f /etc/Ascend/ascend_cann_install.info.backup ] && mv -v /etc/Ascend/ascend_cann_install.info.backup /etc/Ascend/ascend_cann_install.info",
    optional=True
)