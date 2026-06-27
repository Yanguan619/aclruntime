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

#ifndef PIPELINE_H
#define PIPELINE_H

#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <cstdint>

#include "python/PyInferenceSession.h"
#include "python/PyTensor.h"
#include "Tensor/TensorBase.h"
#include "ModelInfer/ModelInferenceProcessor.h"
#include "Tensor/TensorContext.h"
#include "ModelInfer/cnpy.h"
#include "ModelInfer/utils.h"

using Arguments = std::unordered_map<std::string, std::string>;

struct Feeds {
    std::vector<std::string> outputNames;
    std::shared_ptr<std::vector<Base::BaseTensor>> inputs = nullptr;
    std::shared_ptr<std::vector<Base::TensorBase>> outputs = nullptr;
    std::shared_ptr<std::vector<Base::MemoryData>> memory = nullptr;
    std::shared_ptr<std::vector<std::shared_ptr<cnpy::NpyArray>>> arrayPtr = nullptr;
    std::string autoDynamicShape;
    std::string autoDynamicDims;
    std::string outputPrefix;
};

template <typename T>
class ConcurrentQueue {
public:
    explicit ConcurrentQueue(size_t depth = 3): depth_(depth) {}

    T pop()
    {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this] { return !queue_.empty(); });
        T val = std::move(queue_.front());
        queue_.pop();
        lock.unlock();
        cv_.notify_one();
        return val;
    }

    void push(T item)
    {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this] { return queue_.size() < depth_; });
        queue_.push(std::move(item));
        lock.unlock();
        cv_.notify_one();
    }

private:
    std::mutex mtx_;
    std::queue<T> queue_;
    std::condition_variable cv_;
    size_t depth_;
};


namespace Base {
    void PrepareInputData(std::vector<std::string> &files, Base::PyInferenceSession* session,
        std::shared_ptr<Feeds> &feeds, bool autoDymShape,
        bool autoDymDims, const bool pure_infer, std::vector<std::string> &inputNames);
    void FuncPrepare(ConcurrentQueue<std::shared_ptr<Feeds>> &h2dQueue,
                     Base::PyInferenceSession* session,
                     std::vector<std::vector<std::string>> &infilesList,
                     std::shared_ptr<InferOptions> inferOption, size_t numThreads, size_t startIndex);

    void FuncPrepareBaseTensor(ConcurrentQueue<std::shared_ptr<Feeds>> &h2dQueue, uint32_t deviceId,
                               Base::PyInferenceSession* session,
                               std::vector<std::vector<Base::BaseTensor>>& inputsList,
                               std::vector<std::vector<std::vector<size_t>>>& shapesList, bool autoDymShape,
                               bool autoDymDims, std::vector<std::string>& outputNames);

    void FuncH2d(ConcurrentQueue<std::shared_ptr<Feeds>> &h2dQueue,
                 ConcurrentQueue<std::shared_ptr<Feeds>> &computeQueue,
                 Base::PyInferenceSession* session);

    void FuncCompute(ConcurrentQueue<std::shared_ptr<Feeds>> &computeQueue,
                     ConcurrentQueue<std::shared_ptr<Feeds>> &d2hQueue,
                     Base::PyInferenceSession* session,
                     InferSummaryInfo* summaryInfo);

    void FuncD2h(ConcurrentQueue<std::shared_ptr<Feeds>> &d2hQueue,
                 ConcurrentQueue<std::shared_ptr<Feeds>> &saveQueue,
                 Base::PyInferenceSession* session);

    void FuncSave(ConcurrentQueue<std::shared_ptr<Feeds>> &saveQueue, std::shared_ptr<InferOptions> inferOption);

    void FuncSaveTensorBase(ConcurrentQueue<std::shared_ptr<Feeds>> &saveQueue,
                            std::vector<std::vector<TensorBase>> &result, Base::PyInferenceSession* session);

    cnpy::NpyArray CreatePureInferArray(std::string fname, Base::TensorDesc inTensor);
}


#endif
