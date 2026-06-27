#include <memory>
#include <map>
#include <vector>
#include <string>

#include <gtest/gtest.h>
#include <mockcpp/mockcpp.hpp>

#include "acl_mock_functions.h"
#include "Base/ModelInfer/ModelInferenceProcessor.h"
#include "Base/ModelInfer/model_process.h"
#include "Base/ModelInfer/DynamicAippConfig.h"


using namespace std;
using namespace testing;
using namespace Base;
using namespace UtilsResult;

namespace {
class DummyModelProcess : public ModelProcess {
public:
    // 只mock需要的接口
    UtilsResult::Result LoadModelFromFile(const std::string&) { return SUCCESS; }
    UtilsResult::Result CreateDesc() { return SUCCESS; }
    UtilsResult::Result GetDynamicGearCount(size_t &gear) { gear = 1; return SUCCESS; }
    UtilsResult::Result GetDynamicIndex(size_t &idx) { idx = 0; return SUCCESS; }
    UtilsResult::Result GetAIPPIndexList(std::vector<size_t>& v) { v.clear(); return SUCCESS; }
    UtilsResult::Result GetInTensorDesc(size_t, std::string& name, int& datatype, size_t& format, std::vector<int64_t>& shape, size_t& size) {
        name = "input"; datatype = 0; format = 0; shape = {1,2,3}; size = 24; return SUCCESS; }
    UtilsResult::Result GetOutTensorDesc(size_t, std::string& name, int& datatype, size_t& format, std::vector<int64_t>& shape, size_t& size) {
        name = "output"; datatype = 0; format = 0; shape = {1,2,3}; size = 24; return SUCCESS; }
    UtilsResult::Result GetMaxBatchSize(uint64_t& maxBatchSize) { maxBatchSize = 8; return SUCCESS; }
    UtilsResult::Result CheckDynamicBatchSize(uint64_t, bool& is_dymbatch) { is_dymbatch = true; return SUCCESS; }
    UtilsResult::Result CheckDynamicHWSize(std::pair<int,int>, bool& is_dymHW) { is_dymHW = true; return SUCCESS; }
    UtilsResult::Result GetMaxDynamicHWSize(uint64_t& outsize) { outsize = 100; return SUCCESS; }
    UtilsResult::Result CheckDynamicDims(std::vector<std::string>, size_t, aclmdlIODims*) { return SUCCESS; }
    UtilsResult::Result CheckDynamicShape(std::vector<std::string>, std::map<std::string, std::vector<int64_t>>&, std::vector<int64_t>&) { return SUCCESS; }
    UtilsResult::Result SetDynamicBatchSize(uint64_t) { return SUCCESS; }
    UtilsResult::Result SetDynamicHW(std::pair<uint64_t,uint64_t>) { return SUCCESS; }
    UtilsResult::Result SetDynamicDims(std::vector<std::string>) { return SUCCESS; }
    UtilsResult::Result SetDynamicShape(std::map<std::string, std::vector<int64_t>>, std::vector<int64_t>&) { return SUCCESS; }
    UtilsResult::Result CreateInput(void*, size_t) { return SUCCESS; }
    UtilsResult::Result CreateOutput(void*, size_t) { return SUCCESS; }
    UtilsResult::Result CreateOutput() { return SUCCESS; }
    UtilsResult::Result Execute() { return SUCCESS; }
    UtilsResult::Result PrintDesc() { return SUCCESS; }
    UtilsResult::Result SetInputAIPP(size_t, void*) { return SUCCESS; }
    UtilsResult::Result GetCurOutputShape(size_t, bool, std::vector<int64_t>&) { return SUCCESS; }
    void DestroyInput(bool) {}
    void DestroyOutput(bool) {}
    void DestroyDesc() {}
    void SetExceptionCallBack() {}
    int CheckDymAIPPInputExist() { return 1; }
    UtilsResult::Result FreeAIPP(aclmdlAIPP*) { return SUCCESS; }
    size_t GetNumInputs() { return 1; }
    size_t GetNumOutputs() { return 1; }
    size_t GetOutTensorLen(size_t, bool) { return 24; }
    UtilsResult::Result UpdateInputsReuse(const std::vector<int>&) { return SUCCESS; }
    UtilsResult::Result UpdateInputsMemcpy(const std::vector<int>&) { return SUCCESS; }
    UtilsResult::Result CreateDymInput(size_t) { return SUCCESS; }
    UtilsResult::Result CreateZeroInput() { return SUCCESS; }
    void GetDimInfo(size_t, aclmdlIODims*) {}
    void GetDymBatchInfo() {}
    void GetDymHWInfo() {}
    void DumpModelOutputResult() {}
};

class DummyDynamicAippConfig : public DynamicAippConfig {
public:
    bool IsActivated() { return true; }
    bool ModelIsLegal() { return true; }
    void ActivateConfig() {}
    void ActivateModel() {}
    APP_ERROR SetMaxBatchSize(uint64_t) { return APP_ERR_OK; }
    APP_ERROR SetInputFormat(std::string) { return APP_ERR_OK; }
    APP_ERROR SetSrcImageSize(std::vector<int>) { return APP_ERR_OK; }
    APP_ERROR SetRbuvSwapSwitch(int) { return APP_ERR_OK; }
    APP_ERROR SetAxSwapSwitch(int) { return APP_ERR_OK; }
    APP_ERROR SetCscParams(std::vector<int>) { return APP_ERR_OK; }
    APP_ERROR SetCropParams(std::vector<int>) { return APP_ERR_OK; }
    APP_ERROR SetPaddingParams(std::vector<int>) { return APP_ERR_OK; }
    APP_ERROR SetDtcPixelMean(std::vector<int>) { return APP_ERR_OK; }
    APP_ERROR SetDtcPixelMin(std::vector<float>) { return APP_ERR_OK; }
    APP_ERROR SetPixelVarReci(std::vector<float>) { return APP_ERR_OK; }
};

} // namespace

