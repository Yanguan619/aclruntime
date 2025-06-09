# encoding: utf-8
import os
from oec import TestCase,State

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