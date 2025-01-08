# Copyright (c) Huawei Technologies Co., Ltd. 2023-2025. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

#     http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# this file is as same as components/utils/file_opem_check.py, because benchmark might be install without ait

import os
import sys
import stat
import re
import logging

MAX_SIZE_UNLIMITED = -1  # 不限制，必须显式表示不限制，读取必须传入
MAX_SIZE_LIMITED_CONFIG_FILE = 10 * 1024 * 1024  # 10M 普通配置文件，可以根据实际要求变更
MAX_SIZE_LIMITED_NORMAL_FILE = 4 * 1024 * 1024 * 1024  # 4G 普通模型文件，可以根据实际要求变更
MAX_SIZE_LIMITED_MODEL_FILE = 100 * 1024 * 1024 * 1024  # 100G 超大模型文件，需要确定能处理大文件，可以根据实际要求变更

PATH_WHITE_LIST_REGEX_WIN = re.compile(r"[^_:\\A-Za-z0-9/.-]")
PATH_WHITE_LIST_REGEX = re.compile(r"[^_A-Za-z0-9/.-]")
NORMAL_STR_WHITE_LIST_REGEX = re.compile(r"[^_A-Za-z0-9\"'><=\[\])(,}{: /.~-]")  # 常规字符串

PERMISSION_NORMAL = 0o640  # 普通文件
PERMISSION_KEY = 0o600  # 密钥文件
READ_FILE_NOT_PERMITTED_STAT = stat.S_IWGRP | stat.S_IWOTH
WRITE_FILE_NOT_PERMITTED_STAT = stat.S_IWGRP | stat.S_IWOTH | stat.S_IROTH | stat.S_IXOTH

SOLUTION_LEVEL = 35
SOLUTION_LEVEL_WIN = 45
logging.addLevelName(SOLUTION_LEVEL, "\033[1;32m" + "SOLUTION" + "\033[0m")  # green [SOLUTION]
logging.addLevelName(SOLUTION_LEVEL_WIN, "SOLUTION_WIN")
logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='[%(levelname)s] %(message)s')
logger = logging.getLogger(__name__)

SOLUTION_BASE_LOC = '\"gitee repo: Ascend/tools, path:tools/ais_bench-workload/tool/ais_bench/README.md, ' + \
                    'chapter:'
SOFT_LINK_SUB_CHAPTER = ' FAQ/security_error/soft_link_error_log_solution\"'
PATH_LENGTH_SUB_CHAPTER = ' FAQ/security_error/path_length_overflow_error_log_solution\"'
OWNER_SUB_CHAPTER = ' FAQ/security_error/owner_or_ownergroup_error_log_solution\"'
PERMISSION_SUB_CHAPTER = ' FAQ/security_error/path_permission_error_log_solution\"'
ILLEGAL_CHAR_SUB_CHAPTER = ' FAQ/security_error/path_contain_illegal_char_error_log_solution\"'

MAX_LINUX_ABS_PATH_LENGTH = 4096
MAX_LINUX_BASE_NAME_LENGTH = 255
MAX_WIN_ABS_PATH_LENGTH = 260


class FilePermChoice:
    WRITE = "write"
    READ = "read"
    NONE = "none"


def solution_log(content):
    logger.log(SOLUTION_LEVEL, f"visit \033[1;32m {content} \033[0m for detailed solution")  # green content


def solution_log_win(content):
    logger.log(SOLUTION_LEVEL_WIN, f"visit {content} for detailed solution")


def is_platform(platform: str):
    return sys.platform.startswith(platform)


def is_legal_path_length(path):
    if len(path) > MAX_LINUX_ABS_PATH_LENGTH and not is_platform("win"):  # linux total path length limit
        logger.error(f"file total path's length out of range ({MAX_LINUX_ABS_PATH_LENGTH}), " + \
                     "please check the file(or directory) path")
        solution_log(SOLUTION_BASE_LOC + PATH_LENGTH_SUB_CHAPTER)
        return False

    if len(path) > MAX_WIN_ABS_PATH_LENGTH and is_platform("win"):  # windows total path length limit
        logger.error(f"file total path's length out of range ({MAX_WIN_ABS_PATH_LENGTH}), " + \
                     "please check the file(or directory) path")
        solution_log_win(SOLUTION_BASE_LOC + PATH_LENGTH_SUB_CHAPTER)
        return False

    dirnames = path.split("/")
    for dirname in dirnames:
        if len(dirname) > MAX_LINUX_BASE_NAME_LENGTH:  # linux single file path length limit
            logger.error(f"file base name length out of range ({MAX_LINUX_BASE_NAME_LENGTH}), " + \
                         "please check the file(or directory) path")
            solution_log(SOLUTION_BASE_LOC + PATH_LENGTH_SUB_CHAPTER)
            return False
    return True


