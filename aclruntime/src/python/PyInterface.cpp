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
namespace {
const std::string PURE_INFER_DATA = "pure_infer_data";
const std::string PURE_INFER_DATA_ZERO = "pure_infer_data_zero";
const std::string PURE_INFER_DATA_RANDOM = "pure_infer_data_random";

PYBIND11_MODULE(aclruntime, m) {
    RegistPyTensorModule(m);
    RegistInferenceSession(m);

    // Export pure-infer sentinel filenames so Python side uses the canonical
    // C++-defined strings instead of duplicating magic constants.
    m.attr("PURE_INFER_DATA") = PURE_INFER_DATA;
    m.attr("PURE_INFER_DATA_ZERO") = PURE_INFER_DATA_ZERO;
    m.attr("PURE_INFER_DATA_RANDOM") = PURE_INFER_DATA_RANDOM;
}
} // namespace
