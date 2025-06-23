#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include <vector>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>

#include "test_utils.hpp"
#include "acl_mock_functions.h"
#include "base/include/Base/ModelInfer/model_process.h"

using namespace std;
using namespace testing;


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
    SetDebugLogGuard guard;
    SetupCompleteModel(2, {"input1", "input2"});
    
    vector<string> dymShape = {"input1:1,2,3", "input2:4,5"};
    map<string, vector<int64_t>> shapeMap;
    vector<int64_t> dimsNum;
    
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
    SetDebugLogGuard guard;
    SetupCompleteModel(1, {"input1"});
    
    vector<string> dymShape = {"input1:"};
    map<string, vector<int64_t>> shapeMap;
    vector<int64_t> dimsNum;
    
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
    SetDebugLogGuard guard;
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
    
    aclmdlHW dynamicHW = {2, {{128, 128}, {256, 256}}};
    EXPECT_CALL(*mockAcl, aclmdlGetDynamicHW(fakeDesc, -1, _))
        .WillOnce(DoAll(SetArgPointee<2>(dynamicHW), Return(ACL_ERROR_FAILURE)));
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillOnce(Return("aclmdlGetDynamicHW Failed"));
    
    bool isDynamic = false;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CheckDynamicHWSize({512, 512}, isDynamic);
    string logOutput = testing::internal::GetCapturedStdout();
    
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("get DynamicHW failed") != string::npos);
}

TEST_F(ModelProcessTest, TestCheckDynamicHWSize_DynamicHWNotFound)
{
    auto fakeDesc = CreateModelDescSuccess();
    
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
    SetDebugLogGuard guard;
    SetupCompleteModel();
    modelProcess->g_dymindex = 0;
    modelProcess->input_ = reinterpret_cast<aclmdlDataset*>(0x1234);
    
    EXPECT_CALL(*mockAcl, aclmdlSetDynamicHWSize(expectedModelId, 
                                                reinterpret_cast<aclmdlDataset*>(0x1234), 
                                                0, 256, 256))
        .WillOnce(Return(ACL_SUCCESS));
    EXPECT_CALL(*mockAcl, aclmdlDestroyDataset(reinterpret_cast<const aclmdlDataset*>(0x1234)))
        .WillOnce(Return(ACL_SUCCESS));
    
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
    SetDebugLogGuard guard;
    auto fakeDesc = CreateModelDescSuccess();
    
    aclmdlBatch batchInfo = {3, {1, 4, 8}};
    EXPECT_CALL(*mockAcl, aclmdlGetDynamicBatch(fakeDesc, _))
        .WillOnce(DoAll(SetArgPointee<1>(batchInfo), Return(ACL_SUCCESS)));
    
    uint64_t maxBatchSize = 0;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->GetMaxBatchSize(maxBatchSize);
    string logOutput = testing::internal::GetCapturedStdout();
    
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(maxBatchSize, 8);
    EXPECT_TRUE(logOutput.find("get max dynamic batch size success") != string::npos);
}

TEST_F(ModelProcessTest, TestGetMaxBatchSize_NoBatchInfo)
{
    SetDebugLogGuard guard;
    auto fakeDesc = CreateModelDescSuccess();
    
    aclmdlBatch batchInfo = {0, {}};
    EXPECT_CALL(*mockAcl, aclmdlGetDynamicBatch(fakeDesc, _))
        .WillOnce(DoAll(SetArgPointee<1>(batchInfo), Return(ACL_SUCCESS)));
    
    uint64_t maxBatchSize = 0;
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
    SetDebugLogGuard guard;
    // 创建模型描述
    CreateModelDescSuccess();
    
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
    CreateModelDescSuccess();
    
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
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("acl get current output dims failed ret") != string::npos);
    EXPECT_TRUE(logOutput.find("maybe the modle has dynamic shape") != string::npos);
}

// ===================== CheckDynamicDims 测试用例  =====================

TEST_F(ModelProcessTest, TestCheckDynamicDims_Success)
{
    SetDebugLogGuard guard;
    // 创建模型描述
    auto fakeDesc = CreateModelDescSuccess();
    
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
    SetDebugLogGuard guard;
    // 创建模型描述
    CreateModelDescSuccess();
    
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
    CreateModelDescSuccess();
    
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

// ===================== GetDymHWInfo 测试用例 =====================

TEST_F(ModelProcessTest, TestGetDymHWInfo)
{
    // 创建模型描述
    auto fakeDesc = CreateModelDescSuccess();
    
    // 设置模拟期望 - 返回动态HW信息
    EXPECT_CALL(*mockAcl, aclmdlGetDynamicHW(fakeDesc, _, _))
        .WillOnce(Invoke([](const aclmdlDesc*, int profileIndex, aclmdlHW* hwInfo) {
            hwInfo->hwCount = 1;
            hwInfo->hw[0][0] = 128;
            hwInfo->hw[0][1] = 128;
            return ACL_SUCCESS;
        }));
    
    // 执行测试
    testing::internal::CaptureStdout();
    modelProcess->GetDymHWInfo();
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_TRUE(logOutput.find("model has 1 gear of HW") != string::npos);
    EXPECT_TRUE(logOutput.find("please set correct dynamic batch size") != string::npos);
}

// ===================== PrintDesc 测试用例 =====================

TEST_F(ModelProcessTest, TestPrintDesc_Success)
{
    // 创建模型描述
    SetDebugLogGuard guard;
    auto fakeDesc = CreateModelDescSuccess();
    
    // 设置输入输出数量
    size_t numInputs = 2;
    size_t numOutputs = 1;
    EXPECT_CALL(*mockAcl, aclmdlGetNumInputs(fakeDesc)).WillOnce(Return(numInputs));
    EXPECT_CALL(*mockAcl, aclmdlGetNumOutputs(fakeDesc)).WillOnce(Return(numOutputs));
    
    // 设置动态batch信息
    aclmdlBatch batch_info = {2, {1, 2}};
    EXPECT_CALL(*mockAcl, aclmdlGetDynamicBatch(fakeDesc, _))
        .WillOnce(DoAll(SetArgPointee<1>(batch_info), Return(ACL_SUCCESS)));

    SetupMockModelDescription(fakeDesc, numInputs, numOutputs);
    
    // 设置动态HW信息
    aclmdlHW dynamicHW = {2, {{224,224}, {448,448}}};
    EXPECT_CALL(*mockAcl, aclmdlGetDynamicHW(fakeDesc, -1, _))
        .WillOnce(DoAll(SetArgPointee<2>(dynamicHW), Return(ACL_SUCCESS)));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->PrintDesc();
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(logOutput.find("NumInputs: 2") != string::npos);
    EXPECT_TRUE(logOutput.find("NumOutputs: 1") != string::npos);
    EXPECT_TRUE(logOutput.find("DynamicBatch:") != string::npos);
    EXPECT_TRUE(logOutput.find("1 2") != string::npos);
    EXPECT_TRUE(logOutput.find("DynamicHW:") != string::npos);
    EXPECT_TRUE(logOutput.find("224,224") != string::npos);
    EXPECT_TRUE(logOutput.find("448,448") != string::npos);
    EXPECT_TRUE(logOutput.find("end print model description") != string::npos);
}

TEST_F(ModelProcessTest, TestPrintDesc_GetDynamicBatchFailed)
{
    // 创建模型描述
    auto fakeDesc = CreateModelDescSuccess();
    
    // 设置输入输出数量
    size_t numInputs = 2;
    size_t numOutputs = 1;
    EXPECT_CALL(*mockAcl, aclmdlGetNumInputs(fakeDesc)).WillOnce(Return(numInputs));
    EXPECT_CALL(*mockAcl, aclmdlGetNumOutputs(fakeDesc)).WillOnce(Return(numOutputs));
    
    // 模拟获取动态batch失败
    EXPECT_CALL(*mockAcl, aclmdlGetDynamicBatch(fakeDesc, _))
        .WillOnce(Return(ACL_ERROR_INVALID_PARAM));
    const char* errorMsg = "Get dynamic batch failed";
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg()).WillOnce(Return(errorMsg));
    
    SetupMockModelDescription(fakeDesc, numInputs, numOutputs);

    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->PrintDesc();
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("get DynamicBatch failed") != string::npos);
}

TEST_F(ModelProcessTest, TestPrintDesc_GetDynamicHWFailed)
{
    // 创建模型描述
    auto fakeDesc = CreateModelDescSuccess();
    
    // 设置输入输出数量
    size_t numInputs = 2;
    size_t numOutputs = 1;
    EXPECT_CALL(*mockAcl, aclmdlGetNumInputs(fakeDesc)).WillOnce(Return(numInputs));
    EXPECT_CALL(*mockAcl, aclmdlGetNumOutputs(fakeDesc)).WillOnce(Return(numOutputs));
    
    // 设置动态batch信息
    aclmdlBatch batch_info = {0, {}};
    EXPECT_CALL(*mockAcl, aclmdlGetDynamicBatch(fakeDesc, _))
        .WillOnce(DoAll(SetArgPointee<1>(batch_info), Return(ACL_SUCCESS)));
    
    // 模拟获取动态HW失败
    EXPECT_CALL(*mockAcl, aclmdlGetDynamicHW(fakeDesc, -1, _))
        .WillOnce(Return(ACL_ERROR_INVALID_PARAM));
    const char* errorMsg = "Get dynamic HW failed";
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg()).WillOnce(Return(errorMsg));
    
    SetupMockModelDescription(fakeDesc, numInputs, numOutputs);

    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->PrintDesc();
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("Get dynamic HW failed") != string::npos);
    EXPECT_EQ(modelProcess->modelDesc_, nullptr);
}

// // ===================== DestroyDesc 测试用例 =====================

TEST_F(ModelProcessTest, TestDestroyDesc_Success)
{
    auto fakeDesc = reinterpret_cast<aclmdlDesc*>(0x1234);
    modelProcess->modelDesc_ = fakeDesc;
    
    EXPECT_CALL(*mockAcl, aclmdlDestroyDesc(fakeDesc))
        .WillOnce(Return(ACL_SUCCESS));
    
    modelProcess->DestroyDesc();
    
    EXPECT_EQ(modelProcess->modelDesc_, nullptr);
}

TEST_F(ModelProcessTest, TestDestroyDesc_WhenModelDescIsNull)
{
    modelProcess->modelDesc_ = nullptr;

    EXPECT_CALL(*mockAcl, aclmdlDestroyDesc(nullptr))
        .Times(0);
    
    // 不会有调用
    modelProcess->DestroyDesc();
}

// ===================== CreateDymInput 测试用例 =====================

TEST_F(ModelProcessTest, TestCreateDymInput_Success)
{
    SetDebugLogGuard guard;
    // 创建模型描述
    auto fakeDesc = CreateModelDescSuccess();
    
    // 创建空数据集
    modelProcess->input_ = nullptr;
    
    // 设置模拟期望
    size_t index = 0;
    size_t buffer_size = 1024;
    
    // 1. 获取输入大小
    EXPECT_CALL(*mockAcl, aclmdlGetInputSizeByIndex(fakeDesc, index))
        .WillOnce(Return(buffer_size));
    
    // 2. 创建、销毁数据集
    aclmdlDataset* fakeDataset = reinterpret_cast<aclmdlDataset*>(0xABCD);
    EXPECT_CALL(*mockAcl, aclmdlCreateDataset())
        .WillOnce(Return(fakeDataset));
    EXPECT_CALL(*mockAcl, aclmdlDestroyDataset(reinterpret_cast<const aclmdlDataset*>(fakeDataset)))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 3. 分配设备内存
    void* fakeBuffer = reinterpret_cast<void*>(0x1000);
    EXPECT_CALL(*mockAcl, aclrtMalloc(_, buffer_size, ACL_MEM_MALLOC_HUGE_FIRST))
        .WillOnce(DoAll(SetArgPointee<0>(fakeBuffer), Return(ACL_SUCCESS)));
    
    // 4. 创建数据缓冲区
    aclDataBuffer* fakeDataBuffer = reinterpret_cast<aclDataBuffer*>(0x2000);
    EXPECT_CALL(*mockAcl, aclCreateDataBuffer(fakeBuffer, buffer_size))
        .WillOnce(Return(fakeDataBuffer));
    
    // 5. 添加数据集缓冲区
    EXPECT_CALL(*mockAcl, aclmdlAddDatasetBuffer(fakeDataset, fakeDataBuffer))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CreateDymInput(index);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(modelProcess->input_, fakeDataset);
    EXPECT_TRUE(logOutput.find("add input_ at CreateDymInput +1") != string::npos);
}

TEST_F(ModelProcessTest, TestCreateDymInput_AclrtMallocFailed)
{
    // 创建模型描述
    auto fakeDesc = CreateModelDescSuccess();
    
    // 创建空数据集
    modelProcess->input_ = nullptr;
    
    // 设置模拟期望
    size_t index = 0;
    size_t buffer_size = 1024;
    
    // 1. 获取输入大小
    EXPECT_CALL(*mockAcl, aclmdlGetInputSizeByIndex(fakeDesc, index))
        .WillOnce(Return(buffer_size));
    
    // 2. 创建、销毁数据集
    aclmdlDataset* fakeDataset = reinterpret_cast<aclmdlDataset*>(0xABCD);
    EXPECT_CALL(*mockAcl, aclmdlCreateDataset())
        .WillOnce(Return(fakeDataset));
    EXPECT_CALL(*mockAcl, aclmdlDestroyDataset(reinterpret_cast<const aclmdlDataset*>(fakeDataset)))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 3. 分配设备内存失败
    EXPECT_CALL(*mockAcl, aclrtMalloc(_, buffer_size, ACL_MEM_MALLOC_HUGE_FIRST))
        .WillOnce(Return(ACL_ERROR_BAD_ALLOC));
    const char* errorMsg = "aclrtMalloc failed";
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillOnce(Return(errorMsg));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CreateDymInput(index);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("malloc device buffer failed") != string::npos);
}

TEST_F(ModelProcessTest, TestCreateDymInput_CreateDatasetFailed)
{
    // 创建模型描述
    CreateModelDescSuccess();
    
    // 创建空数据集
    modelProcess->input_ = nullptr;
    
    // 创建数据集返回空指针
    EXPECT_CALL(*mockAcl, aclmdlCreateDataset)
        .WillOnce(Return(nullptr));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CreateDymInput(0);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("can't create dataset") != string::npos);
    EXPECT_TRUE(logOutput.find("create input failed") != string::npos);
}

TEST_F(ModelProcessTest, TestCreateDymInput_CreateDataBufferFailed)
{
    // 创建模型描述
    auto fakeDesc = CreateModelDescSuccess();
    
    // 创建空数据集
    modelProcess->input_ = nullptr;
    
    // 设置模拟期望
    size_t index = 0;
    size_t buffer_size = 1024;
    
    // 1. 获取输入大小
    EXPECT_CALL(*mockAcl, aclmdlGetInputSizeByIndex(fakeDesc, index))
        .WillOnce(Return(buffer_size));
    
    // 2. 创建、销毁数据集
    aclmdlDataset* fakeDataset = reinterpret_cast<aclmdlDataset*>(0xABCD);
    EXPECT_CALL(*mockAcl, aclmdlCreateDataset())
        .WillOnce(Return(fakeDataset));
    EXPECT_CALL(*mockAcl, aclmdlDestroyDataset(reinterpret_cast<const aclmdlDataset*>(fakeDataset)))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 3. 分配设备内存
    void* fakeBuffer = reinterpret_cast<void*>(0x1000);
    EXPECT_CALL(*mockAcl, aclrtMalloc(_, buffer_size, ACL_MEM_MALLOC_HUGE_FIRST))
        .WillOnce(DoAll(SetArgPointee<0>(fakeBuffer), Return(ACL_SUCCESS)));
    
    // 4. 创建数据缓冲区失败
    EXPECT_CALL(*mockAcl, aclCreateDataBuffer(fakeBuffer, buffer_size))
        .WillOnce(Return(nullptr));
    
    // 5. 预期释放资源
    EXPECT_CALL(*mockAcl, aclrtFree(fakeBuffer))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CreateDymInput(index);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("can't create data buffer") != string::npos);
}

