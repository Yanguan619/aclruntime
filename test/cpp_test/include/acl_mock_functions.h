#pragma once

#ifndef ACL_MOCK_FUNCTIONS_H
#define ACL_MOCK_FUNCTIONS_H

#include <gmock/gmock.h>
#include <unistd.h>
#include <sys/types.h>
#include "acl/acl.h"

// C++11标准不支持make_unique函数，此处为make_unique的简单实现
#if __cplusplus < 201402L
namespace std {
    // 非数组类型
    template<typename T, typename... Args>
    typename std::enable_if<!std::is_array<T>::value, std::unique_ptr<T>>::type make_unique(Args&&... args)
    {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }

    // 数组类型(T[])
    template <typename T>
    using EnableIfDynamicArray = typename std::enable_if<
        std::is_array<T>::value && std::extent<T>::value == 0,
        std::unique_ptr<T>
    >::type;

    template<typename T>
    EnableIfDynamicArray<T> make_unique(size_t size)
    {
        using U = typename std::remove_extent<T>::type;
        return std::unique_ptr<T>(new U[size]);
    }

    // 禁用定长数组(如T[5])
    template<typename T, typename... Args>
    typename std::enable_if<std::extent<T>::value != 0, std::unique_ptr<T>>::type make_unique(Args&&...) = delete;
}
#endif

namespace {
using ACL_EXCEPTION_CALLBACK = void (*)(aclrtExceptionInfo*);
}

// 创建 ACL 函数的模拟接口
class ACLInterface {
public:
    virtual ~ACLInterface() = default;
    // 模拟 ACL 接口函数
    virtual aclError aclmdlLoadFromFile(const char* modelPath, uint32_t* modelId) = 0;
    virtual aclError aclmdlUnload(uint32_t modelId) = 0;

    virtual aclmdlDesc* aclmdlCreateDesc() = 0;
    virtual aclError aclmdlDestroyDesc(aclmdlDesc* modelDesc) = 0;
    virtual aclError aclmdlGetDesc(aclmdlDesc* modelDesc, uint32_t modelId) = 0;

    virtual size_t aclmdlGetNumInputs(aclmdlDesc* modelDesc) = 0;
    virtual const char* aclmdlGetInputNameByIndex(const aclmdlDesc* modelDesc, size_t index) = 0;
    virtual size_t aclmdlGetInputSizeByIndex(aclmdlDesc* modelDesc, size_t index) = 0;
    virtual aclError aclmdlGetInputIndexByName(const aclmdlDesc* modelDesc, const char* name, size_t* index) = 0;
    virtual aclError aclmdlGetInputDims(const aclmdlDesc* modelDesc, size_t index, aclmdlIODims* dims) = 0;
    virtual aclFormat aclmdlGetInputFormat(const aclmdlDesc* modelDesc, size_t index) = 0;
    virtual aclDataType aclmdlGetInputDataType(const aclmdlDesc* modelDesc, size_t index) = 0;

    virtual size_t aclmdlGetNumOutputs(aclmdlDesc* modelDesc) = 0;
    virtual const char* aclmdlGetOutputNameByIndex(const aclmdlDesc* modelDesc, size_t index) = 0;
    virtual size_t aclmdlGetOutputSizeByIndex(aclmdlDesc* modelDesc, size_t index) = 0;
    virtual aclError aclmdlGetOutputIndexByName(const aclmdlDesc* modelDesc, const char* name, size_t* index) = 0;
    virtual aclError aclmdlGetOutputDims(const aclmdlDesc* modelDesc, size_t index, aclmdlIODims* dims) = 0;
    virtual aclFormat aclmdlGetOutputFormat(const aclmdlDesc* modelDesc, size_t index) = 0;
    virtual aclDataType aclmdlGetOutputDataType(const aclmdlDesc* modelDesc, size_t index) = 0;