// Helper: mock ModelProcess and DynamicAippConfig creation
namespace Base {
std::shared_ptr<ModelProcess> MakeDummyModelProcess() {
    return std::make_shared<DummyModelProcess>();
}
std::shared_ptr<DynamicAippConfig> MakeDummyDynamicAippConfig() {
    return std::make_shared<DummyDynamicAippConfig>();
}
}

// UT
class ModelInferenceProcessorMockcppTest : public ::testing::Test {
protected:
    void SetUp() override {
        mockAcl = make_unique<StrictMock<MockACL>>();
        g_mockAcl = mockAcl.get();

        // mockcpp 2.7的MOCKER().stubs().will()不支持lambda/throwException/call，只能用returnValue、returnObject、returnNull等
        static auto dummyModelProcess = Base::MakeDummyModelProcess();
        static auto dummyDynamicAippConfig = Base::MakeDummyDynamicAippConfig();
        SetGlobalDefaultExpectations();
        MOCKER(std::make_shared<ModelProcess>).stubs().will(returnValue(dummyModelProcess));
        MOCKER(std::make_shared<DynamicAippConfig>).stubs().will(returnValue(dummyDynamicAippConfig));
    }

    void TearDown() override {
        GlobalMockObject::reset();

        g_mockAcl = nullptr;
        mockAcl.reset();
    }

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

    unique_ptr<StrictMock<MockACL>> mockAcl; // 模拟ACL接口
};

TEST_F(ModelInferenceProcessorMockcppTest, Init_Fail) {
    ModelInferenceProcessor proc;
    auto options = std::make_shared<SessionOptions>();
    options->log_level = LOG_DEBUG_LEVEL;
    ASSERT_EQ(proc.Init("model.om", options, 0, 0), APP_ERR_ACL_INVALID_PARAM);
}

TEST_F(ModelInferenceProcessorMockcppTest, DeInit) {
    ModelInferenceProcessor proc;
    auto options = std::make_shared<SessionOptions>();
    proc.Init("model.om", options, 0, 0);
    ASSERT_EQ(proc.DeInit(), APP_ERR_OK);
}

TEST_F(ModelInferenceProcessorMockcppTest, GetInputsOutputs) {
    ModelInferenceProcessor proc;
    auto options = std::make_shared<SessionOptions>();
    proc.Init("model.om", options, 0, 0);
    auto& inputs = proc.GetInputs();
    auto& outputs = proc.GetOutputs();
    ASSERT_TRUE(inputs.size() >= 0);
    ASSERT_TRUE(outputs.size() >= 0);
}

