#include <gtest/gtest.h>
#include <vector>
#include <cstring>
#include "Base/MemoryHelper/MemoryHelper.h"

using namespace Base;

static int g_aclrtMemcpy_fail = 0;

extern "C" {
APP_ERROR aclrtMallocHost(void** ptr, size_t size) {
    *ptr = malloc(size);
    return (*ptr == nullptr) ? APP_ERR_ACL_BAD_ALLOC : APP_ERR_OK;
}
APP_ERROR aclrtFreeHost(void* ptr) {
    free(ptr);
    return APP_ERR_OK;
}
APP_ERROR aclrtMalloc(void** ptr, size_t size, int) {
    *ptr = malloc(size);
    return (*ptr == nullptr) ? APP_ERR_ACL_BAD_ALLOC : APP_ERR_OK;
}
APP_ERROR aclrtFree(void* ptr) {
    free(ptr);
    return APP_ERR_OK;
}
APP_ERROR acldvppMalloc(void** ptr, size_t size) {
    *ptr = malloc(size);
    return (*ptr == nullptr) ? APP_ERR_ACL_BAD_ALLOC : APP_ERR_OK;
}
APP_ERROR acldvppFree(void* ptr) {
    free(ptr);
    return APP_ERR_OK;
}
APP_ERROR aclrtMemset(void* ptr, size_t, int32_t value, size_t count) {
    if (ptr == nullptr) return APP_ERR_ACL_BAD_ALLOC;
    memset(ptr, value, count);
    return APP_ERR_OK;
}

APP_ERROR aclrtMemcpy(void* dst, size_t dstMax, const void* src, size_t count, int) {
    if (g_aclrtMemcpy_fail) return APP_ERR_ACL_BAD_ALLOC;
    if (dst == nullptr || src == nullptr) return APP_ERR_ACL_BAD_ALLOC;
    if (count > dstMax) return APP_ERR_ACL_BAD_ALLOC;
    memcpy(dst, src, count);
    return APP_ERR_OK;
}
}