    virtual aclError aclmdlGetInputDynamicDims(const aclmdlDesc* modelDesc, size_t profileIndex, aclmdlIODims* dims, size_t gearCount) = 0;
    virtual aclError aclmdlSetInputDynamicDims(uint32_t modelId, aclmdlDataset* dataset, size_t index, const aclmdlIODims* dims) = 0;
    virtual aclError aclmdlGetInputDynamicGearCount(const aclmdlDesc* modelDesc, size_t index, size_t* dymGearCount) = 0;
    virtual aclError aclmdlGetCurOutputDims(const aclmdlDesc* modelDesc, size_t index, aclmdlIODims* ioDims) = 0;

    virtual aclTensorDesc* aclCreateTensorDesc(aclDataType dataType, int numDims,
        const int64_t* dims, aclFormat format) = 0;
    virtual aclError aclmdlSetDatasetTensorDesc(aclmdlDataset* dataset, aclTensorDesc* tensorDesc, size_t index) = 0;
    virtual aclmdlDataset* aclmdlCreateDataset() = 0;
    virtual aclError aclmdlDestroyDataset(const aclmdlDataset *dataset) = 0;

    virtual aclError aclmdlGetDynamicHW(const aclmdlDesc* modelDesc, size_t profileIndex, aclmdlHW* dynamicHW) = 0;
    virtual aclError aclmdlSetDynamicHWSize(uint32_t modelId, aclmdlDataset* dataset, size_t index,
                                          uint64_t dynamicHeight, uint64_t dynamicWidth) = 0;
    virtual aclError aclmdlSetDynamicBatchSize(uint32_t modelId, aclmdlDataset* dataset, size_t index,
                                             uint64_t dynamicBatchSize) = 0;
    virtual aclError aclmdlGetDynamicBatch(const aclmdlDesc* modelDesc, aclmdlBatch* batchInfo) = 0;

    virtual aclDataBuffer* aclCreateDataBuffer(void* data, size_t size) = 0;
    virtual aclError aclDestroyDataBuffer(const aclDataBuffer* dataBuffer) = 0;
    virtual aclError aclmdlAddDatasetBuffer(aclmdlDataset* dataset, aclDataBuffer* dataBuffer) = 0;

    virtual size_t aclmdlGetDatasetNumBuffers(const aclmdlDataset* dataset) = 0;
    virtual aclDataBuffer* aclmdlGetDatasetBuffer(const aclmdlDataset* dataset, size_t index) = 0;
    virtual size_t aclGetDataBufferSizeV2(const aclDataBuffer* dataBuffer) = 0;
    virtual void* aclGetDataBufferAddr(const aclDataBuffer* dataBuffer) = 0;
    virtual aclError aclUpdateDataBuffer(aclDataBuffer* dataBuffer, void* addr, size_t size) = 0;

    virtual aclError aclrtMalloc(void** devPtr, size_t size, aclrtMemMallocPolicy policy) = 0;
    virtual aclError aclrtFree(void* devPtr) = 0;
    virtual aclError aclrtMemcpy(void* dst, size_t destMax, const void* src, size_t count, aclrtMemcpyKind kind) = 0;
    virtual aclError aclrtMemset(void* devPtr, size_t maxCount, int32_t value, size_t count) = 0;

    virtual size_t aclGetTensorDescNumDims(const aclTensorDesc*) = 0;
    virtual aclError aclGetTensorDescDimV2(const aclTensorDesc*, size_t, int64_t*) = 0;
    virtual size_t aclGetTensorDescSize(const aclTensorDesc*) = 0;
    virtual aclTensorDesc* aclmdlGetDatasetTensorDesc(const aclmdlDataset*, size_t) = 0;

