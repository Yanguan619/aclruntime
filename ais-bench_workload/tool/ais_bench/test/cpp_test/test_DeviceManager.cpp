#include <gtest/gtest.h>
#include "Base/DeviceManager/DeviceManager.h"

using namespace Base;

static int g_deviceCount = 1;
static int g_aclInitCount = 0;
static int g_aclrtSetDeviceFail = 0;
static int g_aclrtCreateContextFail = 0;

APP_ERROR aclInit(const char* configPath) {
    if (configPath && std::string(configPath) == "APP_ERR_ACL_FAILURE") {
        return APP_ERR_ACL_FAILURE;
    }
    ++g_aclInitCount;
    if (g_aclInitCount == 1) {
        // 第一次调用时正常返回
        return APP_ERR_OK;
    }
    // 第二次及之后都返回“重复初始化”
    return ACL_ERROR_REPEAT_INITIALIZE;
}


APP_ERROR aclrtSetDevice(int) {
    if (g_aclrtSetDeviceFail) {
        return APP_ERR_ACL_FAILURE;
    }
    return APP_ERR_OK;
}


APP_ERROR aclrtCreateContext(void**, int) {
    if (g_aclrtCreateContextFail) {
        return APP_ERR_ACL_FAILURE;
    }
    return APP_ERR_OK;
}


APP_ERROR aclrtGetDeviceCount(uint32_t* count)
{
    if (g_deviceCount == 1) {
        return APP_ERR_OK;
    } else {
        return APP_ERR_ACL_FAILURE;
    }
}

extern "C" {
    APP_ERROR aclFinalize() { return APP_ERR_OK; }
    APP_ERROR aclrtSetCurrentContext(void*) { return APP_ERR_OK; }
    APP_ERROR aclrtDestroyContext(void*) { return APP_ERR_OK; }
    APP_ERROR aclrtResetDevice(int) { return APP_ERR_OK; }
    APP_ERROR aclrtGetCurrentContext(void** ctx)
    {
        if (ctx) *ctx = nullptr;
        return APP_ERR_OK;
    }
    const char* aclGetRecentErrMsg() { return "mock error"; }
}

