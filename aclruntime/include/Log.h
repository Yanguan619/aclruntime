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

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <cstdio>
#include <cstdarg>
#include <csignal>
#include <fstream>
#include <functional>
#include <sstream>
#include <string>
#include <utility>

#define FILELINE __FILE__, __FUNCTION__, __LINE__

#define LOG_DEBUG_LEVEL SPDLOG_LEVEL_DEBUG
#define LOG_INFO_LEVEL SPDLOG_LEVEL_INFO
#define LOG_WARNING_LEVEL SPDLOG_LEVEL_WARN
#define LOG_ERROR_LEVEL SPDLOG_LEVEL_ERROR

#define LOG_BUF_SIZE 4096

namespace Base {
class LogCtrl {
public:
    static void Init();
    static void SetLogLevel(int level) {
        spdlog::set_level(static_cast<spdlog::level::level_enum>(level));
    }
    static bool CheckLogLevel(int level) {
        return level >= static_cast<int>(spdlog::get_level());
    }

private:
    LogCtrl() = delete;
    ~LogCtrl() = delete;
};
}  // namespace Base

#define DEBUG_LOG(fmt, ...)                                                    \
    do {                                                                       \
        if (spdlog::should_log(spdlog::level::debug)) {                        \
            char buf[LOG_BUF_SIZE];                                            \
            snprintf(buf, sizeof(buf), fmt, ##__VA_ARGS__);                    \
            spdlog::default_logger_raw()->log(                                 \
                spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},       \
                spdlog::level::debug, "{}", buf);                              \
        }                                                                      \
    } while (0)

#define INFO_LOG(fmt, ...)                                                     \
    do {                                                                       \
        if (spdlog::should_log(spdlog::level::info)) {                         \
            char buf[LOG_BUF_SIZE];                                            \
            snprintf(buf, sizeof(buf), fmt, ##__VA_ARGS__);                    \
            spdlog::default_logger_raw()->log(                                 \
                spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},       \
                spdlog::level::info, "{}", buf);                               \
        }                                                                      \
    } while (0)

#define WARN_LOG(fmt, ...)                                                     \
    do {                                                                       \
        if (spdlog::should_log(spdlog::level::warn)) {                         \
            char buf[LOG_BUF_SIZE];                                            \
            snprintf(buf, sizeof(buf), fmt, ##__VA_ARGS__);                    \
            spdlog::default_logger_raw()->log(                                 \
                spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},       \
                spdlog::level::warn, "{}", buf);                               \
        }                                                                      \
    } while (0)

#define ERROR_LOG(fmt, ...)                                                    \
    do {                                                                       \
        if (spdlog::should_log(spdlog::level::err)) {                          \
            char buf[LOG_BUF_SIZE];                                            \
            snprintf(buf, sizeof(buf), fmt, ##__VA_ARGS__);                    \
            spdlog::default_logger_raw()->log(                                 \
                spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},       \
                spdlog::level::err, "{}", buf);                                \
        }                                                                      \
    } while (0)

#define PROMPT_MSG(fmt, ...)                                                   \
    do {                                                                       \
        char buf[LOG_BUF_SIZE];                                                \
        snprintf(buf, sizeof(buf), fmt, ##__VA_ARGS__);                        \
        spdlog::default_logger_raw()->log(                                     \
            spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},           \
            spdlog::level::info, "{}", buf);                                   \
    } while (0)

inline void ACLERR_LOG(const char* ErrMsg) {
    spdlog::log(spdlog::level::err, "[ACL ERROR] {}", ErrMsg);
}

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

#define MEM_CHECKED_CALL(func_call) MemCheckedCall(#func_call, func_call)

#define MEM_CHECKED_VOID_CALL(func_call) MemCheckedCall(#func_call, func_call)

#endif  // ACLRUNTIME_INCLUDE_LOG_H_