TEST_F(ModelProcessTest, TestCreateDymInput_AddDatasetBufferFailed)
{
    SetDebugLogGuard guard;
    // 创建模型描述
    auto fakeDesc = CreateModelDescSuccess();
    
    // 创建空数据集
    modelProcess->input_ = nullptr;
    
    // 设置模拟期望
    size_t index = 0;
    size_t buffer_size = 1024;
    
    // 1. 获取输入大小
    EXPECT_CALL(*mockAcl, aclmdlGetInputSizeByIndex(fakeDesc, index))
        .WillOnce(Return(buffer_size));
    
    // 2. 创建、销毁数据集
    aclmdlDataset* fakeDataset = reinterpret_cast<aclmdlDataset*>(0xABCD);
    EXPECT_CALL(*mockAcl, aclmdlCreateDataset())
        .WillOnce(Return(fakeDataset));
    EXPECT_CALL(*mockAcl, aclmdlDestroyDataset(reinterpret_cast<const aclmdlDataset*>(fakeDataset)))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 3. 分配设备内存
    void* fakeBuffer = reinterpret_cast<void*>(0x1000);
    EXPECT_CALL(*mockAcl, aclrtMalloc(_, buffer_size, ACL_MEM_MALLOC_HUGE_FIRST))
        .WillOnce(DoAll(SetArgPointee<0>(fakeBuffer), Return(ACL_SUCCESS)));
    
    // 4. 创建数据缓冲区
    aclDataBuffer* fakeDataBuffer = reinterpret_cast<aclDataBuffer*>(0x2000);
    EXPECT_CALL(*mockAcl, aclCreateDataBuffer(fakeBuffer, buffer_size))
        .WillOnce(Return(fakeDataBuffer));
    
    // 5. 添加数据集缓冲区
    EXPECT_CALL(*mockAcl, aclmdlAddDatasetBuffer(fakeDataset, fakeDataBuffer))
        .WillOnce(Return(ACL_ERROR_FAILURE));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CreateDymInput(index);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_EQ(modelProcess->input_, fakeDataset);
    EXPECT_TRUE(logOutput.find("add input dataset buffer failed") != string::npos);
}

// ===================== UpdateInputsReuse 测试用例 =====================

TEST_F(ModelProcessTest, TestUpdateInputsReuse_InputDatasetNull)
{
    // 输出数据集存在，输入数据集为空
    modelProcess->output_ = reinterpret_cast<aclmdlDataset*>(0x2222);
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->UpdateInputsReuse({0});
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("can't find inputdatas") != string::npos);
}

TEST_F(ModelProcessTest, TestUpdateInputsReuse_OutputDatasetNull)
{
    // 输入数据集存在，输出数据集为空
    modelProcess->input_ = reinterpret_cast<aclmdlDataset*>(0x1111);
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->UpdateInputsReuse({0});
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("can't find outputdatas") != string::npos);
}

TEST_F(ModelProcessTest, TestUpdateInputsReuse_RelationSizeMismatch)
{
    // 设置数据集
    SetupDataset(2, 2);
    
    // 设置动态索引
    modelProcess->g_dymindex = 1;

    EXPECT_CALL(*mockAcl, aclmdlGetDatasetNumBuffers)
        .WillRepeatedly(Return(0));
    // 执行测试 - 关系向量大小小于输入数量（插入后）
    testing::internal::CaptureStdout();
    Result ret = modelProcess->UpdateInputsReuse({0});
    string logOutput = testing::internal::GetCapturedStdout();

    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("wrong in out relation size") != string::npos);
    EXPECT_TRUE(logOutput.find("inputsNum: 0") != string::npos);
}

TEST_F(ModelProcessTest, TestUpdateInputsReuse_OutputIndexOutOfRange)
{
    // 设置数据集
    SetupDataset(2, 1);
    
    // 设置动态索引
    modelProcess->g_dymindex = SIZE_MAX; // 无动态输入
    
    // 执行测试 - 输出索引超出范围
    testing::internal::CaptureStdout();
    Result ret = modelProcess->UpdateInputsReuse({0, 2}); // 索引2大于输出数量1
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("outputdata index out of range") != string::npos);
}

TEST_F(ModelProcessTest, TestUpdateInputsReuse_SizeMismatch)
{
    // 设置数据集
    SetupDataset(2, 1);
    modelProcess->g_dymindex = SIZE_MAX; // 无动态输入
    
    // 设置模拟缓冲区
    aclDataBuffer* fakeInputBuffer = reinterpret_cast<aclDataBuffer*>(0x3333);
    aclDataBuffer* fakeOutputBuffer = reinterpret_cast<aclDataBuffer*>(0x4444);
    
    // 模拟缓冲区获取
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetBuffer(reinterpret_cast<aclmdlDataset*>(0x1111), 0))
        .WillRepeatedly(Return(fakeInputBuffer));
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetBuffer(reinterpret_cast<aclmdlDataset*>(0x2222), 0))
        .WillRepeatedly(Return(fakeOutputBuffer));
    
    // 设置缓冲区大小不匹配
    EXPECT_CALL(*mockAcl, aclGetDataBufferSizeV2(fakeInputBuffer))
        .WillRepeatedly(Return(1024));
    EXPECT_CALL(*mockAcl, aclGetDataBufferSizeV2(fakeOutputBuffer))
        .WillRepeatedly(Return(512)); // 输出缓冲区更小
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->UpdateInputsReuse({0, -1}); // 跳过第二个输入
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("current inputSize and last outputSize not matched") != string::npos);
}

TEST_F(ModelProcessTest, TestUpdateInputsReuse_UpdateDataBufferFailed)
{
    SetDebugLogGuard guard;
    // 设置数据集
    SetupDataset(3, 2);
    modelProcess->g_dymindex = 1; // 动态输入在索引1位置
    
    // 设置模拟缓冲区
    aclDataBuffer* fakeInputBuffer0 = reinterpret_cast<aclDataBuffer*>(0x3333);
    aclDataBuffer* fakeOutputBuffer0 = reinterpret_cast<aclDataBuffer*>(0x4444);
    
    // 模拟索引0的缓冲区
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetBuffer(reinterpret_cast<aclmdlDataset*>(0x1111), 0))
        .WillRepeatedly(Return(fakeInputBuffer0));
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetBuffer(reinterpret_cast<aclmdlDataset*>(0x2222), 0))
        .WillRepeatedly(Return(fakeOutputBuffer0));
    
    // 设置索引2的缓冲区 - 移除重复声明
    aclDataBuffer* fakeInputBuffer2 = reinterpret_cast<aclDataBuffer*>(0x6666);
    aclDataBuffer* fakeOutputBuffer1 = reinterpret_cast<aclDataBuffer*>(0x7777);
    
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetBuffer(reinterpret_cast<aclmdlDataset*>(0x1111), 2))
        .WillRepeatedly(Return(fakeInputBuffer2));
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetBuffer(reinterpret_cast<aclmdlDataset*>(0x2222), 1))
        .WillRepeatedly(Return(fakeOutputBuffer1));
    
    // 设置缓冲区大小匹配
    size_t bufferSize = 1024;
    EXPECT_CALL(*mockAcl, aclGetDataBufferSizeV2(_))
        .WillRepeatedly(Return(bufferSize));
    
    // 设置缓冲区地址
    void* fakeAddrIn0 = reinterpret_cast<void*>(0x1000);
    void* fakeAddrOut0 = reinterpret_cast<void*>(0x2000);
    void* fakeAddrIn2 = reinterpret_cast<void*>(0x3000);
    void* fakeAddrOut1 = reinterpret_cast<void*>(0x4000);
    
    EXPECT_CALL(*mockAcl, aclGetDataBufferAddr(fakeInputBuffer0))
        .WillRepeatedly(Return(fakeAddrIn0));
    EXPECT_CALL(*mockAcl, aclGetDataBufferAddr(fakeOutputBuffer0))
        .WillRepeatedly(Return(fakeAddrOut0));
    EXPECT_CALL(*mockAcl, aclGetDataBufferAddr(fakeInputBuffer2))
        .WillRepeatedly(Return(fakeAddrIn2));
    EXPECT_CALL(*mockAcl, aclGetDataBufferAddr(fakeOutputBuffer1))
        .WillRepeatedly(Return(fakeAddrOut1));
    
    // 设置更新操作失败
    EXPECT_CALL(*mockAcl, aclUpdateDataBuffer)
        .WillRepeatedly(Return(ACL_ERROR_FAILURE));
    
    // 执行测试 - 动态索引在1，所以关系向量索引0对应输入0，索引1跳过，索引2对应输入2
    testing::internal::CaptureStdout();
    Result ret = modelProcess->UpdateInputsReuse({0, 1});
    string logOutput = testing::internal::GetCapturedStdout();

    // 验证结果
    EXPECT_EQ(ret, FAILED);
    // 错误日志
    EXPECT_TRUE(logOutput.find("new input buffer update from last output failed") != string::npos);
}

TEST_F(ModelProcessTest, TestUpdateInputsReuse_SuccessWithDynamicIndex)
{
    SetDebugLogGuard guard;
    // 设置数据集
    SetupDataset(3, 2);
    modelProcess->g_dymindex = 1; // 动态输入在索引1位置
    
    // 设置模拟缓冲区
    aclDataBuffer* fakeInputBuffer0 = reinterpret_cast<aclDataBuffer*>(0x3333);
    aclDataBuffer* fakeOutputBuffer0 = reinterpret_cast<aclDataBuffer*>(0x4444);
    
    // 模拟索引0的缓冲区
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetBuffer(reinterpret_cast<aclmdlDataset*>(0x1111), 0))
        .WillRepeatedly(Return(fakeInputBuffer0));
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetBuffer(reinterpret_cast<aclmdlDataset*>(0x2222), 0))
        .WillRepeatedly(Return(fakeOutputBuffer0));
    
    // 设置索引2的缓冲区 - 移除重复声明
    aclDataBuffer* fakeInputBuffer2 = reinterpret_cast<aclDataBuffer*>(0x6666);
    aclDataBuffer* fakeOutputBuffer1 = reinterpret_cast<aclDataBuffer*>(0x7777);
    
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetBuffer(reinterpret_cast<aclmdlDataset*>(0x1111), 2))
        .WillRepeatedly(Return(fakeInputBuffer2));
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetBuffer(reinterpret_cast<aclmdlDataset*>(0x2222), 1))
        .WillRepeatedly(Return(fakeOutputBuffer1));
    
    // 设置缓冲区大小匹配
    size_t bufferSize = 1024;
    EXPECT_CALL(*mockAcl, aclGetDataBufferSizeV2(_))
        .WillRepeatedly(Return(bufferSize));
    
    // 设置缓冲区地址
    void* fakeAddrIn0 = reinterpret_cast<void*>(0x1000);
    void* fakeAddrOut0 = reinterpret_cast<void*>(0x2000);
    void* fakeAddrIn2 = reinterpret_cast<void*>(0x3000);
    void* fakeAddrOut1 = reinterpret_cast<void*>(0x4000);
    
    EXPECT_CALL(*mockAcl, aclGetDataBufferAddr(fakeInputBuffer0))
        .WillRepeatedly(Return(fakeAddrIn0));
    EXPECT_CALL(*mockAcl, aclGetDataBufferAddr(fakeOutputBuffer0))
        .WillRepeatedly(Return(fakeAddrOut0));
    EXPECT_CALL(*mockAcl, aclGetDataBufferAddr(fakeInputBuffer2))
        .WillRepeatedly(Return(fakeAddrIn2));
    EXPECT_CALL(*mockAcl, aclGetDataBufferAddr(fakeOutputBuffer1))
        .WillRepeatedly(Return(fakeAddrOut1));
    
    // 设置更新操作成功
    EXPECT_CALL(*mockAcl, aclUpdateDataBuffer(fakeInputBuffer0, fakeAddrOut0, bufferSize))
        .WillRepeatedly(Return(ACL_SUCCESS));
    EXPECT_CALL(*mockAcl, aclUpdateDataBuffer(fakeInputBuffer2, fakeAddrOut1, bufferSize))
        .WillRepeatedly(Return(ACL_SUCCESS));
    
    // 资源释放 - reuseOutput_ = false 时释放旧缓冲区
    modelProcess->reuseOutput_ = false;
    EXPECT_CALL(*mockAcl, aclrtFree(fakeAddrIn0))
        .WillRepeatedly(Return(ACL_SUCCESS));
    EXPECT_CALL(*mockAcl, aclrtFree(fakeAddrIn2))
        .WillRepeatedly(Return(ACL_SUCCESS));
    
    // 执行测试 - 动态索引在1，所以关系向量索引0对应输入0，索引1跳过，索引2对应输入2
    testing::internal::CaptureStdout();
    Result ret = modelProcess->UpdateInputsReuse({0, 1});
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(modelProcess->reuseOutput_);
    // 确保没有错误日志
    EXPECT_TRUE(logOutput.find("failed") == string::npos);
}

// ===================== CreateInput 测试用例 =====================

TEST_F(ModelProcessTest, TestCreateInput_SuccessWithNewDataset)
{
    SetDebugLogGuard guard;
    // 设置初始状态
    modelProcess->input_ = nullptr;
    
    // 准备测试数据
    void* inputDataBuffer = reinterpret_cast<void*>(0x1234);
    size_t bufferSize = 1024;
    
    // 设置模拟期望
    aclmdlDataset* fakeDataset = reinterpret_cast<aclmdlDataset*>(0xABCD);
    EXPECT_CALL(*mockAcl, aclmdlCreateDataset())
        .WillRepeatedly(Return(fakeDataset));
    
    aclDataBuffer* fakeDataBuffer = reinterpret_cast<aclDataBuffer*>(0x1000);
    EXPECT_CALL(*mockAcl, aclCreateDataBuffer(inputDataBuffer, bufferSize))
        .WillRepeatedly(Return(fakeDataBuffer));
    
    EXPECT_CALL(*mockAcl, aclmdlAddDatasetBuffer(fakeDataset, fakeDataBuffer))
        .WillRepeatedly(Return(ACL_SUCCESS));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CreateInput(inputDataBuffer, bufferSize);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(modelProcess->input_, fakeDataset);
    EXPECT_TRUE(logOutput.find("add input_ at CreateInput +1") != string::npos);
}

TEST_F(ModelProcessTest, TestCreateInput_SuccessWithExistingDataset)
{
    SetDebugLogGuard guard;
    // 设置初始状态
    modelProcess->input_ = reinterpret_cast<aclmdlDataset*>(0x1111);
    
    // 准备测试数据
    void* inputDataBuffer = reinterpret_cast<void*>(0x1234);
    size_t bufferSize = 1024;
    
    // 设置模拟期望
    aclDataBuffer* fakeDataBuffer = reinterpret_cast<aclDataBuffer*>(0x1000);
    EXPECT_CALL(*mockAcl, aclCreateDataBuffer(inputDataBuffer, bufferSize))
        .WillRepeatedly(Return(fakeDataBuffer));
    
    EXPECT_CALL(*mockAcl, aclmdlAddDatasetBuffer(modelProcess->input_, fakeDataBuffer))
        .WillRepeatedly(Return(ACL_SUCCESS));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CreateInput(inputDataBuffer, bufferSize);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(logOutput.find("add input_ at CreateInput +1") != string::npos);
}

TEST_F(ModelProcessTest, TestCreateInput_CreateDatasetFailed)
{
    // 设置初始状态
    modelProcess->input_ = nullptr;
    
    // 准备测试数据
    void* inputDataBuffer = reinterpret_cast<void*>(0x1234);
    size_t bufferSize = 1024;
    
    // 设置模拟期望
    EXPECT_CALL(*mockAcl, aclmdlCreateDataset())
        .WillRepeatedly(Return(nullptr));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CreateInput(inputDataBuffer, bufferSize);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("can't create dataset") != string::npos);
}

TEST_F(ModelProcessTest, TestCreateInput_CreateDataBufferFailed)
{
    // 设置初始状态
    modelProcess->input_ = nullptr;
    
    // 准备测试数据
    void* inputDataBuffer = reinterpret_cast<void*>(0x1234);
    size_t bufferSize = 1024;
    
    // 设置模拟期望
    aclmdlDataset* fakeDataset = reinterpret_cast<aclmdlDataset*>(0xABCD);
    EXPECT_CALL(*mockAcl, aclmdlCreateDataset())
        .WillRepeatedly(Return(fakeDataset));
    
    EXPECT_CALL(*mockAcl, aclCreateDataBuffer(inputDataBuffer, bufferSize))
        .WillRepeatedly(Return(nullptr));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CreateInput(inputDataBuffer, bufferSize);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("can't create data buffer") != string::npos);
}

TEST_F(ModelProcessTest, TestCreateInput_AddDatasetBufferFailed)
{
    // 设置初始状态
    modelProcess->input_ = nullptr;
    
    // 准备测试数据
    void* inputDataBuffer = reinterpret_cast<void*>(0x1234);
    size_t bufferSize = 1024;
    
    // 设置模拟期望
    aclmdlDataset* fakeDataset = reinterpret_cast<aclmdlDataset*>(0xABCD);
    EXPECT_CALL(*mockAcl, aclmdlCreateDataset())
        .WillRepeatedly(Return(fakeDataset));
    
    aclDataBuffer* fakeDataBuffer = reinterpret_cast<aclDataBuffer*>(0x1000);
    EXPECT_CALL(*mockAcl, aclCreateDataBuffer(inputDataBuffer, bufferSize))
        .WillRepeatedly(Return(fakeDataBuffer));
    
    EXPECT_CALL(*mockAcl, aclmdlAddDatasetBuffer(fakeDataset, fakeDataBuffer))
        .WillRepeatedly(Return(ACL_ERROR_BAD_ALLOC));
    
    // 预期销毁数据缓冲区
    EXPECT_CALL(*mockAcl, aclDestroyDataBuffer(fakeDataBuffer))
        .WillRepeatedly(Return(ACL_SUCCESS));
    
    const char* errorMsg = "Add dataset buffer failed";
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillRepeatedly(Return(errorMsg));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CreateInput(inputDataBuffer, bufferSize);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("add input dataset buffer failed") != string::npos);
    EXPECT_TRUE(logOutput.find(errorMsg) != string::npos);
}