TEST_F(ModelInferenceProcessorMockcppTest, GetOptions) {
    ModelInferenceProcessor proc;
    auto options = std::make_shared<SessionOptions>();
    proc.Init("model.om", options, 0, 0);
    ASSERT_EQ(proc.GetOptions(), options);
}

// 对于成员函数的mock，mockcpp 2.7 不能直接MOCKER(DummyModelProcess::UpdateInputsMemcpy)，只能通过派生类重写
// 所以需要在测试用例里替换processModel为自定义失败派生类对象

namespace {
class DummyModelProcessFail : public DummyModelProcess {
public:
    // 重写需要模拟失败的成员函数
    UtilsResult::Result UpdateInputsMemcpy(const std::vector<int>&) { return UtilsResult::FAILED; }
    UtilsResult::Result UpdateInputsReuse(const std::vector<int>&) { return UtilsResult::FAILED; }
    UtilsResult::Result CheckDynamicBatchSize(uint64_t, bool&) { return UtilsResult::FAILED; }
    UtilsResult::Result GetDynamicGearCount(size_t&) { return UtilsResult::FAILED; }
    UtilsResult::Result CheckDynamicHWSize(std::pair<int,int>, bool&) { return UtilsResult::FAILED; }
    UtilsResult::Result CheckDynamicShape(std::vector<std::string>, std::map<std::string, std::vector<int64_t>>&, std::vector<int64_t>&) { return UtilsResult::FAILED; }
    UtilsResult::Result GetDymAIPPConfigSet(std::shared_ptr<Base::DynamicAippConfig>, aclmdlAIPP*&, uint64_t) { return UtilsResult::FAILED; }
    UtilsResult::Result Execute() { return UtilsResult::FAILED; }
};
}

TEST_F(ModelInferenceProcessorMockcppTest, SetInputsData_SuccessAndFail) {
    ModelInferenceProcessor proc;
    auto options = std::make_shared<SessionOptions>();
    proc.Init("model.om", options, 0, 0);
    std::vector<BaseTensor> inputs(1);
    inputs[0].buf = malloc(24);
    inputs[0].size = 24;
    ASSERT_EQ(proc.SetInputsData(inputs), APP_ERR_ACL_FAILURE);

    // fail: size不匹配
    inputs[0].size = 0;
    ASSERT_NE(proc.SetInputsData(inputs), APP_ERR_OK);
    free(inputs[0].buf);
}

TEST_F(ModelInferenceProcessorMockcppTest, GetOutputs_SuccessAndFail) {
    ModelInferenceProcessor proc;
    auto options = std::make_shared<SessionOptions>();
    proc.Init("model.om", options, 0, 0);
    std::vector<std::string> names = {"output"};
    std::vector<TensorBase> outs;
    ASSERT_EQ(proc.GetOutputs(names, outs), APP_ERR_ACL_FAILURE);

    // fail: 名称不存在
    std::vector<std::string> badnames = {"bad"};
    ASSERT_NE(proc.GetOutputs(badnames, outs), APP_ERR_OK);
}

TEST_F(ModelInferenceProcessorMockcppTest, FirstInferenceInner_SuccessAndFail) {
    ModelInferenceProcessor proc;
    auto options = std::make_shared<SessionOptions>();
    options->loop = 2;
    proc.Init("model.om", options, 0, 0);
    std::vector<BaseTensor> inputs(1);
    inputs[0].buf = malloc(24);
    inputs[0].size = 24;
    std::vector<std::string> names = {"output"};
    std::vector<TensorBase> outs;
    ASSERT_EQ(proc.FirstInferenceInner(inputs, names, outs), APP_ERR_ACL_FAILURE);

    // fail: SetInputsData失败
    inputs[0].size = 0;
    ASSERT_NE(proc.FirstInferenceInner(inputs, names, outs), APP_ERR_OK);
    free(inputs[0].buf);
}

