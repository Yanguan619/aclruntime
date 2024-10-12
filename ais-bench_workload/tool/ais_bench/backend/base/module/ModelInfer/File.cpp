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


static std::string File::GetFullPath(const std::string &originPath)
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
    if (path.empty()) {
        ERROR_LOG("path is empty");
        return false;
    }
    std::string absPath = GetAbsPath(path);
    return chmod(absPath.c_str(), mode) == 0;
}

bool File::Access(const std::string &path, const mode_t &mode)
{
    if (path.empty()) {
        ERROR_LOG("path is empty");
        return false;
    }
    std::string absPath = GetAbsPath(path);
    return access(absPath.c_str(), mode) == 0;
}

bool File::DeleteFile(const std::string &path)
{
    if (path.empty()) {
        ERROR_LOG("path is empty");
        return false;
    }
    std::string absPath = GetAbsPath(path);
    return unlink(absPath.c_str()) == 0;
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

size_t File::GetFileSize(const std::string &path)
{
    struct stat path_stat;
    if (stat(path.c_str(), &path_stat) != 0) {
        ERROR_LOG("file not exists");
        return -1;
    }
    return static_cast<size_t>(path_stat.st_size);
}

std::string File::GetFileName(const std::string& path)
{
    size_t found = path.find_last_of('/');
    if (found != std::string::npos) {
        return path.substr(found + 1);
    }
    return path;
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

bool File::IsExist(const std::string &path)
{
    return Access(GetAbsPath(path), F_OK);
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

bool File::IsFile(const std::string &path)
{
    if (path.empty()) {
        ERROR_LOG("path is empty");
        return false;
    }
    std::string absPath = GetAbsPath(path);
    struct stat fileStat;
    if (lstat(absPath.c_str(), &fileStat) != 0) {
        ERROR_LOG("the file lstat failed");
        return false;
    }
    return fileStat.st_mode & S_IFREG;
}

uint64_t File::Size(const std::string &path)
{
    if (path.empty()) {
        ERROR_LOG("path is empty");
        return 0;
    }
    struct stat fileStat;
    if (lstat(path.c_str(), &fileStat) != 0) {
        ERROR_LOG("the file lstat failed");
        return 0;
    }
    return fileStat.st_size;
}

bool File::CheckDir(const std::string &path)
{
    if (path.empty()) {
        ERROR_LOG("path is empty");
        return false;
    }
    std::string absPath = GetAbsPath(path);
    if (absPath.size() > MAX_PATH_SIZE) {
        ERROR_LOG("path length invalid");
        return false;
    }
    if (StringChecker::HasInvalidChar(absPath)) {
        ERROR_LOG("path is invalid");
        return false;
    }
    if (!IsExist(absPath)) {
        ERROR_LOG("path is not exists");
        return false;
    }
    if (IsSoftLink(absPath)) {
        ERROR_LOG("path is soft link");
        return false;
    }
    if (CheckOwner(absPath)) {
        ERROR_LOG("check usr owner error");
        return false;
    }
    if (Access(absPath, DIR_CHECK_MODE)) {
        ERROR_LOG("path have no rwx access");
        return false;
    }
    return true;
}

bool File::CreateDir(const std::string &path, const mode_t &mode)
{
    if (path.empty()) {
        ERROR_LOG("path is empty");
        return false;
    }
    std::string absPath = GetAbsPath(path);
    if (IsExist(absPath)) {
        if (IsSoftLink(absPath)) {
            ERROR_LOG("path is soft link");
            return false;
        }
        if (Access(absPath, DIR_CHECK_MODE)) {
            ERROR_LOG("path have no rwx access");
            return false;
        }
        return true;
    }
    if (mkdir(absPath.c_str(), mode) != 0) {
        ERROR_LOG("create path failed");
        return false;
    }
    return true;
}

bool File::RemoveDir(const std::string &path, int depth)
{
    if (depth >= MAX_DEPTH) {
        ERROR_LOG("The maximum recursion depth is exceeded");
        return false;
    }
    std::string absPath = GetAbsPath(path);
    if (!File::CheckFile(absPath)) {
        ERROR_LOG("check path error");
        return false;
    }
    DIR *dir = opendir(absPath.c_str());
    if (dir == nullptr) {
        ERROR_LOG("open dir failed");
        return false;
    }
    const struct dirent *entry = nullptr;
    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        auto subPath = absPath + "/" + entry->d_name;
        struct stat st;
        if (lstat(subPath.c_str(), &st) == -1) {
            closedir(dir);
            ERROR_LOG("check lstat error");
            return false;
        }
        if (S_ISDIR(st.st_mode)) {
            if (!RemoveDir(subPath, depth + 1)) {
                closedir(dir);
                ERROR_LOG("remove child dir error");
                return false;
            }
            rmdir(subPath.c_str());
        } else {
            remove(subPath.c_str());
        }
    }
    closedir(dir);
    if (absPath.c_str() != 0) {
        ERROR_LOG("remove dir failed");
        return false;
    }
    return true;
}

bool File::CheckFile(const std::string &path, uint64_t maxReadFileBytes, const int &mode)
{
    if (path.empty()) {
        ERROR_LOG("path is empty");
        return false;
    }
    std::string absPath = GetAbsPath(path);
    if (absPath.size() > FULL_PATH_LENGTH_MAX) {
        ERROR_LOG("path length invalid");
        return false;
    }
    if (IsSoftLink(absPath)) {
        ERROR_LOG("path is soft link");
        return false;
    }
    if (CheckOwner(absPath)) {
        ERROR_LOG("check usr owner error");
        return false;
    }
    if (Access(absPath, mode)) {
        ERROR_LOG("check permission error");
        return false;
    }
    if (Size(absPath) > maxReadFileBytes) {
        ERROR_LOG("file size invalid");
        return false;
    }
    return true;
}

bool File::CheckFileBeforeRead(const std::string &path, const std::string &authority, FileType type)
{
    std::string realPath = GetAbsPath(path);
    if (realPath.empty()) {
        ERROR_LOG("path is empty");
        return false;
    }
    if (!IsPathExist(realPath)) {
        ERROR_LOG("path not exist");
        return false;
    }
    if (!IsRegularFile(realPath)) {
        ERROR_LOG("path is not regular file");
        return false;
    }
    if (!IsPathLengthLegal(realPath)) {
        ERROR_LOG("path length illegal");
        return false;
    }
    if (!IsPathCharactersValid(realPath)) {
        ERROR_LOG("path characters invalid");
        return false;
    }
    if (!IsPathDepthValid(realPath)) {
        ERROR_LOG("path depth invalid");
        return false;
    }
    if (IsSoftLink(realPath)) {
        ERROR_LOG("path is soft link");
        return false;
    }
    if (!CheckFileRWX(realPath, authority)) {
        ERROR_LOG("path permission error");
        return false;
    }
    /* 如果是/dev/random之类的无法计算size的文件，不要用本函数check */
    if (!CheckFileSuffixAndSize(path, type)) {
        ERROR_LOG("path suffix and size invalid");
        return false;
    }

    return true;
}

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

// std::string File::GetAbsPath(const std::string &path)
// {
//     char buffer[MAX_PATH_SIZE];
//     char* result = realpath(path.c_str(), buffer);
//     if (result == nullptr) {
//         ERROR_LOG("convert absPath error");
//         return "";
//     }
//     return std::string(buffer);
// }