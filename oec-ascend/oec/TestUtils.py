# encoding: utf-8
import os
import re
from oec import BaseTest,TestCase,State

def merge_env_variables(env_output, var_list):
    """
    解析env命令输出，提取指定环境变量并与当前环境合并
    
    参数:
        env_output (str): env命令输出的文本
        var_list (list): 需要提取的环境变量名称列表
        
    返回:
        dict: 合并后的环境变量字典，适用于subprocess模块
    """
    # 创建当前环境变量的副本
    merged_env = os.environ.copy()
    
    # 解析env命令的输出
    extracted_env = {}
    for line in env_output.strip().splitlines():
        # 跳过空行和不符合格式的行
        if '=' not in line:
            continue
            
        # 分割变量名和值（只分割第一个等号）
        parts = line.split('=', 1)
        var_name = parts[0]
        var_value = parts[1] if len(parts) > 1 else ''
        
        # 如果变量在目标列表中，则记录
        if var_name in var_list:
            extracted_env[var_name] = var_value
    
    # 合并到环境变量副本中
    merged_env.update(extracted_env)
    return merged_env

class SetEnvTestCase(TestCase):
    def execute_command(self):
        super().execute_command()
        if not self.is_passed():
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
        env = merge_env_variables(self.log,cann_envname)
        self.context.set_env(env)
        self.logger.debug(self.context.env)
        self.set_state(State.PASS)
        

class ResetEnvTestCase(BaseTest):
    def execute_command(self):
        self.context.env = os.environ.copy()
        self.set_state(State.PASS)
        
class NPUTestCase(TestCase):
    """
        1.从Context.infomation中获取和替换cmd中 <key> 包围的信息,其中key为需要获取和替换的键,注意左尖括号前需要有白字符
        2.支持根据NPU型号设置黑名单 NPUTestCase(...,black_list=[r"Ascend310P.*", r"Ascend310B.*"]), 禁用所有310P 310B系列
    """
    def __init__(self, black_list=None, *args, **kwargs):
        super().__init__(*args, **kwargs)
        if black_list is None:
            black_list = []
        if isinstance(black_list,str):
            black_list = [black_list]
        if not isinstance(black_list, list):
            raise TypeError("black_list type is not correct!")
        self._black_list = black_list
    
    @property
    def black_list(self):
        return self._black_list
    
    def replace_cmd_with_info(self, cmd):
        # 正则表达式匹配：空白字符 + <xxx>
        # \s 匹配任何空白字符，[\w]+ 匹配单词字符（字母、数字、下划线）
        pattern = re.compile(r'(\s+)<([^\n\r<>]+)>')
        
        def replacer(match):
            whitespace = match.group(1)  # 前面的空白字符
            key = match.group(2)        # xxx 部分
            return f'{whitespace}{self.context.infomation.get(key, f"<{key}>")}'  # 如果 key 不存在，保留原样
        
        new_cmd = pattern.sub(replacer, cmd)
        return new_cmd
    def check_npu_in_black_list(self):
        if not self.black_list:
            return False
        npu = self.context.infomation.get("NPU", "unknow")
        for name in self.black_list:
            if re.fullmatch(name, npu):
                return True
        return False
        
    def execute_command(self):
        if self.check_npu_in_black_list():
            self.set_state(State.UNSUPPORTED)
            self.set_reason(f"the npu {self.context.infomation.get('NPU', 'unkonw')} is unsupported in this case.")
            return
        cmd:str =  self.replace_cmd_with_info(self.get_cmd())
        self.execute_command_with_cmd(cmd)