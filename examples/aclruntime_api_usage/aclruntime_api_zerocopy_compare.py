# Copyright (c) Huawei Technologies Co., Ltd. 2024-2025. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import time

import numpy as np

import aclruntime

from ais_bench.infer.common.utils import logger_print


def _run_timed(session, feeds_builder, outnames, loop):
    """计时窗口内执行 feeds 构造与推理。
    feeds_builder: 无参 callable，每次调用返回单次推理的一组输入 tensor 列表。
    """
    for _ in range(3):
        session.run(outnames, feeds_builder())
    start = time.perf_counter()
    for _ in range(loop):
        session.run(outnames, feeds_builder())
    return (time.perf_counter() - start) * 1000 / loop


def aclruntime_api_zerocopy_compare():
    device_id = 0
    loop = 50
    model_path = os.getenv("ACLRUNTIME_MODEL_PATH", "/data/workspace/qwen2onnx/resnet50.om")

    # create session of om model for inference
    options = aclruntime.session_options()
    session = aclruntime.InferenceSession(model_path, device_id, options)

    in_shapes = [meta.shape for meta in session.get_inputs()]
    outnames = [meta.name for meta in session.get_outputs()]

    # 预生成随机数据（不计时）
    all_data = []
    for _ in range(loop):
        all_data.append([np.random.rand(*shape).astype(np.float32) for shape in in_shapes])

    # 路径1：host tensor（aclrtMallocHost，RC 设备上对 device 可见），直接喂 run，无 H2D
    idx = [0]
    def build_host():
        d = all_data[idx[0] % loop]
        idx[0] += 1
        return [aclruntime.Tensor(x) for x in d]
    avg_host = _run_timed(session, build_host, outnames, loop)

    # 路径2：同样从 host tensor 出发，但显式 to_device（H2D 拷贝），
    # 模拟 EP 设备下 host 内存对 device 不可见、必须显式拷贝的场景
    idx = [0]
    def build_dev():
        d = all_data[idx[0] % loop]
        idx[0] += 1
        tensors = [aclruntime.Tensor(x) for x in d]
        for t in tensors:
            t.to_device(device_id)
        return tensors
    avg_dev = _run_timed(session, build_dev, outnames, loop)

    logger_print("host tensor (RC zero-copy) avg: %.3f ms/run (n=%d)" % (avg_host, loop))
    logger_print("host tensor + to_device (EP, H2D) avg: %.3f ms/run (n=%d)" % (avg_dev, loop))


aclruntime_api_zerocopy_compare()
