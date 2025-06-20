#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include <vector>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>

#include "test_utils.hpp"
#include "base/include/Base/ModelInfer/model_process.h"

using namespace std;
using namespace testing;

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
    virtual aclError aclrtFreeHost(void* hostData) = 0;
    virtual aclError aclmdlExecute(uint32_t modelId, const aclmdlDataset* input, aclmdlDataset* output) = 0;

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

    MOCK_METHOD(aclError, aclrtFreeHost, (void*), (override));
    MOCK_METHOD(aclError, aclrtSetExceptionInfoCallback, (ACL_EXCEPTION_CALLBACK), (override));
    MOCK_METHOD(aclError, aclmdlExecute, (uint32_t, const aclmdlDataset*, aclmdlDataset*), (override));

    MOCK_METHOD(const char*, aclGetRecentErrMsg, (), (override));
};

// 全局模拟对象
static MockACL* g_mockAcl = nullptr;

// "C" 接口包装器
extern "C" {

// 修复宏定义：使用参数声明列表和参数名称列表分离
#define DEFINE_MOCK_C_API(ReturnType, FuncName, ParamDecl, ParamNames) \
    ReturnType FuncName ParamDecl \
    { \
        return g_mockAcl->FuncName ParamNames; \
    }

DEFINE_MOCK_C_API(aclError, aclmdlLoadFromFile, (const char* modelPath, uint32_t* modelId), (modelPath, modelId))
DEFINE_MOCK_C_API(aclError, aclmdlUnload, (uint32_t modelId), (modelId))
DEFINE_MOCK_C_API(aclmdlDesc*, aclmdlCreateDesc, (), ())
DEFINE_MOCK_C_API(aclError, aclmdlDestroyDesc, (aclmdlDesc* modelDesc), (modelDesc))
DEFINE_MOCK_C_API(aclError, aclmdlGetDesc, (aclmdlDesc* modelDesc, uint32_t modelId), (modelDesc, modelId))
DEFINE_MOCK_C_API(aclError, aclmdlGetInputDynamicGearCount, 
             (const aclmdlDesc* modelDesc, size_t index, size_t* dymGearCount), 
             (modelDesc, index, dymGearCount))

// Input Related
DEFINE_MOCK_C_API(size_t, aclmdlGetNumInputs, (aclmdlDesc* modelDesc), (modelDesc))
DEFINE_MOCK_C_API(const char*, aclmdlGetInputNameByIndex, 
             (const aclmdlDesc* modelDesc, size_t index), 
             (modelDesc, index))
DEFINE_MOCK_C_API(aclError, aclmdlGetInputIndexByName, 
             (const aclmdlDesc* modelDesc, const char* name, size_t* index), 
             (modelDesc, name, index))
DEFINE_MOCK_C_API(size_t, aclmdlGetInputSizeByIndex, 
             (aclmdlDesc* modelDesc, size_t index), 
             (modelDesc, index))
DEFINE_MOCK_C_API(aclError, aclmdlGetInputDims, 
             (const aclmdlDesc* modelDesc, size_t index, aclmdlIODims* dims), 
             (modelDesc, index, dims))
DEFINE_MOCK_C_API(aclFormat, aclmdlGetInputFormat, 
             (const aclmdlDesc* modelDesc, size_t index), 
             (modelDesc, index))
DEFINE_MOCK_C_API(aclDataType, aclmdlGetInputDataType, 
             (const aclmdlDesc* modelDesc, size_t index), 
             (modelDesc, index))

// Output Related
DEFINE_MOCK_C_API(size_t, aclmdlGetNumOutputs, (aclmdlDesc* modelDesc), (modelDesc))
DEFINE_MOCK_C_API(const char*, aclmdlGetOutputNameByIndex, 
             (const aclmdlDesc* modelDesc, size_t index), 
             (modelDesc, index))
DEFINE_MOCK_C_API(aclError, aclmdlGetOutputIndexByName, 
             (const aclmdlDesc* modelDesc, const char* name, size_t* index), 
             (modelDesc, name, index))
DEFINE_MOCK_C_API(size_t, aclmdlGetOutputSizeByIndex, 
             (aclmdlDesc* modelDesc, size_t index), 
             (modelDesc, index))
DEFINE_MOCK_C_API(aclError, aclmdlGetOutputDims, 
             (const aclmdlDesc* modelDesc, size_t index, aclmdlIODims* dims), 
             (modelDesc, index, dims))
DEFINE_MOCK_C_API(aclFormat, aclmdlGetOutputFormat, 
             (const aclmdlDesc* modelDesc, size_t index), 
             (modelDesc, index))
DEFINE_MOCK_C_API(aclDataType, aclmdlGetOutputDataType, 
             (const aclmdlDesc* modelDesc, size_t index), 
             (modelDesc, index))

// Dataset Related
DEFINE_MOCK_C_API(aclTensorDesc*, aclCreateTensorDesc, 
             (aclDataType dataType, int numDims, const int64_t* dims, aclFormat format), 
             (dataType, numDims, dims, format))
DEFINE_MOCK_C_API(aclError, aclmdlSetDatasetTensorDesc, 
             (aclmdlDataset* dataset, aclTensorDesc* tensorDesc, size_t index), 
             (dataset, tensorDesc, index))
DEFINE_MOCK_C_API(aclmdlDataset*, aclmdlCreateDataset, (), ())
DEFINE_MOCK_C_API(aclError, aclmdlDestroyDataset, (const aclmdlDataset* dataset), (dataset))
DEFINE_MOCK_C_API(const char*, aclGetRecentErrMsg, (), ())
DEFINE_MOCK_C_API(aclError, aclmdlGetDynamicHW, 
             (const aclmdlDesc* modelDesc, size_t profileIndex, aclmdlHW* dynamicHW), 
             (modelDesc, profileIndex, dynamicHW))
DEFINE_MOCK_C_API(aclError, aclmdlSetDynamicHWSize, 
             (uint32_t modelId, aclmdlDataset* dataset, size_t index,
              uint64_t dynamicHeight, uint64_t dynamicWidth), 
             (modelId, dataset, index, dynamicHeight, dynamicWidth))
DEFINE_MOCK_C_API(aclError, aclmdlSetDynamicBatchSize, 
             (uint32_t modelId, aclmdlDataset* dataset, size_t index, uint64_t dynamicBatchSize), 
             (modelId, dataset, index, dynamicBatchSize))
DEFINE_MOCK_C_API(aclError, aclmdlGetDynamicBatch, 
             (const aclmdlDesc* modelDesc, aclmdlBatch* batchInfo), 
             (modelDesc, batchInfo))
DEFINE_MOCK_C_API(aclError, aclmdlGetCurOutputDims, 
             (const aclmdlDesc* modelDesc, size_t index, aclmdlIODims* ioDims), 
             (modelDesc, index, ioDims))
DEFINE_MOCK_C_API(aclError, aclmdlGetInputDynamicDims, 
             (const aclmdlDesc* modelDesc, size_t profileIndex, aclmdlIODims* dims, size_t gearCount), 
             (modelDesc, profileIndex, dims, gearCount))
DEFINE_MOCK_C_API(aclError, aclmdlSetInputDynamicDims, 
             (uint32_t modelId, aclmdlDataset* dataset, size_t index, const aclmdlIODims* dims), 
             (modelId, dataset, index, dims))

DEFINE_MOCK_C_API(aclDataBuffer*, aclCreateDataBuffer, (void* data, size_t size), (data, size))
DEFINE_MOCK_C_API(aclError, aclDestroyDataBuffer, (const aclDataBuffer* dataBuffer), (dataBuffer))
DEFINE_MOCK_C_API(aclError, aclmdlAddDatasetBuffer, 
             (aclmdlDataset* dataset, aclDataBuffer* dataBuffer), 
             (dataset, dataBuffer))
DEFINE_MOCK_C_API(size_t, aclmdlGetDatasetNumBuffers, (const aclmdlDataset* dataset), (dataset))
DEFINE_MOCK_C_API(aclDataBuffer*, aclmdlGetDatasetBuffer, 
             (const aclmdlDataset* dataset, size_t index), 
             (dataset, index))
DEFINE_MOCK_C_API(size_t, aclGetDataBufferSizeV2, (const aclDataBuffer* dataBuffer), (dataBuffer))
DEFINE_MOCK_C_API(void*, aclGetDataBufferAddr, (const aclDataBuffer* dataBuffer), (dataBuffer))
DEFINE_MOCK_C_API(aclError, aclUpdateDataBuffer, 
             (aclDataBuffer* dataBuffer, void* addr, size_t size), 
             (dataBuffer, addr, size))

// Memory Operations
DEFINE_MOCK_C_API(aclError, aclrtMalloc, 
             (void** devPtr, size_t size, aclrtMemMallocPolicy policy), 
             (devPtr, size, policy))
DEFINE_MOCK_C_API(aclError, aclrtFree, (void* devPtr), (devPtr))
DEFINE_MOCK_C_API(aclError, aclrtMemcpy, 
             (void* dst, size_t destMax, const void* src, size_t count, aclrtMemcpyKind kind), 
             (dst, destMax, src, count, kind))
DEFINE_MOCK_C_API(aclError, aclrtMemset,
             (void * devPtr, size_t maxCount, int32_t value, size_t count),
             (devPtr, maxCount, value, count))

DEFINE_MOCK_C_API(aclError, aclrtFreeHost, (void* hostData), (hostData))
DEFINE_MOCK_C_API(aclError, aclrtSetExceptionInfoCallback, (ACL_EXCEPTION_CALLBACK callback), (callback))
DEFINE_MOCK_C_API(aclError, aclmdlExecute, 
                 (uint32_t modelId, const aclmdlDataset* input, aclmdlDataset* output), 
                 (modelId, input, output))

// 张量描述函数
DEFINE_MOCK_C_API(size_t, aclGetTensorDescNumDims, (const aclTensorDesc* desc), (desc))
DEFINE_MOCK_C_API(aclError, aclGetTensorDescDimV2, (const aclTensorDesc* desc, size_t idx, int64_t* dim), (desc, idx, dim))
DEFINE_MOCK_C_API(size_t, aclGetTensorDescSize, (const aclTensorDesc* desc), (desc))

// 数据集函数
DEFINE_MOCK_C_API(aclTensorDesc*, aclmdlGetDatasetTensorDesc, (const aclmdlDataset* dataset, size_t index), (dataset, index))

// AIPP 函数
DEFINE_MOCK_C_API(aclmdlAIPP*, aclmdlCreateAIPP, (uint64_t maxBatchSize), (maxBatchSize));
DEFINE_MOCK_C_API(aclError, aclmdlGetAippType, 
    (uint32_t modelId, size_t index, aclmdlInputAippType* aippType, size_t* dynamicAttachedDataIndex), 
    (modelId, index, aippType, dynamicAttachedDataIndex))
DEFINE_MOCK_C_API(aclError, aclmdlSetInputAIPP, 
    (uint32_t modelId, aclmdlDataset* input, size_t index, const aclmdlAIPP* pAippDynamicSet), 
    (modelId, input, index, pAippDynamicSet))
DEFINE_MOCK_C_API(aclError, aclmdlSetAIPPSrcImageSize, 
    (aclmdlAIPP* aippDynamicSet, int32_t srcImageSizeW, int32_t srcImageSizeH), 
    (aippDynamicSet, srcImageSizeW, srcImageSizeH))
DEFINE_MOCK_C_API(aclError, aclmdlSetAIPPInputFormat, 
    (aclmdlAIPP* aippDynamicSet, aclAippInputFormat inputFormat), 
    (aippDynamicSet, inputFormat))
DEFINE_MOCK_C_API(aclError, aclmdlSetAIPPRbuvSwapSwitch,
    (aclmdlAIPP* aippDynamicSet, int8_t rbuvSwapSwitch),
    (aippDynamicSet, rbuvSwapSwitch))
DEFINE_MOCK_C_API(aclError, aclmdlSetAIPPAxSwapSwitch,
    (aclmdlAIPP* aippDynamicSet, int8_t axSwapSwitch),
    (aippDynamicSet, axSwapSwitch))
DEFINE_MOCK_C_API(aclError, aclmdlSetAIPPDtcPixelMean,
    (aclmdlAIPP* aippDynamicSet, int16_t dtcPixelMeanChn0, int16_t dtcPixelMeanChn1, 
     int16_t dtcPixelMeanChn2, int16_t dtcPixelMeanChn3, size_t batchIndex), 
    (aippDynamicSet, dtcPixelMeanChn0, dtcPixelMeanChn1, dtcPixelMeanChn2, dtcPixelMeanChn3, batchIndex));
DEFINE_MOCK_C_API(aclError, aclmdlSetAIPPDtcPixelMin,
    (aclmdlAIPP* aippDynamicSet, float dtcPixelMinChn0, float dtcPixelMinChn1, 
     float dtcPixelMinChn2, float dtcPixelMinChn3, uint64_t batchIndex), 
    (aippDynamicSet, dtcPixelMinChn0, dtcPixelMinChn1, dtcPixelMinChn2, dtcPixelMinChn3, batchIndex));
DEFINE_MOCK_C_API(aclError, aclmdlSetAIPPPixelVarReci, 
    (aclmdlAIPP* aippDynamicSet, float dtcPixelVarReciChn0, float dtcPixelVarReciChn1, 
     float dtcPixelVarReciChn2, float dtcPixelVarReciChn3, uint64_t batchIndex), 
    (aippDynamicSet, dtcPixelVarReciChn0, dtcPixelVarReciChn1, dtcPixelVarReciChn2, dtcPixelVarReciChn3, batchIndex));
DEFINE_MOCK_C_API(aclError, aclmdlSetAIPPCropParams, 
    (aclmdlAIPP* aippDynamicSet, int8_t cropSwitch, int32_t loadStartPosW, 
     int32_t loadStartPosH, int32_t cropSizeW, int32_t cropSizeH, uint64_t batchIndex), 
    (aippDynamicSet, cropSwitch, loadStartPosW, loadStartPosH, cropSizeW, cropSizeH, batchIndex));
DEFINE_MOCK_C_API(aclError, aclmdlSetAIPPPaddingParams, 
    (aclmdlAIPP* aippDynamicSet, int8_t paddingSwitch, int32_t paddingSizeTop, 
     int32_t paddingSizeBottom, int32_t paddingSizeLeft, int32_t paddingSizeRight, uint64_t batchIndex), 
    (aippDynamicSet, paddingSwitch, paddingSizeTop, paddingSizeBottom, paddingSizeLeft, paddingSizeRight, batchIndex));
DEFINE_MOCK_C_API(aclError, aclmdlDestroyAIPP, (const aclmdlAIPP* aippParmsSet), (aippParmsSet))

// 清理宏定义
#undef DEFINE_MOCK_C_API
}


