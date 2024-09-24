/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
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

extern int g_frizyLogLevel;

namespace Base {
void SETLOGLEVEL(int level);
}


#define DEBUG_LOG(fmt, args...)  do { if (g_frizyLogLevel <= LOG_DEBUG_LEVEL) \
    { fprintf(stdout, "[DEBUG] " fmt "\n", ##args); } } while (0)
#define INFO_LOG(fmt, args...)  do { if (g_frizyLogLevel <= LOG_INFO_LEVEL) \
    { fprintf(stdout, "[INFO] " fmt "\n", ##args); } } while (0)
#define WARN_LOG(fmt, args...)  do { if (g_frizyLogLevel <= LOG_WARNING_LEVEL) \
    { fprintf(stdout, "[WARN] " fmt "\n", ##args); } } while (0)
#define ERROR_LOG(fmt, args...)  do { if (g_frizyLogLevel <= LOG_ERROR_LEVEL) \
    { fprintf(stdout, "[ERROR] " fmt "\n", ##args); } } while (0)
#define ACLERR_LOG(ErrMsg) fprintf(stdout, "[ACL ERROR] %s\n", ErrMsg)
#define PROMPT_MSG(fmt, args...) fprintf(stdout, fmt, ##args)

#endif  // CORE_LOG_H