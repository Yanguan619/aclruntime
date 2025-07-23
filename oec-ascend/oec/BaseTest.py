# encoding: utf-8
import os
import threading
import inspect
import time
from datetime import datetime
from oec.Utils import elapsed_time_str
from typing import Tuple # 兼容python3.7
from logging import getLogger
from oec.TestInterface import TestInterface
from oec.TestContext import TestContext
from oec.BaseTypes import State

logger = getLogger("oec-ascend")


Context:TestContext = TestContext()


class BaseTest(TestInterface):
    def __init__(
        self,
        group: Tuple[str, str],
        name: str,
        optional: bool = True,
        auxiliary: bool = False,
        cached: bool = True,
        log_dir: str = "",
    ):
        self._context: TestContext = Context
        self._name: str = name
        self._group = group
        self._optional: bool = optional
        self._state: State = State.NOT_RUNNING
        self._auxiliary: bool = auxiliary
        self._cached: bool = cached
        self._log_dir_path = log_dir if log_dir else self._context.get_log_dir()
        self._lock = threading.Lock()
        self._filename = None
        self._lineno = None
        self._start_time = datetime.now()
        self._end_time =  self._start_time
        self._update_count = 0
        for stack in inspect.stack()[1:]:
            if stack.function != "__init__":
                self._filename = stack.filename
                self._lineno = stack.lineno
                break

        if not name:
            raise ValueError(self.message_with_path("name can not be empty."))
        self.context.add_test(self)

    @property
    def group(self):
        return self._group

    @property
    def context(self):
        return self._context

    def message_with_path(self, message):
        return f"{message} File {self.get_origin_path()}:{self.get_origin_lineno()}"
    
    @property
    def state(self):
        return self._state

    def can_cached(self):
        return self._cached

    def is_finished(self):
        return self.state not in [State.NOT_RUNNING, State.RUNNING]

    def can_continue(self):
        if self.is_passed() or self.state == State.UNSUPPORTED:
            return True

        if self.is_failed() and self.is_optional():
            return True

        return False

    def is_failed(self):
        if self.state in [State.FAIL, State.TIMEOUT]:
            return True
        return False

    def is_passed(self):
        if self.state in [State.PASS, State.NOTHING_TO_DO, State.WARNING]:
            return True
        return False

    def set_reason(self, str):
        raise NotImplementedError()

    def get_reason(self):
        raise NotImplementedError()

    def get_log_dir_path(self):
        if self._log_dir_path is None:
            raise RuntimeError("log dir path is not set")
        return self._log_dir_path

    def set_log_dir_path(self, path):
        if not isinstance(path, str):
            raise TypeError("The path must be a str")
        self._log_dir_path = os.path.abspath(path)

    def get_origin_lineno(self):
        return self._lineno

    def get_origin_path(self):
        return self._filename

    
    def update_elapsed_time(self):
        if self.state in [State.RUNNING, State.NOT_RUNNING]:
            self._end_time = datetime.now()

    @property
    def elapsed_time(self):
        delta = self._end_time - self._start_time 
        return delta

    @property
    def elapsed_time_str(self):
        return elapsed_time_str(self.elapsed_time)
    
    def update_console_message(self):
        self.update_elapsed_time()
        message = f"{self.name} {self.elapsed_time_str}"
        anime = "⠋⠙⠸⠴⠦⠇"
        if self.is_failed():
            message = f"\033[31m✕  {message} - {self.get_reason()}\033[0m"
        elif self.state == State.WARNING:
            message = f"\033[33m!  {message} - {self.get_reason()}\033[0m"
        elif self.is_passed():
            message = f"\033[32m✓  {message}\033[0m"
        elif self.state == State.UNSUPPORTED:
            message = f"\033[0m✓  {message} - {self.get_reason()}\033[0m"
        else:
            charactor = anime[self._update_count % len(anime)]
            message = f"{charactor}  {message}\033[0m"
            
        self.context.set_message(self.name, message)
        self._update_count += 1
    
    def del_console_message(self):
        self.context.del_message(self.name)
    
    def run(self):
        self._lock.acquire()
        if self.is_finished() and self.can_cached():
            logger.debug(
                f"The test {self.name} has been completed, using cached results"
            )
            return
        self.set_state(State.NOT_RUNNING)
        self._start_time = datetime.now()
        self.update_elapsed_time()
        try:
            self.execute_command()
        except Exception as e:
            self.set_state(State.FAIL)
            self.set_reason(f"{e}")
        if self.is_failed():
            logger.debug(
                f"{self.name} is {self.state.value}, reason: {self.get_reason()}"
            )
        self.update_elapsed_time()
        self._lock.release()

    def execute_command(self):
        raise NotImplementedError()

    def count(self):
        return 1

    def set_name(self, name):
        if not isinstance(name, str):
            raise TypeError("name must be a string")
        self._name = name
    
    @property
    def name(self):
        return self._name

    def is_optional(self):
        return self._optional

    def set_optional(self, optional: bool):
        if not isinstance(optional, bool):
            raise TypeError("optional must be bool type")
        self._optional = True

    @property
    def auxiliary(self):
        return self._auxiliary
    
    def set_state_if_not_finished(self, state: State):
        if not self.is_finished():
            self.set_state(state)

    def set_state(self, state: State):
        if not isinstance(state, State):
            raise TypeError("state must be of type State")
        if self.auxiliary and state == State.FAIL:
            state=State.WARNING
            
        self.context.distribution[self.state] -= self.count()
        self._state = state
        self.context.distribution[state] += self.count()

    def get_test_content(self):
        return (
            self.get_relative_log_file_path()
            if self.is_finished()
            else "No information due to the previous error."
        )
    
    @property
    def logger(self):
        return logger

    def __str__(self):
        return self.name

    def __repr__(self):
        return str(self)
