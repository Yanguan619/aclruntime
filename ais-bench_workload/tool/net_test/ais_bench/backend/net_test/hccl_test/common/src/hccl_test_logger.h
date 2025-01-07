/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
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

#ifndef HCCL_TEST_LOGGER_H_
#define HCCL_TEST_LOGGER_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>


#ifdef _WIN32
#define TRIM_FILE_PATH(x) strrchr((x), '\\') ? strrchr((x), '\\') + 1 : (x)
#else  // Linux/Unix
#define TRIM_FILE_PATH(x) strrchr((x), '/') ? strrchr((x), '/') + 1 : (x)
#endif

#define LOG_COLOR_NONE_ "\033[0m"
#define LOG_COLOR_RED_ "\033[0;31m"
#define LOG_COLOR_LIGHT_RED_ "\033[1;31m"
#define LOG_COLOR_YELLOW_ "\033[0;33m"
#define LOG_COLOR_LIGHT_YELLOW_ "\033[1;33m"
#define LOG_COLOR_GREEN_ "\033[0;32m"
#define LOG_COLOR_LIGHT_GREEN_ "\033[1;32m"

#define COLORFUL_PRINT_(color, fmt, ...)                                        \
    do {                                                                         \
        printf(color fmt LOG_COLOR_NONE_, ##__VA_ARGS__);                            \
    } while (0)

#define LOG_ORIGIN(fmt, ...)         \
    do {                               \
        printf(fmt "\n", ##__VA_ARGS__); \
    } while (0)

const int BASE_YEAR = 1900;
const std::string SELF_MODULE_NAME = "HCCL_TEST";

static std::string getCurrentTime()
{
    // 获取当前时间点
    auto now = std::chrono::system_clock::now();

    // 转换为time_t以便使用gmtime
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    // 将time_t格式的时间转换为tm结构（UTC时间）
    std::tm *ptm = std::gmtime(&now_c);

    // 使用ostringstream进行格式化
    std::ostringstream oss;
    oss << std::put_time(ptm, "%Y-%m-%d %H:%M:%S");

    // 返回格式化的UTC时间字符串
    return oss.str();
}


#define PRINT_LOG(color, mode,  fmt, ...)                                             \
    do {                                                                         \
        COLORFUL_PRINT_(color, "[%s][%s][%s]" fmt "\n",                     \
                      getCurrentTime().c_str(), std::string(mode).c_str(), SELF_MODULE_NAME.c_str(), ##__VA_ARGS__);           \
    } while (0)

#define PRINT_LOG_DEBUG(color, mode, fmt, ...)                                             \
    do {                                           \
        COLORFUL_PRINT_(color, "[%s][%s][%s][%s(%d)][%s()]: " fmt "\n",     \
                      getCurrentTime().c_str(), std::string(mode).c_str(), SELF_MODULE_NAME.c_str(), __FILE__, __LINE__, \
                      __FUNCTION__, ##__VA_ARGS__);                               \
    } while (0)

#define LOG_DEBUG 1
#define LOG_INFO 2
#define LOG_WARNING 3
#define LOG_ERROR 4
#define LOG_DEBUG_STR "1"
#define LOG_INFO_STR "2"
#define LOG_WARNING_STR "3"
#define LOG_ERROR_STR "4"

static int GetLogLevel(void)
{
    int logLevel = LOG_INFO;
    char* logLevelEnv = getenv("AISBENCH_CL_TEST_LOG_LEVEL");
    if (logLevelEnv != nullptr) {
        std::string logLevelStr = std::string(logLevelEnv);
        if (logLevelStr == LOG_DEBUG_STR || logLevelStr == LOG_INFO_STR ||
            logLevelStr == LOG_WARNING_STR || logLevelStr == LOG_ERROR_STR) {
            logLevel = std::stoi(logLevelStr);
        }
    }
    return logLevel;
}

static int g_frizy_log_level = GetLogLevel();

#define PRINT_DBG_(mode, fmt, ...) PRINT_LOG_DEBUG(LOG_COLOR_LIGHT_GREEN_, mode, fmt, ##__VA_ARGS__)
#define PRINT_INFO_(mode, fmt, ...) PRINT_LOG(LOG_COLOR_NONE_, mode, fmt, ##__VA_ARGS__)
#define PRINT_WARN_(mode, fmt, ...) PRINT_LOG(LOG_COLOR_LIGHT_YELLOW_, mode, fmt, ##__VA_ARGS__)
#define PRINT_ERR_(mode, fmt, ...) PRINT_LOG(LOG_COLOR_LIGHT_RED_, mode, fmt, ##__VA_ARGS__)

#define LOG(level, fmt, ...) do {                      \
    if (level == LOG_DEBUG) { if (LOG_DEBUG >= g_frizy_log_level) {PRINT_DBG_("DEBUG", fmt, ##__VA_ARGS__);} \
    } else if (level == LOG_INFO) { if (LOG_INFO >= g_frizy_log_level) {PRINT_INFO_("INFO", fmt, ##__VA_ARGS__);} \
    } else if (level == LOG_WARNING) { if (LOG_WARNING >= g_frizy_log_level) {PRINT_WARN_("WARN", fmt, ##__VA_ARGS__);} \
    } else if (level == LOG_ERROR) { if (LOG_ERROR >= g_frizy_log_level) {PRINT_ERR_("ERROR", fmt, ##__VA_ARGS__);} \
    } else {                                                     \
    }                                                            \
} while (0)


#define SETLOGLEVEL(level) {g_frizy_log_level = level;}

#define DEBUG(fmt, ...) LOG(LOG_DEBUG, fmt, ##__VA_ARGS__)
#define INFO(fmt, ...) LOG(LOG_INFO, fmt, ##__VA_ARGS__)
#define WARN(fmt, ...) LOG(LOG_WARNING, fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...) LOG(LOG_ERROR, fmt, ##__VA_ARGS__)


#endif  // __HCCL_TEST_LOGGER_H_
