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
from oec.BaseTest import set_default_context

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
        "-d",
        "--data",
        default=f"./data",
        help="The path to the data file that is necessary during the run",
    )
    
    parser.add_argument(
        "--verbose", action="store_true", default=False, help="print verbose output"
    )
    
    parser.add_argument(
        "-o",
        "--output",
        type=str,
        default="./output",
        help="Director to save results and log output",
    )
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





def print_state(context: TestContext):
    logger.info(context.get_state_distribution_str())
    dynamic = ["|", "/", "-", "\\"]
    count = 0
    while not context.finished:
        logger.info(
            f"\033[2A\033[K{context.get_state_distribution_str()}  {dynamic[count % len(dynamic)]}\033[K"
        )
        count += 1
        time.sleep(0.2)
    logger.info(f"\033[2A\033[K{context.get_state_distribution_str()}\033[K")

def enable_ansi_windows():
    """在 Windows 上启用 ANSI 转义序列支持"""
    if sys.platform == "win32":
        import ctypes
        kernel32 = ctypes.windll.kernel32
        kernel32.SetConsoleMode(kernel32.GetStdHandle(-11), 7)  # 启用 VT100 模式
def main():
    cmd_args = argparse_handler()
    init_logger(logging.DEBUG if cmd_args.verbose else logging.INFO)
    logger.info(cmd_args)
    output = os.path.abspath(cmd_args.output)
    data_path = os.path.realpath(cmd_args.data)
    if not os.path.exists(cmd_args.data):
        logger.fatal(f"{data_path} is not existing, please download it first!")
        exit(1000)
    context = TestContext(output,data_path)
    set_default_context(context)
    resource =f"{os.path.dirname(__file__)}/resource"
    resource = os.path.realpath(resource)

    find_ascend_test_in_dir(resource)

    context.set_test_order(resource)
    logger.info(
        f"Find {len(context.get_tests())} test cases, using {len(context.get_used_tests())} test cases."
    )
    state_monitor = threading.Thread(
        name="state_monitor", target=print_state, args=[context]
    )
    if not cmd_args.verbose:
        context.finished = False
        enable_ansi_windows()
        state_monitor.start()
    result = context.run_tests()
    if not cmd_args.verbose:
        context.finished = True
        state_monitor.join()

    logger.info(f"Complete!")

    gen_report(resource, context)
    logger.info(f"Generate an execution report with the path {output}")


if __name__ == "__main__":
    main()
