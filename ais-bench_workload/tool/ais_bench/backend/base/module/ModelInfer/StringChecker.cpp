/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
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

#include <iostream>
#include <unordered_map>
#include "Base/ModelInfer/StringChecker.h"

namespace {
const std::vector<std::string> INVALID_CHAR = {
    "\n", "\f", "\r", "\b", "\t", "\v", "\007F", "\000D", "\0008", "\000A", "\000C", "\000B", "\0009",
};
}

bool StringChecker::HasInvalidChar(const std::string &text)
{
    for (const auto &item : INVALID_CHAR) {
        if (text.find(item) != std::string::npos) {
            return true;
        }
    }
    return false;
}

std::vector<std::string> StringChecker::Split(const std::string &str, const std::string &delimiter)
{
    std::vector<std::string> res;
    if (delimiter.empty()) {
        res.emplace_back(str);
        return res;
    }
    size_t start = 0;
    size_t end;
    while ((end = str.find(delimiter, start)) != std::string::npos) {
        res.emplace_back(str.substr(start, end - start));
        start = end + delimiter.length();
    }
    res.emplace_back(str.substr(start));
    return res;
}

bool StringChecker::IsNumber(const std::string &text)
{
    std::string::const_iterator it = text.begin();
    while (it != text.end() && std::isdigit(*it)) ++it;
    return !text.empty() && it == text.end();
}

bool StringChecker::StrToU64(uint64_t &dest, const std::string &numStr)
{
    if (!IsNumber(numStr)) {
        ERROR_LOG("str is not number");
        return false;
    }
    size_t pos = 0;
    try {
        dest = std::stoull(numStr, &pos);
    } catch (...) {
        ERROR_LOG("str to uint64_t failed");
        return false;
    }
    if (pos != numStr.size()) {
        ERROR_LOG("str to uint64_t failed");
        return false;
    }
    return true;
}

std::string StringChecker::Trim(const std::string &text)
{
    std::string result = text;
    size_t pos = result.find_first_not_of(" ");
    if (pos == std::string::npos) {
        return " ";
    }
    result.erase(0, pos);
    result.erase(result.find_last_not_of(" ") + 1);
    return result;
}

bool StringChecker::Startswith(const std::string &text, const std::string word)
{
    if (text.find(word) != 0) {
        ERROR_LOG("str not startswith this word");
        return false;
    }
    return true;
}

bool StringChecker::Endswith(const std::string &text, const std::string word)
{
    if (text.size() < word.size()) {
        ERROR_LOG("word length longer than text");
        return false;
    }
    return (text.compare(text.size() - word.size(), word.size(), word) == 0);
}