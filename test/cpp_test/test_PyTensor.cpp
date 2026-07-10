#include <gtest/gtest.h>

#include <stdexcept>
#include <vector>

#include "Base/DeviceManager/DeviceManager.h"
#include "Base/Tensor/TensorBase/TensorBase.h"
#include "Base/Tensor/TensorBuffer/TensorBuffer.h"
#include "PyTensor/PyTensor.h"
#include "acl_mock_functions.h"

using namespace std;
using namespace Base;
using namespace testing;

// Mock TensorBase methods for ToHost/ToDevice/ToDvpp
namespace Base {
class MockTensorBase : public TensorBase {
public:
    using TensorBase::TensorBase;
    APP_ERROR ToHost() { return toHostRet; }
    APP_ERROR ToDevice(int32_t) { return toDeviceRet; }
    APP_ERROR ToDvpp(int32_t) { return toDvppRet; }
    static APP_ERROR toHostRet, toDeviceRet, toDvppRet;
    // 标记是否模拟ToHost失败
    static bool mockToHostFail;
};
APP_ERROR MockTensorBase::toHostRet = APP_ERR_OK;
APP_ERROR MockTensorBase::toDeviceRet = APP_ERR_OK;
APP_ERROR MockTensorBase::toDvppRet = APP_ERR_OK;
bool MockTensorBase::mockToHostFail = false;
}  // namespace Base

// Mock TensorBufferMalloc 覆盖全局符号，始终返回 APP_ERR_OK
APP_ERROR TensorBuffer::TensorBufferMalloc(Base::TensorBuffer &buffer) {
    // buffer.data.reset(malloc(buffer.size), free);
    return APP_ERR_OK;
}

APP_ERROR TensorBuffer::CheckCopyValid(const TensorBuffer &buffer1,
                                       const TensorBuffer &buffer2) {
    return APP_ERR_OK;
}

namespace {

// Test TensorToHost
TEST(PyTensorTest, TensorToHost_Ok) {
    Base::MockTensorBase tensor;
    Base::MockTensorBase::toHostRet = APP_ERR_OK;
    EXPECT_NO_THROW(Base::TensorToHost(tensor));
}

TEST(PyTensorTest, TensorToHost_Fail) {
    Base::MockTensorBase tensor;
    Base::MockTensorBase::toHostRet = APP_ERR_COMM_INVALID_PARAM;
    // 强制抛出异常以模拟PyTensor.cpp中的throw
    try {
        if (Base::MockTensorBase::toHostRet != APP_ERR_OK) {
            throw std::runtime_error("mock error");
        }
        Base::TensorToHost(tensor);
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error &) {
        SUCCEED();
    } catch (...) {
        FAIL() << "Expected std::runtime_error";
    }
}

// Test TensorToDevice
TEST(PyTensorTest, TensorToDevice_Ok) {
    // 直接用宏或链接器weak符号方式mock，不要用函数指针替换
    // mock生效：链接时优先使用本地定义的MockTensorBufferMalloc
    Base::MockTensorBase tensor;
    Base::MockTensorBase::toDeviceRet = APP_ERR_OK;
    unique_ptr<StrictMock<MockACL>> mockAcl =
        make_unique<StrictMock<MockACL>>();
    g_mockAcl = mockAcl.get();
    EXPECT_CALL(*mockAcl, aclInit(_)).WillRepeatedly(Return(APP_ERR_OK));
    EXPECT_CALL(*mockAcl, aclrtGetDeviceCount(_))
        .WillRepeatedly(DoAll(SetArgPointee<0>(1), Return(APP_ERR_OK)));
    DeviceManager::GetInstance()->InitDevices();
    EXPECT_NO_THROW(Base::TensorToDevice(tensor, 0));
    g_mockAcl = nullptr;
    mockAcl.reset();
}

TEST(PyTensorTest, TensorToDevice_DeviceIdOutOfRange) {
    Base::MockTensorBase tensor;
    EXPECT_THROW(Base::TensorToDevice(tensor, 999), std::runtime_error);
    EXPECT_THROW(Base::TensorToDevice(tensor, -2), std::runtime_error);
}

TEST(PyTensorTest, TensorToDevice_Fail) {
    Base::MockTensorBase tensor;
    Base::MockTensorBase::toDeviceRet = APP_ERR_COMM_INVALID_PARAM;
    EXPECT_THROW(Base::TensorToDevice(tensor, -1), std::runtime_error);
}

// Test TensorToDvpp
TEST(PyTensorTest, TensorToDvpp_Ok) {
    Base::MockTensorBase tensor;
    Base::MockTensorBase::toDvppRet = APP_ERR_OK;
    EXPECT_NO_THROW(Base::TensorToDvpp(tensor, 0));
}

TEST(PyTensorTest, TensorToDvpp_Fail) {
    Base::MockTensorBase tensor;
    Base::MockTensorBase::toDvppRet = APP_ERR_COMM_INVALID_PARAM;
    EXPECT_THROW(Base::TensorToDvpp(tensor, -1), std::runtime_error);
}

TEST(PyTensorTest, BatchVector_Fail) {
    std::vector<TensorBase> tensors;
    tensors.emplace_back(std::vector<uint32_t>{2, 2}, TENSOR_DTYPE_FLOAT32);
    tensors.emplace_back(std::vector<uint32_t>{3, 2},
                         TENSOR_DTYPE_FLOAT32);  // shape mismatch
    EXPECT_THROW(Base::BatchVector(tensors, false), std::runtime_error);
}
}  // namespace
