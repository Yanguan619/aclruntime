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

#ifndef MODEL_PROCESS_H_
#define MODEL_PROCESS_H_
#include <string>
#include <vector>
#include <chrono>
#include "acl/acl.h"
#include "ModelInfer/utils.h"
#include "ModelInfer/SessionOptions.h"
#include "Tensor/TensorBase.h"
#include "ModelInfer/DynamicAippConfig.h"

enum class MemoryPolicy {
    FREE_MEMORY,
    RETAIN_MEMORY
};

enum NORMAL_DATA_TYPE {
    TYPE_FLOAT,
    TYPE_ACLFLOAT16,
    TYPE_INT8_T,
    TYPE_INT,
    TYPE_UINT8_T,
    NOT_USED,
    TYPE_INT16_T,
    TYPE_UINT16_T,
    TYPE_UINT32_T,
    TYPE_INT64_T,
    TYPE_UINT64_T,
    TYPE_DOUBLE,
    TYPE_BOOL
};

/**
 * ModelProcess
 */
class ModelProcess {
public:
    /**
    * @brief Constructor
    */
    ModelProcess();

    /**
    * @brief Destructor
    */
    ~ModelProcess();

    /**
    * @brief load model from file with mem
    * @param [in] modelPath: model path
    * @param [in] weightDir: optional directory of external weight files.
    *         When non-empty, the model bytes are loaded with
    *         aclmdlLoadWithConfig after registering every weight file in
    *         weightDir through aclmdlSetExternalWeightAddress. Weight
    *         buffers are shared across ModelProcess instances via
    *         WeightPool, so prefill and decode sessions can reuse the
    *         same device allocations.
    * @param [in] options: optional session options for memory optimization.
    *         When withoutGraph is true, ACL_MDL_WITHOUT_GRAPH_INT32 is
    *         set to release graph and pre-cached info after model load.
    * @return result
    */
    UtilsResult::Result LoadModelFromFile(const std::string& modelPath,
                                          const std::string& weightDir = "",
                                          std::shared_ptr<Base::SessionOptions> options = nullptr);

    UtilsResult::Result LoadModelFromMem(const void* modelData, size_t modelSize);

    /**
    * @brief unload model
    */
    void Unload();

    /**
    * @brief create model desc
    * @return result
    */
    UtilsResult::Result CreateDesc();

    /**
    * @brief PrintDesc
    */
    UtilsResult::Result PrintDesc();

    /**
    * @brief get dynamic gear conut
    */
    UtilsResult::Result GetDynamicGearCount(size_t &dymGearCount);

    /**
    * @brief get dynamic index
    */
    UtilsResult::Result GetDynamicIndex(size_t &dymindex);

    /**
    * @brief check dynamic input dims valid
    */
    UtilsResult::Result CheckDynamicDims(std::vector<std::string> dym_dims, size_t gearCount, aclmdlIODims* dims);

    /**
    * @brief check dynamic input batch valid
    */
    UtilsResult::Result CheckDynamicBatchSize(uint64_t dymbatch, bool& is_dymbatch);

    /**
    * @brief check dynamic input image size valid
    */
    UtilsResult::Result CheckDynamicHWSize(std::pair<int, int> dynamicPair, bool& is_dymHW);

    /**
    * @brief set dynamic input dims
    */
    UtilsResult::Result SetDynamicDims(std::vector<std::string> dym_dims);

    /**
    * @brief check dynamic input image size valid
    */
    UtilsResult::Result CheckDynamicShape(std::vector<std::string> dym_shape_tmp, std::map<std::string,
        std::vector<int64_t>> &dym_shape_map, std::vector<int64_t> &dims_num);

    /**
    * @brief set dynamic input dims
    */
    UtilsResult::Result SetDynamicShape(std::map<std::string, std::vector<int64_t>> dym_shape_map, std::vector<int64_t> &dims_num);

    /**
    * @brief set dynamic batch size
    */
    UtilsResult::Result SetDynamicBatchSize(uint64_t batchSize);

    /**
    * @brief get max dynamic batch size
    */
    UtilsResult::Result GetMaxBatchSize(uint64_t& maxBatchSize);

    /**
    * @brief set dynamic image size
    */
    UtilsResult::Result SetDynamicHW(std::pair<uint64_t, uint64_t > dynamicPair);

    /**
    * @brief check model the amount of dynamic aipp input
    */
    int CheckDymAIPPInputExist();

    /**
    * @brief free aclmdlAIPP
    */
    UtilsResult::Result FreeAIPP(aclmdlAIPP* aippParmsSet);

    // ------------------分别配置具体AIPP参数-----------------
    UtilsResult::Result SetAIPPSrcImageSize(std::shared_ptr<Base::DynamicAippConfig> dyAippCfg, aclmdlAIPP* aippDynamicSet);

    UtilsResult::Result SetAIPPInputFormat(std::shared_ptr<Base::DynamicAippConfig> dyAippCfg, aclmdlAIPP* aippDynamicSet);

    UtilsResult::Result SetAIPPCscParams(std::shared_ptr<Base::DynamicAippConfig> dyAippCfg, aclmdlAIPP* aippDynamicSet);

    UtilsResult::Result SetAIPPRbuvSwapSwitch(std::shared_ptr<Base::DynamicAippConfig> dyAippCfg, aclmdlAIPP* aippDynamicSet);

    UtilsResult::Result SetAIPPAxSwapSwitch(std::shared_ptr<Base::DynamicAippConfig> dyAippCfg, aclmdlAIPP* aippDynamicSet);

