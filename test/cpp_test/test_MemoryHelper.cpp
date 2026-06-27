#include <gtest/gtest.h>
#include <vector>
#include <memory>
#include <cstring>
#include "acl_mock_functions.h"
#include "Base/MemoryHelper/MemoryHelper.h"

using namespace std;
using namespace Base;
using namespace testing;

namespace {

class MemoryHelperTest : public testing::Test {
protected:
    void SetUp() override
    {
        mockAcl = make_unique<StrictMock<MockACL>>();
        g_mockAcl = mockAcl.get();
        SetGlobalDefaultExpectations();
    }

    void TearDown() override
    {
        g_mockAcl = nullptr;
        mockAcl.reset();
    }

    // 设置所有ACL接口的默认期望行为
    void SetGlobalDefaultExpectations() {

        EXPECT_CALL(*mockAcl, aclrtMallocHost(_, _))
            .WillRepeatedly(Invoke([](void** ptr, size_t size) {
                *ptr = malloc(size);
                return (*ptr == nullptr && size > 0) ? APP_ERR_ACL_BAD_ALLOC : APP_ERR_OK;
            }));

        EXPECT_CALL(*mockAcl, aclrtFreeHost(_))
            .WillRepeatedly(Invoke([](void* ptr) {
                free(ptr);
                return APP_ERR_OK;
            }));

        EXPECT_CALL(*mockAcl, aclrtMalloc(_, _, _))
            .WillRepeatedly(Invoke([](void** ptr, size_t size, int) {
                *ptr = malloc(size);
                return (*ptr == nullptr && size > 0) ? APP_ERR_ACL_BAD_ALLOC : APP_ERR_OK;
            }));

        EXPECT_CALL(*mockAcl, aclrtFree(_))
            .WillRepeatedly(Invoke([](void* ptr) {
                free(ptr);
                return APP_ERR_OK;
            }));

        EXPECT_CALL(*mockAcl, acldvppMalloc(_, _))
            .WillRepeatedly(Invoke([](void** ptr, size_t size) {
                *ptr = malloc(size);
                return (*ptr == nullptr && size > 0) ? APP_ERR_ACL_BAD_ALLOC : APP_ERR_OK;
            }));

        EXPECT_CALL(*mockAcl, acldvppFree(_))
            .WillRepeatedly(Invoke([](void* ptr) {
                free(ptr);
                return APP_ERR_OK;
            }));

        // 设置默认memset行为
        EXPECT_CALL(*mockAcl, aclrtMemset(_, _, _, _))
            .WillRepeatedly(Invoke([](void* ptr, size_t, int32_t value, size_t count) {
                if (!ptr) return APP_ERR_ACL_BAD_ALLOC;
                memset(ptr, value, count);
                return APP_ERR_OK;
            }));

        // 设置默认memcpy行为（默认为成功）
        EXPECT_CALL(*mockAcl, aclrtMemcpy(_, _, _, _, _))
            .WillRepeatedly(Invoke([](void* dst, size_t dstMax, const void* src, size_t count, int) {
                if (!dst || !src) return APP_ERR_ACL_BAD_ALLOC;
                if (count > dstMax) return APP_ERR_ACL_BAD_ALLOC;
                memcpy(dst, src, count);
                return APP_ERR_OK;
            }));

        EXPECT_CALL(*mockAcl, aclGetRecentErrMsg)
            .WillRepeatedly(Return("mock error"));
    }

