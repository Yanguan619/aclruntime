/*
 * Copyright (c) 2023-2024 Huawei Technologies Co., Ltd.
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
    static std::string GetAbsPath(const std::string &path);
    // 获取文件大小
    static uint64_t Size(const std::string &path);
};

#endif //MS_SAFE_CHECK_BASE_FILE_H