namespace AISBench_test {

const uint32_t expectedModelId = 12345; // 预期模型ID

// 设置日志Level为Debug级别，用于检查Debug时的输出内容
class SetDebugLogGuard {
public:
    explicit SetDebugLogGuard()
    {
        Base::LogCtrl::SetLogLevel(LOG_DEBUG_LEVEL);
    }
    ~SetDebugLogGuard()
    {
        Base::LogCtrl::SetLogLevel(LOG_INFO_LEVEL);
    }
};

class ModelProcessTest : public ::testing::Test {
    protected:
    void SetUp() override
    {
        // 重置模拟状态
        mockAcl = make_unique<StrictMock<MockACL>>();
        g_mockAcl = mockAcl.get();

        modelProcess = make_unique<ModelProcess>();
        // 验证构造函数初始化
        EXPECT_EQ(modelProcess->modelId_, 0);
        EXPECT_FALSE(modelProcess->loadFlag_);
        EXPECT_EQ(modelProcess->modelDesc_, nullptr);
        EXPECT_EQ(modelProcess->input_, nullptr);
        EXPECT_EQ(modelProcess->output_, nullptr);
        EXPECT_EQ(modelProcess->numInputs_, 0);
        EXPECT_EQ(modelProcess->numOutputs_, 0);
        EXPECT_FALSE(modelProcess->reuseOutput_);
        EXPECT_EQ(modelProcess->g_dymindex, 0);
        
        // 验证AIPP格式映射
        EXPECT_EQ(modelProcess->str2aclAippInputFormat.size(), 4);
        EXPECT_EQ(modelProcess->str2aclAippInputFormat["YUV420SP_U8"], ACL_YUV420SP_U8);
        EXPECT_EQ(modelProcess->str2aclAippInputFormat["XRGB8888_U8"], ACL_XRGB8888_U8);
        EXPECT_EQ(modelProcess->str2aclAippInputFormat["RGB888_U8"], ACL_RGB888_U8);
        EXPECT_EQ(modelProcess->str2aclAippInputFormat["YUV400_U8"], ACL_YUV400_U8);
    
        validModelPath = "valid_model.om";
        invalidModelPath = "invalid_model.om";

        // 配置模拟函数的默认值
        SetGlobalDefaultExpectations();
        
    }

