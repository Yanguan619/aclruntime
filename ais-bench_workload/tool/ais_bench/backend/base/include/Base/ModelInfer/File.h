/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024. All rights reserved.
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
#include <fcntl.h>
#include <cctype>

constexpr int DIR_CHECK_MODE = R_OK | W_OK | X_OK;
constexpr const char PATH_SEPARATOR = '/';

constexpr const uint32_t FULL_PATH_LENGTH_MAX = 4096;
constexpr const uint32_t FILE_NAME_LENGTH_MAX = 255;
constexpr const uint32_t PATH_DEPTH_MAX = 32;

constexpr size_t MAX_NUMPY_SIZE = 10ULL * 1024 * 1024 * 1024;
constexpr size_t MAX_JSON_SIZE = 1024ULL * 1024 * 1024;
constexpr size_t MAX_CSV_SIZE = 1024ULL * 1024 * 1024;
constexpr size_t MAX_OM_SIZE = 64ULL * 1024 * 1024 * 1024;
constexpr size_t MAX_FILE_SIZE_DEFAULT = 10ULL * 1024 * 1024 * 1024;

constexpr mode_t NORMAL_FILE_MODE_DEFAULT = 0640;
constexpr mode_t READONLY_FILE_MODE_DEFAULT = 0440;
constexpr mode_t SCRIPT_FILE_MODE_DEFAULT = 0550;
constexpr mode_t NORMAL_DIR_MODE_DEFAULT = 0750;
constexpr mode_t READ_FILE_NOT_PERMITTED = S_IWGRP | S_IWOTH;
constexpr mode_t WRITE_FILE_NOT_PERMITTED = S_IWGRP | S_IWOTH | S_IROTH | S_IXOTH;
constexpr mode_t CREATE_FILE_MODE_DEFAULT = O_EXCL | O_CREAT;

enum class FileType {
    NUMPY,
    JSON,
    CSV,
    OM,

    /* Add new type before this line. */
    COMMON
};

// File 类主要处理文件相关操作
class File {
public:
    File() = default;
    virtual ~File() = default;
    // 安全的创建目录
    static bool CreateDir(const std::string &path, bool recursion=false, mode_t mode = NORMAL_DIR_MODE_DEFAULT);
    // 打开文件
    static bool OpenFile(const std::string& path, std::ifstream& ifs, std::ios::openmode mode = std::ios::in);
    static bool OpenFile(const std::string& path, std::ofstream& ofs, std::ios::openmode mode = std::ios::out);
    // 文件夹校验：包括路径长度，文件存在性，软链接，属组，权限
    static bool CheckDir(const std::string &path);
    // 文件权限修改
    static bool Chmod(const std::string &path, const mode_t &mode);
    // 文件权限校验
    static bool IsFileReadable(const std::string& path);
    static bool IsFileWritable(const std::string& path);
    static bool IsFileExecutable(const std::string& path);
    static bool CheckFileRWX(const std::string& path, const std::string& permissions);
    static bool IsDirReadable(const std::string& path);
    // 文件存在性校验
    static bool IsPathExist(const std::string& path);
    // 软链接校验
    static bool IsSoftLink(const std::string &path);
    // 校验是否是文件夹
    static bool IsDir(const std::string& path);
    // 安全删除文件的形式
    static bool DeleteFile(const std::string &path);
    // 校验文件属组
    static bool CheckOwner(const std::string &path);
    // 获取文件父目录
    static std::string GetParentDir(const std::string& path);
    // 获取绝对路径
    static std::string GetFullPath(const std::string &originPath);
    static std::string GetAbsPath(const std::string &path);
    // 获取文件大小
    static size_t GetFileSize(const std::string &path);
    // 路径长度校验
    static bool IsPathLengthLegal(const std::string& path);
    // 路径字符校验
    static bool IsPathCharactersValid(const std::string& path);
    // 路径深度校验
    static bool IsPathDepthValid(const std::string& path);
    // 常规文件校验
    static bool IsRegularFile(const std::string& path);
    // 获取文件名
    static std::string GetFileName(const std::string& path);
    // 获取文件权限
    static mode_t GetFilePermissions(const std::string& path);
    // 获取文件后缀
    static std::string GetFileSuffix(const std::string& path);
    // 校验文件后缀和内容长度
    static bool CheckFileSuffixAndSize(const std::string &path, FileType type);
    // 读文件前的校验
    static bool CheckFileBeforeRead(const std::string &path, FileType type = FileType::COMMON);
    static bool CheckFileBeforeCreateOrWrite(const std::string &path, bool overwrite = false);
};

#endif // MS_SAFE_CHECK_BASE_FILE_H