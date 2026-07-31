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

#ifndef ACLRUNTIME_INCLUDE_MODELINFER_SESSIONOPTIONS_H_
#define ACLRUNTIME_INCLUDE_MODELINFER_SESSIONOPTIONS_H_

#include "ModelInfer/utils.h"

namespace Base {
class SessionOptions {
public:
    int log_level = LOG_INFO_LEVEL;
    int loop = 1;
    std::string aclJsonPath = "";
    // When non-empty, weights are loaded from this directory and shared
    // (by file name) across sessions that point to the same directory,
    // via aclmdlSetExternalWeightAddress + aclmdlLoadWithConfig.
    std::string weightDir = "";
    // Release graph and pre-cached info after model load to save CPU memory.
    // Corresponds to ACL_MDL_WITHOUT_GRAPH_INT32=1.
    bool withoutGraph = false;
    // When non-empty, models sharing the same group name reuse the same
    // device workspace memory via ACL_MDL_WORKSPACE_ADDR_PTR / SIZE.
    // Only safe when models do not run concurrently (e.g. prefill + decode).
    std::string workspaceShareGroup = "";
};
}  // namespace Base
#endif  // ACLRUNTIME_INCLUDE_MODELINFER_SESSIONOPTIONS_H_