    void TearDown() override
    {
        // 先释放被测对象
        modelProcess.reset();

        // 再释放模拟对象
        g_mockAcl = nullptr;
        mockAcl.reset();
    }
    
    // 辅助函数：为所有新增的 ACL 接口设置默认返回值
    void SetGlobalDefaultExpectations()
    {   

// 定义宏简化代码
#define SET_DEFAULT_EXPECT(func_name, return_val) \
    EXPECT_CALL(*mockAcl, func_name).WillRepeatedly(Return(return_val));

        // 使用双括号处理带逗号的复杂返回值
        SET_DEFAULT_EXPECT(aclmdlLoadFromFile, (ACL_SUCCESS))
        SET_DEFAULT_EXPECT(aclmdlUnload, (ACL_SUCCESS))
        SET_DEFAULT_EXPECT(aclmdlCreateDesc, (nullptr))
        SET_DEFAULT_EXPECT(aclmdlDestroyDesc, (ACL_SUCCESS))
        SET_DEFAULT_EXPECT(aclmdlGetDesc, (ACL_SUCCESS))
        SET_DEFAULT_EXPECT(aclmdlGetInputDynamicGearCount, (ACL_SUCCESS))
        
        // 其他默认设置保持不变...
        SET_DEFAULT_EXPECT(aclmdlGetDatasetNumBuffers, (0))
        SET_DEFAULT_EXPECT(aclmdlGetDatasetBuffer, (nullptr))
        
        // 输入
        SET_DEFAULT_EXPECT(aclmdlGetNumInputs, (0))
        SET_DEFAULT_EXPECT(aclmdlGetInputNameByIndex, (nullptr))
        SET_DEFAULT_EXPECT(aclmdlGetInputIndexByName, (ACL_SUCCESS))
        SET_DEFAULT_EXPECT(aclmdlGetInputSizeByIndex, (0))
        SET_DEFAULT_EXPECT(aclmdlGetInputDims, (ACL_SUCCESS))
        SET_DEFAULT_EXPECT(aclmdlGetInputFormat, (ACL_FORMAT_UNDEFINED))
        SET_DEFAULT_EXPECT(aclmdlGetInputDataType, (ACL_DT_UNDEFINED))
        
        // 输出
        SET_DEFAULT_EXPECT(aclmdlGetNumOutputs, (0))
        SET_DEFAULT_EXPECT(aclmdlGetOutputNameByIndex, (nullptr))
        SET_DEFAULT_EXPECT(aclmdlGetOutputIndexByName, (ACL_SUCCESS))
        SET_DEFAULT_EXPECT(aclmdlGetOutputSizeByIndex, (0))
        SET_DEFAULT_EXPECT(aclmdlGetOutputDims, (ACL_SUCCESS))
        SET_DEFAULT_EXPECT(aclmdlGetOutputFormat, (ACL_FORMAT_UNDEFINED))
        SET_DEFAULT_EXPECT(aclmdlGetOutputDataType, (ACL_DT_UNDEFINED))
        
        // 动态形状
        SET_DEFAULT_EXPECT(aclmdlGetDynamicHW, (ACL_SUCCESS))
        SET_DEFAULT_EXPECT(aclmdlSetDynamicHWSize, (ACL_SUCCESS))
        SET_DEFAULT_EXPECT(aclmdlSetDynamicBatchSize, (ACL_SUCCESS))
        SET_DEFAULT_EXPECT(aclmdlGetDynamicBatch, (ACL_SUCCESS))
        SET_DEFAULT_EXPECT(aclmdlGetCurOutputDims, (ACL_SUCCESS))
        SET_DEFAULT_EXPECT(aclmdlGetInputDynamicDims, (ACL_SUCCESS))
        SET_DEFAULT_EXPECT(aclmdlSetInputDynamicDims, (ACL_SUCCESS))
        
        // 张量和数据集
        SET_DEFAULT_EXPECT(aclCreateTensorDesc, (nullptr))
        SET_DEFAULT_EXPECT(aclmdlSetDatasetTensorDesc, (ACL_SUCCESS))
        SET_DEFAULT_EXPECT(aclmdlCreateDataset, (nullptr))
        SET_DEFAULT_EXPECT(aclmdlDestroyDataset, (ACL_SUCCESS))
        
        // 内存管理
        SET_DEFAULT_EXPECT(aclrtMalloc, (ACL_SUCCESS))
        SET_DEFAULT_EXPECT(aclrtFree, (ACL_SUCCESS))
        SET_DEFAULT_EXPECT(aclrtMemcpy, (ACL_SUCCESS))
        SET_DEFAULT_EXPECT(aclrtMemset, (ACL_SUCCESS))

        SET_DEFAULT_EXPECT(aclrtFreeHost, (ACL_SUCCESS))
        
        // 数据缓冲区
        SET_DEFAULT_EXPECT(aclCreateDataBuffer, (nullptr))
        SET_DEFAULT_EXPECT(aclDestroyDataBuffer, (ACL_SUCCESS))
        SET_DEFAULT_EXPECT(aclmdlAddDatasetBuffer, (ACL_SUCCESS))
        SET_DEFAULT_EXPECT(aclGetDataBufferSizeV2, (0))
        SET_DEFAULT_EXPECT(aclGetDataBufferAddr, (nullptr))
        SET_DEFAULT_EXPECT(aclUpdateDataBuffer, (ACL_SUCCESS))
        
        SET_DEFAULT_EXPECT(aclrtSetExceptionInfoCallback, (ACL_SUCCESS))
        // 错误处理
        SET_DEFAULT_EXPECT(aclGetRecentErrMsg, ("No error"))

#undef SET_DEFAULT_EXPECT
    }

