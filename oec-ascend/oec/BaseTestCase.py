# encoding: utf-8
import os
import subprocess
from logging import getLogger
import re
from oec.BaseTest import BaseTest
from oec.BaseTypes import State

logger = getLogger("oec-ascend")


class TestCase(BaseTest):
    def __init__(
        self,
        cmd: str = None,
        include: list[str] = None,
        exclude: list[str] = ["failed", "Failed", "FAILED", "error", "ERROR", "Error"],
        expect: list[int] = [0],
        unexpect: list[int] = None,
        count=1,
        *args,
        **kwargs,
    ):
        super(TestCase, self).__init__(*args, **kwargs)
        # if not cmd:
        #     logger.fatal(self.message_with_path("cmd can not be empty."))
        #     exit(60000)
        self._count = count
        self._cmd = cmd

        self._include = include
        self._exclude = exclude
        self._expected_code = expect
        self._unexpected_code = unexpect
        self.__reason = None

        if isinstance(self._include, str):
            self._include = [self._include]
        if isinstance(self._exclude, str):
            self._exclude = [self._exclude]
        if isinstance(self._expected_code, int):
            self._expected_code = [self._expected_code]
        if isinstance(self._unexpected_code, int):
            self._unexpected_code = [self._unexpected_code]

        logger.debug(f"test case{self.group[0]}.{self.group[1]}.{self.name()} ")

    def set_reason(self, reason: str):
        if not isinstance(reason, str):
            raise TypeError(f"reason must be a string")
        self.__reason = reason

    def get_reason(self):
        return self.__reason

    def get_include(self):
        return self._include

    def get_exclude(self):
        return self._exclude

    def get_expected_code(self):
        return self._expected_code

    def get_unexpected_code(self):
        return self._unexpected_code

    def get_log_file_path(self):
        return os.path.join(self.get_log_dir_path(), f"{self.name()}.log")

    def get_test_content(self):
        return (
            self.get_log_file_path()
            if self.is_finished()
            else "No information due to the previous error."
        )

    def execute_command_with_cmd(self, cmd):
        if self.state() != State.NOT_RUNNING:
            return
        if cmd is None:
            self.set_state(State.NOTHING_TO_DO)
            return
        self.set_state(State.RUNNING)
        log = None
        return_code = None
        with open(self.get_log_file_path(), "w+") as f:

            process = subprocess.Popen(
                cmd,
                cwd=os.path.dirname(self.get_origin_path()),
                shell=True,
                stdout=f,
                stderr=subprocess.STDOUT,
                text=True,
            )
            process.wait()
            f.seek(0)
            log = f.read(-1)
            return_code = process.returncode

        self.check_result(log, return_code)

    def execute_command(self):
        self.execute_command_with_cmd(self.get_cmd())

    def get_cmd(self):
        return self._cmd

    def count(self):
        return 0 if self.is_auxiliary() else self._count

    def get_doc(self):
        pass

    def check_result(self, log: str, return_code):
        logger.debug(
            f'\n>> {self.name()}{"(optional)" if self.is_optional() else ""} -> return {return_code} :\n File "{self.get_origin_path()}" :\n{log}'
        )
        if self.get_include() is not None:
            for pattern in self.get_include():
                result = re.search(pattern, log)
                if result is None:
                    self.set_state(State.FAIL)
                    self.set_reason(
                        f"'{pattern}' was not found in the output of {self.name()}, {self.get_log_file_path()}"
                    )
                    return

        if self.get_exclude() is not None:
            for pattern in self.get_exclude():
                result = re.search(pattern, log)
                if result is not None:
                    self.set_state(State.FAIL)
                    span = result.span()

                    lineno = log.count("\n", 0, span[0]) + 1
                    position = log.rfind("\n", 0, span[0])
                    position = span[0] - position
                    self.set_reason(
                        f"Find '{pattern}' in the output of {self.name()}, {self.get_log_file_path()}:{lineno}:{position}"
                    )
                    return

        if self.get_expected_code() is not None:
            if return_code not in self.get_expected_code():
                self.set_state(State.FAIL)
                self.set_reason(
                    f"Then return code {return_code} of {self.name()} does not match any of {self.get_expected_code()}, {self.get_log_file_path()}"
                )
                return

        if self.get_unexpected_code() is not None:
            if return_code in self.get_unexpected_code():
                self.set_state(State.FAIL)
                self.set_reason(
                    f"Then return code {return_code} of {self.name()} matches {self.get_expected_code()}, {self.get_log_file_path()}"
                )
                return

        self.set_state(State.PASS)
