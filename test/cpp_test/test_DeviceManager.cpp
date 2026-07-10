#include <gtest/gtest.h>

#include <memory>

#include "Base/DeviceManager/DeviceManager.h"
#include "acl_mock_functions.h"  // 引入mock头文件

using namespace std;
using namespace Base;
using namespace testing;

namespace {

class DeviceManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        mockAcl = make_unique<StrictMock<MockACL>>();
        g_mockAcl = mockAcl.get();
        SetGlobalDefaultExpectations();

        mgr = DeviceManager::GetInstance();
    }

    void TearDown() override {
        mgr->DestroyDevices();

        g_mockAcl = nullptr;
        mockAcl.reset();
    }

    void SetGlobalDefaultExpectations() {
        // 设置aclInit的默认行为
        EXPECT_CALL(*mockAcl, aclInit(_)).WillRepeatedly(Return(APP_ERR_OK));

        // 设置aclrtGetDeviceCount的默认行为
        EXPECT_CALL(*mockAcl, aclrtGetDeviceCount(_))
            .WillRepeatedly(DoAll(SetArgPointee<0>(1), Return(APP_ERR_OK)));

        // 设置aclrtSetDevice的默认行为
        EXPECT_CALL(*mockAcl, aclrtSetDevice(_))
            .WillRepeatedly(Return(APP_ERR_OK));

        // 设置aclrtCreateContext的默认行为
        EXPECT_CALL(*mockAcl, aclrtCreateContext(_, _))
            .WillRepeatedly(Return(APP_ERR_OK));

        // 设置aclFinalize的默认行为
        EXPECT_CALL(*mockAcl, aclFinalize()).WillRepeatedly(Return(APP_ERR_OK));

        // 设置aclrtSetCurrentContext的默认行为
        EXPECT_CALL(*mockAcl, aclrtSetCurrentContext(_))
            .WillRepeatedly(Return(APP_ERR_OK));

        // 设置aclrtDestroyContext的默认行为
        EXPECT_CALL(*mockAcl, aclrtDestroyContext(_))
            .WillRepeatedly(Return(APP_ERR_OK));

        // 设置aclrtResetDevice的默认行为
        EXPECT_CALL(*mockAcl, aclrtResetDevice(_))
            .WillRepeatedly(Return(APP_ERR_OK));

        // 设置aclrtGetCurrentContext的默认行为
        EXPECT_CALL(*mockAcl, aclrtGetCurrentContext(_))
            .WillRepeatedly(
                DoAll(SetArgPointee<0>(nullptr), Return(APP_ERR_OK)));

        // 设置aclGetRecentErrMsg的默认行为
        EXPECT_CALL(*mockAcl, aclGetRecentErrMsg())
            .WillRepeatedly(Return("mock error"));
    }

    unique_ptr<StrictMock<MockACL>> mockAcl;  // 模拟ACL接口
    DeviceManager* mgr = nullptr;             // 设备管理器实例
};

TEST_F(DeviceManagerTest, GetInstanceAndIsInitDevices) {
    EXPECT_NE(mgr, nullptr);
    // Default not initialized
    EXPECT_FALSE(mgr->IsInitDevices());
}

TEST_F(DeviceManagerTest, SetAclJsonPath) {
    mgr->SetAclJsonPath("/tmp/acl.json");
    // No assert, just cover the setter
}

TEST_F(DeviceManagerTest, CheckDeviceId) {
    // deviceCount_ is private, but CheckDeviceId(-1) must fail
    EXPECT_EQ(mgr->CheckDeviceId(-1), APP_ERR_COMM_INVALID_PARAM);
    // deviceCount_ is 0 by default, so any >=0 will also fail
    EXPECT_EQ(mgr->CheckDeviceId(0), APP_ERR_COMM_INVALID_PARAM);
}

TEST_F(DeviceManagerTest, GetDevicesCount) {
    uint32_t count = 123;
    EXPECT_EQ(mgr->GetDevicesCount(count), APP_ERR_OK);
    // count is not changed by default, but function is covered
}

TEST_F(DeviceManagerTest, GetCurrentDevice) {
    DeviceContext ctx;
    // Should return APP_ERR_OK, but devId is -1 by default
    EXPECT_EQ(mgr->GetCurrentDevice(ctx), APP_ERR_OK);
    EXPECT_EQ(ctx.devId, -1);
}