    virtual aclmdlAIPP* aclmdlCreateAIPP(uint64_t) = 0;
    virtual aclError aclmdlGetAippType(uint32_t, size_t, aclmdlInputAippType*, size_t*) = 0;
    virtual aclError aclmdlSetInputAIPP(uint32_t, aclmdlDataset*, size_t, const aclmdlAIPP*) = 0;
    virtual aclError aclmdlSetAIPPSrcImageSize(aclmdlAIPP*, int32_t, int32_t) = 0;
    virtual aclError aclmdlSetAIPPInputFormat(aclmdlAIPP*, aclAippInputFormat) = 0;
    virtual aclError aclmdlSetAIPPRbuvSwapSwitch(aclmdlAIPP*, int8_t) = 0;
    virtual aclError aclmdlSetAIPPAxSwapSwitch(aclmdlAIPP*, int8_t) = 0;
    virtual aclError aclmdlSetAIPPDtcPixelMean(aclmdlAIPP*, int16_t, int16_t, int16_t, int16_t, size_t) = 0;
    virtual aclError aclmdlSetAIPPDtcPixelMin(aclmdlAIPP*, float, float, float, float, uint64_t) = 0;
    virtual aclError aclmdlSetAIPPPixelVarReci(aclmdlAIPP*, float, float, float, float, uint64_t) = 0;
    virtual aclError aclmdlSetAIPPCropParams(aclmdlAIPP*, int8_t, int32_t, int32_t, int32_t, int32_t, uint64_t) = 0;
    virtual aclError aclmdlSetAIPPPaddingParams(aclmdlAIPP*, int8_t, int32_t, int32_t, int32_t, int32_t, uint64_t) = 0;
    virtual aclError aclmdlDestroyAIPP(const aclmdlAIPP*) = 0;

    virtual aclError aclrtSetExceptionInfoCallback(ACL_EXCEPTION_CALLBACK callback) = 0;
    virtual aclError aclrtMallocHost(void** ptr, size_t size) = 0;
    virtual aclError acldvppMalloc(void** ptr, size_t size) = 0;
    virtual aclError acldvppFree(void* ptr) = 0;
    virtual aclError aclrtFreeHost(void* hostData) = 0;
    virtual aclError aclmdlExecute(uint32_t modelId, const aclmdlDataset* input, aclmdlDataset* output) = 0;

    virtual aclError aclInit(const char* configPath) = 0;
    virtual aclError aclFinalize() = 0;
    virtual aclError aclrtSetDevice(int devId) = 0;
    virtual aclError aclrtGetDeviceCount(uint32_t* count) = 0;
    virtual aclError aclrtCreateContext(void** context, int devId) = 0;
    virtual aclError aclrtDestroyContext(void* context) = 0;
    virtual aclError aclrtSetCurrentContext(void* context) = 0;
    virtual aclError aclrtResetDevice(int devId) = 0;
    virtual aclError aclrtGetCurrentContext(void** context) = 0;

    virtual const char* aclGetRecentErrMsg() = 0;

};

// Mock 实现
class MockACL : public ACLInterface {
public:
    MOCK_METHOD(aclError, aclmdlLoadFromFile, (const char*, uint32_t*), (override));
    MOCK_METHOD(aclError, aclmdlUnload, (uint32_t), (override));

    MOCK_METHOD(aclmdlDesc*, aclmdlCreateDesc, (), (override));
    MOCK_METHOD(aclError, aclmdlDestroyDesc, (aclmdlDesc*), (override));
    MOCK_METHOD(aclError, aclmdlGetDesc, (aclmdlDesc*, uint32_t), (override));

    MOCK_METHOD(size_t, aclmdlGetNumInputs, (aclmdlDesc*), (override));
    MOCK_METHOD(const char*, aclmdlGetInputNameByIndex, (const aclmdlDesc*, size_t), (override));
    MOCK_METHOD(aclError, aclmdlGetInputIndexByName, (const aclmdlDesc*, const char*, size_t*), (override));
    MOCK_METHOD(size_t, aclmdlGetInputSizeByIndex, (aclmdlDesc*, size_t), (override));
    MOCK_METHOD(aclError, aclmdlGetInputDims, (const aclmdlDesc*, size_t, aclmdlIODims*), (override));
    MOCK_METHOD(aclFormat, aclmdlGetInputFormat, (const aclmdlDesc*, size_t), (override));
    MOCK_METHOD(aclDataType, aclmdlGetInputDataType, (const aclmdlDesc*, size_t), (override));

