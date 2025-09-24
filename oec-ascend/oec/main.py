#!python
# encoding: utf-8
import argparse
import logging
import os
import random
import sys
import time
import threading
from datetime import datetime
from importlib import import_module
from logging import getLogger
import shutil
from oec.BaseTestCase import TestCase
from oec.TestContext import TestContext
import oec.BaseTest as BaseTestModule

from oec.common.EnvTestCase import SetEnvTestCase
from oec.TestReport import gen_report
import oec.common.env_test as env
from oec.Utils import check_disk_space, check_memory
logger = getLogger("oec-ascend")


def init_logger(level=logging.INFO):
    class ErrorFilter(logging.Filter):
        def filter(self, record):
            return record.levelno < logging.ERROR

    logger.setLevel(logging.DEBUG)
    stdout = logging.StreamHandler(sys.stdout)
    stdout.setLevel(level)
    stdout.addFilter(ErrorFilter())
    logger.addHandler(stdout)

    stderr = logging.StreamHandler(sys.stderr)
    stderr.setFormatter(
        logging.Formatter("[%(levelname)s][%(pathname)s:%(lineno)d] %(message)s")
    )
    stderr.setLevel(logging.ERROR)
    logger.addHandler(stderr)

def get_targets(resource_root):  
    for _, dirs,_ in os.walk(resource_root,topdown=True):
        return dirs

def argparse_handler(targets):
    parser = argparse.ArgumentParser(
        prog="oec-ascend",
        description="Ascend Operating System Compatibility Verification Tool",
    )
    parser.add_argument(
        "-p",
        "--product",
        required=True,
        choices=['A2', 'A3', 'A5', 'A300'],
    )

    parser.add_argument(
        "-t",
        "--target",
        required=True,
        choices=targets,
        help="offering of testcase.",
    )
    
    args = parser.parse_args()
    return args

def read_dirname_map(path:str):
    if not os.path.exists(path):
        logger.fatal(f"{path} was not found")
        exit(500)
    dirname_map = {}
    with open(path) as f:
        lines = f.readlines()
        for idx, line in enumerate(lines):
            strs = line.split()
            if len(strs) != 2:
                logger.fatal(f"sSyntax error in file {path}, line {idx}")
                exit(510)
            dirname_map[strs[0]] = strs[1]
    return dirname_map
        
def find_ascend_test_in_dir(path: str):
    logger.info(f"test case director is '{path}' loading...")
    sys.path.append(path)
    level = len(path.split(os.path.sep))
    # group_dict = Context.group_dict
    offering = os.path.basename(path)
    dirname_map = read_dirname_map(f"{path}/map.config")
    for prefix,dirs,files in os.walk(path,topdown=True):
        dirs.sort()
        logger.debug(prefix)
        parents = prefix.split(os.path.sep)
        if len(parents) - level == 2:
            level1_group,level2_group =  parents[-2],parents[-1]
            # group_dict[(level1_group,level2_group)] = False
        if len(parents) - level != 3:
            continue
        dirs.clear()
        level1_group,level2_group,testcase_name = parents[-3],parents[-2],parents[-1]
        group1_name = dirname_map.get(level1_group)
        group2_name = dirname_map.get(f"{level1_group}/{level2_group}")
        if group1_name is None or group2_name is None:
            logger.error(f"{level1_group} -> {group1_name}, {level1_group}/{level2_group} -> {group2_name}")
            continue
        test_files = []
        for name in files:
            if name[-3:] != ".sh":
                continue
            if name == "TEST.sh":
                test_files.append(name)
            if name[:len("TEST_")] == "TEST_":
                test_files.append(name)
        if len(test_files) == 0:
            logger.error(f"Test Cases was not found in the director {prefix}")
            continue
        for name in test_files:
            postfix = name[len("TEST_"):len(name)-len(".sh")] if name[:len("TEST_")] == "TEST_" else ""
            TestCase(
                offering=offering,
                group=(group1_name,group2_name),
                name = f"{testcase_name}{'_' if postfix else ''}{postfix}",
                cmd=["bash", name],
                origin_file=f"{prefix}/{name}",
                cwd=prefix,
                timeout=3600 #默认超时时间为1小时
                )


def get_absolute_out_path(output):
    output_path = os.path.abspath(output)

    return output_path


class HideCursor:
    def __init__(self):
        self.state = False

    def hide(self):
        self.state = True
        print("\033[?25l",end="",flush=True)

    def __del__(self):
        if not self.state:
            return
        print("\033[?25h",end="",flush=True)

hider = HideCursor()
def print_state(context: TestContext):
    hider.hide() #隐藏光标显示
    last_lines_len = 0
    def update_state():
        nonlocal last_lines_len
        state = context.get_state_distribution_str()
        lines = state.split('\n')
        lines_len = 0
        logger.info(f"\033[{last_lines_len + 1}A")
        for v in lines:
            terminal_colums, terminal_lines= os.get_terminal_size()
            for l in range(0, len(v), terminal_colums):
                logger.info(f"{v[l:l + terminal_colums]}\033[K")
                lines_len += 1
                
        for _ in range(lines_len, last_lines_len):
            logger.info(f"\033[K")
        delta_lines = last_lines_len -lines_len
        if delta_lines > 0:
            logger.info(f"\033[{delta_lines + 1}A")
        last_lines_len = lines_len

    while not context.finished:
        update_state()
        time.sleep(0.125)
    update_state()
    
def enable_ansi_windows():
    """在 Windows 上启用 ANSI 转义序列支持"""
    if sys.platform == "win32":
        import ctypes
        kernel32 = ctypes.windll.kernel32
        kernel32.SetConsoleMode(kernel32.GetStdHandle(-11), 7)  # 启用 VT100 模式
        
