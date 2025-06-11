#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include <vector>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <acl/acl.h>

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

    virtual aclError aclmdlGetInputDynamicGearCount(const aclmdlDesc* modelDesc, size_t index, size_t* dymGearCount) = 0;
    virtual size_t aclmdlGetNumInputs(aclmdlDesc* modelDesc) = 0;
    virtual const char* aclmdlGetInputNameByIndex(const aclmdlDesc* modelDesc, size_t index) = 0;
    virtual aclError aclmdlGetInputIndexByName(const aclmdlDesc* modelDesc, const char* name, size_t* index) = 0;

    virtual aclTensorDesc* aclCreateTensorDesc(aclDataType dataType, int numDims,
        const int64_t* dims, aclFormat format) = 0;
    virtual aclError aclmdlSetDatasetTensorDesc(aclmdlDataset* dataset, aclTensorDesc* tensorDesc, size_t index) = 0;
    virtual aclError aclmdlDestroyDataset(const aclmdlDataset *dataset) = 0;

    virtual aclError aclmdlGetDynamicHW(const aclmdlDesc* modelDesc, size_t profileIndex, aclmdlHW* dynamicHW) = 0;
    virtual aclError aclmdlSetDynamicHWSize(uint32_t modelId, aclmdlDataset* dataset, size_t index, 
                                          uint64_t dynamicHeight, uint64_t dynamicWidth) = 0;
    virtual aclError aclmdlSetDynamicBatchSize(uint32_t modelId, aclmdlDataset* dataset, size_t index, 
                                             uint64_t dynamicBatchSize) = 0;
    virtual aclError aclmdlGetDynamicBatch(const aclmdlDesc* modelDesc, aclmdlBatch* batchInfo) = 0;

    virtual aclError aclmdlGetCurOutputDims(const aclmdlDesc* modelDesc, size_t index, aclmdlIODims* ioDims) = 0;
    virtual aclError aclmdlGetInputDynamicDims(const aclmdlDesc* modelDesc, size_t profileIndex, aclmdlIODims* dims, size_t gearCount) = 0;
    virtual aclError aclmdlSetInputDynamicDims(uint32_t modelId, aclmdlDataset* dataset, size_t index, const aclmdlIODims* dims) = 0;

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

    MOCK_METHOD(aclError, aclmdlGetInputDynamicGearCount, (const aclmdlDesc*, size_t, size_t*), (override));
    MOCK_METHOD(size_t, aclmdlGetNumInputs, (aclmdlDesc*), (override));
    MOCK_METHOD(const char*, aclmdlGetInputNameByIndex, (const aclmdlDesc*, size_t), (override));
    MOCK_METHOD(aclError, aclmdlGetInputIndexByName, (const aclmdlDesc*, const char*, size_t*), (override));

    MOCK_METHOD(aclTensorDesc*, aclCreateTensorDesc, 
                (aclDataType dataType, int numDims, const int64_t* dims, aclFormat format), (override));
    MOCK_METHOD(aclError, aclmdlSetDatasetTensorDesc, 
                (aclmdlDataset* dataset, aclTensorDesc* tensorDesc, size_t index), (override));
    MOCK_METHOD(aclError, aclmdlDestroyDataset, (const aclmdlDataset *dataset), (override));

    MOCK_METHOD(aclError, aclmdlGetDynamicHW, (const aclmdlDesc*, size_t, aclmdlHW*), (override));
    MOCK_METHOD(aclError, aclmdlSetDynamicHWSize, (uint32_t, aclmdlDataset*, size_t, uint64_t, uint64_t), (override));
    MOCK_METHOD(aclError, aclmdlSetDynamicBatchSize, (uint32_t, aclmdlDataset*, size_t, uint64_t), (override));
    MOCK_METHOD(aclError, aclmdlGetDynamicBatch, (const aclmdlDesc*, aclmdlBatch*), (override));

    MOCK_METHOD(aclError, aclmdlGetCurOutputDims, (const aclmdlDesc*, size_t, aclmdlIODims*), (override));
    MOCK_METHOD(aclError, aclmdlGetInputDynamicDims, (const aclmdlDesc*, size_t, aclmdlIODims*, size_t), (override));
    MOCK_METHOD(aclError, aclmdlSetInputDynamicDims, (uint32_t, aclmdlDataset*, size_t, const aclmdlIODims*), (override));

    MOCK_METHOD(const char*, aclGetRecentErrMsg, (), (override));
};

// 全局模拟对象
static MockACL* g_mockAcl = nullptr;

