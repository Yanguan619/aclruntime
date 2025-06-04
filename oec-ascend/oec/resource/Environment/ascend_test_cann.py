import oec
import os
oec.TestCase(
    group=("运行环境","CANN安装"),
    name="INSTALL_BACKUP_CANN_INSTALL_INFO",
    cmd=f"[ -f /etc/Ascend/ascend_cann_install.info ] && mv -v /etc/Ascend/ascend_cann_install.info /etc/Ascend/ascend_cann_install.info.backup",
    optional=True,
    expect=[0,1]
)


class SetEnvTestCase(oec.TestCase):
    def check_result(self, log, return_code):
        super(SetEnvTestCase,self).check_result(log, return_code)
        if self.is_failed():
            return
        cann_envname = [
            'ASCEND_TOOLKIT_HOME',
            'ASCEND_HOME_PATH',
            'ASCEND_AICPU_PATH',
            'ASCEND_OPP_PATH',
            'TOOLCHAIN_HOME',
            'LD_LIBRARY_PATH',
            'PYTHONPATH',
            'PATH',
            ]
        env = oec.merge_env_variables(log,cann_envname)
        self.context.set_env(env)
        self.logger.debug(self.context.env)
        self.set_state(oec.State.PASS)


oec.TestCase(
    group=("运行环境","CANN安装"),
    name="INSTALL_CANN_TOOLKIT",
    cmd=f"bash ./install_cann.sh {oec.Context.cann_path} Ascend-cann-toolkit"
)    

SetEnvTestCase(
    group=("运行环境","CANN安装"),
    name="INSTALL_CANN_SET_ENV",
    cmd=f"bash -c 'source {oec.Context.cann_path}/Ascend/ascend-toolkit/set_env.sh && env'"
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
    optional=True,
    expect=[0,1]
)