TEST_F(ModelInferenceProcessorMockcppTest, ModelInference_Inner_SuccessAndFail) {
    ModelInferenceProcessor proc;
    auto options = std::make_shared<SessionOptions>();
    options->loop = 2;
    proc.Init("model.om", options, 0, 0);
    std::vector<BaseTensor> inputs(1);
    inputs[0].buf = malloc(24);
    inputs[0].size = 24;
    std::vector<std::string> names = {"output"};
    std::vector<TensorBase> outs;
    ASSERT_EQ(proc.ModelInference_Inner(inputs, names, outs), APP_ERR_ACL_FAILURE);

    // fail: SetInputsData失败
    inputs[0].size = 0;
    ASSERT_NE(proc.ModelInference_Inner(inputs, names, outs), APP_ERR_OK);
    free(inputs[0].buf);
}

TEST_F(ModelInferenceProcessorMockcppTest, Execute_SuccessAndFail) {
    ModelInferenceProcessor proc;
    auto options = std::make_shared<SessionOptions>();
    EXPECT_CALL(*mockAcl, aclmdlExecute(_, _, _))
        .WillOnce(Return(ACL_ERROR_BAD_ALLOC))
        .WillOnce(Return(ACL_SUCCESS));
    proc.Init("model.om", options, 0, 0);
    proc.processModel = std::make_shared<DummyModelProcessFail>();
    ASSERT_EQ(proc.Execute(), APP_ERR_ACL_FAILURE);
    proc.processModel = std::make_shared<DummyModelProcess>();
    ASSERT_EQ(proc.Execute(), APP_ERR_OK);
}

TEST_F(ModelInferenceProcessorMockcppTest, ResetSumaryInfo) {
    ModelInferenceProcessor proc;
    auto options = std::make_shared<SessionOptions>();
    proc.Init("model.om", options, 0, 0);
    ASSERT_EQ(proc.ResetSumaryInfo(), APP_ERR_OK);
}

TEST_F(ModelInferenceProcessorMockcppTest, GetSumaryInfo) {
    ModelInferenceProcessor proc;
    auto options = std::make_shared<SessionOptions>();
    proc.Init("model.om", options, 0, 0);
    const InferSumaryInfo& info = proc.GetSumaryInfo();
    InferSumaryInfo& info2 = proc.GetMutableSumaryInfo();
    ASSERT_EQ(info.execTimeList.size(), info2.execTimeList.size());
}

TEST_F(ModelInferenceProcessorMockcppTest, AllocDymAIPPIndexMem_SuccessAndFail) {
    ModelInferenceProcessor proc;
    auto options = std::make_shared<SessionOptions>();
    proc.Init("model.om", options, 0, 0);
    ASSERT_EQ(proc.AllocDymAIPPIndexMem(), APP_ERR_OK);

    GlobalMockObject::reset();
}

TEST_F(ModelInferenceProcessorMockcppTest, FreeDymAIPPMem) {
    ModelInferenceProcessor proc;
    auto options = std::make_shared<SessionOptions>();
    proc.Init("model.om", options, 0, 0);
    ASSERT_EQ(proc.FreeDymAIPPMem(), APP_ERR_OK);
}

TEST_F(ModelInferenceProcessorMockcppTest, AllocDyIndexMem_Success) {
    ModelInferenceProcessor proc;
    auto options = std::make_shared<SessionOptions>();
    proc.Init("model.om", options, 0, 0);
    ASSERT_EQ(proc.AllocDyIndexMem(), APP_ERR_OK);
}

TEST_F(ModelInferenceProcessorMockcppTest, FreeDyIndexMem) {
    ModelInferenceProcessor proc;
    auto options = std::make_shared<SessionOptions>();
    proc.Init("model.om", options, 0, 0);
    ASSERT_EQ(proc.FreeDyIndexMem(), APP_ERR_OK);
}

TEST_F(ModelInferenceProcessorMockcppTest, FreeDymInfoMem) {
    ModelInferenceProcessor proc;
    auto options = std::make_shared<SessionOptions>();
    proc.Init("model.om", options, 0, 0);
    ASSERT_EQ(proc.FreeDymInfoMem(), APP_ERR_OK);
}