def is_match_path_white_list(path):
    if PATH_WHITE_LIST_REGEX.search(path) and not is_platform("win"):
        logger.error(f"path contains illegal char, legal chars include A-Z a-z 0-9 _ - / .")
        solution_log(SOLUTION_BASE_LOC + ILLEGAL_CHAR_SUB_CHAPTER)
        return False
    if PATH_WHITE_LIST_REGEX_WIN.search(path) and is_platform("win"):
        logger.error(f"path contains illegal char, legal chars include A-Z a-z 0-9 _ - / . : \\")
        solution_log_win(SOLUTION_BASE_LOC + ILLEGAL_CHAR_SUB_CHAPTER)
        return False
    return True


def is_legal_args_path_string(path):
    # only check path string
    if not path:
        return True
    if not is_legal_path_length(path):
        return False
    if not is_match_path_white_list(path):
        return False
    return True


class OpenException(Exception):
    pass


class FileStat:
    def __init__(self, file) -> None:
        self.file = os.path.abspath(file)
        if not is_legal_path_length(self.file) or not is_match_path_white_list(self.file):
            raise OpenException(f"create FileStat failed")
        self.is_file_exist = os.path.exists(self.file)
        if self.is_file_exist:
            self.file_stat = os.stat(self.file)
            self.realpath = os.path.realpath(self.file)
        else:
            self.file_stat = None

    @property
    def is_exists(self):
        return self.is_file_exist

    @property
    def is_softlink(self):
        return os.path.islink(self.file) if self.file_stat else False

    @property
    def is_file(self):
        return stat.S_ISREG(self.file_stat.st_mode) if self.file_stat else False

    @property
    def is_dir(self):
        return stat.S_ISDIR(self.file_stat.st_mode) if self.file_stat else False

    @property
    def file_size(self):
        return self.file_stat.st_size if self.file_stat else 0

    @property
    def permission(self):
        return stat.S_IMODE(self.file_stat.st_mode) if self.file_stat else 0o777

    @property
    def owner(self):
        return self.file_stat.st_uid if self.file_stat else -1

    @property
    def group_owner(self):
        return self.file_stat.st_gid if self.file_stat else -1

    @property
    def is_owner(self):
        return self.owner == (os.geteuid() if hasattr(os, "geteuid") else 0)

    @property
    def is_group_owner(self):
        return self.group_owner in (os.getgroups() if hasattr(os, "getgroups") else [0])

    @property
    def is_user_or_group_owner(self):
        return self.is_owner or self.is_group_owner

    @property
    def is_user_and_group_owner(self):
        return self.is_owner and self.is_group_owner

    def is_basically_legal(self, perm=FilePermChoice.NONE):
        if is_platform("win"):
            return self.check_windows_permission(perm)
        else:
            return self.check_linux_permission(perm)

    def check_linux_permission(self, perm=FilePermChoice.NONE):
        if not self.is_exists and perm != FilePermChoice.WRITE:
            logger.error(f"path: {self.file} not exist, please check if file or dir is exist")
            return False
        if self.is_softlink:
            logger.error(f"path :{self.file} is a soft link, not supported, please import file(or directory) directly")
            solution_log(SOLUTION_BASE_LOC + SOFT_LINK_SUB_CHAPTER)
            return False
        if not self.is_user_or_group_owner and self.is_exists:
            logger.error(
                f"current user isn't path:{self.file}'s owner or ownergroup, make sure current user belong to file(or directory)'s owner or ownergroup"
            )
            solution_log(SOLUTION_BASE_LOC + OWNER_SUB_CHAPTER)
            return False
        if perm == FilePermChoice.READ:
            if self.permission & READ_FILE_NOT_PERMITTED_STAT > 0:
                logger.error(
                    f"The file {self.file} is group writable, or is others writable, as import file(or directory), "
                    "permission should not be over 0o755(rwxr-xr-x)"
                )
                solution_log(SOLUTION_BASE_LOC + PERMISSION_SUB_CHAPTER)
                return False
            if not os.access(self.realpath, os.R_OK) or self.permission & stat.S_IRUSR == 0:
                logger.error(
                    f"Current user doesn't have read permission to the file {self.file}, as import file(or directory), "
                    "permission should be at least 0o400(r--------) "
                )
                solution_log(SOLUTION_BASE_LOC + PERMISSION_SUB_CHAPTER)
                return False
        elif perm == FilePermChoice.WRITE and self.is_exists:
            if self.permission & WRITE_FILE_NOT_PERMITTED_STAT > 0:
                logger.error(
                    f"The file {self.file} is group writable, or is others writable, as export file(or directory), "
                    "permission should not be over 0o750(rwxr-x---)"
                )
                solution_log(SOLUTION_BASE_LOC + PERMISSION_SUB_CHAPTER)
                return False
            if not os.access(self.realpath, os.W_OK):
                logger.error(
                    f"Current user doesn't have write permission to the file {self.file}, as export file(or directory), "
                    "permission should be at least 0o200(-w-------) "
                )
                solution_log(SOLUTION_BASE_LOC + PERMISSION_SUB_CHAPTER)
                return False
        return True

    def check_windows_permission(self, perm=FilePermChoice.NONE):
        if not self.is_exists and perm != FilePermChoice.WRITE:
            logger.error(f"path: {self.file} not exist, please check if file or dir is exist")
            return False
        if self.is_softlink:
            logger.error(f"path :{self.file} is a soft link, not supported, please import file(or directory) directly")
            solution_log(SOLUTION_BASE_LOC + SOFT_LINK_SUB_CHAPTER)
            return False
        return True

    def is_legal_file_size(self, max_size):
        if not self.is_file:
            logger.error(f"path: {self.file} is not a file")
            return False
        if self.file_size > max_size:
            logger.error(f"file_size:{self.file_size} byte out of max limit {max_size} byte")
            return False
        else:
            return True

    def is_legal_file_type(self, file_types: list):
        if not self.is_file and self.is_exists:
            logger.error(f"path: {self.file} is not a file")
            return False
        for file_type in file_types:
            if os.path.splitext(self.file)[1] == f".{file_type}":
                return True
        logger.error(f"path:{self.file}, file type not in {file_types}")
        return False