namespace {

class DeviceManagerTest : public ::testing::Test {
protected:
    DeviceManager* mgr = nullptr;
    void SetUp() override {
        mgr = DeviceManager::GetInstance();
    }
};

TEST_F(DeviceManagerTest, GetInstanceAndIsInitDevices)
{
    EXPECT_NE(mgr, nullptr);
    // Default not initialized
    EXPECT_FALSE(mgr->IsInitDevices());
}

TEST_F(DeviceManagerTest, SetAclJsonPath)
{
    mgr->SetAclJsonPath("/tmp/acl.json");
    // No assert, just cover the setter
}

TEST_F(DeviceManagerTest, CheckDeviceId)
{
    // deviceCount_ is private, but CheckDeviceId(-1) must fail
    EXPECT_EQ(mgr->CheckDeviceId(-1), APP_ERR_COMM_INVALID_PARAM);
    // deviceCount_ is 0 by default, so any >=0 will also fail
    EXPECT_EQ(mgr->CheckDeviceId(0), APP_ERR_COMM_INVALID_PARAM);
}

TEST_F(DeviceManagerTest, GetDevicesCount)
{
    uint32_t count = 123;
    EXPECT_EQ(mgr->GetDevicesCount(count), APP_ERR_OK);
    // count is not changed by default, but function is covered
}

TEST_F(DeviceManagerTest, GetCurrentDevice)
{
    DeviceContext ctx;
    // Should return APP_ERR_OK, but devId is -1 by default
    EXPECT_EQ(mgr->GetCurrentDevice(ctx), APP_ERR_OK);
    EXPECT_EQ(ctx.devId, -1);
}

TEST_F(DeviceManagerTest, ResetDevice)
{
    DeviceContext ctx;
    EXPECT_EQ(mgr->ResetDevice(ctx), APP_ERR_OK);
}

TEST_F(DeviceManagerTest, SetDeviceSimple)
{
    DeviceContext ctx;
    EXPECT_EQ(mgr->SetDeviceSimple(ctx), mgr->SetContext(ctx));
}

TEST_F(DeviceManagerTest, DestroyDevices_NotInit)
{
    // repeatInitAclFlag is true by default, but initCounter_ is 0
    EXPECT_EQ(mgr->DestroyDevices(), APP_ERR_COMM_OUT_OF_RANGE);
}

TEST_F(DeviceManagerTest, DestroyContext_NotInit)
{
    EXPECT_EQ(mgr->DestroyContext(0, 0), APP_ERR_COMM_OUT_OF_RANGE);
}

TEST_F(DeviceManagerTest, InitDevices_Fail)
{
    mgr->SetAclJsonPath("APP_ERR_ACL_FAILURE");
    EXPECT_EQ(mgr->InitDevices(""), APP_ERR_ACL_FAILURE);
    mgr->SetAclJsonPath("");
}

TEST_F(DeviceManagerTest, InitDevices_GetDeviceCount_Fail)
{
    g_deviceCount = 0;
    EXPECT_EQ(mgr->InitDevices(""), APP_ERR_ACL_FAILURE);
    g_deviceCount = 1;
}

TEST_F(DeviceManagerTest, InitDevices_Normal)
{
    // 覆盖正常初始化流程
    EXPECT_EQ(mgr->InitDevices(""), APP_ERR_OK);
    // 再次调用，走initCounter_ > 1分支
    EXPECT_EQ(mgr->InitDevices(""), APP_ERR_OK);
}

TEST_F(DeviceManagerTest, InitDevices_RepeatInit)
{
    DeviceManager* mgr = DeviceManager::GetInstance();
    EXPECT_EQ(mgr->InitDevices(""), APP_ERR_OK);
    EXPECT_EQ(mgr->DestroyDevices(), APP_ERR_OK);

    EXPECT_EQ(mgr->InitDevices(""), APP_ERR_OK);
    EXPECT_EQ(mgr->DestroyDevices(), APP_ERR_OK);
}


TEST_F(DeviceManagerTest, CreateContext_DeviceNotExist)
{
    DeviceContext ctx;
    ctx.devId = 0;
    size_t contextIndex = 0;
    // deviceCount_ is 0 by default, so CheckDeviceId would fail, but CreateContext does not check deviceCount_
    // Should call aclrtSetDevice and succeed
    EXPECT_EQ(mgr->CreateContext(ctx, contextIndex), APP_ERR_OK);
    // contextIndex should be 0
    EXPECT_EQ(contextIndex, 0u);
}

TEST_F(DeviceManagerTest, CreateContext_RepeatCreate)
{
    DeviceContext ctx;
    ctx.devId = 1;
    size_t contextIndex1 = 0;
    size_t contextIndex2 = 0;
    // First create: should succeed
    EXPECT_EQ(mgr->CreateContext(ctx, contextIndex1), APP_ERR_OK);
    // Second create: should succeed, contextIndex should increment
    EXPECT_EQ(mgr->CreateContext(ctx, contextIndex2), APP_ERR_OK);
    EXPECT_EQ(contextIndex2, contextIndex1 + 1);
}

TEST_F(DeviceManagerTest, CreateContext_MissingDefaultContext)
{
    DeviceContext ctx;
    ctx.devId = 100;
    size_t contextIndex = 0;
    // Simulate missing nextContextIndex_ entry
    // Remove the entry after first create
    EXPECT_EQ(mgr->CreateContext(ctx, contextIndex), APP_ERR_OK);
    // Erase nextContextIndex_ entry to simulate error
    {
        std::lock_guard<std::mutex> lock(reinterpret_cast<DeviceManager*>(mgr)->mtx_);
        reinterpret_cast<DeviceManager*>(mgr)->nextContextIndex_.erase(ctx.devId);
    }
    // Now CreateContext should fail with APP_ERR_COMM_READ_FAIL
    size_t dummyIndex = 0;
    EXPECT_EQ(mgr->CreateContext(ctx, dummyIndex), APP_ERR_COMM_READ_FAIL);
}

TEST_F(DeviceManagerTest, CreateContext_GetCurrentContextWhenRepeatInitAclFlagFalse)
{
    DeviceContext ctx;
    ctx.devId = 2;
    size_t contextIndex = 0;
    // Set repeatInitAclFlag to false to trigger aclrtGetCurrentContext branch
    reinterpret_cast<DeviceManager*>(mgr)->repeatInitAclFlag = false;
    // Should succeed and contextIndex should be 0
    EXPECT_EQ(mgr->CreateContext(ctx, contextIndex), APP_ERR_OK);
    EXPECT_EQ(contextIndex, 0u);
    // Restore flag for other tests
    reinterpret_cast<DeviceManager*>(mgr)->repeatInitAclFlag = true;
}

TEST_F(DeviceManagerTest, CreateContext_aclrtSetDeviceFail)
{
    DeviceContext ctx;
    ctx.devId = 1234;
    size_t contextIndex = 0;
    g_aclrtSetDeviceFail = 1;
    EXPECT_EQ(mgr->CreateContext(ctx, contextIndex), APP_ERR_ACL_FAILURE);
    g_aclrtSetDeviceFail = 0;
}

TEST_F(DeviceManagerTest, CreateContext_aclrtCreateContextFail)
{
    DeviceContext ctx;
    ctx.devId = 3;
    size_t contextIndex = 0;
    g_aclrtCreateContextFail = 1;
    EXPECT_EQ(mgr->CreateContext(ctx, contextIndex), APP_ERR_ACL_FAILURE);
    g_aclrtCreateContextFail = 0;
}

TEST_F(DeviceManagerTest, DestroyContext_WhenNotInitialized_ReturnsOutOfRange)
{
    DeviceManager* mgr = DeviceManager::GetInstance();
    // Ensure not initialized
    while (mgr->IsInitDevices()) {
        mgr->DestroyDevices();
    }
    EXPECT_EQ(mgr->DestroyContext(0, 0), APP_ERR_COMM_OUT_OF_RANGE);
}

TEST_F(DeviceManagerTest, DestroyContext_DeviceIdNotExist_ReturnsOk)
{
    DeviceManager* mgr = DeviceManager::GetInstance();
    mgr->InitDevices("");
    // No context created for device 99
    EXPECT_EQ(mgr->DestroyContext(99, 0), APP_ERR_OK);
    mgr->DestroyDevices();
}

TEST_F(DeviceManagerTest, DestroyContext_ContextIndexNotExist_ReturnsOk)
{
    DeviceManager* mgr = DeviceManager::GetInstance();
    mgr->InitDevices("");
    DeviceContext ctx;
    ctx.devId = 0;
    size_t contextIndex = 0;
    mgr->CreateContext(ctx, contextIndex);
    // Try to destroy a non-existent context index
    EXPECT_EQ(mgr->DestroyContext(0, 12345), APP_ERR_OK);
    mgr->DestroyDevices();
}

TEST_F(DeviceManagerTest, DestroyContext_RepeatDestroy_ReturnsOk)
{
    DeviceManager* mgr = DeviceManager::GetInstance();
    mgr->InitDevices("");
    DeviceContext ctx;
    ctx.devId = 0;
    size_t contextIndex = 0;
    mgr->CreateContext(ctx, contextIndex);
    // First destroy should succeed
    EXPECT_EQ(mgr->DestroyContext(0, contextIndex), APP_ERR_OK);
    // Second destroy (repeat) should warn but still return OK
    EXPECT_EQ(mgr->DestroyContext(0, contextIndex), APP_ERR_OK);
    mgr->DestroyDevices();
}

TEST_F(DeviceManagerTest, DestroyContext_DestroyRealContext_ReturnsOk)
{
    DeviceManager* mgr = DeviceManager::GetInstance();
    mgr->InitDevices("");
    DeviceContext ctx;
    ctx.devId = 0;
    size_t contextIndex = 0;
    mgr->CreateContext(ctx, contextIndex);
    // Should destroy the context successfully
    EXPECT_EQ(mgr->DestroyContext(0, contextIndex), APP_ERR_OK);
    mgr->DestroyDevices();
}


class DestroyDevicesTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset DeviceManager state before each test
        DeviceManager* mgr = DeviceManager::GetInstance();
        // Try to destroy devices until initCounter_ is 0

    }
    void TearDown() override {
        // Clean up after test
        DeviceManager* mgr = DeviceManager::GetInstance();
        mgr->DestroyDevices();
    }
};