// C 接口包装器
extern "C" {
aclError aclmdlLoadFromFile(const char* modelPath, uint32_t* modelId)
{
    return g_mockAcl->aclmdlLoadFromFile(modelPath, modelId);
}

aclError aclmdlUnload(uint32_t modelId)
{
    return g_mockAcl->aclmdlUnload(modelId);
}

aclmdlDesc* aclmdlCreateDesc()
{
    return g_mockAcl->aclmdlCreateDesc();
}

aclError aclmdlDestroyDesc(aclmdlDesc* modelDesc)
{
    return g_mockAcl->aclmdlDestroyDesc(modelDesc);
}

aclError aclmdlGetDesc(aclmdlDesc* modelDesc, uint32_t modelId)
{
    return g_mockAcl->aclmdlGetDesc(modelDesc, modelId);
}

aclError aclmdlGetInputDynamicGearCount(const aclmdlDesc* modelDesc, size_t index, size_t* dymGearCount)
{
    return g_mockAcl->aclmdlGetInputDynamicGearCount(modelDesc, index, dymGearCount);
}

size_t aclmdlGetNumInputs(aclmdlDesc* modelDesc)
{
    return g_mockAcl->aclmdlGetNumInputs(modelDesc);
}

const char* aclmdlGetInputNameByIndex(const aclmdlDesc* modelDesc, size_t index)
{
    return g_mockAcl->aclmdlGetInputNameByIndex(modelDesc, index);
}

aclError aclmdlGetInputIndexByName(const aclmdlDesc* modelDesc, const char* name, size_t* index)
{
    return g_mockAcl->aclmdlGetInputIndexByName(modelDesc, name, index);
}

aclTensorDesc* aclCreateTensorDesc(aclDataType dataType, int numDims, const int64_t* dims, aclFormat format)
{
    return g_mockAcl->aclCreateTensorDesc(dataType, numDims, dims, format);
}

aclError aclmdlSetDatasetTensorDesc(aclmdlDataset* dataset, aclTensorDesc* tensorDesc, size_t index)
{
    return g_mockAcl->aclmdlSetDatasetTensorDesc(dataset, tensorDesc, index);
}

aclError aclmdlDestroyDataset(const aclmdlDataset *dataset)
{
    return g_mockAcl->aclmdlDestroyDataset(dataset);
}

const char* aclGetRecentErrMsg()
{
    return g_mockAcl->aclGetRecentErrMsg();
}

aclError aclmdlGetDynamicHW(const aclmdlDesc* modelDesc, size_t profileIndex, aclmdlHW* dynamicHW)
{
    return g_mockAcl->aclmdlGetDynamicHW(modelDesc, profileIndex, dynamicHW);
}

aclError aclmdlSetDynamicHWSize(uint32_t modelId, aclmdlDataset* dataset, size_t index,
                                uint64_t dynamicHeight, uint64_t dynamicWidth)
{
    return g_mockAcl->aclmdlSetDynamicHWSize(modelId, dataset, index, dynamicHeight, dynamicWidth);
}

aclError aclmdlSetDynamicBatchSize(uint32_t modelId, aclmdlDataset* dataset, size_t index, 
                                   uint64_t dynamicBatchSize)
{
    return g_mockAcl->aclmdlSetDynamicBatchSize(modelId, dataset, index, dynamicBatchSize);
}

aclError aclmdlGetDynamicBatch(const aclmdlDesc* modelDesc, aclmdlBatch* batchInfo)
{
    return g_mockAcl->aclmdlGetDynamicBatch(modelDesc, batchInfo);
}

aclError aclmdlGetCurOutputDims(const aclmdlDesc* modelDesc, size_t index, aclmdlIODims* ioDims)
{
    return g_mockAcl->aclmdlGetCurOutputDims(modelDesc, index, ioDims);
}

aclError aclmdlGetInputDynamicDims(const aclmdlDesc* modelDesc, size_t profileIndex, aclmdlIODims* dims, size_t gearCount)
{
    return g_mockAcl->aclmdlGetInputDynamicDims(modelDesc, profileIndex, dims, gearCount);
}

aclError aclmdlSetInputDynamicDims(uint32_t modelId, aclmdlDataset* dataset, size_t index, const aclmdlIODims* dims)
{
    return g_mockAcl->aclmdlSetInputDynamicDims(modelId, dataset, index, dims);
}

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

        // 设置默认的卸载函数模拟
        EXPECT_CALL(*mockAcl, aclmdlUnload(_)).WillRepeatedly(Return(ACL_SUCCESS));
        EXPECT_CALL(*mockAcl, aclmdlDestroyDesc(_)).WillRepeatedly(Return(ACL_SUCCESS));
    }

    void TearDown() override
    {
        // 先释放被测对象
        modelProcess.reset();

        // 再释放模拟对象
        g_mockAcl = nullptr;
        mockAcl.reset();
    }

    // 辅助函数：加载模型成功
    void LoadModelSuccess(uint32_t modelId = expectedModelId)
    {
        EXPECT_CALL(*mockAcl, aclmdlLoadFromFile(validModelPath.c_str(), _))
            .WillOnce(DoAll(SetArgPointee<1>(modelId), Return(ACL_SUCCESS)));
        ASSERT_EQ(modelProcess->LoadModelFromFile(validModelPath), SUCCESS);
    }

    // 辅助函数：创建模型描述
    aclmdlDesc* CreateModelDescSuccess(uint32_t modelId = expectedModelId)
    {
        // 加载模型
        LoadModelSuccess(modelId);
        
        // 创建模型描述
        aclmdlDesc* fakeDesc = reinterpret_cast<aclmdlDesc*>(0x1234);
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

    void SetupModelProcessInput()
    {
        modelProcess->input_ = reinterpret_cast<aclmdlDataset*>(0x1111);
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

// 测试获取动态索引失败
TEST_F(ModelProcessTest, TestGetDynamicIndex_GetIndexFailed)
{
    // 创建模型描述
    aclmdlDesc* fakeDesc = CreateModelDescSuccess();
    
    // 设置输入数量
    const size_t numInputs = 3;
    EXPECT_CALL(*mockAcl, aclmdlGetNumInputs(fakeDesc))
        .WillOnce(Return(numInputs));
    
    // 设置输入名称查询（有动态张量）
    EXPECT_CALL(*mockAcl, aclmdlGetInputNameByIndex(fakeDesc, _))
        .Times(numInputs)
        .WillOnce(Return("input_tensor"))
        .WillOnce(Return(ACL_DYNAMIC_TENSOR_NAME))
        .WillOnce(Return("input_tensor"));
    
    // 设置获取索引失败
    EXPECT_CALL(*mockAcl, aclmdlGetInputIndexByName(
        fakeDesc, 
        StrEq(ACL_DYNAMIC_TENSOR_NAME), // 使用StrEq匹配内容
        NotNull())) // 确保传入的指针非空
        .WillOnce(Return(ACL_ERROR_FAILURE));
    
    // 设置错误消息
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillOnce(Return("Failed to get dynamic tensor index"));
    
    // 执行测试
    testing::internal::CaptureStdout();
    size_t actualIndex = 0;
    Result ret = modelProcess->GetDynamicIndex(actualIndex);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_EQ(modelProcess->g_dymindex, static_cast<size_t>(-1));
    
    // 验证错误日志
    EXPECT_TRUE(logOutput.find("get input index by name failed") != string::npos);
    EXPECT_TRUE(logOutput.find("Failed to get dynamic tensor index") != string::npos);
}

// 测试没有模型描述时动态索引为-1且返回成功
TEST_F(ModelProcessTest, TestGetDynamicIndex_NoModelDesc)
{
    // 创建模型但不创建描述
    LoadModelSuccess();
    
    // 确保模型描述为空
    ASSERT_EQ(modelProcess->modelDesc_, nullptr);
    
    // 设置：当modelDesc_为null时，aclmdlGetNumInputs返回0，而其它函数不被调用
    EXPECT_CALL(*mockAcl, aclmdlGetNumInputs(nullptr))
        .WillOnce(Return(0));
    EXPECT_CALL(*mockAcl, aclmdlGetInputNameByIndex(_, _)).Times(0);
    EXPECT_CALL(*mockAcl, aclmdlGetInputIndexByName(_, _, _)).Times(0);
    
    // 执行测试
    size_t actualIndex = 0;
    Result ret = modelProcess->GetDynamicIndex(actualIndex);
    
    // 验证结果
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(modelProcess->g_dymindex, static_cast<size_t>(-1)); // 应保持原值
}

// 测试输入数量为0的情况
TEST_F(ModelProcessTest, TestGetDynamicIndex_NoInputs)
{
    // 创建模型描述
    aclmdlDesc* fakeDesc = CreateModelDescSuccess();
    
    // 设置输入数量为0
    EXPECT_CALL(*mockAcl, aclmdlGetNumInputs(fakeDesc))
        .WillOnce(Return(0));
    
    // 执行测试
    size_t actualIndex = 0;
    Result ret = modelProcess->GetDynamicIndex(actualIndex);
    
    // 验证结果
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(modelProcess->g_dymindex, static_cast<size_t>(-1));
}

// ===================== CheckDynamicShape 测试用例 =====================

TEST_F(ModelProcessTest, TestCheckDynamicShape_InputCountMismatch)
{
    SetupCompleteModel(3, {"input1", "input2", "input3"});
    
    vector<string> dymShape = {"input1:1,2", "input2:3,4"};
    map<string, vector<int64_t>> shapeMap;
    vector<int64_t> dimsNum;
    
    testing::internal::CaptureStdout();
    Result result = modelProcess->CheckDynamicShape(dymShape, shapeMap, dimsNum);
    string logOutput = testing::internal::GetCapturedStdout();
    
    EXPECT_EQ(result, FAILED);
    EXPECT_TRUE(logOutput.find("ERROR") != string::npos);
    EXPECT_TRUE(logOutput.find("om model has 3 input") != string::npos);
}

TEST_F(ModelProcessTest, TestCheckDynamicShape_GetInputNameFailed)
{
    SetupCompleteModel(1);
    
    EXPECT_CALL(*mockAcl, aclmdlGetInputNameByIndex(_, 0))
        .WillOnce(Return(nullptr));
    
    vector<string> dymShape = {"input1:1"};
    map<string, vector<int64_t>> shapeMap;
    vector<int64_t> dimsNum;
    
    testing::internal::CaptureStdout();
    Result result = modelProcess->CheckDynamicShape(dymShape, shapeMap, dimsNum);
    string logOutput = testing::internal::GetCapturedStdout();
    
    EXPECT_EQ(result, FAILED);
    EXPECT_TRUE(logOutput.find("ERROR") != string::npos);
    EXPECT_TRUE(logOutput.find("get input name failed") != string::npos);
}

TEST_F(ModelProcessTest, TestCheckDynamicShape_InvalidDimensionFormat)
{
    SetupCompleteModel(1, {"input1"});
    
    vector<string> dymShape = {"input1:1,abc,3"};
    map<string, vector<int64_t>> shapeMap;
    vector<int64_t> dimsNum;
    
    testing::internal::CaptureStdout();
    Result result = modelProcess->CheckDynamicShape(dymShape, shapeMap, dimsNum);
    string logOutput = testing::internal::GetCapturedStdout();
    
    EXPECT_EQ(result, FAILED);
    EXPECT_TRUE(logOutput.find("ERROR") != string::npos);
    EXPECT_TRUE(logOutput.find("dim of dymshape string is illegal") != string::npos);
}

TEST_F(ModelProcessTest, TestCheckDynamicShape_OutOfRangeDimension)
{
    SetupCompleteModel(1, {"input1"});
    
    vector<string> dymShape = {"input1:99999999999999999999"};
    map<string, vector<int64_t>> shapeMap;
    vector<int64_t> dimsNum;
    
    testing::internal::CaptureStdout();
    Result result = modelProcess->CheckDynamicShape(dymShape, shapeMap, dimsNum);
    string logOutput = testing::internal::GetCapturedStdout();
    
    EXPECT_EQ(result, FAILED);
    EXPECT_TRUE(logOutput.find("ERROR") != string::npos);
    EXPECT_TRUE(logOutput.find("dim of dymshape string is illegal") != string::npos);
}

TEST_F(ModelProcessTest, TestCheckDynamicShape_MissingInputName)
{
    SetupCompleteModel(2, {"input1", "input2"});
    
    vector<string> dymShape = {"input1:1,2", "input3:3,4"};
    map<string, vector<int64_t>> shapeMap;
    vector<int64_t> dimsNum;
    
    testing::internal::CaptureStdout();
    Result result = modelProcess->CheckDynamicShape(dymShape, shapeMap, dimsNum);
    string logOutput = testing::internal::GetCapturedStdout();
    
    EXPECT_EQ(result, FAILED);
    EXPECT_TRUE(logOutput.find("ERROR") != string::npos);
    EXPECT_TRUE(logOutput.find("dymShape parameter set error") != string::npos);
}

TEST_F(ModelProcessTest, TestCheckDynamicShape_Success)
{
    SetupCompleteModel(2, {"input1", "input2"});
    
    vector<string> dymShape = {"input1:1,2,3", "input2:4,5"};
    map<string, vector<int64_t>> shapeMap;
    vector<int64_t> dimsNum;

    SetDebugLogGuard guard;
    testing::internal::CaptureStdout();
    Result result = modelProcess->CheckDynamicShape(dymShape, shapeMap, dimsNum);
    string logOutput = testing::internal::GetCapturedStdout();

    EXPECT_EQ(result, SUCCESS);
    EXPECT_TRUE(logOutput.find("DEBUG") != string::npos);
    EXPECT_TRUE(logOutput.find("check Dynamic Shape success") != string::npos);
    
    EXPECT_EQ(dimsNum.size(), 2);
    EXPECT_EQ(dimsNum[0], 3);
    EXPECT_EQ(dimsNum[1], 2);
    EXPECT_EQ(shapeMap["input1"], vector<int64_t>({1, 2, 3}));
    EXPECT_EQ(shapeMap["input2"], vector<int64_t>({4, 5}));
}

TEST_F(ModelProcessTest, TestCheckDynamicShape_EmptyShape)
{
    SetupCompleteModel(1, {"input1"});
    
    vector<string> dymShape = {"input1:"};
    map<string, vector<int64_t>> shapeMap;
    vector<int64_t> dimsNum;
    
    SetDebugLogGuard guard;
    testing::internal::CaptureStdout();
    Result result = modelProcess->CheckDynamicShape(dymShape, shapeMap, dimsNum);
    string logOutput = testing::internal::GetCapturedStdout();
    
    EXPECT_EQ(result, SUCCESS);
    EXPECT_TRUE(shapeMap["input1"].empty());
    EXPECT_EQ(dimsNum.size(), 1);
    EXPECT_EQ(dimsNum[0], 0);

    EXPECT_TRUE(logOutput.find("DEBUG") != string::npos);
    EXPECT_TRUE(logOutput.find("check Dynamic Shape success") != string::npos);
}

TEST_F(ModelProcessTest, TestCheckDynamicShape_MissingColon)
{
    SetupCompleteModel(1, {"input1"});
    
    vector<string> dymShape = {"input1"};
    map<string, vector<int64_t>> shapeMap;
    vector<int64_t> dimsNum;
    
    testing::internal::CaptureStdout();
    Result result = modelProcess->CheckDynamicShape(dymShape, shapeMap, dimsNum);
    string logOutput = testing::internal::GetCapturedStdout();

    EXPECT_EQ(result, FAILED);
    EXPECT_TRUE(shapeMap["input1"].empty());
    EXPECT_EQ(dimsNum.size(), 1);
    EXPECT_EQ(dimsNum[0], 0);

    EXPECT_TRUE(logOutput.find("ERROR") != string::npos);
    EXPECT_TRUE(logOutput.find("the dymShape parameter set error, please check input name") != string::npos);
}

// ===================== SetDynamicShape 测试用例 =====================

TEST_F(ModelProcessTest, TestSetDynamicShape_DimNumMismatch)
{
    SetupCompleteModel(2, {"input1", "input2"});
    SetupModelProcessInput();
    
    // 添加销毁期望
    EXPECT_CALL(*mockAcl, aclmdlDestroyDataset(reinterpret_cast<const aclmdlDataset*>(0x1111)))
        .WillOnce(Return(ACL_SUCCESS));
    
    std::map<std::string, std::vector<int64_t>> shape_map = {
        {"input1", {1, 2, 3}},
        {"input2", {4, 5, 6}}
    };
    std::vector<int64_t> dims_num = {3, 3, 2};
    
    testing::internal::CaptureStdout();
    Result ret = modelProcess->SetDynamicShape(shape_map, dims_num);
    string logOutput = testing::internal::GetCapturedStdout();
    
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("dims num size: 3 not equal to input num 2") != string::npos);
}

TEST_F(ModelProcessTest, TestSetDynamicShape_GetInputNameFailed)
{
    SetupCompleteModel(2, {"input1", "input2"});
    SetupModelProcessInput();
    
    // 添加销毁期望
    EXPECT_CALL(*mockAcl, aclmdlDestroyDataset(reinterpret_cast<const aclmdlDataset*>(0x1111)))
        .WillOnce(Return(ACL_SUCCESS));
    
    std::map<std::string, std::vector<int64_t>> shape_map = {
        {"input1", {1, 2, 3}},
        {"input2", {4, 5, 6}}
    };
    std::vector<int64_t> dims_num = {3, 3};
    
    EXPECT_CALL(*mockAcl, aclmdlGetInputNameByIndex(modelProcess->modelDesc_, 0))
        .WillOnce(Return(nullptr));
    const char* errorMsg = "Failed to get input name";
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillOnce(Return(errorMsg));
    
    testing::internal::CaptureStdout();
    Result ret = modelProcess->SetDynamicShape(shape_map, dims_num);
    string logOutput = testing::internal::GetCapturedStdout();
    
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("get input name by index failed") != string::npos);
    EXPECT_TRUE(logOutput.find(errorMsg) != string::npos);
}