    // 辅助函数：加载模型成功
    void LoadModelSuccess(uint32_t modelId = expectedModelId)
    {
        EXPECT_CALL(*mockAcl, aclmdlLoadFromFile(validModelPath.c_str(), _))
            .WillOnce(DoAll(SetArgPointee<1>(modelId), Return(ACL_SUCCESS)));
        ASSERT_EQ(modelProcess->LoadModelFromFile(validModelPath), SUCCESS);
        modelProcess->modelId_ = modelId;
        modelProcess->loadFlag_ = true;
    }

    // 辅助函数：创建模型描述
    aclmdlDesc* CreateModelDescSuccess(uint32_t modelId = expectedModelId)
    {
        // 加载模型
        LoadModelSuccess(modelId);
        
        // 创建模型描述
        aclmdlDesc* fakeDesc = reinterpret_cast<aclmdlDesc*>(0x1234);
        modelProcess->modelDesc_ = fakeDesc;
        EXPECT_CALL(*mockAcl, aclmdlCreateDesc())
            .WillOnce(Return(fakeDesc));
        EXPECT_CALL(*mockAcl, aclmdlGetDesc(fakeDesc, modelId))
            .WillOnce(Return(ACL_SUCCESS));
        
        EXPECT_EQ(modelProcess->CreateDesc(), SUCCESS);
        return fakeDesc;
    }

