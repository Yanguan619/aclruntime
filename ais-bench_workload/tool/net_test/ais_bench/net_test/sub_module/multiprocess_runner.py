# Copyright (c) 2024-2024 Huawei Technologies Co., Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from multiprocessing import Pool
from ais_bench.net_test.ssh.ssh_operation import SSH_EXCEPTION_LIST
from ais_bench.net_test.common.consts import RemoteNodeInfoName
from ais_bench.net_test.common.logger import logger


class MultiProcessRunner:
    def __init__(self):
        self.clean_up_cmd = ""
        self.return_code = False
        self.task_failed = None
        self.remote_func = None
        self.args_dict_list = []

    def __call__(self, func, args_dict_list, clean_up_cmd=""):
        self.clean_up_cmd = clean_up_cmd
        self.args_dict_list = args_dict_list
        self.remote_func = func
        self.task_failed = False
        p = Pool(len(args_dict_list))

        def _callback(value):
            logger.error(f"get some error in remote run:{value}")
            p.terminate()
            if type(value) not in SSH_EXCEPTION_LIST and self.clean_up_cmd != "":
                self._clean_up()
            self.task_failed = True

        try:
            for _, args_dict in enumerate(args_dict_list):
                p.apply_async(func, args=(args_dict,), error_callback=_callback)
            p.close()
            p.join()
        except KeyboardInterrupt as e:
            p.terminate()
            if clean_up_cmd != "":
                self._clean_up()
            raise RuntimeError("multiprocess runner is interrupt by keyboard!") from e

        if self.task_failed:
            raise RuntimeError("multiprocess runner exec failed!")
        
        
    def _run_map(self):
        process_count = len(self.args_dict_list)
        with Pool(processes=process_count) as pool:
            pool.map(self.remote_func, self.args_dict_list)

    def _clean_up(self):
        for i, _ in enumerate(self.args_dict_list):
            self.args_dict_list[i][RemoteNodeInfoName.CMD] = self.clean_up_cmd
        logger.info("start to clean up remote resource ...")
        try:
            self._run_map()
        except Exception as err:
            logger.error(f"clean up remote resource failed, error log: {err}")
        logger.info("clean up remote resource success!")