TEST_F(ModelProcessTest, TestSetDynamicShape_CreateTensorDescOrSetDatasetTensorFailed)
{
    SetupCompleteModel(1, {"input1"});
    SetupModelProcessInput();
    
    // 添加销毁期望
    EXPECT_CALL(*mockAcl, aclmdlDestroyDataset(reinterpret_cast<const aclmdlDataset*>(0x1111)))
        .WillOnce(Return(ACL_SUCCESS));
    
    std::map<std::string, std::vector<int64_t>> shape_map = {
        {"input1", {1, 2, 3}}
    };
    std::vector<int64_t> dims_num = {3};
    
    EXPECT_CALL(*mockAcl, aclmdlGetInputNameByIndex(modelProcess->modelDesc_, 0))
        .WillOnce(Return("input1"));
    
    EXPECT_CALL(*mockAcl, aclCreateTensorDesc(ACL_FLOAT, _, _, ACL_FORMAT_NCHW))
        .WillOnce(Return(nullptr));
    
    EXPECT_CALL(*mockAcl, aclmdlSetDatasetTensorDesc(_, _, _))
        .WillOnce(Return(ACL_ERROR_FAILURE));
    
    const char* errorMsg = "acl failed";
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillOnce(Return(errorMsg));
    
    testing::internal::CaptureStdout();
    Result ret = modelProcess->SetDynamicShape(shape_map, dims_num);
    string logOutput = testing::internal::GetCapturedStdout();
    
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("acl set dataset TensorDesc failed") != string::npos);
    EXPECT_TRUE(logOutput.find(errorMsg) != string::npos);
}

