import re
from oec import TestCase,BaseTest,State
import platform
import distro

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
    
    def count(self):
        return 0 if self.is_auxiliary() else 1
    
    def get_test_content(self):
        return 'Get OS infomation from platform and distro pacakge'



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

        if matches:
            info['昇腾硬件'] = matches[0]

OSInfomationCase(
    group=("运行环境","操作系统"),
    name='READ_OS_INFOMATION')

HDKInfomationCase(
    group=("运行环境","驱动"),
    name='READ_DRIVER_INFOMATION',
    cmd = 'npu-smi info')