    // 辅助函数：完整模型设置（加载+创建描述）
    void SetupCompleteModel(size_t numInputs = 2, 
                           const vector<const char*>& inputNames = {"input1", "input2"})
    {
        // 模拟加载模型成功
        LoadModelSuccess();
        
        // 设置模型描述符
        aclmdlDesc* fakeDesc = reinterpret_cast<aclmdlDesc*>(0x1234);
        modelProcess->modelDesc_ = fakeDesc;
        
        // 设置模型输入信息
        EXPECT_CALL(*mockAcl, aclmdlGetNumInputs(_))
            .WillRepeatedly(Return(numInputs));
        
        for (size_t i = 0; i < numInputs; ++i) {
            EXPECT_CALL(*mockAcl, aclmdlGetInputNameByIndex(_, i))
                .WillRepeatedly(Return(inputNames[i]));
        }
        
        // 确保CreateDesc成功（模拟）
        EXPECT_CALL(*mockAcl, aclmdlCreateDesc())
            .WillOnce(Return(fakeDesc));
        EXPECT_CALL(*mockAcl, aclmdlGetDesc(fakeDesc, expectedModelId))
            .WillOnce(Return(ACL_SUCCESS));
        ASSERT_EQ(modelProcess->CreateDesc(), SUCCESS);
    }