TEST_F(DeviceManagerTest, ResetDevice) {
    DeviceContext ctx;
    EXPECT_EQ(mgr->ResetDevice(ctx), APP_ERR_OK);
}

TEST_F(DeviceManagerTest, SetDeviceSimple) {
    DeviceContext ctx;
    EXPECT_EQ(mgr->SetDeviceSimple(ctx), mgr->SetContext(ctx));
}

TEST_F(DeviceManagerTest, DestroyDevices_NotInit) {
    // repeatInitAclFlag is true by default, but initCounter_ is 0
    EXPECT_EQ(mgr->DestroyDevices(), APP_ERR_COMM_OUT_OF_RANGE);
}

TEST_F(DeviceManagerTest, DestroyContext_NotInit) {
    EXPECT_EQ(mgr->DestroyContext(0, 0), APP_ERR_COMM_OUT_OF_RANGE);
}

TEST_F(DeviceManagerTest, InitDevices_Fail) {
    // 设置aclInit失败
    EXPECT_CALL(*mockAcl, aclInit(_)).WillOnce(Return(APP_ERR_ACL_FAILURE));

    EXPECT_EQ(mgr->InitDevices("APP_ERR_ACL_FAILURE"), APP_ERR_ACL_FAILURE);
}

TEST_F(DeviceManagerTest, InitDevices_GetDeviceCount_Fail) {
    // 设置aclrtGetDeviceCount失败
    EXPECT_CALL(*mockAcl, aclrtGetDeviceCount(_))
        .WillOnce(Return(APP_ERR_ACL_FAILURE));

    EXPECT_EQ(mgr->InitDevices(""), APP_ERR_ACL_FAILURE);
}

TEST_F(DeviceManagerTest, InitDevices_Normal) {
    // 覆盖正常初始化流程
    EXPECT_EQ(mgr->InitDevices(""), APP_ERR_OK);
    // 再次调用，走initCounter_ > 1分支
    EXPECT_EQ(mgr->InitDevices(""), APP_ERR_OK);
}

TEST_F(DeviceManagerTest, InitDevices_RepeatInit) {
    EXPECT_EQ(mgr->InitDevices(""), APP_ERR_OK);
    EXPECT_EQ(mgr->DestroyDevices(), APP_ERR_OK);

    EXPECT_EQ(mgr->InitDevices(""), APP_ERR_OK);
    EXPECT_EQ(mgr->DestroyDevices(), APP_ERR_OK);
}

TEST_F(DeviceManagerTest, CreateContext_DeviceNotExist) {
    DeviceContext ctx;
    ctx.devId = 0;
    size_t contextIndex = 0;
    // deviceCount_ is 0 by default, so CheckDeviceId would fail, but
    // CreateContext does not check deviceCount_ Should call aclrtSetDevice and
    // succeed
    EXPECT_EQ(mgr->CreateContext(ctx, contextIndex), APP_ERR_OK);
    // contextIndex should be 0
    EXPECT_EQ(contextIndex, 0u);
}