    MOCK_METHOD(size_t, aclmdlGetNumOutputs, (aclmdlDesc*), (override));
    MOCK_METHOD(const char*, aclmdlGetOutputNameByIndex, (const aclmdlDesc*, size_t), (override));
    MOCK_METHOD(aclError, aclmdlGetOutputIndexByName, (const aclmdlDesc*, const char*, size_t*), (override));
    MOCK_METHOD(size_t, aclmdlGetOutputSizeByIndex, (aclmdlDesc*, size_t), (override));
    MOCK_METHOD(aclError, aclmdlGetOutputDims, (const aclmdlDesc*, size_t, aclmdlIODims*), (override));
    MOCK_METHOD(aclFormat, aclmdlGetOutputFormat, (const aclmdlDesc*, size_t), (override));
    MOCK_METHOD(aclDataType, aclmdlGetOutputDataType, (const aclmdlDesc*, size_t), (override));

    MOCK_METHOD(aclError, aclmdlGetInputDynamicDims, (const aclmdlDesc*, size_t, aclmdlIODims*, size_t), (override));
    MOCK_METHOD(aclError, aclmdlSetInputDynamicDims, (uint32_t, aclmdlDataset*, size_t, const aclmdlIODims*), (override));
    MOCK_METHOD(aclError, aclmdlGetInputDynamicGearCount, (const aclmdlDesc*, size_t, size_t*), (override));
    MOCK_METHOD(aclError, aclmdlGetCurOutputDims, (const aclmdlDesc*, size_t, aclmdlIODims*), (override));

    MOCK_METHOD(aclTensorDesc*, aclCreateTensorDesc,
                (aclDataType dataType, int numDims, const int64_t* dims, aclFormat format), (override));
    MOCK_METHOD(aclError, aclmdlSetDatasetTensorDesc,
                (aclmdlDataset* dataset, aclTensorDesc* tensorDesc, size_t index), (override));
    MOCK_METHOD(aclmdlDataset*, aclmdlCreateDataset, (), (override));
    MOCK_METHOD(aclError, aclmdlDestroyDataset, (const aclmdlDataset *dataset), (override));

    MOCK_METHOD(aclError, aclmdlGetDynamicHW, (const aclmdlDesc*, size_t, aclmdlHW*), (override));
    MOCK_METHOD(aclError, aclmdlSetDynamicHWSize, (uint32_t, aclmdlDataset*, size_t, uint64_t, uint64_t), (override));
    MOCK_METHOD(aclError, aclmdlSetDynamicBatchSize, (uint32_t, aclmdlDataset*, size_t, uint64_t), (override));
    MOCK_METHOD(aclError, aclmdlGetDynamicBatch, (const aclmdlDesc*, aclmdlBatch*), (override));

    MOCK_METHOD(aclDataBuffer*, aclCreateDataBuffer, (void*, size_t), (override));
    MOCK_METHOD(aclError, aclDestroyDataBuffer, (const aclDataBuffer*), (override));
    MOCK_METHOD(aclError, aclmdlAddDatasetBuffer, (aclmdlDataset*, aclDataBuffer*), (override));

    MOCK_METHOD(size_t, aclmdlGetDatasetNumBuffers, (const aclmdlDataset*), (override));
    MOCK_METHOD(aclDataBuffer*, aclmdlGetDatasetBuffer, (const aclmdlDataset*, size_t), (override));
    MOCK_METHOD(size_t, aclGetDataBufferSizeV2, (const aclDataBuffer*), (override));
    MOCK_METHOD(void*, aclGetDataBufferAddr, (const aclDataBuffer*), (override));
    MOCK_METHOD(aclError, aclUpdateDataBuffer, (aclDataBuffer*, void*, size_t), (override));

    MOCK_METHOD(aclError, aclrtMalloc, (void**, size_t, aclrtMemMallocPolicy), (override));
    MOCK_METHOD(aclError, aclrtFree, (void*), (override));
    MOCK_METHOD(aclError, aclrtMemcpy, (void*, size_t, const void*, size_t, aclrtMemcpyKind), (override));
    MOCK_METHOD(aclError, aclrtMemset, (void *, size_t, int32_t, size_t), (override));

    MOCK_METHOD(size_t, aclGetTensorDescNumDims, (const aclTensorDesc*), (override));
    MOCK_METHOD(aclError, aclGetTensorDescDimV2, (const aclTensorDesc*, size_t, int64_t*), (override));
    MOCK_METHOD(size_t, aclGetTensorDescSize, (const aclTensorDesc*), (override));
    MOCK_METHOD(aclTensorDesc*, aclmdlGetDatasetTensorDesc, (const aclmdlDataset*, size_t), (override));

