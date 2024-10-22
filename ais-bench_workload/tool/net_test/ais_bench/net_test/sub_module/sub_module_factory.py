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

from ais_bench.net_test.sub_module.install_module import InstallModule

from ais_bench.net_test.common.logger import logger
from ais_bench.net_test.common.consts import SUB_MODULE_NAME

sub_module_switch = {
    SUB_MODULE_NAME.INSTALL: InstallModule,
}


class SubModuleFactory():
    @staticmethod
    def get(module_name, **kwargs):
        if sub_module_switch.get(module_name.strip()) is not None:
            logger.debug(f"create module: {module_name} success!")
            return sub_module_switch.get(module_name.strip())(module_name, **kwargs)
        else:
            raise KeyError(f"sub module {module_name} is not supported. " + \
                         f"Currently only {', '.join(list(module_name.keys()))} are supported.")


sub_module_factory = SubModuleFactory()