# encoding: utf-8
import os
import subprocess
from logging import getLogger
import re
from typing import List # 兼容python3.7
from oec.BaseTest import BaseTest
from oec.BaseTypes import State
import oec.Utils as Utils

logger = getLogger("oec-ascend")


class TestCase(BaseTest):
    def __init__(
        self,
        cmd: List[str] = [],
        include: List[str] = None,
        exclude: List[str] =[],
        count=1,
        origin_file: str = "",
        with_case_info: bool = True,
        cwd=None,
        timeout=None,
        *args,
        **kwargs,
    ):
        super(TestCase, self).__init__(*args, **kwargs)
        self._count = count
        self._cmd = cmd
        self.origin_file = origin_file
        self.with_case_info = with_case_info
        self._include = include
        self._exclude = exclude
        self.__reason = None
        self._log = ""
        self._retrun_code = 0
        self._cwd = cwd
        self._timeout = timeout
        if isinstance(self._include, str):
            self._include = [self._include]
        if isinstance(self._exclude, str):
            self._exclude = [self._exclude]

        logger.debug(f"test case{self.group[0]}.{self.group[1]}.{self.name} ")

    @property
    def cwd(self):
        return self._cwd
    
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
    
    def get_relative_log_file_path(self):
        
        return os.path.relpath(self.get_log_file_path(), self.context.work_path)
    
    def get_log_file_path(self):
        return os.path.join(self.get_log_dir_path(), f"{self.name}.log")

    def get_test_content(self):
        return (
            self.get_relative_log_file_path()
            if self.is_finished()
            else "No information due to the previous error."
        )

    def execute_command_with_cmd(self, cmd):
        if self.state != State.NOT_RUNNING:
            return
        if cmd is None:
            self.set_state(State.NOTHING_TO_DO)
            return
        self.set_state(State.RUNNING)
        log = None
        return_code = None
        with open(self.get_log_file_path(), "w+") as f:
            env = self.context.env.copy()
            env["OEC_OUTPUT_PATH"] = f"{self.context.output_dir}/tmp/{self.name}"
            env["OEC_DATA_PATH"] = self.context.data_path
            env["OEC_WORKDIR"] = self.context.work_path
            env["OEC_PRODUCT"] = self.context.procut
            if self.with_case_info:
                f.writelines([
                    "**********************************************\n",
                    self.name + "\n",
                    self.origin_file + "\n",
                    f"cmd = {self.get_cmd()}\n\n",
                    f"export OEC_OUTPUT_PATH={env['OEC_OUTPUT_PATH']}\n",
                    f"export OEC_DATA_PATH={env['OEC_DATA_PATH']}\n",
                    f"export OEC_WORKDIR={env['OEC_WORKDIR']}\n",
                    f"export OEC_PRODUCT={env['OEC_PRODUCT']}\n",
                    f"cd {self.cwd}\n",
                    " ".join(self.get_cmd()) + "\n",
                    "**********************************************\n",
                ])
                f.flush()
            process = subprocess.Popen(
                self.get_cmd(),
                env=env,
                cwd=os.path.dirname(self.get_origin_path()) if self.cwd is None else self.cwd,
                # shell=True,
                stdout=f,
                stderr=subprocess.STDOUT,
                text=True,
            )
            try:
                process.wait(self._timeout)
            except subprocess.TimeoutExpired:
                self.set_state(State.TIMEOUT)
            f.seek(0)
            log = f.read(-1)
            return_code = process.returncode
            self._retrun_code = return_code
            self._log = log

        self.check_result(log, return_code)
        return log, return_code

    def execute_command(self):
        self.execute_command_with_cmd(self.get_cmd())

    def get_cmd(self):
        return self._cmd

    def count(self):
        return self._count

    def get_doc(self):
        pass
    
    @property
    def log(self):
        return self._log
    
    @property
    def return_code(self):
        return self._return_code
    
    def check_result(self, log: str, return_code):
        logger.debug(
            f'\n>> {self.name}{"(optional)" if self.is_optional() else ""} -> return {return_code} :\n File "{self.get_origin_path()}" :\n{log}'
        )
        if self.is_finished():
            return
        if self.get_include() is not None:
            for pattern in self.get_include():
                result = re.search(pattern, log)
                if result is None:
                    self.set_state(State.FAIL)
                    self.set_reason(
                        f"'{pattern}' was not found in the output of {self.name}, {self.get_log_file_path()}"
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
                        f"Find '{pattern}' in the output of {self.name}, {self.get_log_file_path()}:{lineno}:{position}"
                    )
                    return


        if return_code != 0:
            code_map = {
                124: (State.TIMEOUT, f"code: {return_code}, timeout. Log: {Utils.get_file_path(self.get_log_file_path())}"),
                127: (State.FAIL, f"code: {return_code}, command not found. Log: {Utils.get_file_path(self.get_log_file_path())}"),
                191: (State.WARNING, f"code: {return_code}, warning. Log: {Utils.get_file_path(self.get_log_file_path())}"),
                192: (State.UNSUPPORTED, f"code: {return_code}, unsupported."),
            }
            failed = (State.FAIL, f"code: {return_code}, failed. Log: {Utils.get_file_path(self.get_log_file_path())}")
            state = code_map.get(return_code, failed)
            self.set_state(state[0])
            self.set_reason(state[1])
            return

        self.set_state(State.PASS)