TEST_F(ModelProcessTest, TestSetDynamicShape_SingleInputSuccess)
{
    SetupCompleteModel(1, {"input1"});
    SetupModelProcessInput();
    
    // 添加销毁期望
    EXPECT_CALL(*mockAcl, aclmdlDestroyDataset(reinterpret_cast<const aclmdlDataset*>(0x1111)))
        .WillOnce(Return(ACL_SUCCESS));
    
    std::map<std::string, std::vector<int64_t>> shape_map = {
        {"input1", {1, 2, 3}}
    };
    std::vector<int64_t> dims_num = {3};
    
    EXPECT_CALL(*mockAcl, aclmdlGetInputNameByIndex(modelProcess->modelDesc_, 0))
        .WillOnce(Return("input1"));
    
    aclTensorDesc* fakeDesc = reinterpret_cast<aclTensorDesc*>(0x1234);
    EXPECT_CALL(*mockAcl, aclCreateTensorDesc(ACL_FLOAT, 3, _, ACL_FORMAT_NCHW))
        .WillOnce(Return(fakeDesc));
    
    EXPECT_CALL(*mockAcl, aclmdlSetDatasetTensorDesc(modelProcess->input_, fakeDesc, 0))
        .WillOnce(Return(ACL_SUCCESS));
    
    SetDebugLogGuard guard;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->SetDynamicShape(shape_map, dims_num);
    string logOutput = testing::internal::GetCapturedStdout();

    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(logOutput.find("set Dynamic shape success") != string::npos);
}

// ===================== GetMaxDynamicHWSize 测试用例 =====================

