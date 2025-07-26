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

from oec.TestContext import TestContext
from oec.BaseTest import Context

from oec.TestReport import gen_report

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


def argparse_handler():
    parser = argparse.ArgumentParser(
        prog="oec-ascend",
        description="Ascend Operating System Compatibility Verification Tool",
    )
    parser.add_argument(
        "-p",
        "--product",
        help="name of product",
    )
    parser.add_argument(
        "-t",
        "--target",
        default="default",
        help="offering of testcase",
    )
    # parser.add_argument(
    #     "-c",
    #     "--cann",
    #     default="/usr/local/Ascend",
    #     help="The root path for installing CANN is by default `/usr/local/Ascend`.",
    # )
    
    # parser.add_argument(
    #     "-d",
    #     "--data",
    #     default=f"./data",
    #     help="The path to the data file that is necessary during the run",
    # )
    
    # parser.add_argument(
    #     "-o",
    #     "--output",
    #     type=str,
    #     default="./output",
    #     help="Director to save results and log output",
    # )

    # parser.add_argument(
    #     "--verbose", action="store_true", default=False, help="print verbose output"
    # )
    
    args = parser.parse_args()
    return args


def find_ascend_test_in_dir(path: str):
    logger.info(f"test case director is '{path}' loading...")
    sys.path.append(path)
    for (
        prefix,
        dirs,
        files,
    ) in os.walk(path):
        for file in files:
            if file[:11] != "ascend_test" or file[-3:] != ".py":
                continue
            root = os.path.relpath(prefix, path)

            module_name = file[:-3]
            module_path = (
                module_name
                if root == "."
                else ".".join(root.split(os.sep) + [module_name])
            )
            file_path = os.path.join(prefix, file)
            logger.debug(f"import {module_path} path:{file_path}")
            try:
                module = import_module(module_path)
            except Exception as e:
                logger.error(f"import {module_path} error: {e}")
                continue


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
def main():
    cmd_args = argparse_handler()
    target = cmd_args.target
    product = cmd_args.product
    verbose = False
    output_dir = "./output"
    data_dir = os.path.dirname(__file__) + "/data"
    cann_dir = "/usr/local/Ascend"
    work_dir = os.path.realpath("./")
    
    init_logger(logging.DEBUG if verbose else logging.INFO)
    
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
    Context.set_output(output)
    Context.set_work_path(work_dir)
    resource =f"{os.path.dirname(__file__)}/resource"
    resource = os.path.realpath(resource)

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

    logger.info(f"Complete!")

    gen_report(resource, Context)
    logger.info(f"Generate an execution report with the path {Context.get_output_dir()}")


if __name__ == "__main__":
    try:
        main()
    finally:
        del hider #恢复光标显示
