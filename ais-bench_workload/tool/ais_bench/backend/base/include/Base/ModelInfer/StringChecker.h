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

#ifndef MS_SAFE_CHECK_BASE_STRINGCHECKER_H
#define MS_SAFE_CHECK_BASE_STRINGCHECKER_H

#include <string>
#include <vector>
#include <unistd.h>
#include "Base/Log/Log.h"

// StringChecker 主要处理字符串类型相关操作
class StringChecker {
public:
    StringChecker() = default;
    ~StringChecker() = default;

    // 校验字符串中是否包含特殊字符， 包括\t \b等
    static bool HasInvalidChar(const std::string &text);
    // 校验字符串是否为纯数字
    static bool IsNumber(const std::string &text);
    // 字符串转uint64_t 其余类型转换相近
    static bool StrToU64(uint64_t &dest, const std::string &numStr);
    // 校验字符串text是否以word开头
    static bool Startswith(const std::string &text, const std::string word);
    // 校验字符串text是否以word结尾
    static bool Endswith(const std::string &text, const std::string word);
    // 删除字符串开头结尾的空格
    static std::string Trim(const std::string &text);
    // 将str字符串按delimiter的类型进行切割，切割结果以vector形式返回
    static std::vector<std::string> Split(const std::string &str, const std::string &delimiter);
};

#endif // MS_SAFE_CHECK_BASE_STRINGCHECKER_H