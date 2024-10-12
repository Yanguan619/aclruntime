/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MS_SAFE_CHECK_BASE_FILE_H
#define MS_SAFE_CHECK_BASE_FILE_H

#include <string>
#include <fstream>
#include <vector>
#include <unistd.h>

constexpr int DIR_CHECK_MODE = R_OK | W_OK | X_OK;
constexpr const uint16_t MAX_PATH_SIZE = 1024;
constexpr const int MAX_DEPTH = 20;
constexpr const char pathSeparator = '/';
constexpr const char* FILE_VALID_PATTERN = "^[a-zA-Z0-9_.:/-]+$";

constexpr const uint32_t FULL_PATH_LENGTH_MAX = 4096;
constexpr const uint32_t FILE_NAME_LENGTH_MAX = 255;
constexpr const uint32_t PATH_DEPTH_MAX = 32;

constexpr size_t MAX_PKL_SIZE = 1024ULL * 1024 * 1024;
constexpr size_t MAX_NUMPY_SIZE = 10ULL * 1024 * 1024 * 1024;
constexpr size_t MAX_JSON_SIZE = 1024ULL * 1024 * 1024;
constexpr size_t MAX_PT_SIZE = 10ULL * 1024 * 1024 * 1024;
constexpr size_t MAX_CSV_SIZE = 1024ULL * 1024 * 1024;
constexpr size_t MAX_YAML_SIZE = 10ULL * 1024 * 1024;
constexpr size_t MAX_FILE_SIZE_DEFAULT = 10ULL * 1024 * 1024 * 1024;

constexpr mode_t NORMAL_FILE_MODE_DEFAULT = 0640;
constexpr mode_t READONLY_FILE_MODE_DEFAULT = 0440;
constexpr mode_t SCRIPT_FILE_MODE_DEFAULT = 0550;
constexpr mode_t NORMAL_DIR_MODE_DEFAULT = 0750;

enum class FileType {
    PKL,
    NUMPY,
    JSON,
    PT,
    CSV,
    YAML,

    /* Add new type before this line. */
    COMMON
};

// File 类主要处理文件相关操作
class File {
public:
    File() = default;
    virtual ~File() = default;
    // 文件校验：包括路径长度，软链接，属组，权限，文件大小。文件上限和属组设置可通过入参传递
    static bool CheckFile(const std::string &path, uint64_t maxReadFileBytes = 64 * 1024 * 1024,
                          const int &mode = DIR_CHECK_MODE);
    // 安全的创建目录的形式：存在文件时校验该文件权限及软链接
    static bool CreateDir(const std::string &path, const mode_t &mode = 0750);
    // 安全的删除目录的形式，同时可设置递归深度，确保不出现无穷递归
    static bool RemoveDir(const std::string &path, int depth);
    // 文件夹校验：包括路径长度，文件存在性，软链接，属组，权限
    static bool CheckDir(const std::string &path);
    // 文件权限修改
    static bool Chmod(const std::string &path, const mode_t &mode);
    // 文件权限校验
    static bool Access(const std::string &path, const mode_t &mode);
    // 文件存在性校验
    static bool IsExist(const std::string &path);
    // 校验是否为文件
    static bool IsFile(const std::string &path);
    // 软链接校验
    static bool IsSoftLink(const std::string &path);
    // 安全删除文件的形式
    static bool DeleteFile(const std::string &path);
    // 校验文件属组
    static bool CheckOwner(const std::string &path);
    // 获取绝对路径
    static std::string GetFullPath(const std::string &originPath);
    static std::vector<std::string> SplitPath(const std::string &path);
    static std::string GetAbsPath(const std::string &path);
    // 获取文件大小
    static uint64_t Size(const std::string &path);
    // 校验文件是否存在
    static bool IsPathExist(const std::string& path);

    static bool IsPathLengthLegal(const std::string& path);

    static bool IsPathCharactersValid(const std::string& path);

    static bool IsPathDepthValid(const std::string& path);

    static bool CheckFileRWX(const std::string& path, const std::string& permissions);

    static bool IsRegularFile(const std::string& path);

    static size_t GetFileSize(const std::string &path);

    static std::string GetFileName(const std::string& path);

    static std::string GetFileSuffix(const std::string& path);

    static bool CheckFileSuffixAndSize(const std::string &path, FileType type);
    // 读文件前的校验
    static bool CheckFileBeforeRead(const std::string &path, const std::string &authority, FileType type);
};

#endif //MS_SAFE_CHECK_BASE_FILE_H