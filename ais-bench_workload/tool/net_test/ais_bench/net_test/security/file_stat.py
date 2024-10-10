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
import stat
from ais_bench.net_test.security.string_checker import StringChecker


def basic_path_string_check_ok(file: str):
    string_checker = StringChecker(file)
    if string_checker.is_legal_path():
        return True
    return False


class OpenException(Exception):
    pass


class FileStat:
    def __init__(self, file: str) -> None:
        if not basic_path_string_check_ok(file):
            raise OpenException(f"create FileStat failed")
        self.file = file
        self.is_file_exist = os.path.exists(file)
        if self.is_file_exist:
            self.file_stat = os.stat(file)
            self.realpath = os.path.realpath(file)
        else:
            self.file_stat = None

    @property
    def is_exists(self):
        """
        路径是否存在。
        """
        return self.is_file_exist

    @property
    def is_softlink(self):
        """
        路径是否是软链接。
        """
        return os.path.islink(self.file) if self.file_stat else False

    @property
    def is_file(self):
        """
        路径是否是文件。
        """
        return stat.S_ISREG(self.file_stat.st_mode) if self.file_stat else False

    @property
    def is_dir(self):
        """
        路径是否是文件夹。
        """
        return stat.S_ISDIR(self.file_stat.st_mode) if self.file_stat else False

    @property
    def file_size(self):
        """
        文件大小
        """
        return self.file_stat.st_size if self.file_stat else 0

    @property
    def permission(self):
        """
        文件/文件夹权限
        """
        return stat.S_IMODE(self.file_stat.st_mode) if self.file_stat else 0o777

    @property
    def owner(self):
        """
        文件/文件夹所属用户
        """
        return self.file_stat.st_uid if self.file_stat else -1

    @property
    def group_owner(self):
        """
        文件/文件夹所属用户组
        """
        return self.file_stat.st_gid if self.file_stat else -1

    @property
    def is_owner(self):
        """
        文件/文件夹是否属于当前用户
        """
        return self.owner == (os.geteuid() if hasattr(os, "geteuid") else 0)

    @property
    def is_group_owner(self):
        """
        文件/文件夹是否属于当前用户组
        """
        return self.group_owner in (os.getgroups() if hasattr(os, "getgroups") else [0])

    @property
    def suffix(self):
        """
        文件后缀名，以'.'开头，如“.py”
        """
        if self.file_stat and self.is_file:
            _, suffix = os.path.splitext(self.file)
            return suffix
        else:
            return ""
