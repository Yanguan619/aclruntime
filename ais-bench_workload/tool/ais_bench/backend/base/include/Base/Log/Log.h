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

#ifndef CORE_LOG_H
#define CORE_LOG_H

#include <vector>
#include <string>
#include <map>
#include <memory>
#include <ostream>
#include <iostream>
#include <csignal>
#include <cstdarg>
#include <execinfo.h>

using namespace std;

#define FILELINE __FILE__, __FUNCTION__, __LINE__

#define LOG_DEBUG_LEVEL 1
#define LOG_INFO_LEVEL 2
#define LOG_WARNING_LEVEL 3
#define LOG_ERROR_LEVEL 4

namespace Base {
class LogCtrl {
public:
    static void SetLogLevel(int level) {
        frizyLogLevel = level;
    }
    static bool CheckLogLevel(int level) {
        return level >= frizyLogLevel;
    }
private:
    LogCtrl() = delete;
    ~LogCtrl() = delete;

    static int frizyLogLevel;
};
}


#define DEBUG_LOG(fmt, args...)  do { if (Base::LogCtrl::CheckLogLevel(LOG_DEBUG_LEVEL)) \
    { printf("[DEBUG] " fmt "\n", ##args); fflush(stdout); } } while (0)
#define INFO_LOG(fmt, args...)  do { if (Base::LogCtrl::CheckLogLevel(LOG_INFO_LEVEL)) \
    { printf("[INFO] " fmt "\n", ##args); fflush(stdout); } } while (0)
#define WARN_LOG(fmt, args...)  do { if (Base::LogCtrl::CheckLogLevel(LOG_WARNING_LEVEL)) \
    { printf("[WARN] " fmt "\n", ##args); fflush(stdout); } } while (0)
#define ERROR_LOG(fmt, args...)  do { if (Base::LogCtrl::CheckLogLevel(LOG_ERROR_LEVEL)) \
    { printf("[ERROR] " fmt "\n", ##args); fflush(stdout); } } while (0)
#define PROMPT_MSG(fmt, args...) printf(fmt, ##args)

inline void ACLERR_LOG(const char* ErrMsg)
{
    printf("[ACL ERROR] %s\n", ErrMsg);
}

#endif  // CORE_LOG_H