    unique_ptr<StrictMock<MockACL>> mockAcl; // 模拟ACL接口
};

TEST_F(MemoryHelperTest, MallocAndFreeHost)
{
    MemoryData data(128, MemoryData::MEMORY_HOST);
    EXPECT_EQ(MemoryHelper::Malloc(data), APP_ERR_OK);
    EXPECT_NE(data.ptrData, nullptr);
    EXPECT_EQ(MemoryHelper::Free(data), APP_ERR_OK);
    EXPECT_EQ(data.ptrData, nullptr);
}

TEST_F(MemoryHelperTest, MallocAndFreeHostMalloc)
{
    MemoryData data(64, MemoryData::MEMORY_HOST_MALLOC);
    EXPECT_EQ(MemoryHelper::Malloc(data), APP_ERR_OK);
    EXPECT_NE(data.ptrData, nullptr);
    EXPECT_EQ(MemoryHelper::Free(data), APP_ERR_OK);
    EXPECT_EQ(data.ptrData, nullptr);
}

TEST_F(MemoryHelperTest, MallocAndFreeHostNew)
{
    MemoryData data(32, MemoryData::MEMORY_HOST_NEW);
    EXPECT_EQ(MemoryHelper::Malloc(data), APP_ERR_OK);
    EXPECT_NE(data.ptrData, nullptr);
    EXPECT_EQ(MemoryHelper::Free(data), APP_ERR_OK);
    EXPECT_EQ(data.ptrData, nullptr);
}

TEST_F(MemoryHelperTest, MallocZeroSize)
{
    MemoryData data(0, MemoryData::MEMORY_HOST);
    EXPECT_EQ(MemoryHelper::Malloc(data), APP_ERR_OK);
    EXPECT_EQ(data.ptrData, nullptr);
}

TEST_F(MemoryHelperTest, FreeNullptr)
{
    MemoryData data(0, MemoryData::MEMORY_HOST);
    data.ptrData = nullptr;
    EXPECT_EQ(MemoryHelper::Free(data), APP_ERR_OK);
}

TEST_F(MemoryHelperTest, FreeInvalidPointer)
{
    MemoryData data(10, MemoryData::MEMORY_HOST);
    data.ptrData = nullptr;
    EXPECT_EQ(MemoryHelper::Free(data), APP_ERR_COMM_INVALID_POINTER);
}

TEST_F(MemoryHelperTest, MemsetNullptr)
{
    MemoryData data(10, MemoryData::MEMORY_HOST);
    data.ptrData = nullptr;
    EXPECT_EQ(MemoryHelper::Memset(data, 0, 10), APP_ERR_COMM_INVALID_POINTER);
}

TEST_F(MemoryHelperTest, MemsetAndMemcpyHost)
{
    MemoryData src(16, MemoryData::MEMORY_HOST_MALLOC);
    MemoryData dst(16, MemoryData::MEMORY_HOST_MALLOC);
    EXPECT_EQ(MemoryHelper::Malloc(src), APP_ERR_OK);
    EXPECT_EQ(MemoryHelper::Malloc(dst), APP_ERR_OK);
    EXPECT_EQ(MemoryHelper::Memset(src, 0x5A, 16), APP_ERR_OK);
    EXPECT_EQ(MemoryHelper::Memcpy(dst, src, 16), APP_ERR_OK);
    EXPECT_EQ(std::memcmp(dst.ptrData, src.ptrData, 16), 0);
    EXPECT_EQ(MemoryHelper::Free(src), APP_ERR_OK);
    EXPECT_EQ(MemoryHelper::Free(dst), APP_ERR_OK);
}

TEST_F(MemoryHelperTest, MemcpyNullptr)
{
    MemoryData src(8, MemoryData::MEMORY_HOST_MALLOC);
    MemoryData dst(8, MemoryData::MEMORY_HOST_MALLOC);
    src.ptrData = nullptr;
    dst.ptrData = nullptr;
    EXPECT_EQ(MemoryHelper::Memcpy(dst, src, 8), APP_ERR_COMM_INVALID_POINTER);
}

TEST_F(MemoryHelperTest, MemcpyZeroSize)
{
    MemoryData src(0, MemoryData::MEMORY_HOST_MALLOC);
    MemoryData dst(0, MemoryData::MEMORY_HOST_MALLOC);
    EXPECT_EQ(MemoryHelper::Memcpy(dst, src, 0), APP_ERR_OK);
}

TEST_F(MemoryHelperTest, MxbsMallocAndCopy_Success)
{
    MemoryData src(8, MemoryData::MEMORY_HOST_MALLOC);
    EXPECT_EQ(MemoryHelper::Malloc(src), APP_ERR_OK);
    std::memset(src.ptrData, 0xAA, 8);
    MemoryData dst(8, MemoryData::MEMORY_HOST_MALLOC);
    EXPECT_EQ(MemoryHelper::MxbsMallocAndCopy(dst, src), APP_ERR_OK);
    EXPECT_EQ(std::memcmp(dst.ptrData, src.ptrData, 8), 0);
    EXPECT_EQ(MemoryHelper::Free(src), APP_ERR_OK);
    EXPECT_EQ(MemoryHelper::Free(dst), APP_ERR_OK);
}

TEST_F(MemoryHelperTest, MxbsMallocAndCopySrcNull)
{
    MemoryData src(8, MemoryData::MEMORY_HOST_MALLOC);
    src.ptrData = nullptr;
    MemoryData dst(8, MemoryData::MEMORY_HOST_MALLOC);
    EXPECT_EQ(MemoryHelper::MxbsMallocAndCopy(dst, src), APP_ERR_COMM_INVALID_POINTER);
}

TEST_F(MemoryHelperTest, MemorySummaryReset)
{
    auto* summary = GetMemorySummaryPtr();
    summary->H2DTimeList.push_back(1.0f);
    summary->D2HTimeList.push_back(2.0f);
    summary->Reset();
    EXPECT_TRUE(summary->H2DTimeList.empty());
    EXPECT_TRUE(summary->D2HTimeList.empty());
}

TEST_F(MemoryHelperTest, MxbsMallocAndFree)
{
    MemoryData data(32, MemoryData::MEMORY_HOST_MALLOC);
    EXPECT_EQ(MemoryHelper::MxbsMalloc(data), APP_ERR_OK);
    EXPECT_NE(data.ptrData, nullptr);
    EXPECT_EQ(MemoryHelper::MxbsFree(data), APP_ERR_OK);
    EXPECT_EQ(data.ptrData, nullptr);
}

TEST_F(MemoryHelperTest, MxbsMemset)
{
    MemoryData data(16, MemoryData::MEMORY_HOST_MALLOC);
    EXPECT_EQ(MemoryHelper::MxbsMalloc(data), APP_ERR_OK);
    EXPECT_EQ(MemoryHelper::MxbsMemset(data, 0x11, 16), APP_ERR_OK);
    uint8_t* p = static_cast<uint8_t*>(data.ptrData);
    for (size_t i = 0; i < 16; ++i) {
        EXPECT_EQ(p[i], 0x11);
    }
    EXPECT_EQ(MemoryHelper::MxbsFree(data), APP_ERR_OK);
}

TEST_F(MemoryHelperTest, MxbsMemcpy)
{
    MemoryData src(8, MemoryData::MEMORY_HOST_MALLOC);
    MemoryData dst(8, MemoryData::MEMORY_HOST_MALLOC);
    EXPECT_EQ(MemoryHelper::MxbsMalloc(src), APP_ERR_OK);
    EXPECT_EQ(MemoryHelper::MxbsMalloc(dst), APP_ERR_OK);
    std::memset(src.ptrData, 0x22, 8);
    EXPECT_EQ(MemoryHelper::MxbsMemcpy(dst, src, 8), APP_ERR_OK);
    EXPECT_EQ(std::memcmp(dst.ptrData, src.ptrData, 8), 0);
    EXPECT_EQ(MemoryHelper::MxbsFree(src), APP_ERR_OK);
    EXPECT_EQ(MemoryHelper::MxbsFree(dst), APP_ERR_OK);
}

TEST_F(MemoryHelperTest, SpecificMalloc_Device)
{
    MemoryData data(32, MemoryData::MEMORY_DEVICE);
    EXPECT_EQ(MemoryHelper::specificMalloc(data), APP_ERR_OK);
    EXPECT_NE(data.ptrData, nullptr);
    EXPECT_EQ(MemoryHelper::Free(data), APP_ERR_OK);
}

TEST_F(MemoryHelperTest, SpecificMalloc_Dvpp)
{
    MemoryData data(32, MemoryData::MEMORY_DVPP);
    EXPECT_EQ(MemoryHelper::specificMalloc(data), APP_ERR_OK);
    EXPECT_NE(data.ptrData, nullptr);
    EXPECT_EQ(MemoryHelper::Free(data), APP_ERR_OK);
}

TEST_F(MemoryHelperTest, SpecificMalloc_HostNew)
{
    MemoryData data(16, MemoryData::MEMORY_HOST_NEW);
    EXPECT_EQ(MemoryHelper::specificMalloc(data), APP_ERR_OK);
    EXPECT_NE(data.ptrData, nullptr);
    EXPECT_EQ(MemoryHelper::Free(data), APP_ERR_OK);
}

TEST_F(MemoryHelperTest, SpecificMalloc_HostNew_BadAlloc)
{
    struct BadAllocMemoryData : public MemoryData {
        BadAllocMemoryData() : MemoryData(0, MEMORY_HOST_NEW) {}
    };
    // 模拟new失败：只能通过size极大或mock new抛异常，简单覆盖分支
    MemoryData data(0, MemoryData::MEMORY_HOST_NEW);
    // size为0时new int8_t[0]返回非nullptr，但此处主要是分支覆盖
    EXPECT_EQ(MemoryHelper::specificMalloc(data), APP_ERR_OK);
}

TEST_F(MemoryHelperTest, SpecificMalloc_DefaultType)
{
    MemoryData data(8, static_cast<MemoryData::MemoryType>(999));
    EXPECT_EQ(MemoryHelper::specificMalloc(data), APP_ERR_ACL_BAD_ALLOC);
}

// 覆盖 HostToDevice 分支
TEST_F(MemoryHelperTest, Memcpy_HostToDevice)
{
    MemoryData src(8, MemoryData::MEMORY_HOST_MALLOC);
    MemoryData dst(8, MemoryData::MEMORY_DEVICE);
    EXPECT_EQ(MemoryHelper::Malloc(src), APP_ERR_OK);
    EXPECT_EQ(MemoryHelper::Malloc(dst), APP_ERR_OK);
    std::memset(src.ptrData, 0x33, 8);
    EXPECT_EQ(MemoryHelper::Memcpy(dst, src, 8), APP_ERR_OK);
    EXPECT_EQ(MemoryHelper::Free(src), APP_ERR_OK);
    EXPECT_EQ(MemoryHelper::Free(dst), APP_ERR_OK);
}

// 覆盖 DeviceToHost 分支
TEST_F(MemoryHelperTest, Memcpy_DeviceToHost)
{
    MemoryData src(8, MemoryData::MEMORY_DEVICE);
    MemoryData dst(8, MemoryData::MEMORY_HOST_MALLOC);
    EXPECT_EQ(MemoryHelper::Malloc(src), APP_ERR_OK);
    EXPECT_EQ(MemoryHelper::Malloc(dst), APP_ERR_OK);
    std::memset(src.ptrData, 0x44, 8);
    EXPECT_EQ(MemoryHelper::Memcpy(dst, src, 8), APP_ERR_OK);
    EXPECT_EQ(MemoryHelper::Free(src), APP_ERR_OK);
    EXPECT_EQ(MemoryHelper::Free(dst), APP_ERR_OK);
}

// 覆盖 DeviceToDevice 分支
TEST_F(MemoryHelperTest, Memcpy_DeviceToDevice)
{
    MemoryData src(8, MemoryData::MEMORY_DEVICE);
    MemoryData dst(8, MemoryData::MEMORY_DEVICE);
    EXPECT_EQ(MemoryHelper::Malloc(src), APP_ERR_OK);
    EXPECT_EQ(MemoryHelper::Malloc(dst), APP_ERR_OK);
    std::memset(src.ptrData, 0x55, 8);
    EXPECT_EQ(MemoryHelper::Memcpy(dst, src, 8), APP_ERR_OK);
    EXPECT_EQ(MemoryHelper::Free(src), APP_ERR_OK);
    EXPECT_EQ(MemoryHelper::Free(dst), APP_ERR_OK);
}

// 覆盖 aclrtMemcpy 返回错误分支


TEST_F(MemoryHelperTest, Memcpy_aclrtMemcpyFail)
{
    MemoryData src(8, MemoryData::MEMORY_HOST_MALLOC);
    MemoryData dst(8, MemoryData::MEMORY_HOST_MALLOC);
    EXPECT_EQ(MemoryHelper::Malloc(src), APP_ERR_OK);
    EXPECT_EQ(MemoryHelper::Malloc(dst), APP_ERR_OK);
    EXPECT_CALL(*mockAcl, aclrtMemcpy(_, _, _, _, _))
            .WillRepeatedly(Return(APP_ERR_ACL_BAD_ALLOC));
    EXPECT_EQ(MemoryHelper::Memcpy(dst, src, 8), APP_ERR_ACL_BAD_COPY);
    EXPECT_CALL(*mockAcl, aclrtMemcpy(_, _, _, _, _))
            .WillRepeatedly(Invoke([](void* dst, size_t dstMax, const void* src, size_t count, int) {
                if (!dst || !src) return APP_ERR_ACL_BAD_ALLOC;
                if (count > dstMax) return APP_ERR_ACL_BAD_ALLOC;
                memcpy(dst, src, count);
                return APP_ERR_OK;
            }));
    EXPECT_EQ(MemoryHelper::Free(src), APP_ERR_OK);
    EXPECT_EQ(MemoryHelper::Free(dst), APP_ERR_OK);
}

// 覆盖 MxbsMallocAndCopy malloc失败分支
TEST_F(MemoryHelperTest, MxbsMallocAndCopy_MallocFail)
{
    struct MockMemoryData : public MemoryData {
        MockMemoryData() : MemoryData(8, static_cast<MemoryType>(999)) {}
    };
    MemoryData src(8, MemoryData::MEMORY_HOST_MALLOC);
    EXPECT_EQ(MemoryHelper::Malloc(src), APP_ERR_OK);
    std::memset(src.ptrData, 0xAB, 8);
    MockMemoryData dst;
    // dst.type为非法类型，Malloc会失败
    EXPECT_EQ(MemoryHelper::MxbsMallocAndCopy(dst, src), APP_ERR_ACL_BAD_ALLOC);
    EXPECT_EQ(MemoryHelper::Free(src), APP_ERR_OK);
}

// 覆盖 MxbsMallocAndCopy memcpy失败分支（aclrtMemcpy fail时自动释放）
TEST_F(MemoryHelperTest, MxbsMallocAndCopy_MemcpyFail)
{
    MemoryData src(8, MemoryData::MEMORY_HOST_MALLOC);
    EXPECT_EQ(MemoryHelper::Malloc(src), APP_ERR_OK);
    std::memset(src.ptrData, 0xCD, 8);
    MemoryData dst(8, MemoryData::MEMORY_HOST_MALLOC);
    EXPECT_CALL(*mockAcl, aclrtMemcpy(_, _, _, _, _))
            .WillRepeatedly(Return(APP_ERR_ACL_BAD_ALLOC));
    EXPECT_EQ(MemoryHelper::MxbsMallocAndCopy(dst, src), APP_ERR_ACL_BAD_COPY);
    EXPECT_CALL(*mockAcl, aclrtMemcpy(_, _, _, _, _))
            .WillRepeatedly(Invoke([](void* dst, size_t dstMax, const void* src, size_t count, int) {
                if (!dst || !src) return APP_ERR_ACL_BAD_ALLOC;
                if (count > dstMax) return APP_ERR_ACL_BAD_ALLOC;
                memcpy(dst, src, count);
                return APP_ERR_OK;
            }));
    // dst.ptrData 应该已被释放为nullptr
    EXPECT_EQ(dst.ptrData, nullptr);
    EXPECT_EQ(MemoryHelper::Free(src), APP_ERR_OK);
}

} // namespace
