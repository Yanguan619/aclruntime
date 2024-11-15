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

import os
import sys

from ais_bench.net_test.security.standard_consts import PlatformSupport, FileSizeLimit, Permission
from ais_bench.net_test.security.file_stat import FileStat

def get_platform():
    if sys.platform.startswith(PlatformSupport.LINUX):
        return PlatformSupport.LINUX
    if sys.platform.startswith(PlatformSupport.WINDOWS):
        return PlatformSupport.WINDOWS
    return PlatformSupport.UNKNOWN


def ms_open(file, mode="r", max_size=FileSizeLimit.UNLIMITED, softlink=False,
    write_permission=Permission.FILE_TO_WRITE, **kwargs):
    file_stat = FileStat(file)

    if file_stat.is_exists and file_stat.is_dir:
        raise Exception(f"Expecting a file, but it's a folder. {file}")

    if not softlink and file_stat.is_softlink:
        raise Exception(f"Softlink is not allowed to be opened. {file}")

    if "r" in mode:
        if not file_stat.is_exists:
            raise Exception(f"No such file or directory {file}")
        if max_size is None:
            raise Exception(f"Reading files must have a size limit control. {file}")
        if max_size != FileSizeLimit.UNLIMITED and max_size < file_stat.file_size:
            raise Exception(f"The file size has exceeded the specifications and cannot be read. {file}")

    if "w" in mode:
        if file_stat.is_exists and not file_stat.is_owner:
            raise Exception(
                f"The file owner is inconsistent with the current process user and is not allowed to write. {file}"
            )
        if file_stat.is_exists:
            try:
                os.remove(file)
            except Exception as err:
                raise PermissionError(f"current user can't remove {file}!") from err

    if "a" in mode:
        if not file_stat.is_owner:
            raise Exception(
                f"The file owner is inconsistent with the current process user and is not allowed to write. {file}"
            )
        if file_stat.permission != (file_stat.permission & write_permission):
            os.chmod(file, file_stat.permission & write_permission)

    flags = os.O_RDONLY
    if "+" in mode:
        flags = flags | os.O_RDWR
    elif "w" in mode or "a" in mode or "x" in mode:
        flags = flags | os.O_WRONLY

    if "w" in mode or "x" in mode:
        flags = flags | os.O_TRUNC | os.O_CREAT
    if "a" in mode:
        flags = flags | os.O_APPEND | os.O_CREAT
    return os.fdopen(os.open(file, flags, mode=write_permission), mode, **kwargs) # ms_open函数中，file在之前已经完成校验了
