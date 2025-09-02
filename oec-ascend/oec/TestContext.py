# encoding: utf-8
import os
import random
import time
from datetime import datetime
import threading
from importlib import import_module
from oec.BaseTypes import State
from logging import getLogger
from oec.Utils import elapsed_time_str

from oec.TestInterface import TestInterface

logger = getLogger("oec-ascend")


def make_log_dir(log_dir):
    logger.info(f"log dir is {log_dir}")
    logger.info(f"create log path {log_dir}")
    os.makedirs(log_dir)
    return log_dir


class TestContext(object):

    def __init__(self):
        self._all_tests = {}
        self._data_path = ""
        self._cann_path = ""
        self._work_path = ""
        self._output_dir:str = ""
        self._relative_output:str = ""
        self._defaut_log_dir:str = ""
        self._used_tests = {}
        self._test_order = []
        self._infomation = {}
        self._states_distribution = {}
        self._env = os.environ.copy()
        self._console_output = {}
        self._console_position = []
        self.finished = False
        self._running_tests = []
        self._start_time = datetime.now()
        self._tags = {}
        self._target=""
        self._product=""
        self.group_dict = {}
        for state in State:
            self._states_distribution.setdefault(state, 0)
        
        self.infomation.setdefault("NPU", "unknow")
        self.set_message("distribution", "")
        self.set_message("rate", "")
        
    
    def set_env(self,env):
        self._env = env
    
    def set_message(self, key, message:str):
        if key not in self._console_output:
            self._console_position.append(key)
        self._console_output[key] = message
    
    def del_message(self, key):
        if key not in self._console_output:
            return
        del self._console_output[key]
        self._console_position.remove(key)
                
    @property
    def procut(self):
        return self._product
    
    def set_product(self, product):
        self._product = product
    
    @property
    def target(self):
        return self._target
    
    def set_target(self, target):
        self._target = target
    
    @property
    def env(self):
        return self._env
    
    def set_output(self,output:str, timestamp):
        output_path = os.path.join(output, timestamp, self.target)
        log_dir = os.path.join(output_path, "logs")
        make_log_dir(log_dir)
        self._output_dir = output_path
        self._relative_output = timestamp
        self._defaut_log_dir = log_dir
    
    def set_work_path(self,work_path:str):
        self._work_path = work_path   
    
    @property
    def work_path(self):
        return self._work_path
    
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
    
    def update_state(self):
        success = self.distribution[State.PASS] + self.distribution[State.NOTHING_TO_DO]
        
        total = len(self.get_used_tests())
        finished = total - self.distribution[State.NOT_RUNNING] - self.distribution[State.RUNNING]
        ran = finished - self.distribution[State.WARNING]
        if total == 0:
            return "wait for start"

        self.set_message("distribution",
            f"total {total}, running {self.distribution[State.RUNNING]}, not running {self.distribution[State.NOT_RUNNING]}, "
            f"passed {success}, warning {self.distribution[State.WARNING]}, failed {self.distribution[State.FAIL]}, "
            f"timeout {self.distribution[State.TIMEOUT]}, unsupported {self.distribution[State.UNSUPPORTED]}.")
        self.set_message("rate",
            f"Completion rate {round(finished/total*100,2)}%, pass rate { 0 if ran==0 else round(success/ran*100,2)}% - {elapsed_time_str(datetime.now() - self._start_time)}")
        
        for test in self._running_tests:
            test.update_console_message()
    
    
    def get_state_distribution_str(self):
        self.update_state()
        all = [self._console_output[k] for k in self._console_position]
        return '\n'.join(all)

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
        for tag in test.tags:
            self._tags.setdefault(tag,[])
            self._tags[tag].append(test)
        
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

        test_sequence = self.group_dict
        targets = None
        logger.debug("test_sequence is:")
        logger.debug(test_sequence)
        tmp_dict = {}
        for group in test_sequence:
            tmp_dict.setdefault(group, [])
        used_test = {}
        order_list = []
        for name, test in tests.items():
            
            if  test.group in tmp_dict:
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
    
    def clear_unimportented_messages(self, items, seconds):
        time.sleep(seconds)
        for test in items:
            if test.is_failed() or test.state == State.WARNING:
                continue
            test.del_console_message()
    
    def run_tests(self):
        self.distribution[State.NOT_RUNNING] = len(self.get_used_tests())
        order_list = self.test_order
        self._start_time = datetime.now()
        final_thread = None
        for items in order_list:
            threads = []
            self._running_tests = items
            for test in items:
                t = threading.Thread(target=test.run, name=test.name)
                t.start()
                threads.append(t)
            for t in threads:
                t.join()
            self.update_state()
            sleep_seconds = 5 if items is not order_list[-1] else 1.5
            final_thread = threading.Thread(target=self.clear_unimportented_messages, args=(items, sleep_seconds))
            final_thread.start()
            self._running_tests = []
            
            for test in items:
                if not test.can_continue():
                    final_thread.join()
                    return State.FAIL
        if final_thread:
            final_thread.join()
        return State.PASS

    def get_used_tests(self):
        return self._used_tests

    def get_tests(self):
        return self._all_tests