    void SetupModelProcessInput(uintptr_t address = 0x1111) 
    {
        modelProcess->input_ = reinterpret_cast<aclmdlDataset*>(address);
    }

    void SetupModelProcessOutput(uintptr_t address = 0x2222)
    {
        modelProcess->output_ = reinterpret_cast<aclmdlDataset*>(address);
    }

    void SetupModelDesc(uintptr_t address = 0x1234)
    {
        modelProcess->modelDesc_ = reinterpret_cast<aclmdlDesc*>(address);
    }


    void SetupMockModelDescription(aclmdlDesc* fakeDesc, size_t numInputs, size_t numOutputs)
    {
        EXPECT_CALL(*mockAcl, aclmdlGetInputDims(fakeDesc, _, _))
            .Times(numInputs)
            .WillRepeatedly(Invoke([numInputs](const aclmdlDesc*, size_t index, aclmdlIODims* dimsInput) {
                index = numInputs - 1;
                dimsInput->dimCount = 1;
                dimsInput->dims[0] = 3;
                return ACL_SUCCESS;
            }));
        
        EXPECT_CALL(*mockAcl, aclmdlGetOutputDims(fakeDesc, _, _))
            .Times(numOutputs)
            .WillRepeatedly(Invoke([numOutputs](const aclmdlDesc*, size_t index, aclmdlIODims* dimsOutput) {
                index = numOutputs - 1;
                dimsOutput->dimCount = 1;
                dimsOutput->dims[0] = 3;
                return ACL_SUCCESS;
            }));
        
        if (Base::LogCtrl::CheckLogLevel(LOG_DEBUG_LEVEL)) {
            EXPECT_CALL(*mockAcl, aclmdlGetInputSizeByIndex(fakeDesc, _))
                .Times(numInputs)
                .WillRepeatedly(Return(numInputs));
            EXPECT_CALL(*mockAcl, aclmdlGetInputNameByIndex(fakeDesc, _))
                .Times(numInputs)
                .WillRepeatedly(Return("InputName"));
            EXPECT_CALL(*mockAcl, aclmdlGetInputFormat(fakeDesc, _))
                .Times(numInputs)
                .WillRepeatedly(Return(ACL_FORMAT_UNDEFINED));
            EXPECT_CALL(*mockAcl, aclmdlGetInputDataType(fakeDesc, _))
                .Times(numInputs)
                .WillRepeatedly(Return(ACL_DT_UNDEFINED));
            
            EXPECT_CALL(*mockAcl, aclmdlGetOutputSizeByIndex(fakeDesc, _))
                .Times(numOutputs)
                .WillRepeatedly(Return(numOutputs));
            EXPECT_CALL(*mockAcl, aclmdlGetOutputNameByIndex(fakeDesc, _))
                .Times(numOutputs)
                .WillRepeatedly(Return("OutputName"));
            EXPECT_CALL(*mockAcl, aclmdlGetOutputFormat(fakeDesc, _))
                .Times(numOutputs)
                .WillRepeatedly(Return(ACL_FORMAT_UNDEFINED));
            EXPECT_CALL(*mockAcl, aclmdlGetOutputDataType(fakeDesc, _))
                .Times(numOutputs)
                .WillRepeatedly(Return(ACL_DT_UNDEFINED));
        }
    }

    void SetupDataset(size_t inputCount, size_t outputCount)
    {
        // 创建输入数据集
        SetupModelProcessInput();
        SetupModelProcessOutput();
        
        // 设置模拟期望
        EXPECT_CALL(*mockAcl, aclmdlGetDatasetNumBuffers(reinterpret_cast<aclmdlDataset*>(0x1111)))
            .WillRepeatedly(Return(inputCount));
        EXPECT_CALL(*mockAcl, aclmdlGetDatasetNumBuffers(reinterpret_cast<aclmdlDataset*>(0x2222)))
            .WillRepeatedly(Return(outputCount));
    }

