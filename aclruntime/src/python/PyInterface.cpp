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
#include "python/PyInterface.h"
#include "Log.h"
#include "acl/acl.h"
namespace {
const std::string PURE_INFER_DATA = "pure_infer_data";
const std::string PURE_INFER_DATA_ZERO = "pure_infer_data_zero";
const std::string PURE_INFER_DATA_RANDOM = "pure_infer_data_random";

// RC 形态下 aclrtMallocHost 分配的内存对 device 可见，喂 host tensor 即可零拷贝推理；
// EP 形态下 host 内存对 device 不可见，必须先显式 H2D。用 aclrtGetRunMode 自动区分。
int GetRunMode() {
    aclrtRunMode runMode = ACL_DEVICE;
    aclError ret = aclrtGetRunMode(&runMode);
    if (ret != ACL_SUCCESS) {
        ERROR_LOG("aclrtGetRunMode failed. ret=%d", ret);
        return -1;
    }
    return static_cast<int>(runMode);
}

PYBIND11_MODULE(aclruntime, m) {
    Base::LogCtrl::Init();
    RegistPyTensorModule(m);
    RegistInferenceSession(m);

    // Export pure-infer sentinel filenames so Python side uses the canonical
    // C++-defined strings instead of duplicating magic constants.
    m.attr("PURE_INFER_DATA") = PURE_INFER_DATA;
    m.attr("PURE_INFER_DATA_ZERO") = PURE_INFER_DATA_ZERO;
    m.attr("PURE_INFER_DATA_RANDOM") = PURE_INFER_DATA_RANDOM;

    // Log control
    m.def("set_log_level", &Base::LogCtrl::SetLogLevel, "Set spdlog level for aclruntime C++ logs");
    m.def("get_run_mode", &GetRunMode,
          "Query ACL run mode. 0=ACL_DEVICE (RC, aclrtMallocHost memory is "
          "device-visible, zero-copy inference possible), 1=ACL_HOST (EP, "
          "explicit H2D/D2H required), -1=query failed.");
    m.def("log_debug", [](const std::string& msg) { spdlog::debug(msg); });
    m.def("log_info", [](const std::string& msg) { spdlog::info(msg); });
    m.def("log_warning", [](const std::string& msg) { spdlog::warn(msg); });
    m.def("log_error", [](const std::string& msg) { spdlog::error(msg); });
    m.attr("LOG_DEBUG") = LOG_DEBUG_LEVEL;
    m.attr("LOG_INFO") = LOG_INFO_LEVEL;
    m.attr("LOG_WARNING") = LOG_WARNING_LEVEL;
    m.attr("LOG_ERROR") = LOG_ERROR_LEVEL;
}
}  // namespace