// ===================== UpdateInputsMemcpy 测试用例 =====================

TEST_F(ModelProcessTest, TestUpdateInputsMemcpy_SuccessWithDynamicIndex)
{
    SetDebugLogGuard guard;
    // 设置数据集
    SetupDataset(3, 2);
    modelProcess->g_dymindex = 1; // 动态输入在索引1位置
    
    // 准备测试数据
    vector<int> inOutRelation = {0, 1}; // 输入0->输出0, 输入2->输出1
    
    // 设置模拟缓冲区
    aclDataBuffer* fakeInputBuffer0 = reinterpret_cast<aclDataBuffer*>(0x3333);
    aclDataBuffer* fakeOutputBuffer0 = reinterpret_cast<aclDataBuffer*>(0x4444);
    aclDataBuffer* fakeInputBuffer2 = reinterpret_cast<aclDataBuffer*>(0x5555);
    aclDataBuffer* fakeOutputBuffer1 = reinterpret_cast<aclDataBuffer*>(0x6666);
    
    // 模拟缓冲区获取
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetBuffer(modelProcess->input_, 0))
        .WillRepeatedly(Return(fakeInputBuffer0));
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetBuffer(modelProcess->output_, 0))
        .WillRepeatedly(Return(fakeOutputBuffer0));
    
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetBuffer(modelProcess->input_, 2))
        .WillRepeatedly(Return(fakeInputBuffer2));
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetBuffer(modelProcess->output_, 1))
        .WillRepeatedly(Return(fakeOutputBuffer1));
    
    // 设置缓冲区大小匹配
    size_t bufferSize = 1024;
    EXPECT_CALL(*mockAcl, aclGetDataBufferSizeV2(fakeInputBuffer0))
        .WillRepeatedly(Return(bufferSize));
    EXPECT_CALL(*mockAcl, aclGetDataBufferSizeV2(fakeOutputBuffer0))
        .WillRepeatedly(Return(bufferSize));
    
    EXPECT_CALL(*mockAcl, aclGetDataBufferSizeV2(fakeInputBuffer2))
        .WillRepeatedly(Return(bufferSize));
    EXPECT_CALL(*mockAcl, aclGetDataBufferSizeV2(fakeOutputBuffer1))
        .WillRepeatedly(Return(bufferSize));
    
    // 设置缓冲区地址
    void* fakeAddrIn0 = reinterpret_cast<void*>(0x1000);
    void* fakeAddrOut0 = reinterpret_cast<void*>(0x2000);
    void* fakeAddrIn2 = reinterpret_cast<void*>(0x3000);
    void* fakeAddrOut1 = reinterpret_cast<void*>(0x4000);
    
    EXPECT_CALL(*mockAcl, aclGetDataBufferAddr(fakeInputBuffer0))
        .WillRepeatedly(Return(fakeAddrIn0));
    EXPECT_CALL(*mockAcl, aclGetDataBufferAddr(fakeOutputBuffer0))
        .WillRepeatedly(Return(fakeAddrOut0));
    EXPECT_CALL(*mockAcl, aclGetDataBufferAddr(fakeInputBuffer2))
        .WillRepeatedly(Return(fakeAddrIn2));
    EXPECT_CALL(*mockAcl, aclGetDataBufferAddr(fakeOutputBuffer1))
        .WillRepeatedly(Return(fakeAddrOut1));
    
    // 设置内存复制成功
    EXPECT_CALL(*mockAcl, aclrtMemcpy(fakeAddrIn0, bufferSize, fakeAddrOut0, bufferSize, ACL_MEMCPY_DEVICE_TO_DEVICE))
        .WillRepeatedly(Return(ACL_SUCCESS));
    EXPECT_CALL(*mockAcl, aclrtMemcpy(fakeAddrIn2, bufferSize, fakeAddrOut1, bufferSize, ACL_MEMCPY_DEVICE_TO_DEVICE))
        .WillRepeatedly(Return(ACL_SUCCESS));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->UpdateInputsMemcpy(inOutRelation);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, SUCCESS);
}

TEST_F(ModelProcessTest, TestUpdateInputsMemcpy_SuccessWithoutDynamicIndex)
{
    SetDebugLogGuard guard;
    // 设置数据集
    SetupDataset(2, 2);
    modelProcess->g_dymindex = SIZE_MAX; // 无动态索引
    
    // 准备测试数据
    vector<int> inOutRelation = {0, 1}; // 输入0->输出0, 输入1->输出1
    
    // 设置模拟缓冲区
    aclDataBuffer* fakeInputBuffer0 = reinterpret_cast<aclDataBuffer*>(0x3333);
    aclDataBuffer* fakeOutputBuffer0 = reinterpret_cast<aclDataBuffer*>(0x4444);
    aclDataBuffer* fakeInputBuffer1 = reinterpret_cast<aclDataBuffer*>(0x5555);
    aclDataBuffer* fakeOutputBuffer1 = reinterpret_cast<aclDataBuffer*>(0x6666);

    // 模拟缓冲区获取
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetBuffer(modelProcess->input_, 0))
        .WillRepeatedly(Return(fakeInputBuffer0));
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetBuffer(modelProcess->output_, 0))
        .WillRepeatedly(Return(fakeOutputBuffer0));
    
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetBuffer(modelProcess->input_, 1))
        .WillRepeatedly(Return(fakeInputBuffer1));
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetBuffer(modelProcess->output_, 1))
        .WillRepeatedly(Return(fakeOutputBuffer1));
    
    // 设置缓冲区大小匹配
    size_t bufferSize = 1024;
    EXPECT_CALL(*mockAcl, aclGetDataBufferSizeV2(_))
        .Times(6) // 每个缓冲区调用一次
        .WillRepeatedly(Return(bufferSize));
    
    // 设置缓冲区地址
    void* fakeAddrIn0 = reinterpret_cast<void*>(0x1000);
    void* fakeAddrOut0 = reinterpret_cast<void*>(0x2000);
    void* fakeAddrIn1 = reinterpret_cast<void*>(0x3000);
    void* fakeAddrOut1 = reinterpret_cast<void*>(0x4000);
    
    EXPECT_CALL(*mockAcl, aclGetDataBufferAddr(fakeInputBuffer0))
        .WillRepeatedly(Return(fakeAddrIn0));
    EXPECT_CALL(*mockAcl, aclGetDataBufferAddr(fakeOutputBuffer0))
        .WillRepeatedly(Return(fakeAddrOut0));
    EXPECT_CALL(*mockAcl, aclGetDataBufferAddr(fakeInputBuffer1))
        .WillRepeatedly(Return(fakeAddrIn1));
    EXPECT_CALL(*mockAcl, aclGetDataBufferAddr(fakeOutputBuffer1))
        .WillRepeatedly(Return(fakeAddrOut1));
    
    // 设置内存复制成功
    EXPECT_CALL(*mockAcl, aclrtMemcpy(fakeAddrIn0, bufferSize, fakeAddrOut0, bufferSize, ACL_MEMCPY_DEVICE_TO_DEVICE))
        .WillRepeatedly(Return(ACL_SUCCESS));
    EXPECT_CALL(*mockAcl, aclrtMemcpy(fakeAddrIn1, bufferSize, fakeAddrOut1, bufferSize, ACL_MEMCPY_DEVICE_TO_DEVICE))
        .WillRepeatedly(Return(ACL_SUCCESS));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->UpdateInputsMemcpy(inOutRelation);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, SUCCESS);
}

TEST_F(ModelProcessTest, TestUpdateInputsMemcpy_InputSizeExceedsOutputSize)
{
    // 设置数据集
    SetupDataset(1, 1);
    
    // 准备测试数据
    vector<int> inOutRelation = {0}; 
    
    // 设置模拟缓冲区
    aclDataBuffer* fakeInputBuffer  = reinterpret_cast<aclDataBuffer*>(0x3333);
    aclDataBuffer* fakeOutputBuffer = reinterpret_cast<aclDataBuffer*>(0x4444);
    
    
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetNumBuffers(modelProcess->input_))
        .WillRepeatedly(Return(2));

    // 模拟缓冲区获取
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetBuffer(modelProcess->input_, _))
        .WillRepeatedly(Return(fakeInputBuffer));
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetBuffer(modelProcess->output_, 0))
        .WillRepeatedly(Return(fakeOutputBuffer));
    
    // 设置缓冲区大小不匹配（输入 > 输出）
    EXPECT_CALL(*mockAcl, aclGetDataBufferSizeV2(fakeInputBuffer))
        .WillRepeatedly(Return(2048));
    EXPECT_CALL(*mockAcl, aclGetDataBufferSizeV2(fakeOutputBuffer))
        .WillRepeatedly(Return(1024));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->UpdateInputsMemcpy(inOutRelation);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("current inputSize and last outputSize not matched") != string::npos);
}

TEST_F(ModelProcessTest, TestUpdateInputsMemcpy_MemcpyFailed)
{
    // 设置数据集
    SetupDataset(1, 1);
    
    // 准备测试数据
    vector<int> inOutRelation = {0}; 
    
    // 设置模拟缓冲区
    aclDataBuffer* fakeInputBuffer = reinterpret_cast<aclDataBuffer*>(0x3333);
    aclDataBuffer* fakeOutputBuffer = reinterpret_cast<aclDataBuffer*>(0x4444);
    
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetNumBuffers(modelProcess->input_))
        .WillRepeatedly(Return(2));

    // 模拟缓冲区获取
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetBuffer(modelProcess->input_, _))
        .WillRepeatedly(Return(fakeInputBuffer));
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetBuffer(modelProcess->output_, _))
        .WillRepeatedly(Return(fakeOutputBuffer));
    
    // 设置缓冲区大小匹配
    size_t bufferSize = 1024;
    EXPECT_CALL(*mockAcl, aclGetDataBufferSizeV2(fakeInputBuffer))
        .WillRepeatedly(Return(bufferSize));
    EXPECT_CALL(*mockAcl, aclGetDataBufferSizeV2(fakeOutputBuffer))
        .WillRepeatedly(Return(bufferSize));
    
    // 设置缓冲区地址
    void* fakeAddrIn = reinterpret_cast<void*>(0x1000);
    void* fakeAddrOut = reinterpret_cast<void*>(0x2000);
    EXPECT_CALL(*mockAcl, aclGetDataBufferAddr(fakeInputBuffer))
        .WillRepeatedly(Return(fakeAddrIn));
    EXPECT_CALL(*mockAcl, aclGetDataBufferAddr(fakeOutputBuffer))
        .WillRepeatedly(Return(fakeAddrOut));
    
    // 模拟内存复制失败
    EXPECT_CALL(*mockAcl, aclrtMemcpy(fakeAddrIn, bufferSize, fakeAddrOut, bufferSize, ACL_MEMCPY_DEVICE_TO_DEVICE))
        .WillRepeatedly(Return(ACL_ERROR_BAD_ALLOC));
    const char* errorMsg = "Memcpy failed";
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillRepeatedly(Return(errorMsg));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->UpdateInputsMemcpy(inOutRelation);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("acl memcpy from last output failed") != string::npos);
    EXPECT_TRUE(logOutput.find(errorMsg) != string::npos);
}

TEST_F(ModelProcessTest, TestUpdateInputsMemcpy_OutputIndexOutOfRange)
{
    // 设置数据集
    SetupDataset(1, 1);
    
    // 准备测试数据 - 输出索引超出范围
    vector<int> inOutRelation = {1}; // 输出索引1，但只有1个输出（索引0）
    
    // 设置动态索引
    modelProcess->g_dymindex = SIZE_MAX; // 无动态索引
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->UpdateInputsMemcpy(inOutRelation);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("find outputdata index out of range") != string::npos);
}

TEST_F(ModelProcessTest, TestUpdateInputsMemcpy_OutputInputNullptr)
{
    // 设置数据集
    SetupDataset(1, 1);
    
    // 准备测试数据
    vector<int> inOutRelation = {1};
    
    // 设置空指针
    modelProcess->input_ = nullptr;
    modelProcess->output_ = nullptr;
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->UpdateInputsMemcpy(inOutRelation);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("can't find inputdatas") != string::npos);
    EXPECT_TRUE(logOutput.find("can't find outputdatas") != string::npos);
}

TEST_F(ModelProcessTest, TestUpdateInputsMemcpy_InputsNumNotEqInOutListSize)
{
    // 设置数据集
    SetupDataset(1, 1);
    
    // 准备测试数据
    vector<int> inOutRelation = {0}; 
    
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetNumBuffers(modelProcess->input_))
        .WillRepeatedly(Return(3));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->UpdateInputsMemcpy(inOutRelation);
    string logOutput = testing::internal::GetCapturedStdout();
    cout << logOutput << endl;
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("wrong in out relation size") != string::npos);
}


// ===================== CreateZeroInput 测试用例 =====================

TEST_F(ModelProcessTest, TestCreateZeroInput_Success)
{
    // 创建模型描述
    auto fakeDesc = CreateModelDescSuccess();
    
    // 设置创建数据集
    aclmdlDataset* fakeDataset = reinterpret_cast<aclmdlDataset*>(0xABCD);
    EXPECT_CALL(*mockAcl, aclmdlCreateDataset())
        .WillRepeatedly(Return(fakeDataset));
    
    // 设置输入数量为2
    size_t numInputs = 2;
    EXPECT_CALL(*mockAcl, aclmdlGetNumInputs(fakeDesc))
        .WillRepeatedly(Return(numInputs));
    
    // 设置输入名称和大小
    vector<const char*> inputNames = {"input1", "input2"};
    vector<size_t> inputSizes = {1024, 2048};
    
    for (size_t i = 0; i < numInputs; i++) {
        EXPECT_CALL(*mockAcl, aclmdlGetInputNameByIndex(fakeDesc, i))
            .WillOnce(Return(inputNames[i]));
        EXPECT_CALL(*mockAcl, aclmdlGetInputSizeByIndex(fakeDesc, i))
            .WillOnce(Return(inputSizes[i]));
    }
    
    // 设置模拟行为
    for (size_t i = 0; i < numInputs; i++) {
        void* fakeBuffer = reinterpret_cast<void*>(0x1000 + i * 0x1000);
        EXPECT_CALL(*mockAcl, aclrtMalloc(_, inputSizes[i], ACL_MEM_MALLOC_HUGE_FIRST))
            .WillOnce(DoAll(SetArgPointee<0>(fakeBuffer), Return(ACL_SUCCESS)));
        
        // 非动态张量，需要memset
        EXPECT_CALL(*mockAcl, aclrtMemset(fakeBuffer, inputSizes[i], 0, inputSizes[i]))
            .WillOnce(Return(ACL_SUCCESS));
        
        aclDataBuffer* fakeDataBuffer = reinterpret_cast<aclDataBuffer*>(0x2000 + i * 0x1000);
        EXPECT_CALL(*mockAcl, aclCreateDataBuffer(fakeBuffer, inputSizes[i]))
            .WillOnce(Return(fakeDataBuffer));
        
        EXPECT_CALL(*mockAcl, aclmdlAddDatasetBuffer(_, fakeDataBuffer))
            .WillOnce(Return(ACL_SUCCESS));
    }
    
    // 执行测试
    SetDebugLogGuard guard;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CreateZeroInput();
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(logOutput.find("add input_ at CreateZeroInput +1") != string::npos);
}

TEST_F(ModelProcessTest, TestCreateZeroInput_DynamicTensorNoMemset)
{
    // 创建模型描述
    auto fakeDesc = CreateModelDescSuccess();
    
    // 设置创建数据集
    aclmdlDataset* fakeDataset = reinterpret_cast<aclmdlDataset*>(0xABCD);
    EXPECT_CALL(*mockAcl, aclmdlCreateDataset())
        .WillRepeatedly(Return(fakeDataset));
    
    // 设置输入数量为1（动态张量）
    size_t numInputs = 1;
    EXPECT_CALL(*mockAcl, aclmdlGetNumInputs(fakeDesc))
        .WillRepeatedly(Return(numInputs));
    
    // 设置动态张量名称
    EXPECT_CALL(*mockAcl, aclmdlGetInputNameByIndex(fakeDesc, 0))
        .WillOnce(Return(ACL_DYNAMIC_TENSOR_NAME));
    
    // 设置输入大小
    size_t inputSize = 1024;
    EXPECT_CALL(*mockAcl, aclmdlGetInputSizeByIndex(fakeDesc, 0))
        .WillOnce(Return(inputSize));
    
    // 设置模拟行为
    void* fakeBuffer = reinterpret_cast<void*>(0x1000);
    EXPECT_CALL(*mockAcl, aclrtMalloc(_, inputSize, ACL_MEM_MALLOC_HUGE_FIRST))
        .WillOnce(DoAll(SetArgPointee<0>(fakeBuffer), Return(ACL_SUCCESS)));
    
    // 不应调用memset（动态张量）
    EXPECT_CALL(*mockAcl, aclrtMemset(_, _, _, _)).Times(0);
    
    aclDataBuffer* fakeDataBuffer = reinterpret_cast<aclDataBuffer*>(0x2000);
    EXPECT_CALL(*mockAcl, aclCreateDataBuffer(fakeBuffer, inputSize))
        .WillOnce(Return(fakeDataBuffer));
    
    EXPECT_CALL(*mockAcl, aclmdlAddDatasetBuffer(_, fakeDataBuffer))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 执行测试
    SetDebugLogGuard guard;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CreateZeroInput();
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, SUCCESS);
}