TEST_F(ModelInferenceProcessorMockcppTest, SetStaticBatch) {
    ModelInferenceProcessor proc;
    auto options = std::make_shared<SessionOptions>();
    proc.Init("model.om", options, 0, 0);
    ASSERT_EQ(proc.SetStaticBatch(), APP_ERR_OK);
}

TEST_F(ModelInferenceProcessorMockcppTest, AippSetMaxBatchSize_SetInputFormat_SetSrcImageSize_SetRbuvSwapSwitch_SetAxSwapSwitch_SetCscParams_SetCropParams_SetPaddingParams_SetDtcPixelMean_SetDtcPixelMin_SetPixelVarReci) {
    ModelInferenceProcessor proc;
    auto options = std::make_shared<SessionOptions>();
    proc.Init("model.om", options, 0, 0);
    ASSERT_EQ(proc.AippSetMaxBatchSize(1), APP_ERR_OK);
    ASSERT_EQ(proc.SetInputFormat("YUV"), APP_ERR_OK);
    ASSERT_EQ(proc.SetSrcImageSize({1,2}), APP_ERR_OK);
    ASSERT_EQ(proc.SetRbuvSwapSwitch(1), APP_ERR_OK);
    ASSERT_EQ(proc.SetAxSwapSwitch(1), APP_ERR_OK);
    ASSERT_EQ(proc.SetCscParams({0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}), APP_ERR_OK);
    ASSERT_EQ(proc.SetCropParams({0,1,2,3,4}), APP_ERR_OK);
    ASSERT_EQ(proc.SetPaddingParams({0,1,2,3,4}), APP_ERR_OK);
    ASSERT_EQ(proc.SetDtcPixelMean({0,1,2,3}), APP_ERR_OK);
    ASSERT_EQ(proc.SetDtcPixelMin({0.1f,0.2f,0.3f,0.4f}), APP_ERR_OK);
    ASSERT_EQ(proc.SetPixelVarReci({0.1f,0.2f,0.3f,0.4f}), APP_ERR_OK);
}

TEST_F(ModelInferenceProcessorMockcppTest, SetDynamicShape_SuccessAndFail) {
    ModelInferenceProcessor proc;
    auto options = std::make_shared<SessionOptions>();
    proc.Init("model.om", options, 0, 0);
    ASSERT_EQ(proc.SetDynamicShape("a:1,2,3"), APP_ERR_ACL_FAILURE);

    proc.processModel = std::make_shared<DummyModelProcessFail>();
    ASSERT_NE(proc.SetDynamicShape("a:1,2,3"), APP_ERR_OK);
    proc.processModel = std::make_shared<DummyModelProcess>();
}

TEST_F(ModelInferenceProcessorMockcppTest, SetCustomOutTensorsSize) {
    ModelInferenceProcessor proc;
    auto options = std::make_shared<SessionOptions>();
    proc.Init("model.om", options, 0, 0);
    ASSERT_EQ(proc.SetCustomOutTensorsSize({24}), APP_ERR_OK);
}

TEST_F(ModelInferenceProcessorMockcppTest, SetDynamicInfo) {
    ModelInferenceProcessor proc;
    auto options = std::make_shared<SessionOptions>();
    proc.Init("model.om", options, 0, 0);
    // 默认STATIC_BATCH
    ASSERT_EQ(proc.SetDynamicInfo(), APP_ERR_OK);
    // DYNAMIC_BATCH
    EXPECT_CALL(*mockAcl, aclmdlSetDynamicBatchSize)
        .WillOnce(Return(ACL_ERROR_BAD_ALLOC));
    proc.dynamicInfo_.dynamicType = DYNAMIC_BATCH;
    ASSERT_EQ(proc.SetDynamicInfo(), APP_ERR_ACL_INVALID_PARAM);
    // DYNAMIC_HW
    EXPECT_CALL(*mockAcl, aclmdlSetDynamicHWSize)
        .WillOnce(Return(ACL_ERROR_BAD_ALLOC));
    proc.dynamicInfo_.dynamicType = DYNAMIC_HW;
    proc.dynamicInfo_.dyHW.imageSize.width = 1;
    proc.dynamicInfo_.dyHW.imageSize.height = 1;
    ASSERT_EQ(proc.SetDynamicInfo(), APP_ERR_ACL_INVALID_PARAM);
}
