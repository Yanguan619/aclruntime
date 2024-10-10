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

import stat


# 文件/文件夹不能有的权限
class PermForbid:
    USER_MAIN_DIR = stat.S_IWGRP | stat.S_IRWXO  # 0o750
    PROGRAM_FILE = stat.S_IWUSR | stat.S_IWGRP | stat.S_IRWXO  # 0o550
    PROGRAM_DIR = stat.S_IWUSR | stat.S_IWGRP | stat.S_IRWXO  # 0o550
    CONFIG_FILE = stat.S_IXUSR | stat.S_IWGRP | stat.S_IXGRP | stat.S_IWGRP | stat.S_IRWXO  # 0o640
    CONFIG_DIR = stat.S_IWGRP | stat.S_IRWXO  # 0o750
    LOG_FILE_DONE = stat.S_IWUSR | stat.S_IXUSR | stat.S_IWGRP | stat.S_IXGRP | stat.S_IRWXO  # 0o440
    LOG_FILE_RECORDING = stat.S_IXUSR | stat.S_IWGRP | stat.S_IXGRP | stat.S_IWGRP | stat.S_IRWXO  # 0o640
    LOG_DIR = stat.S_IWGRP | stat.S_IRWXO  # 0o750
    DEBUG_FILE = stat.S_IXUSR | stat.S_IWGRP | stat.S_IXGRP | stat.S_IWGRP | stat.S_IRWXO  # 0o640
    DEBUG_DIR = stat.S_IWGRP | stat.S_IRWXO  # 0o750
    TMP_DIR = stat.S_IWGRP | stat.S_IRWXO  # 0o750
    UPGRADE_DIR = stat.S_IRWXO  # 0o770
    DATA_FILE = stat.S_IXUSR | stat.S_IWGRP | stat.S_IXGRP | stat.S_IWGRP | stat.S_IRWXO  # 0o640
    DATA_DIR = stat.S_IWGRP | stat.S_IRWXO  # 0o750
    SECRET_DIR = stat.S_IRWXG | stat.S_IRWXO  # 0o700
    SECRET_FILE = stat.S_IXUSR | stat.S_IRWXG | stat.S_IRWXO  # 0o600
    ENCODE_AND_DECODE_SCRIPTS = stat.S_IWUSR | stat.S_IRWXG | stat.S_IRWXO  # 0o500


# 文件/文件夹最低需要的权限，按类型区分
class PermNeed:
    READ_FILE = stat.S_IREAD  # 当前用户读权限
    WRITE_FILE = stat.S_IWRITE  # 当前用户写权限
    EXEC_FILE = stat.S_IEXEC  # 当前用户执行权限


# 文件大小限制
class FileSizeLimit:
    UNLIMITED = -1  # 不限制，必须显式表示不限制，读取必须传入
    NORMAL_CONFIG_FILE = 10485760  # 10MB, 10 * 1024 * 1024
    NORMAL_READ_FILE_4G = 4294967296  # 4GB, 4 * 1024 * 1024 * 1024
    NORMAL_READ_FILE_32G = 34359738368  # 32GB, 32 * 1024 * 1024 * 1024


# 操作系统
class PlatformSupport:
    LINUX = "lin"
    WINDOWS = "win"
    UNKNOWN = "null"


# 路径相关字符串长度限制
class PathLengthLimit:
    LINUX_TOTAL_LENGTH = 4096  # linux路径总长
    WIN_TOTAL_LENGTH = 260  # windows默认路径总长
    SINGLE_NAME_LENGTH = 255  # 文件名长度


# 字符串校验的白名单
class StrWhitePattern:
    ABS_PATH_LINUX = r"[^_A-Za-z0-9/.-]"  # linux绝对路径
    ABS_PATH_WIN = r"[^_:\\A-Za-z0-9/.-]"  # windows绝对路径
    NORMAL_STR = r"_A-Za-z0-9\"'><=\[\])(,}{: /.~-"  # 常规字符串


# 字符串校验的黑名单
class StrBlackPattern:
    NORMAL_STR = r"\&\%\$\*\^\#\@\\\n\f\r\b\t\v\u007F"


# 危险命令黑名单
class CommandBlackList:
    NORMAL_LINUX = [
        "rm", "mv", ">", "mkfs", "dd",
        "chown", "chmod",
        "shutdown", "reboot",
        "curl", "wget",
    ]


ZIP_DECOMPRESSED_RATIO_LIMIT = 4  # 逆压缩率限制（解压后大小/压缩包大小）