TEST_F(ModelProcessTest, TestCreateZeroInput_CreateDatasetFailed)
{
    // 设置输入数据集为空
    SetupModelProcessInput(0);
    
    // 设置创建数据集失败
    EXPECT_CALL(*mockAcl, aclmdlCreateDataset())
        .WillOnce(Return(nullptr));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CreateZeroInput();
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("can't create dataset") != string::npos);
}

TEST_F(ModelProcessTest, TestCreateZeroInput_GetInputNameFailed)
{
    // 创建模型描述
    auto fakeDesc = CreateModelDescSuccess();
    
    // 设置创建数据集
    aclmdlDataset* fakeDataset = reinterpret_cast<aclmdlDataset*>(0xABCD);
    EXPECT_CALL(*mockAcl, aclmdlCreateDataset())
        .WillRepeatedly(Return(fakeDataset));
    
    // 设置输入数量为1
    size_t numInputs = 1;
    EXPECT_CALL(*mockAcl, aclmdlGetNumInputs(fakeDesc))
        .WillRepeatedly(Return(numInputs));
    
    // 设置获取输入名称失败
    EXPECT_CALL(*mockAcl, aclmdlGetInputNameByIndex(fakeDesc, 0))
        .WillOnce(Return(nullptr));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CreateZeroInput();
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("get input name failed") != string::npos);
}

TEST_F(ModelProcessTest, TestCreateZeroInput_AclrtMallocFailed)
{
    // 创建模型描述
    auto fakeDesc = CreateModelDescSuccess();
    
    // 设置创建数据集
    aclmdlDataset* fakeDataset = reinterpret_cast<aclmdlDataset*>(0xABCD);
    EXPECT_CALL(*mockAcl, aclmdlCreateDataset())
        .WillRepeatedly(Return(fakeDataset));
    
    // 设置输入数量为1
    size_t numInputs = 1;
    EXPECT_CALL(*mockAcl, aclmdlGetNumInputs(fakeDesc))
        .WillRepeatedly(Return(numInputs));
    
    // 设置输入名称
    EXPECT_CALL(*mockAcl, aclmdlGetInputNameByIndex(fakeDesc, 0))
        .WillOnce(Return("input1"));
    
    // 设置分配内存失败
    size_t inputSize = 1024;
    EXPECT_CALL(*mockAcl, aclmdlGetInputSizeByIndex(fakeDesc, 0))
        .WillOnce(Return(inputSize));
    
    EXPECT_CALL(*mockAcl, aclrtMalloc(_, inputSize, ACL_MEM_MALLOC_HUGE_FIRST))
        .WillOnce(Return(ACL_ERROR_BAD_ALLOC));
    
    const char* errorMsg = "Memory allocation failed";
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillOnce(Return(errorMsg));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CreateZeroInput();
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("malloc device buffer failed") != string::npos);
    EXPECT_TRUE(logOutput.find(errorMsg) != string::npos);
}

TEST_F(ModelProcessTest, TestCreateZeroInput_AclrtMemsetFailed)
{
    // 创建模型描述
    auto fakeDesc = CreateModelDescSuccess();

    // 设置创建数据集
    aclmdlDataset* fakeDataset = reinterpret_cast<aclmdlDataset*>(0xABCD);
    EXPECT_CALL(*mockAcl, aclmdlCreateDataset())
        .WillRepeatedly(Return(fakeDataset));
    
    // 设置输入数量为1
    size_t numInputs = 1;
    EXPECT_CALL(*mockAcl, aclmdlGetNumInputs(fakeDesc))
        .WillRepeatedly(Return(numInputs));
    
    // 设置输入名称（非动态张量）
    EXPECT_CALL(*mockAcl, aclmdlGetInputNameByIndex(fakeDesc, 0))
        .WillOnce(Return("input1"));
    
    // 设置输入大小
    size_t inputSize = 1024;
    EXPECT_CALL(*mockAcl, aclmdlGetInputSizeByIndex(fakeDesc, 0))
        .WillOnce(Return(inputSize));
    
    // 分配内存成功
    void* fakeBuffer = reinterpret_cast<void*>(0x1000);
    EXPECT_CALL(*mockAcl, aclrtMalloc(_, inputSize, ACL_MEM_MALLOC_HUGE_FIRST))
        .WillOnce(DoAll(SetArgPointee<0>(fakeBuffer), Return(ACL_SUCCESS)));
    
    // 设置内存清零失败
    EXPECT_CALL(*mockAcl, aclrtMemset(fakeBuffer, inputSize, 0, inputSize))
        .WillOnce(Return(ACL_ERROR_BAD_ALLOC));
    
    const char* errorMsg = "Memory set failed";
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillOnce(Return(errorMsg));
    
    // 预期资源释放
    EXPECT_CALL(*mockAcl, aclrtFree(fakeBuffer))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CreateZeroInput();
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("memory set failed") != string::npos);
    EXPECT_TRUE(logOutput.find(errorMsg) != string::npos);
}

TEST_F(ModelProcessTest, TestCreateZeroInput_CreateDataBufferFailed)
{
    // 创建模型描述
    auto fakeDesc = CreateModelDescSuccess();

    // 设置创建数据集
    aclmdlDataset* fakeDataset = reinterpret_cast<aclmdlDataset*>(0xABCD);
    EXPECT_CALL(*mockAcl, aclmdlCreateDataset())
        .WillRepeatedly(Return(fakeDataset));
    
    // 设置输入数量为1
    size_t numInputs = 1;
    EXPECT_CALL(*mockAcl, aclmdlGetNumInputs(fakeDesc))
        .WillRepeatedly(Return(numInputs));
    
    // 设置输入名称
    EXPECT_CALL(*mockAcl, aclmdlGetInputNameByIndex(fakeDesc, 0))
        .WillOnce(Return("input1"));
    
    // 设置输入大小
    size_t inputSize = 1024;
    EXPECT_CALL(*mockAcl, aclmdlGetInputSizeByIndex(fakeDesc, 0))
        .WillOnce(Return(inputSize));
    
    // 分配内存成功
    void* fakeBuffer = reinterpret_cast<void*>(0x1000);
    EXPECT_CALL(*mockAcl, aclrtMalloc(_, inputSize, ACL_MEM_MALLOC_HUGE_FIRST))
        .WillOnce(DoAll(SetArgPointee<0>(fakeBuffer), Return(ACL_SUCCESS)));
    
    // 设置内存清零成功
    EXPECT_CALL(*mockAcl, aclrtMemset(fakeBuffer, inputSize, 0, inputSize))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 创建数据缓冲区失败
    EXPECT_CALL(*mockAcl, aclCreateDataBuffer(fakeBuffer, inputSize))
        .WillOnce(Return(nullptr));
    
    // 预期资源释放
    EXPECT_CALL(*mockAcl, aclrtFree(fakeBuffer))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CreateZeroInput();
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("can't create data buffer") != string::npos);
}

TEST_F(ModelProcessTest, TestCreateZeroInput_AddDataBufferFailed)
{
    // 创建模型描述
    auto fakeDesc = CreateModelDescSuccess();

    // 设置创建数据集
    aclmdlDataset* fakeDataset = reinterpret_cast<aclmdlDataset*>(0xABCD);
    EXPECT_CALL(*mockAcl, aclmdlCreateDataset())
        .WillRepeatedly(Return(fakeDataset));
    
    // 设置输入数量为1
    size_t numInputs = 1;
    EXPECT_CALL(*mockAcl, aclmdlGetNumInputs(fakeDesc))
        .WillRepeatedly(Return(numInputs));
    
    // 设置输入名称
    EXPECT_CALL(*mockAcl, aclmdlGetInputNameByIndex(fakeDesc, 0))
        .WillOnce(Return("input1"));
    
    // 设置输入大小
    size_t inputSize = 1024;
    EXPECT_CALL(*mockAcl, aclmdlGetInputSizeByIndex(fakeDesc, 0))
        .WillOnce(Return(inputSize));
    
    // 分配内存成功
    void* fakeBuffer = reinterpret_cast<void*>(0x1000);
    EXPECT_CALL(*mockAcl, aclrtMalloc(_, inputSize, ACL_MEM_MALLOC_HUGE_FIRST))
        .WillOnce(DoAll(SetArgPointee<0>(fakeBuffer), Return(ACL_SUCCESS)));
    
    // 设置内存清零成功
    EXPECT_CALL(*mockAcl, aclrtMemset(fakeBuffer, inputSize, 0, inputSize))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 创建数据缓冲区成功
    aclDataBuffer* fakeDataBuffer = reinterpret_cast<aclDataBuffer*>(0x2000);
    EXPECT_CALL(*mockAcl, aclCreateDataBuffer(fakeBuffer, inputSize))
        .WillOnce(Return(fakeDataBuffer));
    
    // 加入数据缓冲区失败
    EXPECT_CALL(*mockAcl, aclmdlAddDatasetBuffer(_, fakeDataBuffer))
        .WillOnce(Return(ACL_ERROR_FAILURE));
    
    // 预期资源释放
    EXPECT_CALL(*mockAcl, aclrtFree(fakeBuffer))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 执行测试
    SetDebugLogGuard guard;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CreateZeroInput();
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("add input_ at CreateZeroInput +1") != string::npos);
}

// ===================== DestroyInput 测试用例 =====================

TEST_F(ModelProcessTest, TestDestroyInput_FreeMemory)
{
    // 创建输入数据集
    SetupModelProcessInput(0x1111);
    
    // 设置模拟行为
    size_t bufferCount = 3;
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetNumBuffers(reinterpret_cast<aclmdlDataset*>(0x1111)))
        .WillOnce(Return(bufferCount));
    
    // 设置缓冲区
    vector<aclDataBuffer*> dataBuffers = {
        reinterpret_cast<aclDataBuffer*>(0x1000),
        reinterpret_cast<aclDataBuffer*>(0x2000),
        reinterpret_cast<aclDataBuffer*>(0x3000)
    };
    
    vector<void*> memoryBuffers = {
        reinterpret_cast<void*>(0x4000),
        reinterpret_cast<void*>(0x5000),
        reinterpret_cast<void*>(0x6000)
    };
    
    for (size_t i = 0; i < bufferCount; i++) {
        EXPECT_CALL(*mockAcl, aclmdlGetDatasetBuffer(reinterpret_cast<aclmdlDataset*>(0x1111), i))
            .WillOnce(Return(dataBuffers[i]));
        
        EXPECT_CALL(*mockAcl, aclGetDataBufferAddr(dataBuffers[i]))
            .WillOnce(Return(memoryBuffers[i]));
        
        // 期望释放内存
        EXPECT_CALL(*mockAcl, aclrtFree(memoryBuffers[i]))
            .WillOnce(Return(ACL_SUCCESS));
        
        EXPECT_CALL(*mockAcl, aclDestroyDataBuffer(dataBuffers[i]))
            .WillOnce(Return(ACL_SUCCESS));
    }
    
    // 期望销毁数据集
    EXPECT_CALL(*mockAcl, aclmdlDestroyDataset(reinterpret_cast<aclmdlDataset*>(0x1111)))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 执行测试
    SetDebugLogGuard guard;
    testing::internal::CaptureStdout();
    modelProcess->DestroyInput(true); // 释放内存
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_TRUE(logOutput.find("destroy model input success") != string::npos);
}

TEST_F(ModelProcessTest, TestDestroyInput_DoNotFreeMemory)
{
    // 创建输入数据集
    SetupModelProcessInput(0x1111);
    
    // 设置模拟行为
    size_t bufferCount = 2;
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetNumBuffers(reinterpret_cast<aclmdlDataset*>(0x1111)))
        .WillOnce(Return(bufferCount));
    
    // 设置缓冲区
    vector<aclDataBuffer*> dataBuffers = {
        reinterpret_cast<aclDataBuffer*>(0x1000),
        reinterpret_cast<aclDataBuffer*>(0x2000)
    };
    
    vector<void*> memoryBuffers = {
        reinterpret_cast<void*>(0x4000),
        reinterpret_cast<void*>(0x5000)
    };
    
    for (size_t i = 0; i < bufferCount; i++) {
        EXPECT_CALL(*mockAcl, aclmdlGetDatasetBuffer(reinterpret_cast<aclmdlDataset*>(0x1111), i))
            .WillOnce(Return(dataBuffers[i]));
        
        EXPECT_CALL(*mockAcl, aclGetDataBufferAddr(dataBuffers[i]))
            .WillOnce(Return(memoryBuffers[i]));
        
        // 不应释放内存
        EXPECT_CALL(*mockAcl, aclrtFree(_)).Times(0);
        
        EXPECT_CALL(*mockAcl, aclDestroyDataBuffer(dataBuffers[i]))
            .WillOnce(Return(ACL_SUCCESS));
    }
    
    // 期望销毁数据集
    EXPECT_CALL(*mockAcl, aclmdlDestroyDataset(reinterpret_cast<aclmdlDataset*>(0x1111)))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 执行测试
    SetDebugLogGuard guard;
    testing::internal::CaptureStdout();
    modelProcess->DestroyInput(false); // 不释放内存
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_TRUE(logOutput.find("destroy model input success") != string::npos);
}

TEST_F(ModelProcessTest, TestDestroyInput_GetBufferFailed)
{
    // 创建输入数据集
    SetupModelProcessInput(0x1111);
    
    // 设置模拟行为
    size_t bufferCount = 3;
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetNumBuffers(reinterpret_cast<aclmdlDataset*>(0x1111)))
        .WillOnce(Return(bufferCount));
    
    // 设置不同响应
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetBuffer(reinterpret_cast<aclmdlDataset*>(0x1111), 0))
        .WillOnce(Return(nullptr)); // 获取缓冲区失败
    
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetBuffer(reinterpret_cast<aclmdlDataset*>(0x1111), 1))
        .WillOnce(Return(reinterpret_cast<aclDataBuffer*>(0x2000)));
    
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetBuffer(reinterpret_cast<aclmdlDataset*>(0x1111), 2))
        .WillOnce(Return(nullptr)); // 另一个获取失败
    
    // 设置获取内存地址成功
    void* memoryBuffer = reinterpret_cast<void*>(0x5000);
    EXPECT_CALL(*mockAcl, aclGetDataBufferAddr(reinterpret_cast<aclDataBuffer*>(0x2000)))
        .WillOnce(Return(memoryBuffer));
    
    // 期望释放内存
    EXPECT_CALL(*mockAcl, aclrtFree(memoryBuffer))
        .WillOnce(Return(ACL_SUCCESS));
    
    EXPECT_CALL(*mockAcl, aclDestroyDataBuffer(reinterpret_cast<aclDataBuffer*>(0x2000)))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 期望销毁数据集
    EXPECT_CALL(*mockAcl, aclmdlDestroyDataset(reinterpret_cast<aclmdlDataset*>(0x1111)))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 执行测试
    SetDebugLogGuard guard;
    testing::internal::CaptureStdout();
    modelProcess->DestroyInput(true);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_TRUE(logOutput.find("destroy model input success") != string::npos);
}

TEST_F(ModelProcessTest, TestDestroyInput_NullDataAddress)
{
    // 创建输入数据集
    SetupModelProcessInput(0x1111);
    
    // 设置模拟行为
    size_t bufferCount = 1;
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetNumBuffers(reinterpret_cast<aclmdlDataset*>(0x1111)))
        .WillOnce(Return(bufferCount));
    
    // 设置获取数据缓冲区
    aclDataBuffer* fakeDataBuffer = reinterpret_cast<aclDataBuffer*>(0x1000);
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetBuffer(reinterpret_cast<aclmdlDataset*>(0x1111), 0))
        .WillOnce(Return(fakeDataBuffer));
    
    // 设置获取内存地址失败（返回空指针）
    EXPECT_CALL(*mockAcl, aclGetDataBufferAddr(fakeDataBuffer))
        .WillOnce(Return(nullptr));
    
    // 只销毁数据缓冲区，不释放内存
    EXPECT_CALL(*mockAcl, aclDestroyDataBuffer(fakeDataBuffer))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 不应释放内存
    EXPECT_CALL(*mockAcl, aclrtFree(_)).Times(0);
    
    // 期望销毁数据集
    EXPECT_CALL(*mockAcl, aclmdlDestroyDataset(reinterpret_cast<aclmdlDataset*>(0x1111)))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 执行测试
    SetDebugLogGuard guard;
    testing::internal::CaptureStdout();
    modelProcess->DestroyInput(true);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_TRUE(logOutput.find("destroy model input success") != string::npos);
}

// ===================== CreateOutput 测试用例 =====================