TEST_F(ModelProcessTest, TestGetMaxDynamicHWSize_Success)
{
    auto fakeDesc = CreateModelDescSuccess();
    modelProcess->modelDesc_ = fakeDesc;
    
    aclmdlHW dynamicHW = {3, {{128, 128}, {256, 256}, {512, 512}}};
    EXPECT_CALL(*mockAcl, aclmdlGetDynamicHW(fakeDesc, -1, _))
        .WillOnce(DoAll(SetArgPointee<2>(dynamicHW), Return(ACL_SUCCESS)));
    
    uint64_t maxSize = 0;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->GetMaxDynamicHWSize(maxSize);
    string logOutput = testing::internal::GetCapturedStdout();
    
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(maxSize, 512 * 512); // 512x512 是最大尺寸
}

TEST_F(ModelProcessTest, TestGetMaxDynamicHWSize_GetDynamicHWFailed)
{
    auto fakeDesc = CreateModelDescSuccess();
    modelProcess->modelDesc_ = fakeDesc;
    
    EXPECT_CALL(*mockAcl, aclmdlGetDynamicHW(fakeDesc, -1, _))
        .WillOnce(Return(ACL_ERROR_INVALID_PARAM));
    const char* errorMsg = "Get dynamic HW failed";
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillOnce(Return(errorMsg));
    
    uint64_t maxSize = 0;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->GetMaxDynamicHWSize(maxSize);
    string logOutput = testing::internal::GetCapturedStdout();
    
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("get DynamicHW failed") != string::npos);
    EXPECT_TRUE(logOutput.find(errorMsg) != string::npos);
}

TEST_F(ModelProcessTest, TestGetMaxDynamicHWSize_NoDynamicHW)
{
    auto fakeDesc = CreateModelDescSuccess();
    modelProcess->modelDesc_ = fakeDesc;
    
    aclmdlHW dynamicHW = {0, {}};
    EXPECT_CALL(*mockAcl, aclmdlGetDynamicHW(fakeDesc, -1, _))
        .WillOnce(DoAll(SetArgPointee<2>(dynamicHW), Return(ACL_SUCCESS)));
    
    uint64_t maxSize = 0;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->GetMaxDynamicHWSize(maxSize);
    string logOutput = testing::internal::GetCapturedStdout();
    
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("the dynamic_image_size parameter is not specified") != string::npos);
}

// ===================== CheckDynamicHWSize 测试用例 =====================

TEST_F(ModelProcessTest, TestCheckDynamicHWSize_Success)
{
    auto fakeDesc = CreateModelDescSuccess();
    modelProcess->modelDesc_ = fakeDesc;
    
    aclmdlHW dynamicHW = {2, {{128, 128}, {256, 256}}};
    EXPECT_CALL(*mockAcl, aclmdlGetDynamicHW(fakeDesc, -1, _))
        .WillOnce(DoAll(SetArgPointee<2>(dynamicHW), Return(ACL_SUCCESS)));
    
    bool isDynamic = false;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CheckDynamicHWSize({256, 256}, isDynamic);
    string logOutput = testing::internal::GetCapturedStdout();
    
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(isDynamic);
    EXPECT_TRUE(logOutput.find("check dynamic image size success") != string::npos);
}

TEST_F(ModelProcessTest, TestCheckDynamicHWSize_GetDynamicHWFailed)
{
    auto fakeDesc = CreateModelDescSuccess();
    modelProcess->modelDesc_ = fakeDesc;
    
    aclmdlHW dynamicHW = {2, {{128, 128}, {256, 256}}};
    EXPECT_CALL(*mockAcl, aclmdlGetDynamicHW(fakeDesc, -1, _))
        .WillOnce(DoAll(SetArgPointee<2>(dynamicHW), Return(ACL_ERROR_FAILURE)));
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillOnce(Return("aclmdlGetDynamicHW Failed"));
    
    bool isDynamic = false;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CheckDynamicHWSize({512, 512}, isDynamic);
    string logOutput = testing::internal::GetCapturedStdout();
    cout << logOutput << endl;
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("get DynamicHW failed") != string::npos);
}

TEST_F(ModelProcessTest, TestCheckDynamicHWSize_DynamicHWNotFound)
{
    auto fakeDesc = CreateModelDescSuccess();
    modelProcess->modelDesc_ = fakeDesc;
    
    aclmdlHW dynamicHW = {2, {{128, 128}, {256, 256}}};
    EXPECT_CALL(*mockAcl, aclmdlGetDynamicHW(fakeDesc, -1, _))
        .WillOnce(DoAll(SetArgPointee<2>(dynamicHW), Return(ACL_SUCCESS)));
    
    bool isDynamic = false;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CheckDynamicHWSize({512, 512}, isDynamic);
    string logOutput = testing::internal::GetCapturedStdout();
    
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("the dymHW parameter is not correct") != string::npos);
}

TEST_F(ModelProcessTest, TestCheckDynamicHWSize_NoDynamicHWSpecified)
{
    auto fakeDesc = CreateModelDescSuccess();
    modelProcess->modelDesc_ = fakeDesc;
    
    aclmdlHW dynamicHW = {0, {}};
    EXPECT_CALL(*mockAcl, aclmdlGetDynamicHW(fakeDesc, -1, _))
        .WillOnce(DoAll(SetArgPointee<2>(dynamicHW), Return(ACL_SUCCESS)));
    
    bool isDynamic = false;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CheckDynamicHWSize({256, 256}, isDynamic);
    string logOutput = testing::internal::GetCapturedStdout();
    
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("the dynamic_image_size parameter is not specified") != string::npos);
}

// ===================== SetDynamicHW 测试用例 =====================

TEST_F(ModelProcessTest, TestSetDynamicHW_Success)
{
    SetupCompleteModel();
    modelProcess->g_dymindex = 0;
    modelProcess->input_ = reinterpret_cast<aclmdlDataset*>(0x1234);
    
    EXPECT_CALL(*mockAcl, aclmdlSetDynamicHWSize(expectedModelId, 
                                                reinterpret_cast<aclmdlDataset*>(0x1234), 
                                                0, 256, 256))
        .WillOnce(Return(ACL_SUCCESS));
    EXPECT_CALL(*mockAcl, aclmdlDestroyDataset(reinterpret_cast<const aclmdlDataset*>(0x1234)))
        .WillOnce(Return(ACL_SUCCESS));

    SetDebugLogGuard guard;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->SetDynamicHW({256, 256});
    string logOutput = testing::internal::GetCapturedStdout();
    
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(logOutput.find("set Dynamic HW success") != string::npos);
}

