import re
import platform
import distro
import oec
from oec import TestCase,BaseTest,State



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
        info.setdefault('NPU', "unknow")
        info.setdefault('Count', 0)
        if matches:
            info['NPU'] = matches[0]
        if matches2:
            info['Count'] = len(matches2)  
        
        if info['Count'] > 1:
            info["昇腾硬件"] = f"{info['NPU']} × {info['Count'] }"
        else:
            info["昇腾硬件"] = f"{info['NPU']}"
        
        self.logger.debug(
            f"HDK NPU:{info['NPU']}, Count:{info['Count']}")

class CANNNPUInfomationCase(TestCase):
    
    def check_result(self, log, return_code):
        super(CANNNPUInfomationCase,self).check_result(log, return_code)
        if self.is_failed():
            return
        if log == "":
            self.set_state(State.FAIL)
            return
        npu_count = log.split('\n')
        if npu_count is None or len(npu_count) != 2:
            self.set_state(State.FAIL)
            return
        npu,count = tuple(npu_count)
        self.logger.debug(f"NPU:{npu}, Count:{count}")
        info = self.context.infomation
        info['NPU'] = npu
        info['Count'] = int(count)
        if info['Count'] > 1:
            info["昇腾硬件"] = f'{npu} × {count}'
        else:
            info["昇腾硬件"] = f'{npu}'
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
