 # Copyright (c) 2024-2024 Huawei Technologies Co., Ltd.
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

import aclruntime
import numpy as np

def aclruntime_api_dymbatch():
    device_id = 0
    model_path = "../../testdata/add_model/model/add_model_dymbatch.om"

    # create session of om model for inference
    options = aclruntime.session_options()
    session = aclruntime.InferenceSession(model_path, device_id, options)

    shapes = []
    feeds = []
    #create new numpy data according inputs info
    shape0 = [4, 3, 32, 32]
    ndata0 = np.full(shape0, 1).astype(np.float32)
    shape1 = [4, 3, 32, 32]
    ndata1 = np.full(shape1, 1).astype(np.float32)
    shapes.append(shape0)
    shapes.append(shape1)

    # move data to device
    tensor0 = aclruntime.Tensor(ndata0)
    tensor0.to_device(device_id)
    feeds.append(tensor0)
    tensor1 = aclruntime.Tensor(ndata1)
    tensor1.to_device(device_id)
    feeds.append(tensor1)

    # set dynamic shape
    indesc = session.get_inputs()
    for i, shape in enumerate(shapes):
        for j, dim in enumerate(shape):
            if (indesc[i].shape[j] < 0):
                session.set_dynamic_batchsize(dim)
                print("input datas and intensors dim matched")
                break
            if (indesc[i].shape[j] != dim):
                raise RuntimeError("input datas and intensors dim not matched!")

    # inference
    outnames = [meta.name for meta in session.get_outputs()]
    outputs = session.run(outnames, feeds)

    print(f"outputs: {outputs}")
    outarray = []
    for out in outputs:
        # convert acltenor to host memory
        out.to_host()
        # convert acltensor to numpy array
        outarray.append(np.array(out))
    print(outarray)
    # summay inference throughput
    print("infer avg:{} ms".format(np.mean(session.sumary().exec_time_list)))

aclruntime_api_dymbatch()