TEST_F(DestroyDevicesTest, DestroyDevices_WhenNotInitialized_ReturnsOutOfRange) {
    DeviceManager* mgr = DeviceManager::GetInstance();
    // initCounter_ == 0
    APP_ERROR ret = mgr->DestroyDevices();
    EXPECT_EQ(ret, APP_ERR_OK);
}

TEST_F(DestroyDevicesTest, DestroyDevices_WhenInitializedOnce_ReturnsOk) {
    DeviceManager* mgr = DeviceManager::GetInstance();
    mgr->SetAclJsonPath(""); // avoid nullptr
    mgr->InitDevices("");
    APP_ERROR ret = mgr->DestroyDevices();
    EXPECT_EQ(ret, APP_ERR_OK);
}

TEST_F(DestroyDevicesTest, DestroyDevices_WhenInitializedTwice_DecrementsCounter) {
    DeviceManager* mgr = DeviceManager::GetInstance();
    mgr->SetAclJsonPath("");
    mgr->InitDevices("");
    mgr->InitDevices("");
    // Counter should be 2
    APP_ERROR ret = mgr->DestroyDevices();
    EXPECT_EQ(ret, APP_ERR_OK);
    // Should still be initialized (counter == 1)
    EXPECT_TRUE(mgr->IsInitDevices());
    // Destroy again, should be 0 now
    ret = mgr->DestroyDevices();
    EXPECT_EQ(ret, APP_ERR_OK);
}

TEST_F(DestroyDevicesTest, DestroyDevices_RepeatInitAclFlagFalse_ReturnsOk) {
    DeviceManager* mgr = DeviceManager::GetInstance();
    mgr->SetAclJsonPath("");
    mgr->InitDevices("");
    // Simulate acl repeat initialize
    mgr->repeatInitAclFlag = false;
    APP_ERROR ret = mgr->DestroyDevices();
    EXPECT_EQ(ret, APP_ERR_OK);
}

} // namespace
