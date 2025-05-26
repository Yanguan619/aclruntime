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
        "-r",
        "--resource",
        default=f"{os.path.dirname(__file__)}/resource",
        help="resource path",
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
    args.resource = os.path.realpath(args.resource)
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
    output_path = os.path.join(
        output_path,
        f'{datetime.now().strftime("%Y%m%d-%H-%M-%S")}-{random.randint(100,999)}',
    )
    return output_path


def make_log_dir(output):
    log_dir = os.path.join(output, "logs")
    logger.info(f"log dir is {log_dir}")
    logger.info(f"create log path {log_dir}")
    os.makedirs(log_dir)
    return log_dir


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


def main():
    cmd_args = argparse_handler()
    init_logger(logging.DEBUG if cmd_args.verbose else logging.INFO)
    logger.info(cmd_args)

    output = get_absolute_out_path(cmd_args.output)
    make_log_dir(output)
    context = TestContext(output)
    set_default_context(context)

    find_ascend_test_in_dir(cmd_args.resource)

    context.set_test_order(cmd_args.resource)
    logger.info(
        f"Find {len(context.get_tests())} test cases, using {len(context.get_used_tests())} test cases."
    )
    state_monitor = threading.Thread(
        name="state_monitor", target=print_state, args=[context]
    )
    if not cmd_args.verbose:
        context.finished = False
        state_monitor.start()
    result = context.run_tests()
    if not cmd_args.verbose:
        context.finished = True
        state_monitor.join()

    logger.info(f"Complete!")

    gen_report(cmd_args.resource, context)
    logger.info(f"Generate an execution report with the path {output}")


if __name__ == "__main__":
    main()