TEST_F(ModelProcessTest, TestSetDynamicHW_Failed)
{
    SetupCompleteModel();
    modelProcess->g_dymindex = 0;
    modelProcess->input_ = reinterpret_cast<aclmdlDataset*>(0x1234);
    
    EXPECT_CALL(*mockAcl, aclmdlSetDynamicHWSize(expectedModelId, 
                                                reinterpret_cast<aclmdlDataset*>(0x1234), 
                                                0, 512, 512))
        .WillOnce(Return(ACL_ERROR_BAD_ALLOC));
    
    EXPECT_CALL(*mockAcl, aclmdlDestroyDataset(reinterpret_cast<const aclmdlDataset*>(0x1234)))
        .WillOnce(Return(ACL_SUCCESS));

    const char* errorMsg = "Set dynamic HW failed";
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillOnce(Return(errorMsg));
    
    testing::internal::CaptureStdout();
    Result ret = modelProcess->SetDynamicHW({512, 512});
    string logOutput = testing::internal::GetCapturedStdout();
    
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("acl set dynamicHW size failed") != string::npos);
    EXPECT_TRUE(logOutput.find(errorMsg) != string::npos);
}

// ===================== CheckDynamicBatchSize 测试用例 =====================

TEST_F(ModelProcessTest, TestCheckDynamicBatchSize_Success)
{
    auto fakeDesc = CreateModelDescSuccess();
    modelProcess->modelDesc_ = fakeDesc;
    
    aclmdlBatch batchInfo = {3, {1, 4, 8}};
    EXPECT_CALL(*mockAcl, aclmdlGetDynamicBatch(fakeDesc, _))
        .WillOnce(DoAll(SetArgPointee<1>(batchInfo), Return(ACL_SUCCESS)));
    
    bool isDynamic = false;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CheckDynamicBatchSize(4, isDynamic);
    string logOutput = testing::internal::GetCapturedStdout();
    
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(isDynamic);
    EXPECT_TRUE(logOutput.find("check dynamic batch success") != string::npos);
}

TEST_F(ModelProcessTest, TestCheckDynamicBatchSize_InvalidBatch)
{
    auto fakeDesc = CreateModelDescSuccess();
    modelProcess->modelDesc_ = fakeDesc;
    
    aclmdlBatch batchInfo = {3, {1, 4, 8}};
    EXPECT_CALL(*mockAcl, aclmdlGetDynamicBatch(fakeDesc, _)).Times(2)
        .WillRepeatedly(DoAll(SetArgPointee<1>(batchInfo), Return(ACL_SUCCESS)));
    
    bool isDynamic = false;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CheckDynamicBatchSize(16, isDynamic);
    string logOutput = testing::internal::GetCapturedStdout();
    
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("the dymBatch parameter is not correct") != string::npos);
}

TEST_F(ModelProcessTest, TestCheckDynamicBatchSize_GetDynamicBatchFailed)
{
    auto fakeDesc = CreateModelDescSuccess();
    modelProcess->modelDesc_ = fakeDesc;
    
    EXPECT_CALL(*mockAcl, aclmdlGetDynamicBatch(fakeDesc, _))
        .WillOnce(Return(ACL_ERROR_INVALID_PARAM));
    const char* errorMsg = "Get dynamic batch failed";
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillOnce(Return(errorMsg));
    
    bool isDynamic = false;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CheckDynamicBatchSize(4, isDynamic);
    string logOutput = testing::internal::GetCapturedStdout();
    
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("get DynamicBatch failed") != string::npos);
    EXPECT_TRUE(logOutput.find(errorMsg) != string::npos);
}

TEST_F(ModelProcessTest, TestCheckDynamicBatchSize_NoDynamicBatch)
{
    auto fakeDesc = CreateModelDescSuccess();
    modelProcess->modelDesc_ = fakeDesc;
    
    aclmdlBatch batchInfo = {0, {}};
    EXPECT_CALL(*mockAcl, aclmdlGetDynamicBatch(fakeDesc, _))
        .WillOnce(DoAll(SetArgPointee<1>(batchInfo), Return(ACL_SUCCESS)));
    
    bool isDynamic = false;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CheckDynamicBatchSize(4, isDynamic);
    string logOutput = testing::internal::GetCapturedStdout();
    
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("the dynamic_batch_size parameter is not specified") != string::npos);
}

// ===================== GetMaxBatchSize 测试用例 =====================

TEST_F(ModelProcessTest, TestGetMaxBatchSize_Success)
{
    auto fakeDesc = CreateModelDescSuccess();
    modelProcess->modelDesc_ = fakeDesc;
    
    aclmdlBatch batchInfo = {3, {1, 4, 8}};
    EXPECT_CALL(*mockAcl, aclmdlGetDynamicBatch(fakeDesc, _))
        .WillOnce(DoAll(SetArgPointee<1>(batchInfo), Return(ACL_SUCCESS)));
    
    uint64_t maxBatchSize = 0;
    SetDebugLogGuard guard;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->GetMaxBatchSize(maxBatchSize);
    string logOutput = testing::internal::GetCapturedStdout();
    
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(maxBatchSize, 8);
    EXPECT_TRUE(logOutput.find("get max dynamic batch size success") != string::npos);
}

TEST_F(ModelProcessTest, TestGetMaxBatchSize_NoBatchInfo)
{
    auto fakeDesc = CreateModelDescSuccess();
    modelProcess->modelDesc_ = fakeDesc;
    
    aclmdlBatch batchInfo = {0, {}};
    EXPECT_CALL(*mockAcl, aclmdlGetDynamicBatch(fakeDesc, _))
        .WillOnce(DoAll(SetArgPointee<1>(batchInfo), Return(ACL_SUCCESS)));
    
    uint64_t maxBatchSize = 0;
    SetDebugLogGuard guard;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->GetMaxBatchSize(maxBatchSize);
    string logOutput = testing::internal::GetCapturedStdout();
    
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(maxBatchSize, 0);
    EXPECT_TRUE(logOutput.find("get max dynamic batch size success") != string::npos);
}

TEST_F(ModelProcessTest, TestGetMaxBatchSize_GetDynamicBatchFailed)
{
    auto fakeDesc = CreateModelDescSuccess();
    modelProcess->modelDesc_ = fakeDesc;
    
    EXPECT_CALL(*mockAcl, aclmdlGetDynamicBatch(fakeDesc, _))
        .WillOnce(Return(ACL_ERROR_BAD_ALLOC));
    const char* errorMsg = "Get dynamic batch failed";
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillOnce(Return(errorMsg));
    
    uint64_t maxBatchSize = 0;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->GetMaxBatchSize(maxBatchSize);
    string logOutput = testing::internal::GetCapturedStdout();
    
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("get DynamicBatch failed") != string::npos);
    EXPECT_TRUE(logOutput.find(errorMsg) != string::npos);
}