    UtilsResult::Result SetAIPPDtcPixelMean(std::shared_ptr<Base::DynamicAippConfig> dyAippCfg,
        aclmdlAIPP* aippDynamicSet, size_t batchIndex);

    UtilsResult::Result SetAIPPDtcPixelMin(std::shared_ptr<Base::DynamicAippConfig> dyAippCfg,
        aclmdlAIPP* aippDynamicSet, size_t batchIndex);

    UtilsResult::Result SetAIPPPixelVarReci(std::shared_ptr<Base::DynamicAippConfig> dyAippCfg,
        aclmdlAIPP* aippDynamicSet, size_t batchIndex);

    UtilsResult::Result SetAIPPCropParams(std::shared_ptr<Base::DynamicAippConfig> dyAippCfg,
        aclmdlAIPP* aippDynamicSet, size_t batchIndex);

    UtilsResult::Result SetAIPPPaddingParams(std::shared_ptr<Base::DynamicAippConfig> dyAippCfg,
        aclmdlAIPP* aippDynamicSet, size_t batchIndex);
    // ------------------分别配置具体AIPP参数-----------------

    /**
    * @brief set single dynamic aipp config
    */
    UtilsResult::Result GetDymAIPPConfigSet(std::shared_ptr<Base::DynamicAippConfig> dyAippCfg,
        aclmdlAIPP* &pAIPPSet, uint64_t maxBatchSize);

    /**
    * @brief set single or multiple dynamic aipp config
    */
    UtilsResult::Result SetDynamicAipp();

    /**
    * @brief set gived single  dynamic aipp config
    */
    UtilsResult::Result SetInputAIPP(size_t index, void* pAippDynamicSet);
    /**
    * @brief get dynamic input dims info
    */
    void GetDimInfo(size_t gearCount, aclmdlIODims* dims);

    /**
    * @brief get dynamic input batch info
    */
    void GetDymBatchInfo();

    /**
    * @brief get dynamic image size info
    */
    void GetDymHWInfo();

    /**
    * @brief get dynamic aipp list, just support single index
    */
    UtilsResult::Result GetAIPPIndexList(std::vector<size_t> &dataNeedDynamicAipp);
    /**
    * @brief destroy desc
    */
    void DestroyDesc();

    /**
    * @brief create model input
    * @return result
    */
    UtilsResult::Result CreateDymInput(size_t index);

    /**
    * @brief create model input
    * @param [in] inputDataBuffer: input buffer
    * @param [in] bufferSize: input buffer size
    * @return result
    */
    UtilsResult::Result CreateInput(void* inputDataBuffer, size_t bufferSize);

    /**
    * @brief update model inputs(without memory copy)
    * @param [in] inOutRelation: inputs update method
    * @return result
    */
    UtilsResult::Result UpdateInputsReuse(const std::vector<int> &inOutRelation);

    /**
    * @brief update model inputs(need one memory copy per iteration)
    * @param [in] inOutRelation: inputs update method
    * @return result
    */
    UtilsResult::Result UpdateInputsMemcpy(const std::vector<int> &inOutRelation);

    /**
    * @brief create model input
    * @return result
    */
    UtilsResult::Result CreateZeroInput();

    /**
    * @brief destroy input resource
    */
    void DestroyInput(MemoryPolicy policy);

    /**
    * @brief create output buffer
    * @return result
    */
    UtilsResult::Result CreateOutput();

    /**
    * @brief destroy output resource
    */
    void DestroyOutput(MemoryPolicy policy);

    /**
    * @brief model execute
    * @return result
    */
    UtilsResult::Result Execute();

    /**
    * @brief dump model output result to file
    */
    void DumpModelOutputResult();

    /**
    * @brief get current output dims mul
    */
    UtilsResult::Result GetCurOutputDimsMul(size_t index,  std::vector<int64_t>& curOutputDimsMul);

    UtilsResult::Result CreateOutput(void* outputBuffer, size_t bufferSize);

    size_t GetNumInputs();
    size_t GetNumOutputs();

    UtilsResult::Result GetInTensorDesc(size_t i, std::string& name, int& datatype,
        size_t& format, std::vector<int64_t>& shape, size_t& size);
    UtilsResult::Result GetOutTensorDesc(size_t i, std::string& name, int& datatype,
        size_t& format, std::vector<int64_t>& shape, size_t& size);

    size_t GetOutTensorLen(size_t i, bool is_dymshape);

    UtilsResult::Result GetCurOutputShape(size_t index, bool is_dymshape, std::vector<int64_t>& shape);

    UtilsResult::Result GetMaxDynamicHWSize(size_t &outsize);

    void SetExceptionCallBack();
    void InitReuseOutput();
private:
    uint32_t modelId_;
    bool reuseOutput_;
    bool loadFlag_; // model load flag
    aclmdlDesc* modelDesc_;
    aclmdlDataset* input_;
    aclmdlDataset* output_;
    size_t numInputs_;
    size_t numOutputs_;
    size_t g_dymindex;
    // Keeps the OM file bytes alive for the model lifetime when loaded via
    // ACL_MDL_MEM_ADDR_PTR (shallow copy) for external-weight models.
    std::vector<uint8_t> modelData_;
    std::map<std::string, aclAippInputFormat> str2aclAippInputFormat;
    void model_description(aclError ret, size_t& numInputs, size_t& numOutputs, aclmdlIODims& dimsInput, aclmdlIODims& dimsOutput);
    // External weight directory registered to the pool; released on Unload.
    std::string weightDir_;
    bool weightsAcquired_ = false;

};
#endif