TEST_F(ModelProcessTest, TestCreateOutput_SuccessWithModelOutputSize)
{
    // 创建模型描述
    auto fakeDesc = CreateModelDescSuccess();
    
    // 设置创建、销毁数据集
    aclmdlDataset* fakeDataset = reinterpret_cast<aclmdlDataset*>(0xABCD);
    EXPECT_CALL(*mockAcl, aclmdlCreateDataset())
        .WillOnce(Return(fakeDataset));
    EXPECT_CALL(*mockAcl, aclmdlDestroyDataset(reinterpret_cast<const aclmdlDataset*>(fakeDataset)))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 设置输出数量为2
    size_t outputNum = 2;
    EXPECT_CALL(*mockAcl, aclmdlGetNumOutputs(fakeDesc))
        .WillRepeatedly(Return(outputNum));
    
    // 设置输出大小
    vector<size_t> outputSizes = {1024, 2048};
    for (size_t i = 0; i < outputNum; ++i) {
        EXPECT_CALL(*mockAcl, aclmdlGetOutputSizeByIndex(fakeDesc, i))
            .WillOnce(Return(outputSizes[i]));
    }
    
    // 设置模拟行为
    for (size_t i = 0; i < outputNum; ++i) {
        void* fakeBuffer = reinterpret_cast<void*>(0x1000 + i * 0x1000);
        EXPECT_CALL(*mockAcl, aclrtMalloc(_, outputSizes[i], ACL_MEM_MALLOC_HUGE_FIRST))
            .WillOnce(DoAll(SetArgPointee<0>(fakeBuffer), Return(ACL_SUCCESS)));
        
        aclDataBuffer* fakeDataBuffer = reinterpret_cast<aclDataBuffer*>(0x2000 + i * 0x1000);
        EXPECT_CALL(*mockAcl, aclCreateDataBuffer(fakeBuffer, outputSizes[i]))
            .WillOnce(Return(fakeDataBuffer));
        
        EXPECT_CALL(*mockAcl, aclmdlAddDatasetBuffer(_, fakeDataBuffer))
            .WillOnce(Return(ACL_SUCCESS));
    }
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CreateOutput();
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(logOutput.find("create model output success") != string::npos);
}

TEST_F(ModelProcessTest, TestCreateOutput_SuccessWithCustomOutputSize)
{
    // 创建模型描述
    auto fakeDesc = CreateModelDescSuccess();

    // 设置创建、销毁数据集
    aclmdlDataset* fakeDataset = reinterpret_cast<aclmdlDataset*>(0xABCD);
    EXPECT_CALL(*mockAcl, aclmdlCreateDataset())
        .WillOnce(Return(fakeDataset));
    EXPECT_CALL(*mockAcl, aclmdlDestroyDataset(reinterpret_cast<const aclmdlDataset*>(fakeDataset)))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 设置输出数量为2
    size_t outputNum = 2;
    EXPECT_CALL(*mockAcl, aclmdlGetNumOutputs(fakeDesc))
        .WillRepeatedly(Return(outputNum));
    
    // 设置模拟行为（使用自定义大小，而不是查询模型）
    for (size_t i = 0; i < outputNum; ++i) {
        void* fakeBuffer = reinterpret_cast<void*>(0x1000 + i * 0x1000);
        EXPECT_CALL(*mockAcl, aclrtMalloc(_, _, ACL_MEM_MALLOC_HUGE_FIRST))
            .WillRepeatedly(DoAll(SetArgPointee<0>(fakeBuffer), Return(ACL_SUCCESS)));
        
        aclDataBuffer* fakeDataBuffer = reinterpret_cast<aclDataBuffer*>(0x2000 + i * 0x1000);
        EXPECT_CALL(*mockAcl, aclCreateDataBuffer(fakeBuffer, _))
            .WillRepeatedly(Return(fakeDataBuffer));
        
        EXPECT_CALL(*mockAcl, aclmdlAddDatasetBuffer(_, fakeDataBuffer))
            .WillRepeatedly(Return(ACL_SUCCESS));
    }
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CreateOutput();
    string logOutput = testing::internal::GetCapturedStdout();

    // 验证结果
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(logOutput.find("create model output success") != string::npos);
}

TEST_F(ModelProcessTest, TestCreateOutput_NoModelDesc)
{
    // 设置模型描述为空
    modelProcess->modelDesc_ = nullptr;
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CreateOutput();
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("no model description") != string::npos);
}

TEST_F(ModelProcessTest, TestCreateOutput_CreateDatasetFailed)
{
    // 创建模型描述
    auto fakeDesc = CreateModelDescSuccess();
    
    // 设置输出数量
    EXPECT_CALL(*mockAcl, aclmdlGetNumOutputs(fakeDesc))
        .WillRepeatedly(Return(2));
    
    // 设置创建数据集失败
    EXPECT_CALL(*mockAcl, aclmdlCreateDataset())
        .WillOnce(Return(nullptr));
    
    modelProcess->output_ = nullptr;
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CreateOutput();
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("can't create dataset") != string::npos);
}

TEST_F(ModelProcessTest, TestCreateOutput_AclrtMallocFailed)
{
    // 创建模型描述
    auto fakeDesc = CreateModelDescSuccess();

    // 设置创建、销毁数据集
    aclmdlDataset* fakeDataset = reinterpret_cast<aclmdlDataset*>(0xABCD);
    EXPECT_CALL(*mockAcl, aclmdlCreateDataset())
        .WillOnce(Return(fakeDataset));
    EXPECT_CALL(*mockAcl, aclmdlDestroyDataset(reinterpret_cast<const aclmdlDataset*>(fakeDataset)))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 设置输出数量为1
    size_t outputNum = 1;
    EXPECT_CALL(*mockAcl, aclmdlGetNumOutputs(fakeDesc))
        .WillRepeatedly(Return(outputNum));
    
    // 设置输出大小
    size_t outputSize = 1024;
    EXPECT_CALL(*mockAcl, aclmdlGetOutputSizeByIndex(fakeDesc, 0))
        .WillOnce(Return(outputSize));
    
    // 设置内存分配失败
    EXPECT_CALL(*mockAcl, aclrtMalloc(_, outputSize, ACL_MEM_MALLOC_HUGE_FIRST))
        .WillOnce(Return(ACL_ERROR_BAD_ALLOC));
    
    const char* errorMsg = "Memory allocation failed";
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillOnce(Return(errorMsg));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CreateOutput();
    string logOutput = testing::internal::GetCapturedStdout();

    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("can't malloc buffer") != string::npos);
    EXPECT_TRUE(logOutput.find("size is 1024") != string::npos);
    EXPECT_TRUE(logOutput.find(errorMsg) != string::npos);
}

TEST_F(ModelProcessTest, TestCreateOutput_CreateDataBufferFailed)
{
    // 创建模型描述
    auto fakeDesc = CreateModelDescSuccess();

    // 设置创建、销毁数据集
    aclmdlDataset* fakeDataset = reinterpret_cast<aclmdlDataset*>(0xABCD);
    EXPECT_CALL(*mockAcl, aclmdlCreateDataset())
        .WillOnce(Return(fakeDataset));
    EXPECT_CALL(*mockAcl, aclmdlDestroyDataset(reinterpret_cast<const aclmdlDataset*>(fakeDataset)))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 设置输出数量为1
    size_t outputNum = 1;
    EXPECT_CALL(*mockAcl, aclmdlGetNumOutputs(fakeDesc))
        .WillRepeatedly(Return(outputNum));
    
    // 设置输出大小
    size_t outputSize = 1024;
    EXPECT_CALL(*mockAcl, aclmdlGetOutputSizeByIndex(fakeDesc, 0))
        .WillOnce(Return(outputSize));
    
    // 分配内存成功
    void* fakeBuffer = reinterpret_cast<void*>(0x1000);
    EXPECT_CALL(*mockAcl, aclrtMalloc(_, outputSize, ACL_MEM_MALLOC_HUGE_FIRST))
        .WillOnce(DoAll(SetArgPointee<0>(fakeBuffer), Return(ACL_SUCCESS)));
    
    // 创建数据缓冲区失败
    EXPECT_CALL(*mockAcl, aclCreateDataBuffer(fakeBuffer, outputSize))
        .WillOnce(Return(nullptr));
    
    const char* errorMsg = "Data buffer creation failed";
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillOnce(Return(errorMsg));
    
    // 预期释放内存
    EXPECT_CALL(*mockAcl, aclrtFree(fakeBuffer))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CreateOutput();
    string logOutput = testing::internal::GetCapturedStdout();

    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("can't create data buffer") != string::npos);
    EXPECT_TRUE(logOutput.find(errorMsg) != string::npos);
}

TEST_F(ModelProcessTest, TestCreateOutput_AddDatasetBufferFailed)
{
    // 创建模型描述
    auto fakeDesc = CreateModelDescSuccess();

    // 设置创建、销毁数据集
    aclmdlDataset* fakeDataset = reinterpret_cast<aclmdlDataset*>(0xABCD);
    EXPECT_CALL(*mockAcl, aclmdlCreateDataset())
        .WillOnce(Return(fakeDataset));
    EXPECT_CALL(*mockAcl, aclmdlDestroyDataset(reinterpret_cast<const aclmdlDataset*>(fakeDataset)))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 设置输出数量为1
    size_t outputNum = 1;
    EXPECT_CALL(*mockAcl, aclmdlGetNumOutputs(fakeDesc))
        .WillRepeatedly(Return(outputNum));
    
    // 设置输出大小
    size_t outputSize = 1024;
    EXPECT_CALL(*mockAcl, aclmdlGetOutputSizeByIndex(fakeDesc, 0))
        .WillOnce(Return(outputSize));
    
    // 分配内存成功
    void* fakeBuffer = reinterpret_cast<void*>(0x1000);
    EXPECT_CALL(*mockAcl, aclrtMalloc(_, outputSize, ACL_MEM_MALLOC_HUGE_FIRST))
        .WillOnce(DoAll(SetArgPointee<0>(fakeBuffer), Return(ACL_SUCCESS)));
    
    // 创建数据缓冲区成功
    aclDataBuffer* fakeDataBuffer = reinterpret_cast<aclDataBuffer*>(0x2000);
    EXPECT_CALL(*mockAcl, aclCreateDataBuffer(fakeBuffer, outputSize))
        .WillOnce(Return(fakeDataBuffer));
    
    // 添加缓冲区到数据集失败
    EXPECT_CALL(*mockAcl, aclmdlAddDatasetBuffer(_, fakeDataBuffer))
        .WillOnce(Return(ACL_ERROR_BAD_ALLOC));
    
    const char* errorMsg = "Add dataset buffer failed";
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillOnce(Return(errorMsg));
    
    // 预期释放内存和数据缓冲区
    EXPECT_CALL(*mockAcl, aclrtFree(fakeBuffer))
        .WillOnce(Return(ACL_SUCCESS));
    EXPECT_CALL(*mockAcl, aclDestroyDataBuffer(fakeDataBuffer))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CreateOutput();
    string logOutput = testing::internal::GetCapturedStdout();

    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("can't add data buffer, create output failed") != string::npos);
    EXPECT_TRUE(logOutput.find(errorMsg) != string::npos);
}

// ===================== Free_Host_Try 测试用例 =====================
TEST_F(ModelProcessTest, TestFreeHostTry_SuccessOnDevice)
{
    // 准备测试数据
    void* hostData = reinterpret_cast<void*>(0x1234);
    
    // 不应调用释放函数
    EXPECT_CALL(*mockAcl, aclrtFreeHost(_)).Times(0);

    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->Free_Host_Try(ACL_SUCCESS, hostData);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(logOutput.empty());
}

// ===================== SetExceptionCallBack 测试用例 =====================

TEST_F(ModelProcessTest, TestSetExceptionCallBack_Success)
{
    EXPECT_CALL(*mockAcl, aclrtSetExceptionInfoCallback(_))
        .WillOnce(Return(ACL_SUCCESS));
    
    modelProcess->SetExceptionCallBack();
}

// ===================== InitReuseOutput 测试用例 =====================

TEST_F(ModelProcessTest, TestInitReuseOutput_SetsToFalse)
{
    // 初始化为 true
    modelProcess->reuseOutput_ = true;
    
    // 执行函数
    modelProcess->InitReuseOutput();
    
    // 验证结果
    EXPECT_FALSE(modelProcess->reuseOutput_);
}

// ===================== Execute 测试用例 =====================

TEST_F(ModelProcessTest, TestExecute_Success)
{
    // 加载模型
    LoadModelSuccess();
    modelProcess->modelId_ = expectedModelId;
    
    // 设置输入输出数据集
    SetupModelProcessInput(0x1111);
    SetupModelProcessOutput(0x2222);
    
    // 模拟执行成功
    EXPECT_CALL(*mockAcl, aclmdlExecute(expectedModelId, 
                reinterpret_cast<aclmdlDataset*>(0x1111), 
                reinterpret_cast<aclmdlDataset*>(0x2222)))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 执行测试
    SetDebugLogGuard guard;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->Execute();
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(logOutput.find("model execute success") != string::npos);
}

TEST_F(ModelProcessTest, TestExecute_Failed)
{
    // 加载模型
    LoadModelSuccess();
    modelProcess->modelId_ = expectedModelId;
    
    // 设置输入输出数据集
    SetupModelProcessInput(0x1111);
    SetupModelProcessOutput(0x2222);
    
    // 模拟执行失败
    aclError executeError = ACL_ERROR_BAD_ALLOC;
    const char* errorMsg = "Execution failed";
    EXPECT_CALL(*mockAcl, aclmdlExecute(expectedModelId, 
                reinterpret_cast<aclmdlDataset*>(0x1111), 
                reinterpret_cast<aclmdlDataset*>(0x2222)))
        .WillOnce(Return(executeError));
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillOnce(Return(errorMsg));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->Execute();
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("execute model failed") != string::npos);
    EXPECT_TRUE(logOutput.find("modelId is " + std::to_string(expectedModelId)) != string::npos);
    EXPECT_TRUE(logOutput.find(errorMsg) != string::npos);
}

// ===================== Unload 测试用例 =====================

TEST_F(ModelProcessTest, TestUnload_NotLoaded)
{
    // 设置模型未加载
    modelProcess->loadFlag_ = false;
    
    // 执行测试
    testing::internal::CaptureStdout();
    modelProcess->Unload();
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_TRUE(logOutput.find("no model had been loaded") != string::npos);
}

TEST_F(ModelProcessTest, TestUnload_SuccessWithModelDesc)
{
    // 加载模型
    LoadModelSuccess();
    modelProcess->modelId_ = expectedModelId;
    modelProcess->loadFlag_ = true;
    
    // 设置模型描述
    modelProcess->modelDesc_ = reinterpret_cast<aclmdlDesc*>(0x1234);
    
    // 设置模拟期望
    EXPECT_CALL(*mockAcl, aclmdlUnload(expectedModelId))
        .WillOnce(Return(ACL_SUCCESS));
    EXPECT_CALL(*mockAcl, aclmdlDestroyDesc(reinterpret_cast<aclmdlDesc*>(0x1234)))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 执行测试
    testing::internal::CaptureStdout();
    modelProcess->Unload();
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_FALSE(modelProcess->loadFlag_);
    EXPECT_EQ(modelProcess->modelDesc_, nullptr);
    EXPECT_TRUE(logOutput.find("unload model success") != string::npos);
    EXPECT_TRUE(logOutput.find("model Id is " + std::to_string(expectedModelId)) != string::npos);
}

TEST_F(ModelProcessTest, TestUnload_SuccessWithoutModelDesc)
{
    // 加载模型
    LoadModelSuccess();
    modelProcess->modelId_ = expectedModelId;
    modelProcess->loadFlag_ = true;
    
    // 设置模型描述为空
    modelProcess->modelDesc_ = nullptr;
    
    // 设置模拟期望
    EXPECT_CALL(*mockAcl, aclmdlUnload(expectedModelId))
        .WillOnce(Return(ACL_SUCCESS));
    // 不应调用销毁描述
    EXPECT_CALL(*mockAcl, aclmdlDestroyDesc(_)).Times(0);
    
    // 执行测试
    testing::internal::CaptureStdout();
    modelProcess->Unload();
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_FALSE(modelProcess->loadFlag_);
    EXPECT_EQ(modelProcess->modelDesc_, nullptr);
    EXPECT_TRUE(logOutput.find("unload model success") != string::npos);
}

