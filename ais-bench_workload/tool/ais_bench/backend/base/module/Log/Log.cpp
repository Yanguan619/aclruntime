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

#include "Base/Log/Log.h"

int g_frizyLogLevel;

namespace Base {
void SETLOGLEVEL(int level)
{
    g_frizyLogLevel = level;
}
}

bool validate_log(char* log_buffer) {
    // valid
    return true;
}

void log_print(const char* fmt, ...) {
    char log_buffer[LOG_BUFFER_SIZE];
    va_list args;
    va_start(args, fmt);
    vsnprintf(log_buffer, sizeof(log_buffer), fmt, args);
    va_end(args);
    if (validate_log(log_buffer)) {
        fprintf(stdout, "%s", log_buffer);
        fflush(stdout);
    }
}