TEST_F(DeviceManagerTest, CreateContext_RepeatCreate) {
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

TEST_F(DeviceManagerTest, CreateContext_MissingDefaultContext) {
    DeviceContext ctx;
    ctx.devId = 100;
    size_t contextIndex = 0;
    // Simulate missing nextContextIndex_ entry
    // Remove the entry after first create
    EXPECT_EQ(mgr->CreateContext(ctx, contextIndex), APP_ERR_OK);
    // Erase nextContextIndex_ entry to simulate error
    {
        std::lock_guard<std::mutex> lock(mgr->mtx_);
        mgr->nextContextIndex_.erase(ctx.devId);
    }
    // Now CreateContext should fail with APP_ERR_COMM_READ_FAIL
    size_t dummyIndex = 0;
    EXPECT_EQ(mgr->CreateContext(ctx, dummyIndex), APP_ERR_COMM_READ_FAIL);
}

TEST_F(DeviceManagerTest,
       CreateContext_GetCurrentContextWhenRepeatInitAclFlagFalse) {
    DeviceContext ctx;
    ctx.devId = 2;
    size_t contextIndex = 0;
    // Set repeatInitAclFlag to false to trigger aclrtGetCurrentContext branch
    mgr->repeatInitAclFlag = false;
    // Should succeed and contextIndex should be 0
    EXPECT_EQ(mgr->CreateContext(ctx, contextIndex), APP_ERR_OK);
    EXPECT_EQ(contextIndex, 0u);
    // Restore flag for other tests
    mgr->repeatInitAclFlag = true;
}

TEST_F(DeviceManagerTest, CreateContext_aclrtSetDeviceFail) {
    DeviceContext ctx;
    ctx.devId = 1234;
    size_t contextIndex = 0;
    // 设置aclrtSetDevice失败
    EXPECT_CALL(*mockAcl, aclrtSetDevice(_))
        .WillOnce(Return(APP_ERR_ACL_FAILURE));

    EXPECT_EQ(mgr->CreateContext(ctx, contextIndex), APP_ERR_ACL_FAILURE);
}

TEST_F(DeviceManagerTest, CreateContext_aclrtCreateContextFail) {
    DeviceContext ctx;
    ctx.devId = 3;
    size_t contextIndex = 0;
    // 设置aclrtCreateContext失败
    EXPECT_CALL(*mockAcl, aclrtCreateContext(_, _))
        .WillOnce(Return(APP_ERR_ACL_FAILURE));

    EXPECT_EQ(mgr->CreateContext(ctx, contextIndex), APP_ERR_ACL_FAILURE);
}

TEST_F(DeviceManagerTest, DestroyContext_WhenNotInitialized_ReturnsOutOfRange) {
    // Ensure not initialized
    while (mgr->IsInitDevices()) {
        mgr->DestroyDevices();
    }
    EXPECT_EQ(mgr->DestroyContext(0, 0), APP_ERR_COMM_OUT_OF_RANGE);
}

TEST_F(DeviceManagerTest, DestroyContext_DeviceIdNotExist_ReturnsOk) {
    mgr->InitDevices("");
    // No context created for device 99
    EXPECT_EQ(mgr->DestroyContext(99, 0), APP_ERR_OK);
    mgr->DestroyDevices();
}

TEST_F(DeviceManagerTest, DestroyContext_ContextIndexNotExist_ReturnsOk) {
    mgr->InitDevices("");
    DeviceContext ctx;
    ctx.devId = 0;
    size_t contextIndex = 0;
    mgr->CreateContext(ctx, contextIndex);
    // Try to destroy a non-existent context index
    EXPECT_EQ(mgr->DestroyContext(0, 12345), APP_ERR_OK);
    mgr->DestroyDevices();
}

TEST_F(DeviceManagerTest, DestroyContext_RepeatDestroy_ReturnsOk) {
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

TEST_F(DeviceManagerTest, DestroyContext_DestroyRealContext_ReturnsOk) {
    mgr->InitDevices("");
    DeviceContext ctx;
    ctx.devId = 0;
    size_t contextIndex = 0;
    mgr->CreateContext(ctx, contextIndex);
    // Should destroy the context successfully
    EXPECT_EQ(mgr->DestroyContext(0, contextIndex), APP_ERR_OK);
    mgr->DestroyDevices();
}

TEST_F(DeviceManagerTest, DestroyDevices_WhenNotInitialized_ReturnsOutOfRange) {
    // initCounter_ == 0
    APP_ERROR ret = mgr->DestroyDevices();
    EXPECT_EQ(ret, APP_ERR_COMM_OUT_OF_RANGE);
}

TEST_F(DeviceManagerTest, DestroyDevices_WhenInitializedOnce_ReturnsOk) {
    mgr->SetAclJsonPath("");  // avoid nullptr
    mgr->InitDevices("");
    APP_ERROR ret = mgr->DestroyDevices();
    EXPECT_EQ(ret, APP_ERR_OK);
}

TEST_F(DeviceManagerTest,
       DestroyDevices_WhenInitializedTwice_DecrementsCounter) {
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

TEST_F(DeviceManagerTest, DestroyDevices_RepeatInitAclFlagFalse_ReturnsOk) {
    mgr->SetAclJsonPath("");
    mgr->InitDevices("");
    // Simulate acl repeat initialize
    mgr->repeatInitAclFlag = false;
    APP_ERROR ret = mgr->DestroyDevices();
    EXPECT_EQ(ret, APP_ERR_OK);
}

}  // namespace