TEST_F(ModelProcessTest, TestUnload_UnloadFailedButModelDescDestroyed)
{
    // 加载模型
    LoadModelSuccess();
    modelProcess->modelId_ = expectedModelId;
    modelProcess->loadFlag_ = true;
    modelProcess->modelDesc_ = reinterpret_cast<aclmdlDesc*>(0x1234);
    
    // 设置模拟期望
    aclError unloadError = ACL_ERROR_BAD_ALLOC;
    const char* errorMsg = "Unload failed";
    EXPECT_CALL(*mockAcl, aclmdlUnload(expectedModelId))
        .WillOnce(Return(unloadError));
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillOnce(Return(errorMsg));
    // 即使卸载失败，仍然销毁模型描述
    EXPECT_CALL(*mockAcl, aclmdlDestroyDesc(reinterpret_cast<aclmdlDesc*>(0x1234)))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 执行测试
    testing::internal::CaptureStdout();
    modelProcess->Unload();
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_FALSE(modelProcess->loadFlag_);
    EXPECT_EQ(modelProcess->modelDesc_, nullptr);
    EXPECT_TRUE(logOutput.find("unload model failed") != string::npos);
    EXPECT_TRUE(logOutput.find("modelId is " + std::to_string(expectedModelId)) != string::npos);
    EXPECT_TRUE(logOutput.find(errorMsg) != string::npos);
    EXPECT_TRUE(logOutput.find("unload model success") != string::npos); // 代码中会打印成功
}

TEST_F(ModelProcessTest, TestUnload_DescDestroyFailed)
{
    // 加载模型
    LoadModelSuccess();
    modelProcess->modelId_ = expectedModelId;
    modelProcess->loadFlag_ = true;
    modelProcess->modelDesc_ = reinterpret_cast<aclmdlDesc*>(0x1234);
    
    // 设置模拟期望
    EXPECT_CALL(*mockAcl, aclmdlUnload(expectedModelId))
        .WillOnce(Return(ACL_SUCCESS));
    // 模型描述销毁失败
    EXPECT_CALL(*mockAcl, aclmdlDestroyDesc(modelProcess->modelDesc_))
        .WillOnce(Return(ACL_ERROR_BAD_ALLOC));
    
    // 执行测试
    testing::internal::CaptureStdout();
    modelProcess->Unload();
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_FALSE(modelProcess->loadFlag_);
    EXPECT_EQ(modelProcess->modelDesc_, nullptr);
    EXPECT_TRUE(logOutput.find("unload model success") != string::npos);
    // 尽管有错误，但代码中没有处理销毁失败的错误日志
}

// ===================== GetCurOutputShape 测试用例 =====================

TEST_F(ModelProcessTest, TestGetCurOutputShape_DynamicShapeSuccess)
{
    // 准备输出数据集
    SetupModelProcessOutput(0x2222);
    modelProcess->output_ = reinterpret_cast<aclmdlDataset*>(0x2222);
    
    size_t index = 0;
    bool is_dymshape = true;
    std::vector<int64_t> shape;
    
    // 设置模拟行为
    aclTensorDesc* fakeTensorDesc = reinterpret_cast<aclTensorDesc*>(0x3000);
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetTensorDesc(reinterpret_cast<aclmdlDataset*>(0x2222), index))
        .WillOnce(Return(fakeTensorDesc));
    
    size_t dimNums = 3;
    EXPECT_CALL(*mockAcl, aclGetTensorDescNumDims(fakeTensorDesc))
        .WillOnce(Return(dimNums));
    
    // 设置维度值
    std::vector<int64_t> dimValues = {16, 32, 64};
    for (size_t i = 0; i < dimNums; ++i) {
        EXPECT_CALL(*mockAcl, aclGetTensorDescDimV2(fakeTensorDesc, i, _))
            .WillOnce(DoAll(SetArgPointee<2>(dimValues[i]), Return(ACL_SUCCESS)));
    }
    
    // 执行测试
    Result ret = modelProcess->GetCurOutputShape(index, is_dymshape, shape);
    
    // 验证结果
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(shape.size(), dimNums);
    for (size_t i = 0; i < dimNums; ++i) {
        EXPECT_EQ(shape[i], dimValues[i]);
    }
}

TEST_F(ModelProcessTest, TestGetCurOutputShape_DynamicShapeUnknownRank)
{
    // 准备输出数据集
    SetupModelProcessOutput(0x2222);
    size_t index = 0;
    bool is_dymshape = true;
    std::vector<int64_t> shape;
    
    // 设置模拟行为
    aclTensorDesc* fakeTensorDesc = reinterpret_cast<aclTensorDesc*>(0x3000);
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetTensorDesc(reinterpret_cast<aclmdlDataset*>(0x2222), index))
        .WillOnce(Return(fakeTensorDesc));
    
    EXPECT_CALL(*mockAcl, aclGetTensorDescNumDims(fakeTensorDesc))
        .WillOnce(Return(ACL_UNKNOWN_RANK));
    
    // 执行测试
    Result ret = modelProcess->GetCurOutputShape(index, is_dymshape, shape);
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
}

TEST_F(ModelProcessTest, TestGetCurOutputShape_StaticShapeSuccess)
{
    // 准备模型描述
    auto fakeDesc = CreateModelDescSuccess();
    
    size_t index = 0;
    bool is_dymshape = false;
    std::vector<int64_t> shape;
    
    // 设置模拟行为
    aclmdlIODims ioDims;
    ioDims.dimCount = 3;
    ioDims.dims[0] = 16;
    ioDims.dims[1] = 32;
    ioDims.dims[2] = 64;
    
    EXPECT_CALL(*mockAcl, aclmdlGetCurOutputDims(fakeDesc, index, _))
        .WillOnce(DoAll(SetArgPointee<2>(ioDims), Return(ACL_SUCCESS)));
    
    // 执行测试
    Result ret = modelProcess->GetCurOutputShape(index, is_dymshape, shape);
    
    // 验证结果
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(shape.size(), ioDims.dimCount);
    for (size_t i = 0; i < ioDims.dimCount; ++i) {
        EXPECT_EQ(shape[i], ioDims.dims[i]);
    }
}

TEST_F(ModelProcessTest, TestGetCurOutputShape_StaticShapeFailed)
{
    // 准备模型描述
    auto fakeDesc = CreateModelDescSuccess();
    
    size_t index = 0;
    bool is_dymshape = false;
    std::vector<int64_t> shape;
    
    // 设置模拟行为 - 获取维度失败
    EXPECT_CALL(*mockAcl, aclmdlGetCurOutputDims(fakeDesc, index, _))
        .WillOnce(Return(ACL_ERROR_BAD_ALLOC));
    
    // 执行测试
    SetDebugLogGuard guard;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->GetCurOutputShape(index, is_dymshape, shape);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("aclmdlGetCurOutputDims failed") != string::npos);
}

// ===================== GetNumInputs/GetNumOutputs 测试用例 =====================

TEST_F(ModelProcessTest, TestGetNumInputs_Success)
{
    // 准备模型描述
    auto fakeDesc = CreateModelDescSuccess();
    
    size_t expectedNum = 3;
    EXPECT_CALL(*mockAcl, aclmdlGetNumInputs(fakeDesc))
        .WillOnce(Return(expectedNum));
    
    // 执行测试
    size_t num = modelProcess->GetNumInputs();
    
    // 验证结果
    EXPECT_EQ(num, expectedNum);
}

TEST_F(ModelProcessTest, TestGetNumOutputs_Success)
{
    // 准备模型描述
    auto fakeDesc = CreateModelDescSuccess();
    
    size_t expectedNum = 2;
    EXPECT_CALL(*mockAcl, aclmdlGetNumOutputs(fakeDesc))
        .WillOnce(Return(expectedNum));
    
    // 执行测试
    size_t num = modelProcess->GetNumOutputs();
    
    // 验证结果
    EXPECT_EQ(num, expectedNum);
}

// ===================== GetInTensorDesc 测试用例 =====================

TEST_F(ModelProcessTest, TestGetInTensorDesc_Success)
{
    // 准备模型描述
    auto fakeDesc = CreateModelDescSuccess();
    size_t index = 0;
    
    // 设置模拟行为
    const char* inputName = "input0";
    aclDataType dataType = ACL_DT_UNDEFINED;
    aclFormat format = ACL_FORMAT_UNDEFINED;
    size_t size = 1024;
    
    aclmdlIODims dims;
    dims.dimCount = 3;
    dims.dims[0] = 1;
    dims.dims[1] = 3;
    dims.dims[2] = 224;
    
    EXPECT_CALL(*mockAcl, aclmdlGetInputNameByIndex(fakeDesc, index))
        .WillOnce(Return(inputName));
    EXPECT_CALL(*mockAcl, aclmdlGetInputDataType(fakeDesc, index))
        .WillOnce(Return(dataType));
    EXPECT_CALL(*mockAcl, aclmdlGetInputFormat(fakeDesc, index))
        .WillOnce(Return(format));
    EXPECT_CALL(*mockAcl, aclmdlGetInputSizeByIndex(fakeDesc, index))
        .WillOnce(Return(size));
    EXPECT_CALL(*mockAcl, aclmdlGetInputDims(fakeDesc, index, _))
        .WillOnce(DoAll(SetArgPointee<2>(dims), Return(ACL_SUCCESS)));
    
    // 准备输出变量
    std::string name;
    int outDataType;
    size_t outFormat;
    std::vector<int64_t> shape;
    size_t outSize;
    
    // 执行测试
    Result ret = modelProcess->GetInTensorDesc(index, name, outDataType, outFormat, shape, outSize);
    
    // 验证结果
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(name, inputName);
    EXPECT_EQ(outDataType, dataType);
    EXPECT_EQ(outFormat, format);
    EXPECT_EQ(outSize, size);
    ASSERT_EQ(shape.size(), dims.dimCount);
    for (size_t i = 0; i < dims.dimCount; ++i) {
        EXPECT_EQ(shape[i], dims.dims[i]);
    }
}

// ===================== GetOutTensorDesc 测试用例 =====================

TEST_F(ModelProcessTest, TestGetOutTensorDesc_Success)
{
    // 准备模型描述
    auto fakeDesc = CreateModelDescSuccess();
    size_t index = 0;
    
    // 设置模拟行为
    const char* outputName = "output7";
    aclDataType dataType = ACL_DT_UNDEFINED;
    aclFormat format = ACL_FORMAT_UNDEFINED;
    size_t size = 2048;
    
    aclmdlIODims dims;
    dims.dimCount = 2;
    dims.dims[0] = 1;
    dims.dims[1] = 1000;
    
    EXPECT_CALL(*mockAcl, aclmdlGetOutputNameByIndex(fakeDesc, index))
        .WillOnce(Return(outputName));
    EXPECT_CALL(*mockAcl, aclmdlGetOutputDataType(fakeDesc, index))
        .WillOnce(Return(dataType));
    EXPECT_CALL(*mockAcl, aclmdlGetOutputFormat(fakeDesc, index))
        .WillOnce(Return(format));
    EXPECT_CALL(*mockAcl, aclmdlGetOutputSizeByIndex(fakeDesc, index))
        .WillOnce(Return(size));
    EXPECT_CALL(*mockAcl, aclmdlGetOutputDims(fakeDesc, index, _))
        .WillOnce(DoAll(SetArgPointee<2>(dims), Return(ACL_SUCCESS)));
    
    // 准备输出变量
    std::string name;
    int outDataType;
    size_t outFormat;
    std::vector<int64_t> shape;
    size_t outSize;
    
    // 执行测试
    Result ret = modelProcess->GetOutTensorDesc(index, name, outDataType, outFormat, shape, outSize);
    
    // 验证结果
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(name, outputName);
    EXPECT_EQ(outDataType, dataType);
    EXPECT_EQ(outFormat, format);
    EXPECT_EQ(outSize, size);
    ASSERT_EQ(shape.size(), dims.dimCount);
    for (size_t i = 0; i < dims.dimCount; ++i) {
        EXPECT_EQ(shape[i], dims.dims[i]);
    }
}

// ===================== GetOutTensorLen 测试用例 =====================

TEST_F(ModelProcessTest, TestGetOutTensorLen_DynamicShapeSuccess)
{
    // 准备输出数据集
    SetupModelProcessOutput(0x2222);
    auto fakeDesc = CreateModelDescSuccess();

    size_t index = 0;
    bool is_dymshape = true;
    
    // 设置模拟行为
    aclDataBuffer* fakeDataBuffer = reinterpret_cast<aclDataBuffer*>(0x1000);
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetBuffer(modelProcess->output_, index))
        .WillOnce(Return(fakeDataBuffer));
    
    aclTensorDesc* fakeTensorDesc = reinterpret_cast<aclTensorDesc*>(0x3000);
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetTensorDesc(modelProcess->output_, index))
        .WillOnce(Return(fakeTensorDesc));
    
    aclmdlBatch batchInfo = {3, {1, 4, 8}};
    EXPECT_CALL(*mockAcl, aclmdlGetDynamicBatch(fakeDesc, _))
        .WillOnce(DoAll(SetArgPointee<1>(batchInfo), Return(ACL_SUCCESS)));

    size_t expectedSize = 99;
    EXPECT_CALL(*mockAcl, aclGetTensorDescSize(fakeTensorDesc))
        .WillOnce(Return(expectedSize));
    
    // 执行测试
    size_t size = modelProcess->GetOutTensorLen(index, is_dymshape);
    
    // 验证结果
    EXPECT_EQ(size, expectedSize);
}

TEST_F(ModelProcessTest, TestGetOutTensorLen_StaticShapeSuccess)
{
    // 准备输出数据集
    SetupModelProcessOutput(0x2222);
    auto fakeDesc = CreateModelDescSuccess();
    
    size_t index = 0;
    bool is_dymshape = false;
    
    // 设置模拟行为
    aclmdlBatch batchInfo = {3, {1, 4, 8}};
    EXPECT_CALL(*mockAcl, aclmdlGetDynamicBatch(fakeDesc, _))
        .WillOnce(DoAll(SetArgPointee<1>(batchInfo), Return(ACL_SUCCESS)));
    
    aclDataBuffer* fakeDataBuffer = reinterpret_cast<aclDataBuffer*>(0x1000);
    EXPECT_CALL(*mockAcl, aclmdlGetDatasetBuffer(modelProcess->output_, index))
        .WillOnce(Return(fakeDataBuffer));
    
    size_t expectedSize = 2048;
    EXPECT_CALL(*mockAcl, aclGetDataBufferSizeV2(fakeDataBuffer))
        .WillOnce(Return(expectedSize));
    
    // 执行测试
    size_t size = modelProcess->GetOutTensorLen(index, is_dymshape);
    
    // 验证结果
    EXPECT_EQ(size, expectedSize);
}

// ===================== CreateOutput 测试用例 =====================

TEST_F(ModelProcessTest, TestCreateOutput_WithParams_Success)
{
    // 准备测试数据
    void* outputBuffer = reinterpret_cast<void*>(0x1234);
    size_t bufferSize = 1024;
    
    // 设置初始状态 - 输出数据集为空
    SetupModelProcessOutput(0);
    
    // 设置模拟期望
    aclmdlDataset* fakeDataset = reinterpret_cast<aclmdlDataset*>(0xABCD);
    EXPECT_CALL(*mockAcl, aclmdlCreateDataset())
        .WillOnce(Return(fakeDataset));
    
    aclDataBuffer* fakeDataBuffer = reinterpret_cast<aclDataBuffer*>(0x1000);
    EXPECT_CALL(*mockAcl, aclCreateDataBuffer(outputBuffer, bufferSize))
        .WillOnce(Return(fakeDataBuffer));
    
    EXPECT_CALL(*mockAcl, aclmdlAddDatasetBuffer(fakeDataset, fakeDataBuffer))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 执行测试
    Result ret = modelProcess->CreateOutput(outputBuffer, bufferSize);
    
    // 验证结果
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(modelProcess->output_, fakeDataset);
}

TEST_F(ModelProcessTest, TestCreateOutput_WithParams_CreateDatasetFailed)
{
    // 准备测试数据
    void* outputBuffer = reinterpret_cast<void*>(0x1234);
    size_t bufferSize = 1024;
    
    // 设置初始状态 - 输出数据集为空
    SetupModelProcessOutput(0);
    
    // 设置模拟期望
    modelProcess->output_ = nullptr;
    EXPECT_CALL(*mockAcl, aclmdlCreateDataset())
        .WillOnce(Return(nullptr));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CreateOutput(outputBuffer, bufferSize);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("can't create dataset, create output failed") != string::npos);
}

TEST_F(ModelProcessTest, TestCreateOutput_WithParams_CreateBufferFailed)
{
    // 准备测试数据
    void* outputBuffer = reinterpret_cast<void*>(0x1234);
    size_t bufferSize = 1024;
    
    // 设置初始状态 - 输出数据集为空
    SetupModelProcessOutput(0);
    
    // 设置模拟期望
    aclmdlDataset* fakeDataset = reinterpret_cast<aclmdlDataset*>(0xABCD);
    EXPECT_CALL(*mockAcl, aclmdlCreateDataset())
        .WillOnce(Return(fakeDataset));
    
    // 创建数据缓冲区失败
    EXPECT_CALL(*mockAcl, aclCreateDataBuffer(outputBuffer, bufferSize))
        .WillOnce(Return(nullptr));
    
    // 预期释放内存
    EXPECT_CALL(*mockAcl, aclrtFree(outputBuffer))
        .WillOnce(Return(ACL_SUCCESS));
    
    const char* errorMsg = "Data buffer creation failed";
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillOnce(Return(errorMsg));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CreateOutput(outputBuffer, bufferSize);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("can't create data buffer, create output failed") != string::npos);
    EXPECT_TRUE(logOutput.find(errorMsg) != string::npos);
}