    MOCK_METHOD(aclmdlAIPP*, aclmdlCreateAIPP, (uint64_t), (override));
    MOCK_METHOD(aclError, aclmdlGetAippType, (uint32_t, size_t, aclmdlInputAippType*, size_t*), (override));
    MOCK_METHOD(aclError, aclmdlSetInputAIPP, (uint32_t, aclmdlDataset*, size_t, const aclmdlAIPP*), (override));
    MOCK_METHOD(aclError, aclmdlSetAIPPSrcImageSize, (aclmdlAIPP*, int32_t, int32_t), (override));
    MOCK_METHOD(aclError, aclmdlSetAIPPInputFormat, (aclmdlAIPP*, aclAippInputFormat), (override));
    MOCK_METHOD(aclError, aclmdlSetAIPPRbuvSwapSwitch, (aclmdlAIPP*, int8_t), (override));
    MOCK_METHOD(aclError, aclmdlSetAIPPAxSwapSwitch, (aclmdlAIPP*, int8_t), (override));
    MOCK_METHOD(aclError, aclmdlSetAIPPDtcPixelMean, (aclmdlAIPP*, int16_t, int16_t, int16_t, int16_t, size_t), (override));
    MOCK_METHOD(aclError, aclmdlSetAIPPDtcPixelMin, (aclmdlAIPP*, float, float, float, float, uint64_t), (override));
    MOCK_METHOD(aclError, aclmdlSetAIPPPixelVarReci, (aclmdlAIPP*, float, float, float, float, uint64_t), (override));
    MOCK_METHOD(aclError, aclmdlSetAIPPCropParams, (aclmdlAIPP*, int8_t, int32_t, int32_t, int32_t, int32_t, uint64_t), (override));
    MOCK_METHOD(aclError, aclmdlSetAIPPPaddingParams, (aclmdlAIPP*, int8_t, int32_t, int32_t, int32_t, int32_t, uint64_t), (override));
    MOCK_METHOD(aclError, aclmdlDestroyAIPP, (const aclmdlAIPP*), (override));

    MOCK_METHOD(aclError, aclrtMallocHost, (void** ptr, size_t size), (override));
    MOCK_METHOD(aclError, aclrtFreeHost, (void*), (override));
    MOCK_METHOD(aclError, acldvppMalloc, (void** ptr, size_t size), (override));
    MOCK_METHOD(aclError, acldvppFree, (void* ptr), (override));
    MOCK_METHOD(aclError, aclrtSetExceptionInfoCallback, (ACL_EXCEPTION_CALLBACK), (override));
    MOCK_METHOD(aclError, aclmdlExecute, (uint32_t, const aclmdlDataset*, aclmdlDataset*), (override));

    MOCK_METHOD(aclError, aclInit, (const char*), (override));
    MOCK_METHOD(aclError, aclFinalize, (), (override));
    MOCK_METHOD(aclError, aclrtSetDevice, (int), (override));
    MOCK_METHOD(aclError, aclrtGetDeviceCount, (uint32_t*), (override));
    MOCK_METHOD(aclError, aclrtCreateContext, (void**, int), (override));
    MOCK_METHOD(aclError, aclrtDestroyContext, (void*), (override));
    MOCK_METHOD(aclError, aclrtSetCurrentContext, (void*), (override));
    MOCK_METHOD(aclError, aclrtResetDevice, (int), (override));
    MOCK_METHOD(aclError, aclrtGetCurrentContext, (void**), (override));

    MOCK_METHOD(const char*, aclGetRecentErrMsg, (), (override));
};

// 全局模拟对象
extern MockACL* g_mockAcl;

