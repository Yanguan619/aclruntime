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

#include <map>
#include <iostream>
#include <cstring>
#include <regex>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <unordered_map>
#include "StringChecker.h"
#include "Base/ModelInfer/File.h"


std::string File::GetFullPath(const std::string &originPath)
{
    if (originPath.empty()) {
        return "";
    }
    if (originPath[0] == '/') {
        return originPath;
    }

    char* cwd = nullptr;
    char* cwdBuf = new char[PATH_MAX];
    if (cwdBuf == nullptr) {
        throw std::runtime_error("No memory.");
    }
    cwd = getcwd(cwdBuf, PATH_MAX);
    if (cwd == nullptr) {
        delete[] cwdBuf;
        return "";
    }

    std::string fullPath = std::move(std::string(cwd) + pathSeparator + originPath);
    delete[] cwdBuf;
    cwdBuf = nullptr;

    return fullPath;
}

std::vector<std::string> File::SplitPath(const std::string &path)
{
    std::vector<std::string> tokens;
    size_t len = path.length();
    size_t start = 0;

    while (start < len) {
        size_t end = path.find(pathSeparator, start);
        if (end == std::string::npos) {
            end = len;
        }
        if (start != end) {
            tokens.push_back(path.substr(start, end - start));
        }
        start = end + 1;
    }
    return tokens;
}

std::string File::GetAbsPath(const std::string &originPath) 
{
    std::string fullPath = GetFullPath(originPath);
    if (fullPath.empty()) {
        return "";
    }

    std::vector<std::string> tokens = SplitPath(fullPath);
    std::vector<std::string> tokensRefined;

    for (std::string& token : tokens) {
        if (token.empty() || token == ".") {
            continue;
        } else if (token == "..") {
            if (tokensRefined.empty()) {
                return "";
            }
            tokensRefined.pop_back();
        } else {
            tokensRefined.emplace_back(token);
        }
    }

    if (tokensRefined.empty()) {
        return "/";
    }
    std::string resolvedPath("");
    for (std::string& token : tokensRefined) {
        resolvedPath.append("/").append(token);
    }
    return resolvedPath;
}

bool File::Chmod(const std::string &path, const mode_t &mode)
{
    if (!IsPathExist(path)) {
        ERROR_LOG("path not exist");
        return false;
    }
    if (IsSoftLink(path)) {
        ERROR_LOG("path is soft link");
        return false;
    }
    std::string absPath = GetAbsPath(path);
    if (absPath.empty()) {
        ERROR_LOG("path is empty");
        return false;
    }
    return chmod(absPath.c_str(), mode) == 0;
}

bool File::Access(const std::string &path, const mode_t &mode)
{
    if (path.empty()) {
        ERROR_LOG("path is empty");
        return false;
    }
    return access(path.c_str(), mode) == 0;
}

bool File::IsFileReadable(const std::string& path)
{
    return access(path.c_str(), R_OK) == 0;
}

bool File::IsFileWritable(const std::string& path)
{
    return access(path.c_str(), W_OK) == 0;
}

bool File::IsFileExecutable(const std::string& path)
{
    return (access(path.c_str(), R_OK) == 0) && (access(path.c_str(), X_OK) == 0);
}

bool File::IsDirReadable(const std::string& path)
{
    return (access(path.c_str(), R_OK) == 0) && (access(path.c_str(), X_OK) == 0);
}