TEST_F(ModelProcessTest, TestCreateOutput_WithParams_AddBufferFailed)
{
    // 准备测试数据
    void* outputBuffer = reinterpret_cast<void*>(0x1234);
    size_t bufferSize = 1024;
    
    // 设置初始状态 - 输出数据集为空
    SetupModelProcessOutput(0);
    
    aclmdlDataset* fakeDataset = reinterpret_cast<aclmdlDataset*>(0xABCD);
    EXPECT_CALL(*mockAcl, aclmdlCreateDataset())
        .WillOnce(Return(fakeDataset));
    
    aclDataBuffer* fakeDataBuffer = reinterpret_cast<aclDataBuffer*>(0x1000);
    EXPECT_CALL(*mockAcl, aclCreateDataBuffer(outputBuffer, bufferSize))
        .WillOnce(Return(fakeDataBuffer));
    
    EXPECT_CALL(*mockAcl, aclmdlAddDatasetBuffer(fakeDataset, fakeDataBuffer))
        .WillOnce(Return(ACL_ERROR_FAILURE));
    
    const char* errorMsg = "Add data buffer failed";
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillOnce(Return(errorMsg));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->CreateOutput(outputBuffer, bufferSize);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("add input dataset buffer failed") != string::npos);
    EXPECT_TRUE(logOutput.find(errorMsg) != string::npos);
}

// ===================== FreeAIPP 测试用例 =====================

TEST_F(ModelProcessTest, TestFreeAIPP_Success)
{
    aclmdlAIPP* aippParmsSet = reinterpret_cast<aclmdlAIPP*>(0x5000);
    
    // 设置模拟期望
    EXPECT_CALL(*mockAcl, aclmdlDestroyAIPP(aippParmsSet))
        .WillOnce(Return(ACL_SUCCESS));
    
    // 执行测试
    Result ret = modelProcess->FreeAIPP(aippParmsSet);
    
    // 验证结果
    EXPECT_EQ(ret, SUCCESS);
}

TEST_F(ModelProcessTest, TestFreeAIPP_Failed)
{
    aclmdlAIPP* aippParmsSet = reinterpret_cast<aclmdlAIPP*>(0x5000);
    
    // 设置模拟期望
    aclError freeError = ACL_ERROR_BAD_ALLOC;
    const char* errorMsg = "Destroy AIPP failed";
    EXPECT_CALL(*mockAcl, aclmdlDestroyAIPP(aippParmsSet))
        .WillOnce(Return(freeError));
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillOnce(Return(errorMsg));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->FreeAIPP(aippParmsSet);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("free acl AIPP failed") != string::npos);
    EXPECT_TRUE(logOutput.find(errorMsg) != string::npos);
}

// ===================== CheckDymAIPPInputExist 测试用例 ===================== 617

TEST_F(ModelProcessTest, TestCheckDymAIPPInputExist_Success)
{
    // 准备模型描述
    auto fakeDesc = CreateModelDescSuccess();
    modelProcess->modelDesc_ = fakeDesc;
    
    // 设置模拟行为
    size_t numInputs = 2;
    EXPECT_CALL(*mockAcl, aclmdlGetNumInputs(fakeDesc))
        .WillOnce(Return(numInputs));
    
    // 第一个输入有动态AIPP
    EXPECT_CALL(*mockAcl, aclmdlGetAippType(expectedModelId, 0, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(ACL_DATA_WITH_DYNAMIC_AIPP), 
                        SetArgPointee<3>(0), 
                        Return(ACL_SUCCESS)));
    
    // 第二个输入无动态AIPP
    EXPECT_CALL(*mockAcl, aclmdlGetAippType(expectedModelId, 1, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(ACL_DATA_WITH_STATIC_AIPP), 
                        SetArgPointee<3>(0), 
                        Return(ACL_SUCCESS)));
    
    // 执行测试
    int ret = modelProcess->CheckDymAIPPInputExist();
    
    // 验证结果
    EXPECT_EQ(ret, 1);
}

TEST_F(ModelProcessTest, TestCheckDymAIPPInputExist_GetAippTypeFailed)
{
    // 准备模型描述
    auto fakeDesc = CreateModelDescSuccess();
    modelProcess->modelDesc_ = fakeDesc;
    
    // 设置模拟行为
    size_t numInputs = 1;
    EXPECT_CALL(*mockAcl, aclmdlGetNumInputs(fakeDesc))
        .WillOnce(Return(numInputs));
    
    // 模拟ACL函数失败
    EXPECT_CALL(*mockAcl, aclmdlGetAippType(expectedModelId, 0, _, _))
        .WillOnce(Return(ACL_ERROR_INVALID_PARAM));
    
    // 执行测试
    testing::internal::CaptureStdout();
    int ret = modelProcess->CheckDymAIPPInputExist();
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, -1);
    EXPECT_TRUE(logOutput.find("acl get AIPP type failed") != string::npos);
}

// ===================== GetAIPPIndexList 测试用例 =====================

TEST_F(ModelProcessTest, TestGetAIPPIndexList_Success)
{
    // 准备模型描述
    SetupCompleteModel(2, {"input1", ACL_DYNAMIC_AIPP_NAME});
    
    // 执行测试
    std::vector<size_t> indices;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->GetAIPPIndexList(indices);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, SUCCESS);
    ASSERT_EQ(indices.size(), 1);
    EXPECT_EQ(indices[0], 1);
    EXPECT_TRUE(logOutput.find("get AIPP index success") != string::npos);
}

TEST_F(ModelProcessTest, TestGetAIPPIndexList_GetNameFailed)
{
    // 准备模型描述
    SetupCompleteModel(1, {"input1"});
    
    // 设置模拟行为
    EXPECT_CALL(*mockAcl, aclmdlGetInputNameByIndex(modelProcess->modelDesc_, 0))
        .WillOnce(Return(nullptr));
    
    // 执行测试
    std::vector<size_t> indices;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->GetAIPPIndexList(indices);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("get input name by index failed") != string::npos);
}

TEST_F(ModelProcessTest, TestGetAIPPIndexList_NotFound)
{
    // 准备模型描述
    SetupCompleteModel(1, {"input1"});
    
    // 执行测试
    std::vector<size_t> indices;
    Result ret = modelProcess->GetAIPPIndexList(indices);
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(indices.empty());
}

// ===================== SetInputAIPP 测试用例 =====================

TEST_F(ModelProcessTest, TestSetInputAIPP_Success)
{
    // 准备模型
    SetupCompleteModel();
    
    // 创建模拟AIPP结构
    aclmdlAIPP* dummyAipp = reinterpret_cast<aclmdlAIPP*>(0x1234);
    
    // 设置模拟行为
    EXPECT_CALL(*mockAcl, aclmdlSetInputAIPP(expectedModelId, modelProcess->input_, 0, dummyAipp))
        .WillOnce(Return(ACL_ERROR_NONE));
    
    // 执行测试
    SetDebugLogGuard guard;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->SetInputAIPP(0, dummyAipp);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(logOutput.find("PREPARE aclmdlSetInputAIPP") != string::npos);
}

TEST_F(ModelProcessTest, TestSetInputAIPP_Failed)
{
    // 准备模型
    SetupCompleteModel();
    
    // 创建模拟AIPP结构
    aclmdlAIPP* dummyAipp = reinterpret_cast<aclmdlAIPP*>(0x1234);
    
    // 设置模拟行为
    EXPECT_CALL(*mockAcl, aclmdlSetInputAIPP(expectedModelId, modelProcess->input_, 0, dummyAipp))
        .WillOnce(Return(ACL_ERROR_BAD_ALLOC));
    
    // 执行测试
    testing::internal::CaptureStdout();
    Result ret = modelProcess->SetInputAIPP(0, dummyAipp);
    string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证结果
    EXPECT_EQ(ret, FAILED);
    EXPECT_TRUE(logOutput.find("acl set input AIPP failed") != string::npos);
}

// ===================== SetAIPPSrcImageSize 测试用例 =====================

TEST_F(ModelProcessTest, TestSetAIPPSrcImageSize_Success)
{
    // 创建模拟AIPP结构
    aclmdlAIPP* dummyAipp = reinterpret_cast<aclmdlAIPP*>(0x1234);
    
    // 创建配置对象
    auto config = std::make_shared<Base::DynamicAippConfig>();
    config->SetSrcImageSize({800, 600});
    
    // 设置模拟行为
    EXPECT_CALL(*mockAcl, aclmdlSetAIPPSrcImageSize(dummyAipp, 800, 600))
        .WillOnce(Return(ACL_ERROR_NONE));
    
    // 执行测试
    Result ret = modelProcess->SetAIPPSrcImageSize(config, dummyAipp);
    
    // 验证结果
    EXPECT_EQ(ret, SUCCESS);
}

TEST_F(ModelProcessTest, TestSetAIPPSrcImageSize_FailedWithException)
{
    // 创建模拟AIPP结构
    aclmdlAIPP* dummyAipp = reinterpret_cast<aclmdlAIPP*>(0x1234);
    
    // 创建配置对象
    auto config = std::make_shared<Base::DynamicAippConfig>();
    config->SetSrcImageSize({800, 600});
    
    // 设置模拟行为
    EXPECT_CALL(*mockAcl, aclmdlSetAIPPSrcImageSize(dummyAipp, 800, 600))
        .WillOnce(Return(ACL_ERROR_BAD_ALLOC));
    
    // 执行测试并验证异常
    testing::internal::CaptureStdout();
    EXPECT_THROW({
        modelProcess->SetAIPPSrcImageSize(config, dummyAipp);
    }, const char*);
    string logOutput = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(logOutput.find("acl set AIPP SrcImage size failed") != string::npos);
}

// ===================== SetAIPPInputFormat 测试用例 =====================

TEST_F(ModelProcessTest, TestSetAIPPInputFormat_Success)
{
    // 创建模拟AIPP结构
    aclmdlAIPP* dummyAipp = reinterpret_cast<aclmdlAIPP*>(0x1234);
    
    // 创建配置对象和映射
    auto config = std::make_shared<Base::DynamicAippConfig>();
    config->SetInputFormat("YUV420SP_U8");
    modelProcess->str2aclAippInputFormat["YUV420SP_U8"] = ACL_YUV420SP_U8;
    
    // 设置模拟行为
    EXPECT_CALL(*mockAcl, aclmdlSetAIPPInputFormat(dummyAipp, ACL_YUV420SP_U8))
        .WillOnce(Return(ACL_ERROR_NONE));
    
    // 执行测试
    Result ret = modelProcess->SetAIPPInputFormat(config, dummyAipp);
    
    // 验证结果
    EXPECT_EQ(ret, SUCCESS);
}

TEST_F(ModelProcessTest, TestSetAIPPInputFormat_FailedWithException)
{
    // 创建模拟AIPP结构
    aclmdlAIPP* dummyAipp = reinterpret_cast<aclmdlAIPP*>(0x1234);
    
    // 创建配置对象和映射
    auto config = std::make_shared<Base::DynamicAippConfig>();
    config->SetInputFormat("YUV420SP_U8");
    modelProcess->str2aclAippInputFormat["YUV420SP_U8"] = ACL_YUV420SP_U8;
    
    // 设置模拟行为
    EXPECT_CALL(*mockAcl, aclmdlSetAIPPInputFormat(dummyAipp, ACL_YUV420SP_U8))
        .WillOnce(Return(ACL_ERROR_BAD_ALLOC));
    
    // 执行测试并验证异常
    testing::internal::CaptureStdout();
    EXPECT_THROW({
        modelProcess->SetAIPPInputFormat(config, dummyAipp);
    }, const char*);
    string logOutput = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(logOutput.find("acl set AIPP input format failed") != string::npos);
}

// ===================== SetAIPPCscParams测试 =====================
// GMock 的 MOCK_METHOD 宏支持的参数数量是有限制的。MOCK_METHOD 宏最多支持10个参数的函数模拟。
// 而需要mock的 aclmdlSetAIPPCscParams 有17个参数，故略过

// ===================== SetAIPPRbuvSwapSwitch测试 =====================
TEST_F(ModelProcessTest, TestSetAIPPRbuvSwapSwitch_Success)
{
    auto dyAippCfg = std::make_shared<Base::DynamicAippConfig>();
    dyAippCfg->SetRbuvSwapSwitch(1);
    aclmdlAIPP* fakeAipp = reinterpret_cast<aclmdlAIPP*>(0x3333);
    
    EXPECT_CALL(*mockAcl, aclmdlSetAIPPRbuvSwapSwitch(fakeAipp, 1))
        .WillOnce(Return(ACL_SUCCESS));
    
    SetDebugLogGuard guard;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->SetAIPPRbuvSwapSwitch(dyAippCfg, fakeAipp);
    std::string logOutput = testing::internal::GetCapturedStdout();
    
    // 验证
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(logOutput.find("aclmdlSetAIPPRbuvSwapSwitch") != std::string::npos);
}

TEST_F(ModelProcessTest, TestSetAIPPRbuvSwitch_Failure)
{
    auto dyAippCfg = std::make_shared<Base::DynamicAippConfig>();
    aclmdlAIPP* fakeAipp = reinterpret_cast<aclmdlAIPP*>(0x4444);
    
    EXPECT_CALL(*mockAcl, aclmdlSetAIPPRbuvSwapSwitch(_, 0))
        .WillOnce(Return(ACL_ERROR_INTERNAL_ERROR));
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillOnce(Return("Mock error message"));
    
    testing::internal::CaptureStdout();
    EXPECT_THROW({
        try {
            modelProcess->SetAIPPRbuvSwapSwitch(dyAippCfg, fakeAipp);
        } catch (const char* e) {
            EXPECT_STREQ(e, "AippData set failed!");
            throw;
        }
    }, const char*);
    string logOutput = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(logOutput.find("acl set AIPP swap switch params failed") != std::string::npos);
}

// ===================== SetAIPPAxSwapSwitch测试 =====================
TEST_F(ModelProcessTest, TestSetAIPPAxSwapSwitch_Success)
{
    auto dyAippCfg = std::make_shared<Base::DynamicAippConfig>();
    dyAippCfg->SetAxSwapSwitch(1);
    aclmdlAIPP* fakeAipp = reinterpret_cast<aclmdlAIPP*>(0x5555);
    
    EXPECT_CALL(*mockAcl, aclmdlSetAIPPAxSwapSwitch(fakeAipp, 1))
        .WillOnce(Return(ACL_SUCCESS));
    
    SetDebugLogGuard guard;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->SetAIPPAxSwapSwitch(dyAippCfg, fakeAipp);
    std::string logOutput = testing::internal::GetCapturedStdout();

    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(logOutput.find("aclmdlSetAIPPAxSwapSwitch") != std::string::npos);
}

TEST_F(ModelProcessTest, TestSetAIPPAxSwapSwitch_Failure)
{
    auto dyAippCfg = std::make_shared<Base::DynamicAippConfig>();
    aclmdlAIPP* fakeAipp = reinterpret_cast<aclmdlAIPP*>(0x6666);
    
    EXPECT_CALL(*mockAcl, aclmdlSetAIPPAxSwapSwitch(_, 0))
        .WillOnce(Return(ACL_ERROR_INTERNAL_ERROR));
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillOnce(Return("Mock error message"));
    
    testing::internal::CaptureStdout();
    EXPECT_THROW({
        try {
            modelProcess->SetAIPPAxSwapSwitch(dyAippCfg, fakeAipp);
        } catch (const char* e) {
            EXPECT_STREQ(e, "AippData set failed!");
            throw;
        }
    }, const char*);
    string logOutput = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(logOutput.find("acl set AIPP Ax swap switch params failed") != std::string::npos);
}

// ===================== SetAIPPDtcPixelMean测试 =====================
TEST_F(ModelProcessTest, TestSetAIPPDtcPixelMean_CustomValue_Success)
{
    auto dyAippCfg = std::make_shared<Base::DynamicAippConfig>();
    aclmdlAIPP* fakeAipp = reinterpret_cast<aclmdlAIPP*>(0x7777);
    
    // 设置dtcPixelMean数据
    std::vector<int> pixelMean = {10, 20, 30, 40};
    dyAippCfg->SetMaxBatchSize(pixelMean.size());
    dyAippCfg->SetDtcPixelMean(pixelMean);
    
    // 设置期望行为（batchIndex=1 应使用第二个值）
    EXPECT_CALL(*mockAcl, aclmdlSetAIPPDtcPixelMean(fakeAipp, 10, 20, 30, 40, 1))
        .WillOnce(Return(ACL_SUCCESS));
    
    SetDebugLogGuard guard;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->SetAIPPDtcPixelMean(dyAippCfg, fakeAipp, 1);
    std::string logOutput = testing::internal::GetCapturedStdout();

    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(logOutput.find("dtcPixelMeanChn0: 10 dtcPixelMeanChn1: 20") != std::string::npos);
}