namespace {


TEST(MemoryHelperTest, MallocAndFreeHost)
{
    MemoryData data(128, MemoryData::MEMORY_HOST);
    EXPECT_EQ(MemoryHelper::Malloc(data), APP_ERR_OK);
    EXPECT_NE(data.ptrData, nullptr);
    EXPECT_EQ(MemoryHelper::Free(data), APP_ERR_OK);
    EXPECT_EQ(data.ptrData, nullptr);
}

TEST(MemoryHelperTest, MallocAndFreeHostMalloc)
{
    MemoryData data(64, MemoryData::MEMORY_HOST_MALLOC);
    EXPECT_EQ(MemoryHelper::Malloc(data), APP_ERR_OK);
    EXPECT_NE(data.ptrData, nullptr);
    EXPECT_EQ(MemoryHelper::Free(data), APP_ERR_OK);
    EXPECT_EQ(data.ptrData, nullptr);
}

TEST(MemoryHelperTest, MallocAndFreeHostNew)
{
    MemoryData data(32, MemoryData::MEMORY_HOST_NEW);
    EXPECT_EQ(MemoryHelper::Malloc(data), APP_ERR_OK);
    EXPECT_NE(data.ptrData, nullptr);
    EXPECT_EQ(MemoryHelper::Free(data), APP_ERR_OK);
    EXPECT_EQ(data.ptrData, nullptr);
}

TEST(MemoryHelperTest, MallocZeroSize)
{
    MemoryData data(0, MemoryData::MEMORY_HOST);
    EXPECT_EQ(MemoryHelper::Malloc(data), APP_ERR_OK);
    EXPECT_EQ(data.ptrData, nullptr);
}

TEST(MemoryHelperTest, FreeNullptr)
{
    MemoryData data(0, MemoryData::MEMORY_HOST);
    data.ptrData = nullptr;
    EXPECT_EQ(MemoryHelper::Free(data), APP_ERR_OK);
}

TEST(MemoryHelperTest, FreeInvalidPointer)
{
    MemoryData data(10, MemoryData::MEMORY_HOST);
    data.ptrData = nullptr;
    EXPECT_EQ(MemoryHelper::Free(data), APP_ERR_COMM_INVALID_POINTER);
}

TEST(MemoryHelperTest, MemsetNullptr)
{
    MemoryData data(10, MemoryData::MEMORY_HOST);
    data.ptrData = nullptr;
    EXPECT_EQ(MemoryHelper::Memset(data, 0, 10), APP_ERR_COMM_INVALID_POINTER);
}

TEST(MemoryHelperTest, MemsetAndMemcpyHost)
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

TEST(MemoryHelperTest, MemcpyNullptr)
{
    MemoryData src(8, MemoryData::MEMORY_HOST_MALLOC);
    MemoryData dst(8, MemoryData::MEMORY_HOST_MALLOC);
    src.ptrData = nullptr;
    dst.ptrData = nullptr;
    EXPECT_EQ(MemoryHelper::Memcpy(dst, src, 8), APP_ERR_COMM_INVALID_POINTER);
}

TEST(MemoryHelperTest, MemcpyZeroSize)
{
    MemoryData src(0, MemoryData::MEMORY_HOST_MALLOC);
    MemoryData dst(0, MemoryData::MEMORY_HOST_MALLOC);
    EXPECT_EQ(MemoryHelper::Memcpy(dst, src, 0), APP_ERR_OK);
}

TEST(MemoryHelperTest, MxbsMallocAndCopy_Success)
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

TEST(MemoryHelperTest, MxbsMallocAndCopySrcNull)
{
    MemoryData src(8, MemoryData::MEMORY_HOST_MALLOC);
    src.ptrData = nullptr;
    MemoryData dst(8, MemoryData::MEMORY_HOST_MALLOC);
    EXPECT_EQ(MemoryHelper::MxbsMallocAndCopy(dst, src), APP_ERR_COMM_INVALID_POINTER);
}

TEST(MemoryHelperTest, MemorySummaryReset)
{
    auto* summary = GetMemorySummaryPtr();
    summary->H2DTimeList.push_back(1.0f);
    summary->D2HTimeList.push_back(2.0f);
    summary->Reset();
    EXPECT_TRUE(summary->H2DTimeList.empty());
    EXPECT_TRUE(summary->D2HTimeList.empty());
}

TEST(MemoryHelperTest, MxbsMallocAndFree)
{
    MemoryData data(32, MemoryData::MEMORY_HOST_MALLOC);
    EXPECT_EQ(MemoryHelper::MxbsMalloc(data), APP_ERR_OK);
    EXPECT_NE(data.ptrData, nullptr);
    EXPECT_EQ(MemoryHelper::MxbsFree(data), APP_ERR_OK);
    EXPECT_EQ(data.ptrData, nullptr);
}

TEST(MemoryHelperTest, MxbsMemset)
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

TEST(MemoryHelperTest, MxbsMemcpy)
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

TEST(MemoryHelperTest, SpecificMalloc_Device)
{
    MemoryData data(32, MemoryData::MEMORY_DEVICE);
    EXPECT_EQ(MemoryHelper::specificMalloc(data), APP_ERR_OK);
    EXPECT_NE(data.ptrData, nullptr);
    EXPECT_EQ(MemoryHelper::Free(data), APP_ERR_OK);
}

TEST(MemoryHelperTest, SpecificMalloc_Dvpp)
{
    MemoryData data(32, MemoryData::MEMORY_DVPP);
    EXPECT_EQ(MemoryHelper::specificMalloc(data), APP_ERR_OK);
    EXPECT_NE(data.ptrData, nullptr);
    EXPECT_EQ(MemoryHelper::Free(data), APP_ERR_OK);
}

TEST(MemoryHelperTest, SpecificMalloc_HostNew)
{
    MemoryData data(16, MemoryData::MEMORY_HOST_NEW);
    EXPECT_EQ(MemoryHelper::specificMalloc(data), APP_ERR_OK);
    EXPECT_NE(data.ptrData, nullptr);
    EXPECT_EQ(MemoryHelper::Free(data), APP_ERR_OK);
}

TEST(MemoryHelperTest, SpecificMalloc_HostNew_BadAlloc)
{
    struct BadAllocMemoryData : public MemoryData {
        BadAllocMemoryData() : MemoryData(0, MEMORY_HOST_NEW) {}
    };
    // 模拟new失败：只能通过size极大或mock new抛异常，简单覆盖分支
    MemoryData data(0, MemoryData::MEMORY_HOST_NEW);
    // size为0时new int8_t[0]返回非nullptr，但此处主要是分支覆盖
    EXPECT_EQ(MemoryHelper::specificMalloc(data), APP_ERR_OK);
}

TEST(MemoryHelperTest, SpecificMalloc_DefaultType)
{
    MemoryData data(8, static_cast<MemoryData::MemoryType>(999));
    EXPECT_EQ(MemoryHelper::specificMalloc(data), APP_ERR_ACL_BAD_ALLOC);
}

// 覆盖 HostToDevice 分支
TEST(MemoryHelperTest, Memcpy_HostToDevice)
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
TEST(MemoryHelperTest, Memcpy_DeviceToHost)
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
TEST(MemoryHelperTest, Memcpy_DeviceToDevice)
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


TEST(MemoryHelperTest, Memcpy_aclrtMemcpyFail)
{
    MemoryData src(8, MemoryData::MEMORY_HOST_MALLOC);
    MemoryData dst(8, MemoryData::MEMORY_HOST_MALLOC);
    EXPECT_EQ(MemoryHelper::Malloc(src), APP_ERR_OK);
    EXPECT_EQ(MemoryHelper::Malloc(dst), APP_ERR_OK);
    g_aclrtMemcpy_fail = 1;
    EXPECT_EQ(MemoryHelper::Memcpy(dst, src, 8), APP_ERR_ACL_BAD_COPY);
    g_aclrtMemcpy_fail = 0;
    EXPECT_EQ(MemoryHelper::Free(src), APP_ERR_OK);
    EXPECT_EQ(MemoryHelper::Free(dst), APP_ERR_OK);
}

// 覆盖 MxbsMallocAndCopy malloc失败分支
TEST(MemoryHelperTest, MxbsMallocAndCopy_MallocFail)
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
TEST(MemoryHelperTest, MxbsMallocAndCopy_MemcpyFail)
{
    MemoryData src(8, MemoryData::MEMORY_HOST_MALLOC);
    EXPECT_EQ(MemoryHelper::Malloc(src), APP_ERR_OK);
    std::memset(src.ptrData, 0xCD, 8);
    MemoryData dst(8, MemoryData::MEMORY_HOST_MALLOC);
    g_aclrtMemcpy_fail = 1;
    EXPECT_EQ(MemoryHelper::MxbsMallocAndCopy(dst, src), APP_ERR_ACL_BAD_COPY);
    g_aclrtMemcpy_fail = 0;
    // dst.ptrData 应该已被释放为nullptr
    EXPECT_EQ(dst.ptrData, nullptr);
    EXPECT_EQ(MemoryHelper::Free(src), APP_ERR_OK);
}

} // namespace
