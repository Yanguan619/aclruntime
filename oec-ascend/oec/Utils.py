import os
from datetime import timedelta

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


def elapsed_time_str(delta:timedelta):

    hours = delta.seconds // 3600
    minutes = (delta.seconds // 60) % 60
    seconds = round(delta.seconds % 60 + delta.microseconds // 10000 * 0.01,1)
    x = [delta.days,hours,minutes,seconds]
    y = ['d','h','m','s']
    for i in range(len(x)):
        if x[i] > 0 or i == len(x) - 1:
            x = x[i:]
            y = y[i:]
            break
            
    result = ""
    for i in range(len(x)):
        result += f"{x[i]}{y[i]}"
    return result

def get_file_path(path: str):
    cwd = os.getcwd()
    relpath = os.path.relpath(path, cwd)
    return path if len(path) < len(relpath) else relpath