bool File::IsPathExist(const std::string& path) 
{
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

bool File::IsPathLengthLegal(const std::string& path)
{
    if (path.length() > FULL_PATH_LENGTH_MAX || path.length() == 0) {
        return false;
    }
    size_t pos = path.find_last_of(pathSeparator);
    if (pos == std::string::npos) {
        pos = 0;
    } else {
        pos = pos + 1;
    }
    if (path.length() - pos > FILE_NAME_LENGTH_MAX) {
        return false;
    }
    return true;
}

bool File::IsPathCharactersValid(const std::string& path)
{
    return std::regex_match(path, std::regex(FILE_VALID_PATTERN));
}

bool File::IsPathDepthValid(const std::string& path)
{
    return std::count(path.begin(), path.end(), pathSeparator) <= PATH_DEPTH_MAX;
}

bool File::CheckFileRWX(const std::string& path, const std::string& permissions)
{
    if (permissions.find('r') != std::string::npos && !IsFileReadable(path)) {
        return false;
    }
    if (permissions.find('w') != std::string::npos && !IsFileWritable(path)) {
        return false;
    }
    if (permissions.find('x') != std::string::npos && !IsFileExecutable(path)) {
        return false;
    }
    return true;
}

bool File::IsRegularFile(const std::string& path)
{
    struct stat path_stat;
    if (stat(path.c_str(), &path_stat) == 0) {
        return S_ISREG(path_stat.st_mode);
    }
    return false;
}

bool File::IsDir(const std::string& path) 
{
    struct stat buffer;
    if (stat(path.c_str(), &buffer) == 0) {
        return (buffer.st_mode & S_IFDIR) != 0;
    }
    return false;
}

size_t File::GetFileSize(const std::string &path)
{
    struct stat path_stat;
    if (stat(path.c_str(), &path_stat) != 0) {
        ERROR_LOG("file not exists");
        return -1;
    }
    return static_cast<size_t>(path_stat.st_size);
}

std::string File::GetParentDir(const std::string& path)
{
    size_t found = path.find_last_of('/');
    if (found != std::string::npos) {
        return path.substr(0, found);
    }
    return ".";
}

std::string File::GetFileName(const std::string& path)
{
    size_t found = path.find_last_of('/');
    if (found != std::string::npos) {
        return path.substr(found + 1);
    }
    return path;
}

mode_t File::GetFilePermissions(const std::string& path)
{
    struct stat path_stat;
    if (stat(path.c_str(), &path_stat) != 0) {
        ERROR_LOG("file not exists");
        return 0777;
    }
    mode_t permissions = path_stat.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
    return permissions;
}

std::string File::GetFileSuffix(const std::string& path)
{
    std::string fileName = GetFileName(path);
    size_t dotPos = fileName.find_last_of('.');
    if (dotPos != std::string::npos && dotPos + 1 < fileName.size()) {
        return fileName.substr(dotPos + 1);
    }
    return "";
}

bool File::CheckFileSuffixAndSize(const std::string &path, FileType type)
{
    static const std::map<FileType, std::pair<std::string, size_t>> FileTypeCheckTbl = {
        {FileType::PKL, {"kpl", MAX_PKL_SIZE}},
        {FileType::NUMPY, {"npy", MAX_NUMPY_SIZE}},
        {FileType::JSON, {"json", MAX_JSON_SIZE}},
        {FileType::PT, {"pt", MAX_PT_SIZE}},
        {FileType::CSV, {"csv", MAX_CSV_SIZE}},
        {FileType::YAML, {"yaml", MAX_YAML_SIZE}},
        {FileType::OM, {"om", MAX_OM_SIZE}},
    };
    
    size_t size = GetFileSize(path);
    if (size < 0) {
        ERROR_LOG("get file size error");
        return false;
    }

    if (type == FileType::COMMON) {
        if (size > MAX_FILE_SIZE_DEFAULT) {
            ERROR_LOG("file size invalid");
            return false;
        }
        return true;
    }

    auto iter = FileTypeCheckTbl.find(type);
    if (iter == FileTypeCheckTbl.end()) {
        ERROR_LOG("unknown file suffix");
        return false;
    }

    std::string suffix = GetFileSuffix(path);
    if (suffix != iter->second.first) {
        ERROR_LOG("unknown file suffix");
        return false;
    }
    if (size > iter->second.second) {
        ERROR_LOG("file size invalid");
        return false;
    }
    return true;
}

bool File::IsSoftLink(const std::string &path)
{
    if (path.empty()) {
        ERROR_LOG("path is empty");
        return false;
    }
    struct stat fileStat;
    if (lstat(path.c_str(), &fileStat) != 0) {
        ERROR_LOG("the file lstat failed");
        return false;
    }
    return S_ISLNK(fileStat.st_mode);
}

/****************** 文件操作函数库，会对入参做基本检查 ************************/
bool File::DeleteFile(const std::string &path)
{
    if (!IsPathExist(path)) {
        WARN_LOG("path not exist");
        return true;
    }
    if (IsSoftLink(path)) {
        ERROR_LOG("path is soft link");
        return false;
    }
    std::string absPath = GetAbsPath(path);
    if (path.empty()) {
        ERROR_LOG("path is empty");
        return false;
    }
    return remove(absPath.c_str()) == 0;
}

bool File::CreateDirAux(const std::string& path, bool recursion, mode_t mode)
{
    std::string parent = GetParentDir(path);

    if (!IsPathExist(parent)) {
        if (!recursion) {
            ERROR_LOG("dir path not exist");
            return false;
        }
        /* 递归创建父目录，由于前面已经判断过目录深度，此处递归是安全的 */
        if (!CreateDirAux(parent, recursion, mode)) {
            ERROR_LOG("recursive creation of parent directory failed");
            return false;
        }
    }

    if (mkdir(path.c_str(), mode) != 0) {
        if (errno == EACCES || errno == EROFS) {
            ERROR_LOG("mkdir permission denined");
            return false;
        } else {
            ERROR_LOG("syscall failed");
            return false;
        }
    }
    return true;
}

bool File::CreateDir(const std::string &path, bool recursion, mode_t mode)
{
    if (IsPathExist(path)) {
        INFO_LOG("dir already exist, no need to create");
        return true;
    }
    std::string absPath = GetAbsPath(path);
    if (absPath.empty()) {
        ERROR_LOG("path is empty");
        return false;
    }
    if (!IsPathLengthLegal(absPath)) {
        ERROR_LOG("path length illegal");
        return false;
    }
    if (!IsPathCharactersValid(absPath)) {
        ERROR_LOG("path characters invalid");
        return false;
    }
    if (!IsPathDepthValid(absPath)) {
        ERROR_LOG("path depth invalid");
        return false;
    }
    return CreateDirAux(absPath, recursion, mode);
}

bool File::OpenFile(const std::string& path, std::ifstream& ifs, std::ios::openmode mode)
{
    std::string absPath = GetAbsPath(path);
    if (!CheckFileBeforeRead(absPath)) {
        ERROR_LOG("before read, path is illegal");
        return false;
    }

    std::ifstream tmpifs(absPath, mode);
    if (!tmpifs.is_open()) {
        ERROR_LOG("file open failed");
        return false;
    }

    ifs = std::move(tmpifs);
    return true;
}

bool File::OpenFile(const std::string& path, std::ofstream& ofs, std::ios::openmode mode)
{
    std::string absPath = GetAbsPath(path);
    if (absPath.empty()) {
        ERROR_LOG("path is empty");
        return false;
    }

    std::string parent = GetParentDir(absPath);
    if (!CheckFileBeforeCreateOrWrite(absPath, true)) {
        ERROR_LOG("before write, path is illegal");
        return false;
    }

    if (!IsPathExist(parent)) {
        if (!CreateDir(parent, true)) {
            ERROR_LOG("path not exist, create failed");
            return false;
        }
    }

    std::ofstream tmpofs(absPath, mode);
    if (!tmpofs.is_open()) {
        ERROR_LOG("file open failed");
        return false;
    }

    ofs = std::move(tmpofs);
    return true;
}

/****************************** 通用检查函数 ********************************/
bool File::CheckOwner(const std::string &path)
{
    std::string absPath = GetAbsPath(path);
    struct stat buf;
    if (stat(absPath.c_str(), &buf)) {
        ERROR_LOG("get file stat failed");
        return false;
    }
    if (buf.st_uid != getuid()) {
        ERROR_LOG("file owner is not process usr");
        return false;
    }
    return true;
}

bool File::CheckDir(const std::string &path)
{
    std::string absPath = GetAbsPath(path);
    if (absPath.empty()) {
        ERROR_LOG("path is empty");
        return false;
    }
    if (!IsPathExist(absPath)) {
        ERROR_LOG("path not exist");
        return false;
    }
    if (!IsDir(absPath)) {
        ERROR_LOG("path is not dir");
        return false;
    }
    if (!IsPathLengthLegal(absPath)) {
        ERROR_LOG("path length illegal");
        return false;
    }
    if (!IsPathCharactersValid(absPath)) {
        ERROR_LOG("path characters invalid");
        return false;
    }
    if (!IsPathDepthValid(absPath)) {
        ERROR_LOG("path depth invalid");
        return false;
    }
    if (IsSoftLink(absPath)) {
        ERROR_LOG("path is soft link");
        return false;
    }
    if (!IsDirReadable(absPath)) {
        ERROR_LOG("path have no rw access");
        return false;
    }
    return true;
}

bool File::CheckFileBeforeRead(const std::string &path, FileType type)
{
    std::string absPath = GetAbsPath(path);
    INFO_LOG("check file before read, abs path: %s", absPath.c_str());
    if (absPath.empty()) {
        ERROR_LOG("path is empty");
        return false;
    }
    if (!IsRegularFile(absPath)) {
        ERROR_LOG("path is not regular file");
        return false;
    }
    if (!IsPathLengthLegal(absPath)) {
        ERROR_LOG("path length illegal");
        return false;
    }
    if (!IsPathCharactersValid(absPath)) {
        ERROR_LOG("path characters invalid");
        return false;
    }
    if (!IsPathDepthValid(absPath)) {
        ERROR_LOG("path depth invalid");
        return false;
    }
    if (IsSoftLink(absPath)) {
        ERROR_LOG("path is soft link");
        return false;
    }
    if ((GetFilePermissions(absPath) & READ_FILE_NOT_PERMITTED) > 0) {
        ERROR_LOG("path permission should not be over 0o755(rwxr-xr-x)");
        return false;
    }
    if (!IsFileReadable(absPath) || (GetFilePermissions(absPath) & S_IRUSR) == 0) {
        ERROR_LOG("path permission should be at least 0o400(r--------)");
        return false;
    }
    /* 如果是/dev/random之类的无法计算size的文件，不要用本函数check */
    if (!CheckFileSuffixAndSize(path, type)) {
        ERROR_LOG("path suffix and size invalid");
        return false;
    }
    return true;
}

bool File::CheckFileBeforeCreateOrWrite(const std::string &path, bool overwrite)
{
    std::string absPath = GetAbsPath(path);
    if (absPath.empty()) {
        ERROR_LOG("path is empty");
        return false;
    }
    if (!IsPathLengthLegal(absPath)) {
        ERROR_LOG("path length illegal");
        return false;
    }
    if (!IsPathCharactersValid(absPath)) {
        ERROR_LOG("path characters invalid");
        return false;
    }
    if (!IsPathDepthValid(absPath)) {
        ERROR_LOG("path depth invalid");
        return false;
    }
    if (IsPathExist(absPath)) {
        if (!overwrite) {
            ERROR_LOG("path already exist and not allow to overwrite");
            return false;
        }
        if ((GetFilePermissions(absPath) & WRITE_FILE_NOT_PERMITTED) > 0) {
            ERROR_LOG("path permission should not be over 0o750(rwxr-x---)");
            return false;
        }
        /* 默认不允许覆盖其他用户创建的文件，若有特殊需求（如多用户通信管道等）由业务自行校验 */
        if (!IsFileWritable(absPath) || !CheckOwner(absPath)) {
            ERROR_LOG("path already create by other owner");
            return false;
        }
    }
    return true;
}
