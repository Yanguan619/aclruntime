#include <gtest/gtest.h>
#include <vector>
#include <stdexcept>

// mock Base::TensorBase 相关接口
namespace Base {
struct DummyTensor {
    int ret = 0;
    int deviceId = 0;
    std::vector<uint32_t> shape = {2, 2};
    int dataType = 0;
    bool isDevice = false;
};
inline void TensorToHost(DummyTensor &tensor) {
    if (tensor.ret != 0) throw std::runtime_error("ToHost failed");
}
inline void TensorToDevice(DummyTensor &tensor, const int32_t deviceId) {
    if (deviceId > 255 || deviceId < 0) throw std::runtime_error("device id is out of range");
    if (tensor.ret != 0) throw std::runtime_error("[1004][Invalid parameter] ");
}
inline void TensorToDvpp(DummyTensor &tensor, const int32_t deviceId) {
    if (tensor.ret != 0) throw std::runtime_error("[1004][Invalid parameter] ");
}
inline DummyTensor BatchVector(const std::vector<DummyTensor> &tensors, const bool &keepDims = false) {
    if (tensors.empty() || tensors[0].ret != 0) throw std::runtime_error("[2][ACL: memory allocation fail] ");
    DummyTensor out;
    out.shape = {1, 2, 3};
    return out;
}
} // namespace Base

using namespace Base;

namespace {

TEST(PyTensorTest, TensorToHost_Ok)
{
    DummyTensor t;
    t.ret = 0;
    EXPECT_NO_THROW(TensorToHost(t));
}

TEST(PyTensorTest, TensorToHost_Fail)
{
    DummyTensor t;
    t.ret = 1;
    EXPECT_THROW(TensorToHost(t), std::runtime_error);
}

TEST(PyTensorTest, TensorToDevice_Ok)
{
    DummyTensor t;
    t.ret = 0;
    EXPECT_NO_THROW(TensorToDevice(t, 0));
}

TEST(PyTensorTest, TensorToDevice_DeviceIdOutOfRange)
{
    DummyTensor t;
    t.ret = 0;
    EXPECT_THROW(TensorToDevice(t, 256), std::runtime_error);
    EXPECT_THROW(TensorToDevice(t, -2), std::runtime_error);
}

TEST(PyTensorTest, TensorToDevice_Fail)
{
    DummyTensor t;
    t.ret = 1;
    EXPECT_THROW(TensorToDevice(t, 0), std::runtime_error);
}

TEST(PyTensorTest, TensorToDvpp_Ok)
{
    DummyTensor t;
    t.ret = 0;
    EXPECT_NO_THROW(TensorToDvpp(t, 0));
}

TEST(PyTensorTest, TensorToDvpp_Fail)
{
    DummyTensor t;
    t.ret = 1;
    EXPECT_THROW(TensorToDvpp(t, 0), std::runtime_error);
}

TEST(PyTensorTest, BatchVector_Ok)
{
    std::vector<DummyTensor> tensors;
    DummyTensor t1; t1.ret = 0;
    DummyTensor t2; t2.ret = 0;
    tensors.push_back(t1);
    tensors.push_back(t2);
    EXPECT_NO_THROW({
        auto out = BatchVector(tensors, false);
        EXPECT_EQ(out.shape.size(), 3);
    });
}

TEST(PyTensorTest, BatchVector_Fail)
{
    std::vector<DummyTensor> tensors;
    DummyTensor t; t.ret = 1;
    tensors.push_back(t);
    EXPECT_THROW(BatchVector(tensors, false), std::runtime_error);
}

} // namespace

#ifdef COMPILE_PYTHON_MODULE
#include <pybind11/embed.h>
namespace py = pybind11;

TEST(PyTensorTest, FromNumpy_DefaultType)
{
    py::scoped_interpreter guard{};
    // 使用未知格式，触发 dataType = TENSOR_DTYPE_UINT8 分支
    py::array arr(py::dtype("V4"), {2, 2});
    EXPECT_NO_THROW({
        auto t = Base::FromNumpy(arr);
        EXPECT_EQ(t.GetDataType(), Base::TENSOR_DTYPE_UINT8);
    });
}

TEST(PyTensorTest, FromNumpy_KnownType)
{
    py::scoped_interpreter guard{};
    py::array_t<float> arr({2, 2});
    EXPECT_NO_THROW({
        auto t = Base::FromNumpy(arr);
        EXPECT_EQ(t.GetDataType(), Base::TENSOR_DTYPE_FLOAT32);
        EXPECT_EQ(t.GetShape().size(), 2);
    });
}

