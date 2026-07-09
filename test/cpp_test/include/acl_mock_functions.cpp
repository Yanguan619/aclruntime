#include "acl_mock_functions.h"

MockACL* g_mockAcl = nullptr;  // 唯一实际定义

// "C" 接口包装器
extern "C" {

#define DEFINE_MOCK_C_API(ReturnType, FuncName, ParamDecl, ParamNames) \
  ReturnType FuncName ParamDecl { return g_mockAcl->FuncName ParamNames; }

DEFINE_MOCK_C_API(aclError, aclmdlLoadFromFile,
                  (const char* modelPath, uint32_t* modelId),
                  (modelPath, modelId))
DEFINE_MOCK_C_API(aclError, aclmdlUnload, (uint32_t modelId), (modelId))
DEFINE_MOCK_C_API(aclmdlDesc*, aclmdlCreateDesc, (), ())
DEFINE_MOCK_C_API(aclError, aclmdlDestroyDesc, (aclmdlDesc * modelDesc),
                  (modelDesc))
DEFINE_MOCK_C_API(aclError, aclmdlGetDesc,
                  (aclmdlDesc * modelDesc, uint32_t modelId),
                  (modelDesc, modelId))
DEFINE_MOCK_C_API(aclError, aclmdlGetInputDynamicGearCount,
                  (const aclmdlDesc* modelDesc, size_t index,
                   size_t* dymGearCount),
                  (modelDesc, index, dymGearCount))

// Input Related
DEFINE_MOCK_C_API(size_t, aclmdlGetNumInputs, (aclmdlDesc * modelDesc),
                  (modelDesc))
DEFINE_MOCK_C_API(const char*, aclmdlGetInputNameByIndex,
                  (const aclmdlDesc* modelDesc, size_t index),
                  (modelDesc, index))
DEFINE_MOCK_C_API(aclError, aclmdlGetInputIndexByName,
                  (const aclmdlDesc* modelDesc, const char* name,
                   size_t* index),
                  (modelDesc, name, index))
DEFINE_MOCK_C_API(size_t, aclmdlGetInputSizeByIndex,
                  (aclmdlDesc * modelDesc, size_t index), (modelDesc, index))
DEFINE_MOCK_C_API(aclError, aclmdlGetInputDims,
                  (const aclmdlDesc* modelDesc, size_t index,
                   aclmdlIODims* dims),
                  (modelDesc, index, dims))
DEFINE_MOCK_C_API(aclFormat, aclmdlGetInputFormat,
                  (const aclmdlDesc* modelDesc, size_t index),
                  (modelDesc, index))
DEFINE_MOCK_C_API(aclDataType, aclmdlGetInputDataType,
                  (const aclmdlDesc* modelDesc, size_t index),
                  (modelDesc, index))

// Output Related
DEFINE_MOCK_C_API(size_t, aclmdlGetNumOutputs, (aclmdlDesc * modelDesc),
                  (modelDesc))
DEFINE_MOCK_C_API(const char*, aclmdlGetOutputNameByIndex,
                  (const aclmdlDesc* modelDesc, size_t index),
                  (modelDesc, index))
DEFINE_MOCK_C_API(aclError, aclmdlGetOutputIndexByName,
                  (const aclmdlDesc* modelDesc, const char* name,
                   size_t* index),
                  (modelDesc, name, index))
DEFINE_MOCK_C_API(size_t, aclmdlGetOutputSizeByIndex,
                  (aclmdlDesc * modelDesc, size_t index), (modelDesc, index))
DEFINE_MOCK_C_API(aclError, aclmdlGetOutputDims,
                  (const aclmdlDesc* modelDesc, size_t index,
                   aclmdlIODims* dims),
                  (modelDesc, index, dims))
DEFINE_MOCK_C_API(aclFormat, aclmdlGetOutputFormat,
                  (const aclmdlDesc* modelDesc, size_t index),
                  (modelDesc, index))
DEFINE_MOCK_C_API(aclDataType, aclmdlGetOutputDataType,
                  (const aclmdlDesc* modelDesc, size_t index),
                  (modelDesc, index))

// Dataset Related
DEFINE_MOCK_C_API(aclTensorDesc*, aclCreateTensorDesc,
                  (aclDataType dataType, int numDims, const int64_t* dims,
                   aclFormat format),
                  (dataType, numDims, dims, format))
DEFINE_MOCK_C_API(aclError, aclmdlSetDatasetTensorDesc,
                  (aclmdlDataset * dataset, aclTensorDesc* tensorDesc,
                   size_t index),
                  (dataset, tensorDesc, index))
DEFINE_MOCK_C_API(aclmdlDataset*, aclmdlCreateDataset, (), ())
DEFINE_MOCK_C_API(aclError, aclmdlDestroyDataset,
                  (const aclmdlDataset* dataset), (dataset))
DEFINE_MOCK_C_API(const char*, aclGetRecentErrMsg, (), ())
DEFINE_MOCK_C_API(aclError, aclmdlGetDynamicHW,
                  (const aclmdlDesc* modelDesc, size_t profileIndex,
                   aclmdlHW* dynamicHW),
                  (modelDesc, profileIndex, dynamicHW))
DEFINE_MOCK_C_API(aclError, aclmdlSetDynamicHWSize,
                  (uint32_t modelId, aclmdlDataset* dataset, size_t index,
                   uint64_t dynamicHeight, uint64_t dynamicWidth),
                  (modelId, dataset, index, dynamicHeight, dynamicWidth))
DEFINE_MOCK_C_API(aclError, aclmdlSetDynamicBatchSize,
                  (uint32_t modelId, aclmdlDataset* dataset, size_t index,
                   uint64_t dynamicBatchSize),
                  (modelId, dataset, index, dynamicBatchSize))
DEFINE_MOCK_C_API(aclError, aclmdlGetDynamicBatch,
                  (const aclmdlDesc* modelDesc, aclmdlBatch* batchInfo),
                  (modelDesc, batchInfo))
DEFINE_MOCK_C_API(aclError, aclmdlGetCurOutputDims,
                  (const aclmdlDesc* modelDesc, size_t index,
                   aclmdlIODims* ioDims),
                  (modelDesc, index, ioDims))
DEFINE_MOCK_C_API(aclError, aclmdlGetInputDynamicDims,
                  (const aclmdlDesc* modelDesc, size_t profileIndex,
                   aclmdlIODims* dims, size_t gearCount),
                  (modelDesc, profileIndex, dims, gearCount))
DEFINE_MOCK_C_API(aclError, aclmdlSetInputDynamicDims,
                  (uint32_t modelId, aclmdlDataset* dataset, size_t index,
                   const aclmdlIODims* dims),
                  (modelId, dataset, index, dims))

DEFINE_MOCK_C_API(aclDataBuffer*, aclCreateDataBuffer,
                  (void* data, size_t size), (data, size))
DEFINE_MOCK_C_API(aclError, aclDestroyDataBuffer,
                  (const aclDataBuffer* dataBuffer), (dataBuffer))
DEFINE_MOCK_C_API(aclError, aclmdlAddDatasetBuffer,
                  (aclmdlDataset * dataset, aclDataBuffer* dataBuffer),
                  (dataset, dataBuffer))
DEFINE_MOCK_C_API(size_t, aclmdlGetDatasetNumBuffers,
                  (const aclmdlDataset* dataset), (dataset))
DEFINE_MOCK_C_API(aclDataBuffer*, aclmdlGetDatasetBuffer,
                  (const aclmdlDataset* dataset, size_t index),
                  (dataset, index))
DEFINE_MOCK_C_API(size_t, aclGetDataBufferSizeV2,
                  (const aclDataBuffer* dataBuffer), (dataBuffer))
DEFINE_MOCK_C_API(void*, aclGetDataBufferAddr,
                  (const aclDataBuffer* dataBuffer), (dataBuffer))
DEFINE_MOCK_C_API(aclError, aclUpdateDataBuffer,
                  (aclDataBuffer * dataBuffer, void* addr, size_t size),
                  (dataBuffer, addr, size))

// Memory Operations
DEFINE_MOCK_C_API(aclError, aclrtMalloc,
                  (void** devPtr, size_t size, aclrtMemMallocPolicy policy),
                  (devPtr, size, policy))
DEFINE_MOCK_C_API(aclError, aclrtFree, (void* devPtr), (devPtr))
DEFINE_MOCK_C_API(aclError, aclrtMemcpy,
                  (void* dst, size_t destMax, const void* src, size_t count,
                   aclrtMemcpyKind kind),
                  (dst, destMax, src, count, kind))
DEFINE_MOCK_C_API(aclError, aclrtMemset,
                  (void* devPtr, size_t maxCount, int32_t value, size_t count),
                  (devPtr, maxCount, value, count))
DEFINE_MOCK_C_API(aclError, aclrtMallocHost, (void** ptr, size_t size),
                  (ptr, size))
DEFINE_MOCK_C_API(aclError, aclrtFreeHost, (void* hostData), (hostData))
DEFINE_MOCK_C_API(aclError, acldvppMalloc, (void** ptr, size_t size),
                  (ptr, size))
DEFINE_MOCK_C_API(aclError, acldvppFree, (void* ptr), (ptr))
DEFINE_MOCK_C_API(aclError, aclrtSetExceptionInfoCallback,
                  (ACL_EXCEPTION_CALLBACK callback), (callback))
DEFINE_MOCK_C_API(aclError, aclmdlExecute,
                  (uint32_t modelId, const aclmdlDataset* input,
                   aclmdlDataset* output),
                  (modelId, input, output))

// 张量描述函数
DEFINE_MOCK_C_API(size_t, aclGetTensorDescNumDims, (const aclTensorDesc* desc),
                  (desc))
DEFINE_MOCK_C_API(aclError, aclGetTensorDescDimV2,
                  (const aclTensorDesc* desc, size_t idx, int64_t* dim),
                  (desc, idx, dim))
DEFINE_MOCK_C_API(size_t, aclGetTensorDescSize, (const aclTensorDesc* desc),
                  (desc))

// 数据集函数
DEFINE_MOCK_C_API(aclTensorDesc*, aclmdlGetDatasetTensorDesc,
                  (const aclmdlDataset* dataset, size_t index),
                  (dataset, index))

// AIPP 函数
DEFINE_MOCK_C_API(aclmdlAIPP*, aclmdlCreateAIPP, (uint64_t maxBatchSize),
                  (maxBatchSize));
DEFINE_MOCK_C_API(aclError, aclmdlGetAippType,
                  (uint32_t modelId, size_t index,
                   aclmdlInputAippType* aippType,
                   size_t* dynamicAttachedDataIndex),
                  (modelId, index, aippType, dynamicAttachedDataIndex))
DEFINE_MOCK_C_API(aclError, aclmdlSetInputAIPP,
                  (uint32_t modelId, aclmdlDataset* input, size_t index,
                   const aclmdlAIPP* pAippDynamicSet),
                  (modelId, input, index, pAippDynamicSet))
DEFINE_MOCK_C_API(aclError, aclmdlSetAIPPSrcImageSize,
                  (aclmdlAIPP * aippDynamicSet, int32_t srcImageSizeW,
                   int32_t srcImageSizeH),
                  (aippDynamicSet, srcImageSizeW, srcImageSizeH))
DEFINE_MOCK_C_API(aclError, aclmdlSetAIPPInputFormat,
                  (aclmdlAIPP * aippDynamicSet, aclAippInputFormat inputFormat),
                  (aippDynamicSet, inputFormat))
DEFINE_MOCK_C_API(aclError, aclmdlSetAIPPRbuvSwapSwitch,
                  (aclmdlAIPP * aippDynamicSet, int8_t rbuvSwapSwitch),
                  (aippDynamicSet, rbuvSwapSwitch))
DEFINE_MOCK_C_API(aclError, aclmdlSetAIPPAxSwapSwitch,
                  (aclmdlAIPP * aippDynamicSet, int8_t axSwapSwitch),
                  (aippDynamicSet, axSwapSwitch))
DEFINE_MOCK_C_API(aclError, aclmdlSetAIPPDtcPixelMean,
                  (aclmdlAIPP * aippDynamicSet, int16_t dtcPixelMeanChn0,
                   int16_t dtcPixelMeanChn1, int16_t dtcPixelMeanChn2,
                   int16_t dtcPixelMeanChn3, size_t batchIndex),
                  (aippDynamicSet, dtcPixelMeanChn0, dtcPixelMeanChn1,
                   dtcPixelMeanChn2, dtcPixelMeanChn3, batchIndex));
DEFINE_MOCK_C_API(aclError, aclmdlSetAIPPDtcPixelMin,
                  (aclmdlAIPP * aippDynamicSet, float dtcPixelMinChn0,
                   float dtcPixelMinChn1, float dtcPixelMinChn2,
                   float dtcPixelMinChn3, uint64_t batchIndex),
                  (aippDynamicSet, dtcPixelMinChn0, dtcPixelMinChn1,
                   dtcPixelMinChn2, dtcPixelMinChn3, batchIndex));
DEFINE_MOCK_C_API(aclError, aclmdlSetAIPPPixelVarReci,
                  (aclmdlAIPP * aippDynamicSet, float dtcPixelVarReciChn0,
                   float dtcPixelVarReciChn1, float dtcPixelVarReciChn2,
                   float dtcPixelVarReciChn3, uint64_t batchIndex),
                  (aippDynamicSet, dtcPixelVarReciChn0, dtcPixelVarReciChn1,
                   dtcPixelVarReciChn2, dtcPixelVarReciChn3, batchIndex));
DEFINE_MOCK_C_API(aclError, aclmdlSetAIPPCropParams,
                  (aclmdlAIPP * aippDynamicSet, int8_t cropSwitch,
                   int32_t loadStartPosW, int32_t loadStartPosH,
                   int32_t cropSizeW, int32_t cropSizeH, uint64_t batchIndex),
                  (aippDynamicSet, cropSwitch, loadStartPosW, loadStartPosH,
                   cropSizeW, cropSizeH, batchIndex));
DEFINE_MOCK_C_API(aclError, aclmdlSetAIPPPaddingParams,
                  (aclmdlAIPP * aippDynamicSet, int8_t paddingSwitch,
                   int32_t paddingSizeTop, int32_t paddingSizeBottom,
                   int32_t paddingSizeLeft, int32_t paddingSizeRight,
                   uint64_t batchIndex),
                  (aippDynamicSet, paddingSwitch, paddingSizeTop,
                   paddingSizeBottom, paddingSizeLeft, paddingSizeRight,
                   batchIndex));
DEFINE_MOCK_C_API(aclError, aclmdlDestroyAIPP, (const aclmdlAIPP* aippParmsSet),
                  (aippParmsSet))

// DeviceManager 测试需要的ACL接口声明
DEFINE_MOCK_C_API(aclError, aclInit, (const char* configPath), (configPath))
DEFINE_MOCK_C_API(aclError, aclFinalize, (), ())
DEFINE_MOCK_C_API(aclError, aclrtSetDevice, (int devId), (devId))
DEFINE_MOCK_C_API(aclError, aclrtGetDeviceCount, (uint32_t* count), (count))
DEFINE_MOCK_C_API(aclError, aclrtCreateContext, (void** context, int devId),
                  (context, devId))
DEFINE_MOCK_C_API(aclError, aclrtDestroyContext, (void* context), (context))
DEFINE_MOCK_C_API(aclError, aclrtSetCurrentContext, (void* context), (context))
DEFINE_MOCK_C_API(aclError, aclrtResetDevice, (int devId), (devId))
DEFINE_MOCK_C_API(aclError, aclrtGetCurrentContext, (void** context), (context))

// 清理宏定义
#undef DEFINE_MOCK_C_API
}
