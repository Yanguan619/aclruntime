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
#define SPDLOG_LEVEL_NAMES \
    { "TRACE", "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL", "OFF" }
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "Log.h"

namespace {
bool g_logInitialized = false;
}

namespace Base {
void LogCtrl::Init() {
    if (g_logInitialized) {
        return;
    }
    g_logInitialized = true;

    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    consoleSink->set_pattern("[%^%l%$] %v");

    auto logger = std::make_shared<spdlog::logger>("aclruntime",
                                                   std::move(consoleSink));
    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::info);
    spdlog::flush_on(spdlog::level::err);
}
}  // namespace Base
