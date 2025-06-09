# encoding: utf-8
import os
import random
from datetime import datetime
import threading
from importlib import import_module
from oec.BaseTypes import State
from logging import getLogger
import pandas as pd

from oec.TestInterface import TestInterface

logger = getLogger("oec-ascend")


def make_log_dir(log_dir):
    logger.info(f"log dir is {log_dir}")
    logger.info(f"create log path {log_dir}")
    os.makedirs(log_dir)
    return log_dir


class TestContext(object):

    def __init__(self):
        self._all_tests: dict[str, TestInterface] = {}
        self._data_path = ""
        self._cann_path = ""
        self._output_dir:str = ""
        self._relative_output:str = ""
        self._defaut_log_dir:str = ""
        self._used_tests: dict[str, TestInterface] = {}
        self._test_order: list[list[TestInterface]] = []
        self._infomation: dict[str, str] = {}
        self._states_distribution: dict[State, int] = {}
        self._env:dict[str,str] = os.environ.copy()
        self.finished = False
        for state in State:
            self._states_distribution.setdefault(state, 0)
    
    def set_env(self,env:dict[str,str]):
        self._env = env
    
    @property
    def env(self):
        return self._env
    
    def set_output(self,output:str):
        relative_output = (
            f'{datetime.now().strftime("%Y%m%d-%H-%M-%S")}-{random.randint(100,999)}'
        )
        output_path = os.path.join(output, relative_output)
        log_dir = os.path.join(output_path, "logs")
        make_log_dir(log_dir)
        self._output_dir = output_path
        self._relative_output = relative_output
        self._defaut_log_dir = log_dir
    
    def set_cann_path(self,cann_path:str):
        self._cann_path = cann_path   
    
    @property
    def cann_path(self):
        return self._cann_path

    def set_data_path(self,data_path:str):
        self._data_path = data_path
    
    @property
    def data_path(self):
        return self._data_path
    
    def get_state_distribution_str(self):
        success = self.distribution[State.PASS] + self.distribution[State.NOTHING_TO_DO]
        failed = (
            self.distribution[State.FAIL]
            + self.distribution[State.TIMEOUT]
            + self.distribution[State.UNSUPPORTED]
        )
        ran = success + failed
        total = len(self.get_used_tests())
        if total == 0:
            return "wait for start"

        return (
            f"total {total}, running {self.distribution[State.RUNNING]}, not running {self.distribution[State.NOT_RUNNING]}, "
            f"passed {success} , failed {self.distribution[State.FAIL]}, unsupported {self.distribution[State.UNSUPPORTED]}, "
            f"timeout {self.distribution[State.TIMEOUT]}.\n"
            f"Completion rate {round(ran/total*100,2)}%, pass rate { 0 if ran==0 else round(success/ran*100,2)}%"
        )

    @property
    def relative_output(self):
        return self._relative_output

    @property
    def distribution(self):
        return self._states_distribution

    @property
    def infomation(self):
        return self._infomation

    @property
    def output_dir(self):
        return self._output_dir
    
    def get_output_dir(self):
        return self.output_dir

    def get_log_dir(self):
        return self._defaut_log_dir

    def set_log_dir(self, path: str):
        self._defaut_log_dir = path

    def add_test(self, test: TestInterface):
        if test.name in self._all_tests:
            t2 = self._all_tests[test.name]
            raise RuntimeError(
                f'"{test.name}" in {test.get_origin_path()}:{test.get_origin_lineno()}'
                f" has been used in {t2.get_origin_path()}:{t2.get_origin_lineno()}"
            )
        self._all_tests[test.name] = test

    @property
    def test_order(self):
        return self._test_order

    def set_test_order(self, path):
        if not os.path.exists(path):
            logger.fatal(f"Can not find the path: {path}")
            exit(6500)
        tests = self.get_tests()
        path = os.path.join(path, "test_sequence.py")

        test_sequence = None
        try:
            test_sequence_module = import_module("test_sequence")
            test_sequence = test_sequence_module.test_sequence
        except Exception as e:
            logger.fatal(f"Errors were found in test_sequence.py, error: {e}")
            exit(7000)

        logger.debug("test_sequence is:")
        logger.debug(test_sequence)
        tmp_dict: dict[tuple, list[TestInterface]] = {}
        for group in test_sequence:
            tmp_dict.setdefault(group, [])
        used_test = {}
        order_list: list[list[TestInterface]] = []
        for name, test in tests.items():
            if test.group in tmp_dict:
                tmp_dict[test.group].append(test)
                used_test[test.name] = test

        for group, t in tmp_dict.items():
            if not t:
                continue
            if test_sequence[group]:
                order_list.append(t)
            else:
                for test in t:
                    order_list.append([test])
        logger.debug(f"test sequence detials:")
        logger.debug(order_list)
        self._test_order = order_list
        self._used_tests = used_test

    def run_tests(self):
        self.distribution[State.NOT_RUNNING] = len(self.get_used_tests())
        order_list = self.test_order
        for item in order_list:
            threads: list[threading.Thread] = []
            for test in item:
                t = threading.Thread(target=test.run, name=test.name)
                t.start()
                threads.append(t)
            for t in threads:
                t.join()
            for test in item:
                if not test.can_continue():
                    return State.FAIL
        return State.PASS

    def get_used_tests(self):
        return self._used_tests

    def get_tests(self):
        return self._all_tests