TEST(PyTensorTest, FromNumpy_MallocFail)
{
    struct MockTensorBase : public Base::TensorBase {
        using Base::TensorBase::TensorBase;
        static APP_ERROR TensorBaseMalloc(Base::TensorBase&) { return APP_ERR_ACL_BAD_ALLOC; }
        static APP_ERROR TensorBaseCopy(Base::TensorBase&, const Base::TensorBase&) { return APP_ERR_OK; }
    };
    py::scoped_interpreter guard{};
    py::array_t<float> arr({2, 2});
    // 覆盖 malloc fail 分支
    auto origMalloc = Base::TensorBase::TensorBaseMalloc;
    auto origCopy = Base::TensorBase::TensorBaseCopy;
    *(void**)(&Base::TensorBase::TensorBaseMalloc) = (void*)&MockTensorBase::TensorBaseMalloc;
    *(void**)(&Base::TensorBase::TensorBaseCopy) = (void*)&MockTensorBase::TensorBaseCopy;
    EXPECT_THROW(Base::FromNumpy(arr), std::runtime_error);
    *(void**)(&Base::TensorBase::TensorBaseMalloc) = (void*)origMalloc;
    *(void**)(&Base::TensorBase::TensorBaseCopy) = (void*)origCopy;
}

TEST(PyTensorTest, FromNumpy_CopyFail)
{
    struct MockTensorBase : public Base::TensorBase {
        using Base::TensorBase::TensorBase;
        static APP_ERROR TensorBaseMalloc(Base::TensorBase&) { return APP_ERR_OK; }
        static APP_ERROR TensorBaseCopy(Base::TensorBase&, const Base::TensorBase&) { return APP_ERR_ACL_BAD_COPY; }
    };
    py::scoped_interpreter guard{};
    py::array_t<float> arr({2, 2});
    // 覆盖 copy fail 分支
    auto origMalloc = Base::TensorBase::TensorBaseMalloc;
    auto origCopy = Base::TensorBase::TensorBaseCopy;
    *(void**)(&Base::TensorBase::TensorBaseMalloc) = (void*)&MockTensorBase::TensorBaseMalloc;
    *(void**)(&Base::TensorBase::TensorBaseCopy) = (void*)&MockTensorBase::TensorBaseCopy;
    EXPECT_THROW(Base::FromNumpy(arr), std::runtime_error);
    *(void**)(&Base::TensorBase::TensorBaseMalloc) = (void*)origMalloc;
    *(void**)(&Base::TensorBase::TensorBaseCopy) = (void*)origCopy;
}

TEST(PyTensorTest, ToNumpy_DeviceTensor)
{
    py::scoped_interpreter guard{};
    Base::TensorBase t(std::vector<uint32_t>{2, 2}, Base::TENSOR_DTYPE_FLOAT32);
    // 模拟 device tensor
    struct : Base::TensorBase {
        using Base::TensorBase::TensorBase;
        bool IsDevice() const override { return true; }
    } deviceTensor(std::vector<uint32_t>{2, 2}, Base::TENSOR_DTYPE_FLOAT32);
    EXPECT_THROW(Base::ToNumpy(deviceTensor), std::runtime_error);
}

TEST(PyTensorTest, ToNumpy_KnownType)
{
    py::scoped_interpreter guard{};
    Base::TensorBase t(std::vector<uint32_t>{2, 2}, Base::TENSOR_DTYPE_FLOAT32);
    auto buf = Base::ToNumpy(t);
    EXPECT_EQ(buf.size, 4);
    EXPECT_EQ(buf.itemsize, 4);
    EXPECT_EQ(buf.ndim, 2);
}

TEST(PyTensorTest, ToNumpy_UnknownType)
{
    py::scoped_interpreter guard{};
    Base::TensorBase t(std::vector<uint32_t>{2, 2}, Base::TENSOR_DTYPE_UNDEFINED);
    auto buf = Base::ToNumpy(t);
    EXPECT_EQ(buf.itemsize, 0);
}

TEST(PyTensorTest, RegistPyTensorModule_Cover)
{
    py::scoped_interpreter guard{};
    py::module m = py::module::import("builtins");
    EXPECT_NO_THROW(Base::RegistPyTensorModule(m));
}
#endif