    unique_ptr<StrictMock<MockACL>> mockAcl; // 模拟ACL接口
    unique_ptr<ModelProcess> modelProcess;
    string validModelPath;
    string invalidModelPath;
};

// ===================== LoadModelFromFile 测试用例 =====================

// 测试首次加载成功
TEST_F(ModelProcessTest, TestLoadModelFromFile_Success)
{
    // 设置期望
    EXPECT_CALL(*mockAcl, aclmdlLoadFromFile(validModelPath.c_str(), _))
        .WillOnce(DoAll(SetArgPointee<1>(expectedModelId), Return(ACL_SUCCESS)));
    
    // 执行测试并捕获日志以验证时间和成功消息
    testing::internal::CaptureStdout();
    Result ret = modelProcess->LoadModelFromFile(validModelPath);
    string logOutput = testing::internal::GetCapturedStdout();

    // 验证返回结果和状态
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(modelProcess->loadFlag_);
    EXPECT_EQ(modelProcess->modelId_, expectedModelId);

    // 验证日志
    string expectedOutput = "load model " + validModelPath + " success";
    EXPECT_TRUE(logOutput.find(expectedOutput) != string::npos);
}

// 测试重复加载失败
TEST_F(ModelProcessTest, TestLoadModelFromFile_AlreadyLoaded)
{
    // 设置加载成功
    // const uint32_t expectedModelId = 12345;
    EXPECT_CALL(*mockAcl, aclmdlLoadFromFile(validModelPath.c_str(), _))
        .WillOnce(DoAll(SetArgPointee<1>(expectedModelId), Return(ACL_SUCCESS)));
    // 第一次加载 - 应调用ACL
    modelProcess->LoadModelFromFile(validModelPath);
    
    // 尝试第二次加载 - 不应调用ACL
    EXPECT_CALL(*mockAcl, aclmdlLoadFromFile(_, _)).Times(0);
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->LoadModelFromFile(validModelPath);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证失败状态
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(modelProcess->loadFlag_);

    // 验证日志
    string expectedOutput = "has already loaded a model";
    EXPECT_TRUE(logOutput.find("ERROR") != string::npos);
    EXPECT_TRUE(logOutput.find(expectedOutput) != string::npos);
}

// 测试加载失败路径
TEST_F(ModelProcessTest, TestLoadModelFromFile_AclFailure)
{
    // 设置加载失败
    EXPECT_CALL(*mockAcl, aclmdlLoadFromFile(invalidModelPath.c_str(), _))
        .WillOnce(Return(ACL_ERROR_FAILURE));
    
    // 设置错误消息
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillOnce(Return("Invalid model format"));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->LoadModelFromFile(invalidModelPath);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_FALSE(modelProcess->loadFlag_);
    EXPECT_EQ(modelProcess->modelId_, 0);

    // 验证日志
    string expectedOutput = "load model from file failed";
    EXPECT_TRUE(logOutput.find("ERROR") != string::npos);
    EXPECT_TRUE(logOutput.find(expectedOutput) != string::npos);
}

// ===================== CreateDesc 测试用例 =====================

// 测试创建描述成功
TEST_F(ModelProcessTest, TestCreateDesc_Success)
{
    // 创建模拟描述对象
    aclmdlDesc* fakeDesc = reinterpret_cast<aclmdlDesc*>(0x1234);
    
    // 先加载模型
    LoadModelSuccess();
    
    // 设置创建描述的成功路径
    EXPECT_CALL(*mockAcl, aclmdlCreateDesc())
        .WillOnce(Return(fakeDesc));
    EXPECT_CALL(*mockAcl, aclmdlGetDesc(fakeDesc, modelProcess->modelId_))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CreateDesc();
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(modelProcess->modelDesc_, fakeDesc);
    
    // 验证日志
    EXPECT_TRUE(logOutput.find("create model description success") != string::npos);
}

// 测试创建描述失败（aclmdlCreateDesc 返回 nullptr）
TEST_F(ModelProcessTest, TestCreateDesc_CreateFailed)
{
    // 先加载模型
    LoadModelSuccess();
    
    // 设置创建描述失败
    EXPECT_CALL(*mockAcl, aclmdlCreateDesc())
        .WillOnce(Return(nullptr));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CreateDesc();
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_EQ(modelProcess->modelDesc_, nullptr);
    
    // 验证错误日志
    EXPECT_TRUE(logOutput.find("ERROR") != string::npos);
    EXPECT_TRUE(logOutput.find("create model description failed") != string::npos);
}

// 测试获取描述失败（aclmdlGetDesc 返回错误）
TEST_F(ModelProcessTest, TestCreateDesc_GetDescFailed)
{
    // 创建模拟描述对象
    aclmdlDesc* fakeDesc = reinterpret_cast<aclmdlDesc*>(0x1234);
    
    // 先加载模型
    LoadModelSuccess();
    
    // 设置创建成功但获取失败
    EXPECT_CALL(*mockAcl, aclmdlCreateDesc())
        .WillOnce(Return(fakeDesc));
    EXPECT_CALL(*mockAcl, aclmdlGetDesc(fakeDesc, modelProcess->modelId_))
        .WillOnce(Return(ACL_ERROR_FAILURE));
    
    // 设置错误消息
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillOnce(Return("Failed to get model description"));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CreateDesc();
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_EQ(modelProcess->modelDesc_, fakeDesc);
    
    // 验证错误日志
    EXPECT_TRUE(logOutput.find("ERROR") != string::npos);
    EXPECT_TRUE(logOutput.find("get model description failed") != string::npos);
    EXPECT_TRUE(logOutput.find("Failed to get model description") != string::npos);
}

// ===================== GetDynamicGearCount 测试用例 =====================

// 测试获取动态档位成功
TEST_F(ModelProcessTest, TestGetDynamicGearCount_Success)
{
    // 创建模型描述
    aclmdlDesc* fakeDesc = CreateModelDescSuccess();
    
    // 设置获取档位成功
    size_t expectedGearCount = 3;
    EXPECT_CALL(*mockAcl, aclmdlGetInputDynamicGearCount(fakeDesc, -1, _))
        .WillOnce(DoAll(SetArgPointee<2>(expectedGearCount), Return(ACL_SUCCESS)));
    
    // 执行测试
    size_t actualGearCount = 0;
    Result ret = modelProcess->GetDynamicGearCount(actualGearCount);
    
    // 验证结果
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(actualGearCount, expectedGearCount);
}

// 测试获取动态档位失败
TEST_F(ModelProcessTest, TestGetDynamicGearCount_Failure)
{
    // 创建模型描述
    aclmdlDesc* fakeDesc = CreateModelDescSuccess();
    
    // 设置获取档位失败
    EXPECT_CALL(*mockAcl, aclmdlGetInputDynamicGearCount(fakeDesc, -1, _))
        .WillOnce(Return(ACL_ERROR_FAILURE));
    
    // 设置错误消息
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillOnce(Return("Failed to get dynamic gear count"));
    
    // 执行测试
    testing::internal::CaptureStdout();
    size_t actualGearCount = 0;
    Result ret = modelProcess->GetDynamicGearCount(actualGearCount);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    
    // 验证错误日志
    EXPECT_TRUE(logOutput.find("get input dynamic gear count failed") != string::npos);
}

// ===================== GetDynamicIndex 测试用例 =====================

// 测试有动态张量时成功
TEST_F(ModelProcessTest, TestGetDynamicIndex_DynamicTensorExists)
{
    // 创建模型描述
    aclmdlDesc* fakeDesc = CreateModelDescSuccess();
    
    // 设置输入数量
    const size_t numInputs = 3;
    EXPECT_CALL(*mockAcl, aclmdlGetNumInputs(fakeDesc))
        .WillOnce(Return(numInputs));
    
    // 设置输入名称查询
    EXPECT_CALL(*mockAcl, aclmdlGetInputNameByIndex(fakeDesc, _))
        .Times(numInputs)
        .WillRepeatedly([](const aclmdlDesc*, size_t index) {
            if (index == 1) { // 第2个输入是动态张量
                return ACL_DYNAMIC_TENSOR_NAME;  // 使用静态常量
            }
            return "input_tensor";
        });
    
    // 关键修复：使用StrEq匹配字符串内容
    size_t expectedIndex = 1;
    EXPECT_CALL(*mockAcl, aclmdlGetInputIndexByName(
        fakeDesc, 
        StrEq(ACL_DYNAMIC_TENSOR_NAME), // 使用字符串内容匹配
        NotNull()))
        .WillOnce(DoAll(
            SetArgPointee<2>(expectedIndex), 
            Return(ACL_SUCCESS)));
    
    // 执行测试
    size_t actualIndex = 0;
    Result ret = modelProcess->GetDynamicIndex(actualIndex);
    
    // 验证结果
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(actualIndex, expectedIndex);
    EXPECT_EQ(modelProcess->g_dymindex, expectedIndex);
}

// 测试没有动态张量时成功
TEST_F(ModelProcessTest, TestGetDynamicIndex_NoDynamicTensor)
{
    // 创建模型描述
    aclmdlDesc* fakeDesc = CreateModelDescSuccess();
    
    // 设置输入数量
    const size_t numInputs = 3;
    EXPECT_CALL(*mockAcl, aclmdlGetNumInputs(fakeDesc))
        .WillOnce(Return(numInputs));
    
    // 设置输入名称查询（无动态张量）
    EXPECT_CALL(*mockAcl, aclmdlGetInputNameByIndex(fakeDesc, _))
        .Times(numInputs)
        .WillRepeatedly(Return("input_tensor"));
    
    // 执行测试
    size_t actualIndex = 0;
    Result ret = modelProcess->GetDynamicIndex(actualIndex);
    
    // 验证结果
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(modelProcess->g_dymindex, static_cast<size_t>(-1));
}

// 测试获取输入名称失败
TEST_F(ModelProcessTest, TestGetDynamicIndex_GetNameFailed)
{
    // 创建模型描述
    aclmdlDesc* fakeDesc = CreateModelDescSuccess();
    
    // 设置输入数量
    const size_t numInputs = 3;
    EXPECT_CALL(*mockAcl, aclmdlGetNumInputs(fakeDesc))
        .WillOnce(Return(numInputs));
    
    // 设置名称查询失败（返回nullptr）
    EXPECT_CALL(*mockAcl, aclmdlGetInputNameByIndex(fakeDesc, _))
        .WillOnce(Return(nullptr)); // 查询第一个输入时失败
    
    // 设置错误消息
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillOnce(Return("Invalid input index"));
    
    // 执行测试
    testing::internal::CaptureStdout();
    size_t actualIndex = 0;
    Result ret = modelProcess->GetDynamicIndex(actualIndex);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    
    // 验证错误日志
    EXPECT_TRUE(logOutput.find("get input name by index failed") != string::npos);
}
}