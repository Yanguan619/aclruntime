import re
import platform
import distro
import oec
from oec import TestCase,BaseTest,State,SetEnvTestCase



class OSInfomationCase(BaseTest):
    def get_os_version(self):
        system = platform.system().lower()
        
        # Windows 系统
        if system == "windows":
            try:
                import winreg
                with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, r"SOFTWARE\Microsoft\Windows NT\CurrentVersion") as key:
                    product_name = winreg.QueryValueEx(key, "ProductName")[0]
                    display_version = winreg.QueryValueEx(key, "DisplayVersion")[0]
                    return f"{product_name} ({display_version})"
            except:
                return platform.version()

        # macOS 系统
        elif system == "darwin":
            try:
                mac_version = platform.mac_ver()[0]
                return f"macOS {mac_version}"
            except:
                return "macOS (version unknown)"

        # Linux 系统
        elif system == "linux":
            # 尝试通过 distro 库获取（推荐）

            return f"{distro.name(pretty=True)} {distro.version(pretty=True)}"

        # 其他系统
        else:
            return platform.platform()
    
    def execute_command(self):
        self.set_state(State.RUNNING)
        info = self.context.infomation
        info['OS Version'] = self.get_os_version()
        info['架构'] = platform.machine()
        self.set_state(State.PASS)
    
    def get_test_content(self):
        return 'Get OS infomation from platform and distro package'



class HDKInfomationCase(TestCase):
        
    def check_result(self, log, return_code):
        super(HDKInfomationCase,self).check_result(log, return_code)
        if self.is_failed():
            return
        info = self.context.infomation
        rst = re.search(r"Version:\s+(\S+)\s",log)
        if rst:
            
            info['Ascend HDK Version'] = rst.group(1)
        matches = re.findall(r'\|\s+\d+\s+(\S+)\s+\|', log)
        matches2 = re.findall(r'\|\s+\w{4}:\w{2}:\w{2}.\w\s+\|', log)
        if matches:
            info['昇腾硬件'] = f"{matches[0]} × {len(matches2)}"

class CANNNPUInfomationCase(TestCase):
    
    def check_result(self, log, return_code):
        super(CANNNPUInfomationCase,self).check_result(log, return_code)
        if self.is_failed():
            return
        if log == "":
            self.set_state(State.FAIL)
            return
        info = log.split('\n')
        if info is None or len(info) != 2:
            self.set_state(State.FAIL)
            return
        npu,count = tuple(info)
        self.logger.debug(f"NPU:{npu}, Count:{count}")
        self.context.infomation['NPU'] = npu
        self.context.infomation['Count'] = count
        self.set_state(State.PASS)

class CANNVersionInfomationCase(TestCase):
    
    def check_result(self, log, return_code):
        super(CANNVersionInfomationCase,self).check_result(log, return_code)
        if self.is_failed():
            return
        if log == "":
            self.set_state(State.FAIL)
            return
        
        self.logger.debug(f"CANN Version = {log}")
        self.context.infomation['CANN Version'] = log
        self.set_state(State.PASS)

OSInfomationCase(
    group=("运行环境","环境信息"),
    name='READ_OS_INFOMATION')

HDKInfomationCase(
    group=("运行环境","环境信息"),
    name='READ_DRIVER_INFOMATION',
    cmd = 'npu-smi info')

SetEnvTestCase(
    group=("运行环境","CANN信息"),
    name="READ_CANN_SET_ENV",
    cmd=f"bash -c 'source {oec.Context.cann_path}/ascend-toolkit/set_env.sh && env'"
)

CANNVersionInfomationCase(
    group=("运行环境","CANN信息"),
    name='READ_CANN_VERSION_INFOMATION',
    cmd = 'python3 get_cann_version.py'
)

CANNNPUInfomationCase(
    group=("运行环境","CANN信息"),
    name='READ_CANN_NPU_INFOMATION',
    cmd = 'python3 get_npu_info.py'
)