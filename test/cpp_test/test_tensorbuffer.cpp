#include "Base/Tensor/TensorBuffer/TensorBuffer.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "Base/MemoryHelper/MemoryHelper.h"
#include "Base/ErrorCode/ErrorCode.h"

using namespace testing;
using namespace Base;

namespace AISBench_test {
class TensorBufferTest : public Test {
protected:
    void SetUp() override {
        // 初始化测试用的内存指针
        test_ptr = malloc(1024);
    }

    void TearDown() override {
        free(test_ptr);
    }

    void* test_ptr = nullptr;
};

// 测试不同构造函数的参数初始化
TEST_F(TensorBufferTest, ConstructorInitializesMembersCorrectly) {
    // 测试带完整参数的构造函数
    Base::TensorBuffer buf1(1024, MemoryData::MEMORY_DEVICE, 0);
    EXPECT_EQ(buf1.size, 1024);
    EXPECT_EQ(buf1.type, MemoryData::MEMORY_DEVICE);
    EXPECT_EQ(buf1.deviceId, 0);

    // 测试带size和deviceId的构造函数
    Base::TensorBuffer buf2(512, 1);
    EXPECT_EQ(buf2.size, 512);
    EXPECT_EQ(buf2.deviceId, 1);

    // 测试带指针的构造函数
    Base::TensorBuffer buf3(test_ptr, 2048);
    EXPECT_EQ(buf3.size, 2048);
    EXPECT_EQ(buf3.data.get(), test_ptr);
}

// 测试IsDevice和IsHost方法
TEST_F(TensorBufferTest, MemoryTypeDetectionWorks) {
    // 设备内存测试
    Base::TensorBuffer device_buf(1024, MemoryData::MEMORY_DEVICE, 0);
    EXPECT_TRUE(device_buf.IsDevice());
    EXPECT_FALSE(device_buf.IsHost());

    // DVPP内存测试
    Base::TensorBuffer dvpp_buf(1024, MemoryData::MEMORY_DVPP, 0);
    EXPECT_TRUE(dvpp_buf.IsDevice());

    // 主机内存测试
    Base::TensorBuffer host_buf(1024);
    host_buf.type = MemoryData::MEMORY_HOST;
    EXPECT_TRUE(host_buf.IsHost());
}

// 测试默认构造函数
TEST_F(TensorBufferTest, DefaultConstructorInitializesToZero) {
    Base::TensorBuffer default_buf;
    EXPECT_EQ(default_buf.size, 0);
    EXPECT_EQ(default_buf.deviceId, -1);
    EXPECT_EQ(default_buf.type, MemoryData::MEMORY_HOST);
}
} // namespace AISBench_test