def ms_open(file, mode="r", max_size=None, softlink=False, write_permission=PERMISSION_NORMAL, **kwargs):
    file_stat = FileStat(file)

    if file_stat.is_exists and file_stat.is_dir:
        raise OpenException(f"Expecting a file, but it's a folder. {file}")

    if not softlink and file_stat.is_softlink:
        raise OpenException(f"Softlink is not allowed to be opened. {file}")

    if "r" in mode:
        if not file_stat.is_exists:
            raise OpenException(f"No such file or directory {file}")
        if max_size is None:
            raise OpenException(f"Reading files must have a size limit control. {file}")
        if max_size != MAX_SIZE_UNLIMITED and max_size < file_stat.file_size:
            raise OpenException(f"The file size has exceeded the specifications and cannot be read. {file}")

    if "w" in mode:
        if file_stat.is_exists and not file_stat.is_owner:
            raise OpenException(
                f"The file owner is inconsistent with the current process user and is not allowed to write. {file}"
            )
        if file_stat.is_exists:
            try:
                os.remove(file)
            except Exception as err:
                raise PermissionError(f"current user can't remove {file}!") from err

    if "a" in mode:
        if not file_stat.is_owner:
            raise OpenException(
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
    return os.fdopen(os.open(file, flags, mode=write_permission), mode, **kwargs)  # ms_open函数中，file在之前已经完成校验了


def check_normal_string(str_to_check):
    if not str_to_check:
        return
    if NORMAL_STR_WHITE_LIST_REGEX.search(str_to_check):
        raise ValueError(f"string: {str_to_check} contain illegal char")


def check_path_legality(path, perm=FilePermChoice.WRITE, max_size=MAX_SIZE_LIMITED_CONFIG_FILE, is_file=True,
                        suffix=None):
    try:
        file_stat = FileStat(path)
    except Exception as err:
        raise ValueError(f"The path string is illegal. Please check.") from err
    if is_file != file_stat.is_file:
        if is_file:
            raise ValueError(f"The path:{path} is not a file.")  # check path string content when init FileStat
        else:
            raise ValueError(f"The path:{path} is not a directory.")  # check path string content when init FileStat
    if not file_stat.is_basically_legal(perm):
        raise ValueError(f"The path:{path} is illegal. Please check the error log for more detail.")
    if suffix and file_stat.is_file and not file_stat.is_legal_file_type(suffix):
        raise ValueError(f"The suffix of path:{path} not in {suffix}. Please check.")
    if file_stat.is_file and not file_stat.is_legal_file_size(max_size):
        raise ValueError(f"The file:{path} size is larger than {max_size}. Please check.")


def makedirs_safe(path, mode=0o750):
    # only linux
    abs_path = os.path.abspath(path)
    parts = abs_path.split("/")
    current_path = ""
    for _, part in enumerate(parts):
        if not part:
            continue
        current_path += "/" + part
        if os.path.exists(current_path):
            continue
        try:
            os.mkdir(current_path, mode=mode)
        except Exception as err:
            raise RuntimeError(f"can not make dir: {current_path}") from err
