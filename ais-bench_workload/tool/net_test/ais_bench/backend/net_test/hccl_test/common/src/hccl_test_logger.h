#ifndef __HCCL_TEST_LOGGER_H_
#define __HCCL_TEST_LOGGER_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>


#ifdef _WIN32
#define TrimFilePath(x) strrchr(x, '\\') ? strrchr(x, '\\') + 1 : x
#else  // Linux/Unix
#define TrimFilePath(x) strrchr(x, '/') ? strrchr(x, '/') + 1 : x
#endif

#define LogColorNone_ "\033[0m"
#define LogColorRed_ "\033[0;31m"
#define LogColorLightRed_ "\033[1;31m"
#define LogColorYellow_ "\033[0;33m"
#define LogColorLightYellow_ "\033[1;33m"
#define LogColorGreen_ "\033[0;32m"
#define LogColorLightGreen_ "\033[1;32m"

#define ColorfulPrint_(color, fmt, ...)                                        \
    do {                                                                         \
        printf(color fmt LogColorNone_, ##__VA_ARGS__);                            \
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


#define PrintLog_(color, mode,  fmt, ...)                                             \
    do {                                                                         \
        ColorfulPrint_(color, "[%s][%s][%s]" fmt "\n",                     \
                      getCurrentTime().c_str(), std::string(mode).c_str(), SELF_MODULE_NAME.c_str(), ##__VA_ARGS__);           \
    } while (0)

#define PrintLog_debug(color, mode, fmt, ...)                                             \
    do {                                           \
        ColorfulPrint_(color, "[%s][%s][%s][%s(%d)][%s()]: " fmt "\n",     \
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

static int getLogLevel()
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

static  int FRIZY_LOG_LEVEL = getLogLevel();

#define PrintDbg_(mode, fmt, ...) PrintLog_debug(LogColorLightGreen_, mode, fmt, ##__VA_ARGS__)
#define PrintInfo_(mode, fmt, ...) PrintLog_(LogColorNone_, mode, fmt, ##__VA_ARGS__)
#define PrintWarn_(mode, fmt, ...) PrintLog_(LogColorLightYellow_, mode, fmt, ##__VA_ARGS__)
#define PrintErr_(mode, fmt, ...) PrintLog_(LogColorLightRed_, mode, fmt, ##__VA_ARGS__)

#define LOG(level, fmt, ...) do {                      \
    if (level == LOG_DEBUG) { if (LOG_DEBUG >= FRIZY_LOG_LEVEL) {PrintDbg_("DEBUG", fmt, ##__VA_ARGS__);}            \
    } else if (level == LOG_INFO) { if (LOG_INFO >= FRIZY_LOG_LEVEL) {PrintInfo_("INFO", fmt, ##__VA_ARGS__);}        \
    } else if (level == LOG_WARNING) { if (LOG_WARNING >= FRIZY_LOG_LEVEL) {PrintWarn_("WARN", fmt, ##__VA_ARGS__);} \
    } else if (level == LOG_ERROR) { if (LOG_ERROR >= FRIZY_LOG_LEVEL) {PrintErr_("ERROR", fmt, ##__VA_ARGS__);}     \
    } else {                                                     \
    }                                                            \
} while (0)


#define SETLOGLEVEL(level) {FRIZY_LOG_LEVEL = level;}

#define DEBUG(fmt, ...) LOG(LOG_DEBUG, fmt, ##__VA_ARGS__)
#define INFO(fmt, ...) LOG(LOG_INFO, fmt, ##__VA_ARGS__)
#define WARN(fmt, ...) LOG(LOG_WARNING, fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...) LOG(LOG_ERROR, fmt, ##__VA_ARGS__)


#endif  // __HCCL_TEST_LOGGER_H_