TEST_F(ModelProcessTest, TestSetAIPPDtcPixelMean_DefaultValue_Success)
{
    auto dyAippCfg = std::make_shared<Base::DynamicAippConfig>();
    aclmdlAIPP* fakeAipp = reinterpret_cast<aclmdlAIPP*>(0x8888);
    
    // SetMaxBatchSize 为 0 将导致使用默认值
    std::vector<int> pixelMean = {10, 20, 30, 40};
    dyAippCfg->SetMaxBatchSize(0);
    dyAippCfg->SetDtcPixelMean(pixelMean);
    
    // 设置期望行为（使用默认值0,0,0,0）
    EXPECT_CALL(*mockAcl, aclmdlSetAIPPDtcPixelMean(fakeAipp, 0, 0, 0, 0, 0))
        .WillOnce(Return(ACL_SUCCESS));
    
    SetDebugLogGuard guard;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->SetAIPPDtcPixelMean(dyAippCfg, fakeAipp, 0);
    std::string logOutput = testing::internal::GetCapturedStdout();

    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(logOutput.find("dtcPixelMeanChn0: 0 dtcPixelMeanChn1: 0") != std::string::npos);
}

TEST_F(ModelProcessTest, TestSetAIPPDtcPixelMean_Failure)
{
    auto dyAippCfg = std::make_shared<Base::DynamicAippConfig>();
    aclmdlAIPP* fakeAipp = reinterpret_cast<aclmdlAIPP*>(0x9999);
    
    // SetMaxBatchSize 为 0 将导致使用默认值
    std::vector<int> pixelMean = {10, 20, 30, 40};
    dyAippCfg->SetMaxBatchSize(0);
    dyAippCfg->SetDtcPixelMean(pixelMean);
    
    // 设置模拟ACL失败
    EXPECT_CALL(*mockAcl, aclmdlSetAIPPDtcPixelMean(_, _, _, _, _, _))
        .WillOnce(Return(ACL_ERROR_INTERNAL_ERROR));
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillOnce(Return("Mock error message"));
    
    testing::internal::CaptureStdout();
    EXPECT_THROW({
        try {
            modelProcess->SetAIPPDtcPixelMean(dyAippCfg, fakeAipp, 0);
        } catch (const char* e) {
            EXPECT_STREQ(e, "AippData set failed!");
            throw;
        }
    }, const char*);
    std::string logOutput = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(logOutput.find("acl set AIPP Dtc pixel mean params failed") != std::string::npos);
}

// ===================== SetAIPPDtcPixelMin测试 =====================
TEST_F(ModelProcessTest, TestSetAIPPDtcPixelMin_CustomValue_Success)
{
    auto dyAippCfg = std::make_shared<Base::DynamicAippConfig>();
    aclmdlAIPP* fakeAipp = reinterpret_cast<aclmdlAIPP*>(0x7777);
    
    // 设置dtcPixelMin数据
    std::vector<float> pixelMin = {10.0, 20.0, 30.0, 40.0};
    dyAippCfg->SetMaxBatchSize(pixelMin.size());
    dyAippCfg->SetDtcPixelMin(pixelMin);
    
    // 设置期望行为（batchIndex=1 应使用第二个值）
    EXPECT_CALL(*mockAcl, aclmdlSetAIPPDtcPixelMin(fakeAipp, 10.0, 20.0, 30.0, 40.0, 1))
        .WillOnce(Return(ACL_SUCCESS));
    
    SetDebugLogGuard guard;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->SetAIPPDtcPixelMin(dyAippCfg, fakeAipp, 1);
    std::string logOutput = testing::internal::GetCapturedStdout();

    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(logOutput.find("dtcPixelMinChn0: 10.0") != std::string::npos);
    EXPECT_TRUE(logOutput.find("dtcPixelMinChn1: 20.0") != std::string::npos);
}

TEST_F(ModelProcessTest, TestSetAIPPDtcPixelMin_DefaultValue_Success)
{
    auto dyAippCfg = std::make_shared<Base::DynamicAippConfig>();
    aclmdlAIPP* fakeAipp = reinterpret_cast<aclmdlAIPP*>(0x8888);
    
    // SetMaxBatchSize 为 0 将导致使用默认值
    std::vector<float> pixelMin = {10.0, 20.0, 30.0, 40.0};
    dyAippCfg->SetMaxBatchSize(0);
    dyAippCfg->SetDtcPixelMin(pixelMin);
    
    // 设置期望行为（使用默认值0,0,0,0）
    EXPECT_CALL(*mockAcl, aclmdlSetAIPPDtcPixelMin(fakeAipp, 0, 0, 0, 0, 0))
        .WillOnce(Return(ACL_SUCCESS));
    
    SetDebugLogGuard guard;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->SetAIPPDtcPixelMin(dyAippCfg, fakeAipp, 0);
    std::string logOutput = testing::internal::GetCapturedStdout();

    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(logOutput.find("dtcPixelMinChn0: 0.0") != std::string::npos);
    EXPECT_TRUE(logOutput.find("dtcPixelMinChn1: 0.0") != std::string::npos);
}

TEST_F(ModelProcessTest, TestSetAIPPDtcPixelMin_Failure)
{
    auto dyAippCfg = std::make_shared<Base::DynamicAippConfig>();
    aclmdlAIPP* fakeAipp = reinterpret_cast<aclmdlAIPP*>(0x9999);
    
    // SetMaxBatchSize 为 0 将导致使用默认值
    std::vector<float> pixelMin = {10.0, 20.0, 30.0, 40.0};
    dyAippCfg->SetMaxBatchSize(0);
    dyAippCfg->SetDtcPixelMin(pixelMin);
    
    // 设置模拟ACL失败
    EXPECT_CALL(*mockAcl, aclmdlSetAIPPDtcPixelMin(_, _, _, _, _, _))
        .WillOnce(Return(ACL_ERROR_INTERNAL_ERROR));
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillOnce(Return("Mock error message"));
    
    testing::internal::CaptureStdout();
    EXPECT_THROW({
        try {
            modelProcess->SetAIPPDtcPixelMin(dyAippCfg, fakeAipp, 0);
        } catch (const char* e) {
            EXPECT_STREQ(e, "AippData set failed!");
            throw;
        }
    }, const char*);
    std::string logOutput = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(logOutput.find("acl set AIPP dtc pixel min params failed") != std::string::npos);
}

// ===================== SetAIPPPixelVarReci测试 =====================

TEST_F(ModelProcessTest, TestSetAIPPPixelVarReci_CustomValue_Success)
{
    auto dyAippCfg = std::make_shared<Base::DynamicAippConfig>();
    aclmdlAIPP* fakeAipp = reinterpret_cast<aclmdlAIPP*>(0x7777);
    
    // 设置dtcPixelVarReci数据
    std::vector<float> varReci = {10.0, 20.0, 30.0, 40.0};
    dyAippCfg->SetMaxBatchSize(varReci.size());
    dyAippCfg->SetPixelVarReci(varReci);
    
    // 设置期望行为（batchIndex=1 应使用第二个值）
    EXPECT_CALL(*mockAcl, aclmdlSetAIPPPixelVarReci(fakeAipp, 10.0, 20.0, 30.0, 40.0, 1))
        .WillOnce(Return(ACL_SUCCESS));
    
    SetDebugLogGuard guard;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->SetAIPPPixelVarReci(dyAippCfg, fakeAipp, 1);
    std::string logOutput = testing::internal::GetCapturedStdout();

    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(logOutput.find("dtcPixelVarReciChn0: 10.0") != std::string::npos);
    EXPECT_TRUE(logOutput.find("dtcPixelVarReciChn1") != std::string::npos);
}

TEST_F(ModelProcessTest, TestSetAIPPPixelVarReci_DefaultValue_Success)
{
    auto dyAippCfg = std::make_shared<Base::DynamicAippConfig>();
    aclmdlAIPP* fakeAipp = reinterpret_cast<aclmdlAIPP*>(0x7777);
    
    // SetMaxBatchSize 为 0 将导致使用默认值
    std::vector<float> varReci = {10.0, 20.0, 30.0, 40.0};
    dyAippCfg->SetMaxBatchSize(0);
    dyAippCfg->SetPixelVarReci(varReci);
    
    // 设置期望行为（使用默认值0,0,0,0）
    EXPECT_CALL(*mockAcl, aclmdlSetAIPPPixelVarReci(fakeAipp, 0, 0, 0, 0, 0))
        .WillOnce(Return(ACL_SUCCESS));
    
    SetDebugLogGuard guard;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->SetAIPPPixelVarReci(dyAippCfg, fakeAipp, 0);
    std::string logOutput = testing::internal::GetCapturedStdout();

    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(logOutput.find("dtcPixelVarReciChn0: 0.0") != std::string::npos);
    EXPECT_TRUE(logOutput.find("dtcPixelVarReciChn1") != std::string::npos);
}

TEST_F(ModelProcessTest, TestSetAIPPPixelVarReci_Failure)
{
    auto dyAippCfg = std::make_shared<Base::DynamicAippConfig>();
    aclmdlAIPP* fakeAipp = reinterpret_cast<aclmdlAIPP*>(0x7777);
    
    // SetMaxBatchSize 为 0 将导致使用默认值
    std::vector<float> varReci = {10.0, 20.0, 30.0, 40.0};
    dyAippCfg->SetMaxBatchSize(0);
    dyAippCfg->SetPixelVarReci(varReci);
    
    // 设置模拟ACL失败
    EXPECT_CALL(*mockAcl, aclmdlSetAIPPPixelVarReci(_, _, _, _, _, _))
        .WillOnce(Return(ACL_ERROR_INTERNAL_ERROR));
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillOnce(Return("Mock error message"));
    
    testing::internal::CaptureStdout();
    EXPECT_THROW({
        try {
            modelProcess->SetAIPPPixelVarReci(dyAippCfg, fakeAipp, 0);
        } catch (const char* e) {
            EXPECT_STREQ(e, "AippData set failed!");
            throw;
        }
    }, const char*);
    std::string logOutput = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(logOutput.find("acl set AIPP pixel variance params failed") != std::string::npos);
}

// ===================== SetAIPPCropParams 测试 =====================

TEST_F(ModelProcessTest, TestSetAIPPCropParams_CustomValue_Success)
{
    auto dyAippCfg = std::make_shared<Base::DynamicAippConfig>();
    aclmdlAIPP* fakeAipp = reinterpret_cast<aclmdlAIPP*>(0x7777);
    
    // 设置 cropInputParams 数据
    std::vector<int> cropInputParams = {10, 20, 30, 40, 50};
    dyAippCfg->SetMaxBatchSize(cropInputParams.size());
    dyAippCfg->SetCropParams(cropInputParams);
    
    // 设置期望行为（batchIndex=1 应使用第二个值）
    EXPECT_CALL(*mockAcl, aclmdlSetAIPPCropParams(fakeAipp, 10, 20, 30, 40, 50, 1))
        .WillOnce(Return(ACL_SUCCESS));
    
    SetDebugLogGuard guard;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->SetAIPPCropParams(dyAippCfg, fakeAipp, 1);
    std::string logOutput = testing::internal::GetCapturedStdout();

    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(logOutput.find("cropSwitch: 10") != std::string::npos);
    EXPECT_TRUE(logOutput.find("loadStartPosW: 20") != std::string::npos);
}

TEST_F(ModelProcessTest, TestSetAIPPCropParams_DefaultValue_Success)
{
    auto dyAippCfg = std::make_shared<Base::DynamicAippConfig>();
    aclmdlAIPP* fakeAipp = reinterpret_cast<aclmdlAIPP*>(0x7777);
    
    // SetMaxBatchSize 为 0 将导致使用默认值
    std::vector<int> cropInputParams = {10, 20, 30, 40, 50};
    dyAippCfg->SetMaxBatchSize(0);
    dyAippCfg->SetCropParams(cropInputParams);
    
    // 设置期望行为（使用默认值0,0,0,0,0）
    EXPECT_CALL(*mockAcl, aclmdlSetAIPPCropParams(fakeAipp, _, 0, 0, 
                                                  Base::CROP_SIZE_W_DEFAULT, Base::CROP_SIZE_H_DEFAULT, 0))
        .WillOnce(Return(ACL_SUCCESS));
    
    SetDebugLogGuard guard;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->SetAIPPCropParams(dyAippCfg, fakeAipp, 0);
    std::string logOutput = testing::internal::GetCapturedStdout();

    EXPECT_EQ(ret, SUCCESS);
}

TEST_F(ModelProcessTest, TestSetAIPPCropParams_Failure)
{
    auto dyAippCfg = std::make_shared<Base::DynamicAippConfig>();
    aclmdlAIPP* fakeAipp = reinterpret_cast<aclmdlAIPP*>(0x7777);
    
    // SetMaxBatchSize 为 0 将导致使用默认值
    std::vector<int> cropInputParams = {10, 20, 30, 40, 50};
    dyAippCfg->SetMaxBatchSize(0);
    dyAippCfg->SetCropParams(cropInputParams);
    
    // 设置模拟ACL失败
    EXPECT_CALL(*mockAcl, aclmdlSetAIPPCropParams(_, _, _, _, _, _, _))
        .WillOnce(Return(ACL_ERROR_INTERNAL_ERROR));
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillOnce(Return("Mock error message"));
    
    testing::internal::CaptureStdout();
    EXPECT_THROW({
        try {
            modelProcess->SetAIPPCropParams(dyAippCfg, fakeAipp, 0);
        } catch (const char* e) {
            EXPECT_STREQ(e, "AippData set failed!");
            throw;
        }
    }, const char*);
    std::string logOutput = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(logOutput.find("acl set AIPP crop params failed") != std::string::npos);
}

// ===================== SetAIPPPaddingParams 测试 =====================

TEST_F(ModelProcessTest, TestSetAIPPPaddingParams_CustomValue_Success)
{
    auto dyAippCfg = std::make_shared<Base::DynamicAippConfig>();
    aclmdlAIPP* fakeAipp = reinterpret_cast<aclmdlAIPP*>(0x7777);
    
    // 设置 padInputParams 数据
    std::vector<int> padInputParams = {10, 20, 30, 40, 50};
    dyAippCfg->SetMaxBatchSize(padInputParams.size());
    dyAippCfg->SetPaddingParams(padInputParams);
    
    // 设置期望行为（batchIndex=1 应使用第二个值）
    EXPECT_CALL(*mockAcl, aclmdlSetAIPPPaddingParams(fakeAipp, 10, 20, 30, 40, 50, 1))
        .WillOnce(Return(ACL_SUCCESS));
    
    SetDebugLogGuard guard;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->SetAIPPPaddingParams(dyAippCfg, fakeAipp, 1);
    std::string logOutput = testing::internal::GetCapturedStdout();

    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(logOutput.find("paddingSwitch: 10") != std::string::npos);
    EXPECT_TRUE(logOutput.find("paddingSizeTop: 20") != std::string::npos);
}

TEST_F(ModelProcessTest, TestSetAIPPPaddingParams_DefaultValue_Success)
{
    auto dyAippCfg = std::make_shared<Base::DynamicAippConfig>();
    aclmdlAIPP* fakeAipp = reinterpret_cast<aclmdlAIPP*>(0x7777);
    
    // SetMaxBatchSize 为 0 将导致使用默认值
    std::vector<int> padInputParams = {10, 20, 30, 40, 50};
    dyAippCfg->SetMaxBatchSize(0);
    dyAippCfg->SetPaddingParams(padInputParams);
    
    // 设置期望行为（使用默认值0,0,0,0,0）
    EXPECT_CALL(*mockAcl, aclmdlSetAIPPPaddingParams(fakeAipp, 0, 0, 0, 0, 0, 0))
        .WillOnce(Return(ACL_SUCCESS));
    
    SetDebugLogGuard guard;
    testing::internal::CaptureStdout();
    Result ret = modelProcess->SetAIPPPaddingParams(dyAippCfg, fakeAipp, 0);
    std::string logOutput = testing::internal::GetCapturedStdout();

    EXPECT_EQ(ret, SUCCESS);
}

TEST_F(ModelProcessTest, TestSetAIPPPaddingParams_Failure)
{
    auto dyAippCfg = std::make_shared<Base::DynamicAippConfig>();
    aclmdlAIPP* fakeAipp = reinterpret_cast<aclmdlAIPP*>(0x7777);
    
    // SetMaxBatchSize 为 0 将导致使用默认值
    std::vector<int> padInputParams = {10, 20, 30, 40, 50};
    dyAippCfg->SetMaxBatchSize(0);
    dyAippCfg->SetPaddingParams(padInputParams);
    
    // 设置模拟ACL失败
    EXPECT_CALL(*mockAcl, aclmdlSetAIPPPaddingParams(_, _, _, _, _, _, _))
        .WillOnce(Return(ACL_ERROR_INTERNAL_ERROR));
    EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
        .WillOnce(Return("Mock error message"));
    
    testing::internal::CaptureStdout();
    EXPECT_THROW({
        try {
            modelProcess->SetAIPPPaddingParams(dyAippCfg, fakeAipp, 0);
        } catch (const char* e) {
            EXPECT_STREQ(e, "AippData set failed!");
            throw;
        }
    }, const char*);
    std::string logOutput = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(logOutput.find("acl set AIPP padding params failed") != std::string::npos);
}
}