// ===================== SetDynamicBatchSize 测试用例 =====================

TEST_F(ModelProcessTest, TestSetDynamicBatchSize_Success)
{
    // 创建模型描述
    auto fakeDesc = CreateModelDescSuccess();
    modelProcess->modelDesc_ = fakeDesc;
    
    // 配置输入 dataset
    aclmdlDataset* inputDataset = reinterpret_cast<aclmdlDataset*>(0xABCD);
    modelProcess->input_ = inputDataset;
    modelProcess->g_dymindex = 0;
    
    // 设置销毁期望
    EXPECT_CALL(*mockAcl, aclmdlDestroyDataset(reinterpret_cast<const aclmdlDataset*>(inputDataset)))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 设置期望
    EXPECT_CALL(*mockAcl, aclmdlSetDynamicBatchSize(expectedModelId, inputDataset, 0, 8))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 执行测试
    SetDebugLogGuard guard;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->SetDynamicBatchSize(8);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(logOutput.find("set dynamic batch size success") != string::npos);
}

TEST_F(ModelProcessTest, TestSetDynamicBatchSize_Failed)
{
    // 创建模型描述
    auto fakeDesc = CreateModelDescSuccess();
    modelProcess->modelDesc_ = fakeDesc;
    
    // 配置输入 dataset
    aclmdlDataset* inputDataset = reinterpret_cast<aclmdlDataset*>(0xABCD);
    modelProcess->input_ = inputDataset;
    modelProcess->g_dymindex = 0;
    
    // 设置销毁期望
    EXPECT_CALL(*mockAcl, aclmdlDestroyDataset(reinterpret_cast<const aclmdlDataset*>(inputDataset)))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 设置期望
    EXPECT_CALL(*mockAcl, aclmdlSetDynamicBatchSize(expectedModelId, inputDataset, 0, 16))
        .WillOnce(Return(ACL_ERROR_BAD_ALLOC));
    const char* errorMsg = "Set dynamic batch failed";
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillOnce(Return(errorMsg));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->SetDynamicBatchSize(16);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("acl set dynamic batch size failed") != string::npos);
    EXPECT_TRUE(logOutput.find(errorMsg) != string::npos);
}

// ===================== GetCurOutputDimsMul 测试用例 =====================

TEST_F(ModelProcessTest, TestGetCurOutputDimsMul_Success)
{
    // 创建模型描述
    auto fakeDesc = CreateModelDescSuccess();
    modelProcess->modelDesc_ = fakeDesc;
    
    // 安全初始化 ioDims
    aclmdlIODims ioDims;
    ioDims.dimCount = 3;
    int64_t dimsArray[ACL_MAX_DIM_CNT] = {1, 2, 3}; // 假设 ACL_MAX_DIM_CNT 足够大
    memcpy(ioDims.dims, dimsArray, sizeof(int64_t) * 3);
    
    EXPECT_CALL(*mockAcl, aclmdlGetCurOutputDims(fakeDesc, 0, _))
        .WillOnce(DoAll(SetArgPointee<2>(ioDims), Return(ACL_SUCCESS)));
    
    // 执行测试
    vector<int64_t> dimsMul;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->GetCurOutputDimsMul(0, dimsMul);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(dimsMul.size(), 2);
    EXPECT_EQ(dimsMul[0], 3); // 3 = 3
    EXPECT_EQ(dimsMul[1], 6); // 6 = 3 * 2
    EXPECT_TRUE(logOutput.empty());
}

TEST_F(ModelProcessTest, TestGetCurOutputDimsMul_Failed)
{
    // 创建模型描述
    auto fakeDesc = CreateModelDescSuccess();
    modelProcess->modelDesc_ = fakeDesc;
    
    // 安全初始化 ioDims
    aclmdlIODims ioDims;
    ioDims.dimCount = 3;
    int64_t dimsArray[ACL_MAX_DIM_CNT] = {1, 2, 3}; // 假设 ACL_MAX_DIM_CNT 足够大
    memcpy(ioDims.dims, dimsArray, sizeof(int64_t) * 3);

    EXPECT_CALL(*mockAcl, aclmdlGetCurOutputDims(fakeDesc, 0, _))
        .WillOnce(DoAll(SetArgPointee<2>(ioDims), Return(ACL_ERROR_FAILURE)));

    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillOnce(Return("aclmdlGetCurOutputDims Failed"));
    
    // 执行测试
    vector<int64_t> dimsMul;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->GetCurOutputDimsMul(0, dimsMul);
    string logOutput = testing::internal::GetCapturedStdout();
    cout << logOutput << endl;
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("acl get current output dims failed ret") != string::npos);
    EXPECT_TRUE(logOutput.find("maybe the modle has dynamic shape") != string::npos);
}

// ===================== CheckDynamicDims 测试用例  =====================

TEST_F(ModelProcessTest, TestCheckDynamicDims_Success)
{
    // 创建模型描述
    auto fakeDesc = CreateModelDescSuccess();
    modelProcess->modelDesc_ = fakeDesc;
    
    // 准备测试数据
    vector<string> dymDims = {"1", "2", "3"};
    size_t gearCount = 1;
    
    // 安全初始化 dims
    aclmdlIODims dims;
    dims.dimCount = 3;
    int64_t dimsArray[3] = {1, 2, 3};
    memcpy(dims.dims, dimsArray, sizeof(int64_t) * 3);
    
    // 设置模拟期望
    EXPECT_CALL(*mockAcl, aclmdlGetInputDynamicDims(fakeDesc, -1, _, gearCount))
        .WillOnce(DoAll(SetArgPointee<2>(dims), Return(ACL_SUCCESS)));
    
    // 执行测试
    SetDebugLogGuard guard;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CheckDynamicDims(dymDims, gearCount, &dims);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(logOutput.find("check dynamic dims success") != string::npos);
}

