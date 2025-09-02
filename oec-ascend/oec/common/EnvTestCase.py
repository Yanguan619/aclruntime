# encoding: utf-8
import os
from oec import TestCase,State
from oec.BaseTest import BaseTest
from oec.Utils import merge_env_variables
class SetEnvTestCase(TestCase):
    def execute_command(self):
        super().execute_command()
        if not self.is_passed():
            return
        cann_envname = [
            'ASCEND_TOOLKIT_HOME',
            'ASCEND_HOME_PATH',
            'ASCEND_AICPU_PATH',
            'ASCEND_OPP_PATH',
            'TOOLCHAIN_HOME',
            'LD_LIBRARY_PATH',
            'PYTHONPATH',
            'PATH',
            ]
        env = merge_env_variables(self.log,cann_envname)
        self.context.set_env(env)
        self.logger.debug(self.context.env)
        self.set_state(State.PASS)
        

class ResetEnvTestCase(BaseTest):
    def execute_command(self):
        self.context.env = os.environ.copy()
        self.set_state(State.PASS)