def init_env_test_case(offering):
    env.OSInfomationCase(
        offering=offering,
        group=("运行环境","环境信息"),
        name='READ_OS_INFOMATION',
        )
    

    env.HDKInfomationCase(
        offering=offering,
        group=("运行环境","环境信息"),
        name='READ_DRIVER_INFOMATION',
        cmd = ['npu-smi', 'info'],
        cwd = f"{os.path.dirname(__file__)}/common",
        with_case_info=False
        )

    SetEnvTestCase(
        offering=offering,
        group=("运行环境","CANN信息"),
        name="READ_CANN_SET_ENV",
        cmd=['bash', '-c',f"source {BaseTestModule.Context.cann_path}/ascend-toolkit/set_env.sh && env"],
        exclude=None,
        cwd = f"{os.path.dirname(__file__)}/common",
        with_case_info=False
    )

    env.CANNVersionInfomationCase(
        offering=offering,
        group=("运行环境","CANN信息"),
        name='READ_CANN_VERSION_INFOMATION',
        cmd = ['python3', 'get_cann_version.py'],
        cwd=f"{os.path.dirname(__file__)}/common",
        with_case_info=False
    )

    env.CANNNPUInfomationCase(
        offering=offering,
        group=("运行环境","CANN信息"),
        name='READ_CANN_NPU_INFOMATION',
        cmd = ['python3', 'get_npu_info.py'],
        cwd = f"{os.path.dirname(__file__)}/common",
        with_case_info=False
    )


def get_confirmation(prompt):
    """
    Boolean return version
    - Returns True for y/yes
    - Returns False for n/no
    - Prompts for re-entry for other inputs
    """
    logger.warning(f"{prompt} Do you want to continue? [yes/no]")
    while True:
        response = input().strip().lower()
        if response in ('y', 'yes'):
            return
        elif response in ('n', 'no'):
            exit(550)
        else:
            logger.warning("\033[33mInvalid input. Do you want to continue? [yes/no]\033[0m")
            
            


def run_target_test(resource_root, cmd_args, target, verbose, timestamp):
    # 检查剩余系统资源
    # 剩余资源阈值
    disk_space = 100#GB 
    memory_space = 96#GB
    if not check_disk_space(disk_space):
        get_confirmation(f"The available disk space of the current directory is less than {disk_space}GB, which may cause exceptions.")
    if not check_memory(memory_space):
        get_confirmation(f"The currently available running memory is less than {memory_space}GB, which may cause exceptions.")
    # 重置上下文
    Context = BaseTestModule.reset_context()
    
    product = cmd_args.product
    output_dir = "./output"
    data_dir = os.path.dirname(__file__) + "/data"
    cann_dir = "/usr/local/Ascend"
    work_dir = os.path.realpath("./")
    
    # 如果source了环境变量则提取组合包安装路径
    ascend_home_path = os.environ.get('ASCEND_HOME_PATH')
    if ascend_home_path is not None:
        cann_dir = os.path.realpath(f"{ascend_home_path}/../..")
        logger.info(f"Ascend install path is {cann_dir}")
    
    output = os.path.abspath(output_dir)
    data_path = os.path.realpath(data_dir)
    if not os.path.exists(data_path):
        logger.fatal(f"{data_path} is not existing, please create it first!")
        exit(1000)
    cann_path = os.path.realpath(cann_dir)
    if not os.path.exists(cann_path):
        logger.fatal(f"{cann_path} is not existing, please install CANN first!")
        exit(2000)
    Context.set_product(product)
    Context.set_target(target)
    Context.set_data_path(data_path)
    Context.set_cann_path(cann_path)
    Context.set_output(output, timestamp)
    Context.set_work_path(work_dir)
    resource = f"{resource_root}/{target}"
    resource = os.path.realpath(resource)
    
    init_env_test_case(target)
    find_ascend_test_in_dir(resource)
    
    Context.set_test_order(resource)
    logger.info(
        f"Find {len(Context.get_tests())} test cases, using {len(Context.get_used_tests())} test cases."
    )
    state_monitor = threading.Thread(
        name="state_monitor", target=print_state, args=[Context]
    )
    if not verbose:
        Context.finished = False
        enable_ansi_windows()
        state_monitor.start()
    result = Context.run_tests()
    if not verbose:
        Context.finished = True
        state_monitor.join()
    logger.info(f"Clean up tmp.")
    shutil.rmtree(f"{Context.output_dir}/tmp")
    logger.info(f"Complete!")

    gen_report(resource, Context)
    logger.info(f"Generate an execution report with the path {Context.get_output_dir()}")

def main():
    # 日志模块
    verbose = False
    init_logger(logging.DEBUG if verbose else logging.INFO)
    
    # 获取支持的targtes
    resource_root = os.path.realpath(os.path.dirname(__file__) + "/test_cases")
    targets = get_targets(resource_root)
    # 解析参数
    cmd_args = argparse_handler(['all'] + targets)
    
    if cmd_args.target != "all":
        targets = [cmd_args.target]
    timestamp = f'{datetime.now().strftime("%Y%m%d-%H-%M-%S")}-{random.randint(100,999)}'
    # 执行测试
    for i, target in enumerate(targets):
        logger.info(f"Targets: {targets}  Target: {target} ({i+1}/{len(targets)})")
        run_target_test(resource_root, cmd_args, target, verbose, timestamp)
        logger.info("")


if __name__ == "__main__":
    try:
        main()
    finally:
        del hider #恢复光标显示
