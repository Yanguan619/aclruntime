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

#ifndef ACLRUNTIME_INCLUDE_LOG_H_
#define ACLRUNTIME_INCLUDE_LOG_H_

#include <csignal>
#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <functional>
#include <sstream>
#include <string>
#include <utility>  // 新增：提供 std::forward

// Logging macros use printf-style formatting; no std:: prefix needed for C
// headers.

#define FILELINE __FILE__, __FUNCTION__, __LINE__

#define LOG_DEBUG_LEVEL 1
#define LOG_INFO_LEVEL 2
#define LOG_WARNING_LEVEL 3
#define LOG_ERROR_LEVEL 4

namespace Base {
class LogCtrl {
public:
    static void SetLogLevel(int level) { frizyLogLevel = level; }
    static bool CheckLogLevel(int level) { return level >= frizyLogLevel; }

private:
    LogCtrl() = delete;
    ~LogCtrl() = delete;

    static int frizyLogLevel;
};
}  // namespace Base

#define DEBUG_LOG(fmt, args...)                              \
    do {                                                     \
        if (Base::LogCtrl::CheckLogLevel(LOG_DEBUG_LEVEL)) { \
            printf("[DEBUG] " fmt "\n", ##args);             \
            fflush(stdout);                                  \
        }                                                    \
    } while (0)
#define INFO_LOG(fmt, args...)                              \
    do {                                                    \
        if (Base::LogCtrl::CheckLogLevel(LOG_INFO_LEVEL)) { \
            printf("[INFO] " fmt "\n", ##args);             \
            fflush(stdout);                                 \
        }                                                   \
    } while (0)
#define WARN_LOG(fmt, args...)                                 \
    do {                                                       \
        if (Base::LogCtrl::CheckLogLevel(LOG_WARNING_LEVEL)) { \
            printf("[WARN] " fmt "\n", ##args);                \
            fflush(stdout);                                    \
        }                                                      \
    } while (0)
#define ERROR_LOG(fmt, args...)                              \
    do {                                                     \
        if (Base::LogCtrl::CheckLogLevel(LOG_ERROR_LEVEL)) { \
            printf("[ERROR] " fmt "\n", ##args);             \
            fflush(stdout);                                  \
        }                                                    \
    } while (0)
#define PROMPT_MSG(fmt, args...) printf(fmt, ##args)

inline void ACLERR_LOG(const char* ErrMsg) {
    printf("[ACL ERROR] %s\n", ErrMsg);
}

// 内存检查工具 - 自动打印函数调用前后的内存使用情况
// 使用示例：
//   int ret = MEM_CHECKED_CALL("aclmdlLoadWithConfig", aclmdlLoadWithConfig,
//   handle, &modelId_); MEM_CHECKED_VOID_CALL("aclrtSynchronizeStream",
//   aclrtSynchronizeStream, stream);

// 内存使用量查询 - inline 定义在头文件中
inline size_t GetSystemMemoryUsedMB() {
    std::ifstream status("/proc/self/status");
    std::string line;
    while (std::getline(status, line)) {
        if (line.find("VmRSS:") == 0) {
            size_t kb = 0;
            std::istringstream iss(line.substr(6));
            iss >> kb;
            return kb / 1024;
        }
    }
    return 0;
}

// MemCheckedCall - 支持有返回值和无返回值的函数
template <typename Func, typename... Args>
auto MemCheckedCall(const char* funcName, Func&& func, Args&&... args)
    -> decltype(std::forward<Func>(func)(std::forward<Args>(args)...)) {
    size_t before = GetSystemMemoryUsedMB();
    if constexpr (std::is_void_v<decltype(std::forward<Func>(func)(
                      std::forward<Args>(args)...))>) {
        std::forward<Func>(func)(std::forward<Args>(args)...);
        size_t after = GetSystemMemoryUsedMB();
        int64_t delta =
            static_cast<int64_t>(after) - static_cast<int64_t>(before);
        DEBUG_LOG("[MEM_CHECK] %s: before=%zuMB, after=%zuMB, delta=%+ldMB",
                  funcName, before, after, delta);
    } else {
        auto result = std::forward<Func>(func)(std::forward<Args>(args)...);
        size_t after = GetSystemMemoryUsedMB();
        int64_t delta =
            static_cast<int64_t>(after) - static_cast<int64_t>(before);
        DEBUG_LOG("[MEM_CHECK] %s: before=%zuMB, after=%zuMB, delta=%+ldMB",
                  funcName, before, after, delta);
        return result;
    }
}

// 便捷宏 - 自动提取函数名作为字符串
#define MEM_CHECKED_CALL(func_call) MemCheckedCall(#func_call, func_call)

#define MEM_CHECKED_VOID_CALL(func_call) MemCheckedCall(#func_call, func_call)

#endif  // ACLRUNTIME_INCLUDE_LOG_H_