TEST_F(ModelProcessTest, TestCheckDynamicDims_DimCountMismatch)
{
    // 创建模型描述
    auto fakeDesc = CreateModelDescSuccess();
    modelProcess->modelDesc_ = fakeDesc;
    
    // 准备测试数据
    vector<string> dymDims = {"1", "2", "3"};
    size_t gearCount = 1;
    
    // 安全初始化 dims - 维度数量不匹配
    aclmdlIODims dims;
    dims.dimCount = 2; // 只有 2 个维度
    int64_t dimsArray[2] = {1, 2};
    memcpy(dims.dims, dimsArray, sizeof(int64_t) * 2);
    
    // 设置模拟期望
    EXPECT_CALL(*mockAcl, aclmdlGetInputDynamicDims(fakeDesc, -1, _, gearCount)).Times(2)
        .WillRepeatedly(DoAll(SetArgPointee<2>(dims), Return(ACL_SUCCESS)));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CheckDynamicDims(dymDims, gearCount, &dims);
    string logOutput = testing::internal::GetCapturedStdout();

    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("the dymDims parameter is not correct") != string::npos);
    EXPECT_TRUE(logOutput.find("dysize:3 dimcount:2") != string::npos);
}

TEST_F(ModelProcessTest, TestCheckDynamicDims_InvalidInteger)
{
    // 创建模型描述
    auto fakeDesc = CreateModelDescSuccess();
    modelProcess->modelDesc_ = fakeDesc;
    
    // 准备测试数据
    vector<string> dymDims = {"1", "2", "abc"}; // "abc" 不是有效整数
    size_t gearCount = 1;
    
    // 安全初始化 dims
    aclmdlIODims dims;
    dims.dimCount = 3;
    int64_t dimsArray[3] = {1, 2, 3};
    memcpy(dims.dims, dimsArray, sizeof(int64_t) * 3);
    
    // 设置模拟期望
    EXPECT_CALL(*mockAcl, aclmdlGetInputDynamicDims(fakeDesc, -1, _, gearCount))
        .WillOnce(DoAll(SetArgPointee<2>(dims), Return(ACL_SUCCESS)));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CheckDynamicDims(dymDims, gearCount, &dims);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("dim of dymdims string is illegal") != string::npos);
}

TEST_F(ModelProcessTest, TestCheckDynamicDims_DimsNotMatching)
{
    // 创建模型描述
    auto fakeDesc = CreateModelDescSuccess();
    modelProcess->modelDesc_ = fakeDesc;
    
    // 准备测试数据
    vector<string> dymDims = {"1", "2", "4"}; // 与动态维度不匹配
    size_t gearCount = 1;
    
    // 安全初始化 dims
    aclmdlIODims dims;
    dims.dimCount = 3;
    int64_t dimsArray[3] = {1, 2, 3};
    memcpy(dims.dims, dimsArray, sizeof(int64_t) * 3);
    
    // 设置模拟期望
    EXPECT_CALL(*mockAcl, aclmdlGetInputDynamicDims(fakeDesc, -1, _, gearCount)).Times(2)
        .WillRepeatedly(DoAll(SetArgPointee<2>(dims), Return(ACL_SUCCESS)));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CheckDynamicDims(dymDims, gearCount, &dims);
    string logOutput = testing::internal::GetCapturedStdout();

    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("the dynamic_dims parameter is not correct") != string::npos);
}

// ===================== SetDynamicDims 测试用例  =====================

TEST_F(ModelProcessTest, TestSetDynamicDims_Success)
{
    // 创建模型描述
    auto fakeDesc = CreateModelDescSuccess();
    modelProcess->modelDesc_ = fakeDesc;
    
    // 配置输入 dataset
    aclmdlDataset* inputDataset = reinterpret_cast<aclmdlDataset*>(0x1234);
    modelProcess->input_ = inputDataset;
    modelProcess->g_dymindex = 0;
    
    // 设置销毁期望
    EXPECT_CALL(*mockAcl, aclmdlDestroyDataset(reinterpret_cast<const aclmdlDataset*>(inputDataset)))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 准备测试数据
    vector<string> dymDims = {"1", "2", "3"};
    
    // 设置模拟期望 - 验证传入的 dims
    EXPECT_CALL(*mockAcl, aclmdlSetInputDynamicDims(expectedModelId, inputDataset, 0, _))
        .WillOnce([](uint32_t, aclmdlDataset*, size_t, const aclmdlIODims* dims) {
            EXPECT_EQ(dims->dimCount, 3);
            EXPECT_EQ(dims->dims[0], 1);
            EXPECT_EQ(dims->dims[1], 2);
            EXPECT_EQ(dims->dims[2], 3);
            return ACL_SUCCESS;
        });
    
    // 执行测试
    SetDebugLogGuard guard;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->SetDynamicDims(dymDims);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(logOutput.find("set dynamic dims success") != string::npos);
}

TEST_F(ModelProcessTest, TestSetDynamicDims_InvalidInteger)
{
    // 准备测试数据
    vector<string> dymDims = {"1", "2", "abc"}; // "abc" 不是有效整数
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->SetDynamicDims(dymDims);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("Invalid input for conversion: abc") != string::npos);
}

TEST_F(ModelProcessTest, TestSetDynamicDims_OutOfRange)
{
    // 准备测试数据
    vector<string> dymDims = {"1", "2", "99999999999999999999"}; // 超出 long 范围
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->SetDynamicDims(dymDims);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("Out of range input for conversion: 99999999999999999999") != string::npos);
}

TEST_F(ModelProcessTest, TestSetDynamicDims_SetDimsFailed)
{
    // 创建模型描述
    auto fakeDesc = CreateModelDescSuccess();
    modelProcess->modelDesc_ = fakeDesc;
    
    // 配置输入 dataset
    aclmdlDataset* inputDataset = reinterpret_cast<aclmdlDataset*>(0x1234);
    modelProcess->input_ = inputDataset;
    modelProcess->g_dymindex = 0;
    
    // 设置销毁期望
    EXPECT_CALL(*mockAcl, aclmdlDestroyDataset(reinterpret_cast<const aclmdlDataset*>(inputDataset)))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 准备测试数据 - 不再初始化 aclmdlIODims
    vector<string> dymDims = {"1", "2", "3"};
    
    // 设置模拟期望 - 使用通配符匹配任何传入的 dims 参数
    EXPECT_CALL(*mockAcl, aclmdlSetInputDynamicDims(expectedModelId, inputDataset, 0, _))
        .WillOnce(Return(ACL_ERROR_BAD_ALLOC));
    const char* errorMsg = "Set dynamic dims failed";
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillOnce(Return(errorMsg));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->SetDynamicDims(dymDims);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("acl set input dynamic dims failed") != string::npos);
    EXPECT_TRUE(logOutput.find(errorMsg) != string::npos);
}
}