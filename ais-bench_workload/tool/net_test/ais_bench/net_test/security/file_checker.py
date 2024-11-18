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

from ais_bench.net_test.security.file_stat import FileStat
from ais_bench.net_test.security.standard_consts import PermForbid, PermNeed, FileSizeLimit


class FileChecker:
    def __init__(self, file: str):
        self.file_stat = FileStat(file)

    def is_file(self):
        """
        校验路径是否是文件。
        Parameters:
            None

        Returns:
            bool (True:路径是文件, False:路径非文件)
        """
        return self.file_stat.is_file

    def is_dir(self):
        """
        校验路径是否是文件夹。
        Parameters:
            None
        Returns:
            bool (True:路径是文件夹, False:路径非文件夹)
        """
        return self.file_stat.is_dir

    def is_exists(self):
        """
        校验路径是否存在。
        Parameters:
            None
        Returns:
            bool (True:路径存在, False:路径不存在)
        """
        return self.file_stat.is_exists

    def is_not_softlink(self):  # only linux
        """
        校验路径是否不是软链接。
        Parameters:
            None
        Returns:
            bool (True:路径不是软链接, False:路径是软链接)
        """
        return not self.file_stat.is_softlink

    def is_permission_legal(
            self,
            perm_need=PermNeed.READ_FILE,
            perm_forbid=PermForbid.USER_MAIN_DIR,
    ):  # only linux
        """
        路径权限是否合法。
        Parameters:
            perm_need: 路径至少需要的权限
            perm_forbid: 路径不能拥有的权限
        Returns:
            bool (True:路径权限合法, False:路径权限非法)
        """
        cur_perm = self.file_stat.permission
        if cur_perm & perm_forbid > 0:
            return False
        if perm_need & cur_perm == 0:
            return False
        return True

    def is_user_and_group_owner(self):  # only linux
        return self.file_stat.is_owner and self.file_stat.is_group_owner

    def is_user_or_group_owner(self):  # only linux
        return self.file_stat.is_owner or self.file_stat.is_group_owner

    def is_size_legal(self, max_size: int = FileSizeLimit.UNLIMITED):
        if max_size == FileSizeLimit.UNLIMITED:
            return True
        if self.file_stat.is_dir:  # 文件夹不校验
            return True
        if self.file_stat.file_size > max_size:
            return False
        return True

    def is_suffix_legal(self, legal_suffixes: list):
        """
            legal_suffixes: list of suffixes, such as [".json", ".yaml"]
        """
        if not legal_suffixes:
            return True
        if self.file_stat.is_dir:  # 文件夹不校验
            return True

        cur_suffix = self.file_stat.suffix
        for suffix in legal_suffixes:
            if cur_suffix == suffix:
                return True
        return False

def check_linux_path_format(file_path: str):
    _ = FileChecker(file_path) # if format illegal raise OpenException

def check_linux_file_path(
        file_path: str,
        max_size=FileSizeLimit.NORMAL_READ_FILE_4G,
        perm_forbid=PermForbid.USER_MAIN_DIR,
        perm_need=PermNeed.READ_FILE,
        legal_suffixes=None,
):
    legal_suffixes = legal_suffixes if legal_suffixes is not None else []

    file_checker = FileChecker(file_path)
    if not file_checker.is_exists():
        raise ValueError("path not exist.")
    if not file_checker.is_file():
        raise ValueError("path is not a file.")
    if not file_checker.is_not_softlink():
        raise ValueError("path is a softlink.")
    if not file_checker.is_user_or_group_owner():
        raise ValueError("path is not belong to current user or user group.")
    if not file_checker.is_permission_legal(perm_need=perm_need, perm_forbid=perm_forbid):
        raise ValueError("path's permission is illegal.")
    if not file_checker.is_size_legal(max_size=max_size):
        raise ValueError(f"file size over max size: {max_size}.")
    if not file_checker.is_suffix_legal(legal_suffixes=legal_suffixes):
        raise ValueError(f"file suffix is not in : {legal_suffixes}.")


def check_linux_readable_file(
        file_path: str,
        max_size=FileSizeLimit.NORMAL_READ_FILE_4G,
        perm_forbid=PermForbid.USER_MAIN_DIR,
        legal_suffixes=None,
):
    check_linux_file_path(file_path, max_size=max_size, perm_forbid=perm_forbid, perm_need=PermNeed.READ_FILE,
                          legal_suffixes=legal_suffixes)


def check_linux_executable_file(
        file_path: str,
        max_size=FileSizeLimit.NORMAL_READ_FILE_4G,
        perm_forbid=PermForbid.USER_MAIN_DIR,
        legal_suffixes=None,
):
    check_linux_file_path(file_path, max_size=max_size, perm_forbid=perm_forbid, perm_need=PermNeed.EXEC_FILE,
                          legal_suffixes=legal_suffixes)
