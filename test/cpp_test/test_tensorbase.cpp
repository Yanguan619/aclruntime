#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include "Base/Tensor/TensorBase/TensorBase.h"
#include "Base/Tensor/TensorBuffer/TensorBuffer.h"
#include "Base/Tensor/TensorShape/TensorShape.h"

namespace {
const uint32_t ZERO_BYTE = 0;
const uint32_t ONE_BYTE = 1;
const uint32_t TWO_BYTE = 2;
const uint32_t FOUR_BYTE = 4;
const uint32_t EIGHT_BYTE = 8;

const std::map<Base::TensorDataType, uint32_t> DATA_TYPE_TO_BYTE_SIZE_MAP = {
    {Base::TENSOR_DTYPE_UNDEFINED, ZERO_BYTE},
    {Base::TENSOR_DTYPE_UINT8, ONE_BYTE},
    {Base::TENSOR_DTYPE_INT8, ONE_BYTE},
    {Base::TENSOR_DTYPE_UINT16, TWO_BYTE},
    {Base::TENSOR_DTYPE_INT16, TWO_BYTE},
    {Base::TENSOR_DTYPE_UINT32, FOUR_BYTE},
    {Base::TENSOR_DTYPE_INT32, FOUR_BYTE},
    {Base::TENSOR_DTYPE_UINT64, EIGHT_BYTE},
    {Base::TENSOR_DTYPE_INT64, EIGHT_BYTE},
    {Base::TENSOR_DTYPE_FLOAT16, TWO_BYTE},
    {Base::TENSOR_DTYPE_FLOAT32, FOUR_BYTE},
    {Base::TENSOR_DTYPE_DOUBLE64, EIGHT_BYTE},
    {Base::TENSOR_DTYPE_BOOL, ONE_BYTE}
};

const std::map<Base::TensorDataType, std::string> DATA_TYPE_TO_STRING_MAP = {
    {Base::TENSOR_DTYPE_UNDEFINED, "undefined"},
    {Base::TENSOR_DTYPE_UINT8, "uint8"},
    {Base::TENSOR_DTYPE_INT8, "int8"},
    {Base::TENSOR_DTYPE_UINT16, "uint16"},
    {Base::TENSOR_DTYPE_INT16, "int16"},
    {Base::TENSOR_DTYPE_UINT32, "uint32"},
    {Base::TENSOR_DTYPE_INT32, "int32"},
    {Base::TENSOR_DTYPE_UINT64, "uint64"},
    {Base::TENSOR_DTYPE_INT64, "int64"},
    {Base::TENSOR_DTYPE_FLOAT16, "float16"},
    {Base::TENSOR_DTYPE_FLOAT32, "float32"},
    {Base::TENSOR_DTYPE_DOUBLE64, "double64"},
    {Base::TENSOR_DTYPE_BOOL, "bool"}
};
}


using namespace Base;
// 假设 MemoryData 和 MemoryHelper 类已经定义
class MockMemoryHelper {
public:
    MOCK_METHOD(APP_ERROR, MxbsMalloc, (Base::MemoryData&), ());
    MOCK_METHOD(APP_ERROR, MxbsMemcpy, (Base::MemoryData&, const Base::MemoryData&, uint64_t), ());
    MOCK_METHOD(void, Free, (const Base::MemoryData&), ());
};

class TensorBaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        mockMemoryHelper_ = std::make_shared<MockMemoryHelper>();
    }

    std::shared_ptr<MockMemoryHelper> mockMemoryHelper_;
};

TEST_F(TensorBaseTest, DefaultConstructor) {
    Base::TensorBase tensor;
    EXPECT_EQ(tensor.GetShape().size(), 0);
    EXPECT_EQ(tensor.GetBuffer(), nullptr);
}

TEST_F(TensorBaseTest, ConstructorWithMemoryData) {
    Base::MemoryData memoryData(nullptr, 1024, Base::MemoryData::MemoryType::MEMORY_HOST, 0);
    std::vector<uint32_t> shape = {1, 2, 3};
    Base::TensorDataType type = Base::TENSOR_DTYPE_FLOAT32;
    size_t contextIndex = 0;

    EXPECT_CALL(*mockMemoryHelper_, Free(testing::_)).Times(testing::AtLeast(0));

    Base::TensorBase tensor(memoryData, false, shape, type, contextIndex);
    EXPECT_EQ(tensor.GetShape(), shape);
    EXPECT_EQ(tensor.GetDataType(), type);
}

TEST_F(TensorBaseTest, ConstructorWithShapeAndType) {
    std::vector<uint32_t> shape = {1, 2, 3};
    Base::TensorDataType type = Base::TENSOR_DTYPE_FLOAT32;

    Base::TensorBase tensor(shape, type);
    EXPECT_EQ(tensor.GetShape(), shape);
    EXPECT_EQ(tensor.GetDataType(), type);
}

TEST_F(TensorBaseTest, ConstructorWithShapeTypeBufferTypeDeviceIdContextIndex) {
    std::vector<uint32_t> shape = {1, 2, 3};
    Base::TensorDataType type = Base::TENSOR_DTYPE_FLOAT32;
    Base::MemoryData::MemoryType bufferType = Base::MemoryData::MemoryType::MEMORY_DEVICE;
    int32_t deviceId = 0;
    size_t contextIndex = 0;

    Base::TensorBase tensor(shape, type, bufferType, deviceId, contextIndex);
    EXPECT_EQ(tensor.GetShape(), shape);
    EXPECT_EQ(tensor.GetDataType(), type);
    EXPECT_EQ(tensor.GetDeviceId(), deviceId);
}

TEST_F(TensorBaseTest, ConstructorWithShapeTypeDeviceIdContextIndex) {
    std::vector<uint32_t> shape = {1, 2, 3};
    Base::TensorDataType type = Base::TENSOR_DTYPE_FLOAT32;
    int32_t deviceId = 0;
    size_t contextIndex = 0;

    Base::TensorBase tensor(shape, type, deviceId, contextIndex);
    EXPECT_EQ(tensor.GetShape(), shape);
    EXPECT_EQ(tensor.GetDeviceId(), deviceId);
}

TEST_F(TensorBaseTest, ConstructorWithShape) {
    std::vector<uint32_t> shape = {1, 2, 3};

    Base::TensorBase tensor(shape);
    EXPECT_EQ(tensor.GetShape(), shape);
}