extern "C" {

// 模型管理函数
aclError aclmdlLoadFromFile(const char* modelPath, uint32_t* modelId);
aclError aclmdlUnload(uint32_t modelId);
aclmdlDesc* aclmdlCreateDesc();
aclError aclmdlDestroyDesc(aclmdlDesc* modelDesc);
aclError aclmdlGetDesc(aclmdlDesc* modelDesc, uint32_t modelId);
aclError aclmdlGetInputDynamicGearCount(const aclmdlDesc* modelDesc, size_t index, size_t* dymGearCount);

// 输入相关函数
size_t aclmdlGetNumInputs(aclmdlDesc* modelDesc);
const char* aclmdlGetInputNameByIndex(const aclmdlDesc* modelDesc, size_t index);
aclError aclmdlGetInputIndexByName(const aclmdlDesc* modelDesc, const char* name, size_t* index);
size_t aclmdlGetInputSizeByIndex(aclmdlDesc* modelDesc, size_t index);
aclError aclmdlGetInputDims(const aclmdlDesc* modelDesc, size_t index, aclmdlIODims* dims);
aclFormat aclmdlGetInputFormat(const aclmdlDesc* modelDesc, size_t index);
aclDataType aclmdlGetInputDataType(const aclmdlDesc* modelDesc, size_t index);

// 输出相关函数
size_t aclmdlGetNumOutputs(aclmdlDesc* modelDesc);
const char* aclmdlGetOutputNameByIndex(const aclmdlDesc* modelDesc, size_t index);
aclError aclmdlGetOutputIndexByName(const aclmdlDesc* modelDesc, const char* name, size_t* index);
size_t aclmdlGetOutputSizeByIndex(aclmdlDesc* modelDesc, size_t index);
aclError aclmdlGetOutputDims(const aclmdlDesc* modelDesc, size_t index, aclmdlIODims* dims);
aclFormat aclmdlGetOutputFormat(const aclmdlDesc* modelDesc, size_t index);
aclDataType aclmdlGetOutputDataType(const aclmdlDesc* modelDesc, size_t index);

// 数据集相关函数
aclTensorDesc* aclCreateTensorDesc(aclDataType dataType, int numDims, const int64_t* dims, aclFormat format);
aclError aclmdlSetDatasetTensorDesc(aclmdlDataset* dataset, aclTensorDesc* tensorDesc, size_t index);
aclmdlDataset* aclmdlCreateDataset();
aclError aclmdlDestroyDataset(const aclmdlDataset* dataset);
const char* aclGetRecentErrMsg();
aclError aclmdlGetDynamicHW(const aclmdlDesc* modelDesc, size_t profileIndex, aclmdlHW* dynamicHW);
aclError aclmdlSetDynamicHWSize(uint32_t modelId, aclmdlDataset* dataset, size_t index, uint64_t dynamicHeight, uint64_t dynamicWidth);
aclError aclmdlSetDynamicBatchSize(uint32_t modelId, aclmdlDataset* dataset, size_t index, uint64_t dynamicBatchSize);
aclError aclmdlGetDynamicBatch(const aclmdlDesc* modelDesc, aclmdlBatch* batchInfo);
aclError aclmdlGetCurOutputDims(const aclmdlDesc* modelDesc, size_t index, aclmdlIODims* ioDims);
aclError aclmdlGetInputDynamicDims(const aclmdlDesc* modelDesc, size_t profileIndex, aclmdlIODims* dims, size_t gearCount);
aclError aclmdlSetInputDynamicDims(uint32_t modelId, aclmdlDataset* dataset, size_t index, const aclmdlIODims* dims);

// 数据缓冲区函数
aclDataBuffer* aclCreateDataBuffer(void* data, size_t size);
aclError aclDestroyDataBuffer(const aclDataBuffer* dataBuffer);
aclError aclmdlAddDatasetBuffer(aclmdlDataset* dataset, aclDataBuffer* dataBuffer);
size_t aclmdlGetDatasetNumBuffers(const aclmdlDataset* dataset);
aclDataBuffer* aclmdlGetDatasetBuffer(const aclmdlDataset* dataset, size_t index);
size_t aclGetDataBufferSizeV2(const aclDataBuffer* dataBuffer);
void* aclGetDataBufferAddr(const aclDataBuffer* dataBuffer);
aclError aclUpdateDataBuffer(aclDataBuffer* dataBuffer, void* addr, size_t size);

// 内存操作函数
aclError aclrtMalloc(void** devPtr, size_t size, aclrtMemMallocPolicy policy);
aclError aclrtFree(void* devPtr);
aclError aclrtMemcpy(void* dst, size_t destMax, const void* src, size_t count, aclrtMemcpyKind kind);
aclError aclrtMemset(void * devPtr, size_t maxCount, int32_t value, size_t count);
aclError aclrtMallocHost(void** ptr, size_t size);
aclError aclrtFreeHost(void* hostData);
aclError acldvppMalloc(void** ptr, size_t size);
aclError acldvppFree(void* ptr);
aclError aclrtSetExceptionInfoCallback(ACL_EXCEPTION_CALLBACK callback);
aclError aclmdlExecute(uint32_t modelId, const aclmdlDataset* input, aclmdlDataset* output);

// 张量描述函数
size_t aclGetTensorDescNumDims(const aclTensorDesc* desc);
aclError aclGetTensorDescDimV2(const aclTensorDesc* desc, size_t idx, int64_t* dim);
size_t aclGetTensorDescSize(const aclTensorDesc* desc);

// 数据集函数
aclTensorDesc* aclmdlGetDatasetTensorDesc(const aclmdlDataset* dataset, size_t index);

// AIPP函数
aclmdlAIPP* aclmdlCreateAIPP(uint64_t maxBatchSize);
aclError aclmdlGetAippType(uint32_t modelId, size_t index, aclmdlInputAippType* aippType, size_t* dynamicAttachedDataIndex);
aclError aclmdlSetInputAIPP(uint32_t modelId, aclmdlDataset* input, size_t index, const aclmdlAIPP* pAippDynamicSet);
aclError aclmdlSetAIPPSrcImageSize(aclmdlAIPP* aippDynamicSet, int32_t srcImageSizeW, int32_t srcImageSizeH);
aclError aclmdlSetAIPPInputFormat(aclmdlAIPP* aippDynamicSet, aclAippInputFormat inputFormat);
aclError aclmdlSetAIPPRbuvSwapSwitch(aclmdlAIPP* aippDynamicSet, int8_t rbuvSwapSwitch);
aclError aclmdlSetAIPPAxSwapSwitch(aclmdlAIPP* aippDynamicSet, int8_t axSwapSwitch);
aclError aclmdlSetAIPPDtcPixelMean(aclmdlAIPP* aippDynamicSet, int16_t dtcPixelMeanChn0, int16_t dtcPixelMeanChn1,
                                 int16_t dtcPixelMeanChn2, int16_t dtcPixelMeanChn3, size_t batchIndex);
aclError aclmdlSetAIPPDtcPixelMin(aclmdlAIPP* aippDynamicSet, float dtcPixelMinChn0, float dtcPixelMinChn1,
                                float dtcPixelMinChn2, float dtcPixelMinChn3, uint64_t batchIndex);
aclError aclmdlSetAIPPPixelVarReci(aclmdlAIPP* aippDynamicSet, float dtcPixelVarReciChn0, float dtcPixelVarReciChn1,
                                 float dtcPixelVarReciChn2, float dtcPixelVarReciChn3, uint64_t batchIndex);
aclError aclmdlSetAIPPCropParams(aclmdlAIPP* aippDynamicSet, int8_t cropSwitch, int32_t loadStartPosW,
                               int32_t loadStartPosH, int32_t cropSizeW, int32_t cropSizeH, uint64_t batchIndex);
aclError aclmdlSetAIPPPaddingParams(aclmdlAIPP* aippDynamicSet, int8_t paddingSwitch, int32_t paddingSizeTop,
                                  int32_t paddingSizeBottom, int32_t paddingSizeLeft, int32_t paddingSizeRight, uint64_t batchIndex);
aclError aclmdlDestroyAIPP(const aclmdlAIPP* aippParmsSet);

// DeviceManager 测试需要的ACL接口声明
aclError aclInit(const char* configPath);
aclError aclFinalize();
aclError aclrtSetDevice(int devId);
aclError aclrtGetDeviceCount(uint32_t* count);
aclError aclrtCreateContext(void** context, int devId);
aclError aclrtDestroyContext(void* context);
aclError aclrtSetCurrentContext(void* context);
aclError aclrtResetDevice(int devId);
aclError aclrtGetCurrentContext(void** context);

}

#endif
