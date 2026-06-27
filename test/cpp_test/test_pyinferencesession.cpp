/*
 * @Author: yanhe13 yanhe13@huawei.com
 * @Date: 2025-06-12 14:56:37
 * @LastEditors: yanhe13 yanhe13@huawei.com
 * @LastEditTime: 2025-06-12 15:31:16
 * @FilePath: \ais_bench\test\cpp_test\test_pyinferencesession.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include "PyInferenceSession/PyInferenceSession.h"
#include "Base/ModelInfer/File.h"

using namespace Base;
using ::testing::_;
using ::testing::Return;

namespace AISBench_test {
// 添加文件检查接口
class IFileChecker {
public:
    virtual ~IFileChecker() = default;
    virtual bool CheckFileBeforeRead(const std::string& path, FileType type) = 0;
};

// 真实实现（调用原始File类）
class RealFileChecker : public IFileChecker {
public:
    bool CheckFileBeforeRead(const std::string& path, FileType type) override {
        return File::CheckFileBeforeRead(path, type);
    }
};

// Mock实现
class MockFileChecker : public IFileChecker {
public:
    MOCK_METHOD2(CheckFileBeforeRead, bool(const std::string&, FileType));
};

class PyInferenceSessionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 默认有效参数
        options = std::make_shared<SessionOptions>();
        options->loop = 1;
        options->log_level = LOG_INFO_LEVEL; // 2
        options->aclJsonPath = "";

        // 创建MockFileChecker实例
        mockFileChecker = std::make_shared<MockFileChecker>();

        // 默认Mock行为：有效模型和配置返回true
        testing::Mock::VerifyAndClearExpectations(mockFileChecker.get());
        ON_CALL(*mockFileChecker, CheckFileBeforeRead("valid_model.om", FileType::OM))
            .WillByDefault(Return(true));
        ON_CALL(*mockFileChecker, CheckFileBeforeRead("valid_config.json", FileType::JSON))
            .WillByDefault(Return(true));
    }

    std::shared_ptr<SessionOptions> options;
    std::shared_ptr<MockFileChecker> mockFileChecker; // Mock实例作为成员变量
};

// loop参数越界测试
TEST_F(PyInferenceSessionTest, InvalidLoopSize) {
    // loop=0
    options->loop = 0;
    EXPECT_THROW(PyInferenceSession("valid_model.om", 0, options), std::runtime_error);

    // loop=100001（假设LOOP_MAX_SIZE=100000）
    options->loop = 100001;
    EXPECT_THROW(PyInferenceSession("valid_model.om", 0, options), std::runtime_error);
}

// log_level参数越界测试
TEST_F(PyInferenceSessionTest, InvalidLogLevel) {
    // log_level=0（低于LOG_DEBUG_LEVEL=1）
    options->log_level = 0;
    EXPECT_THROW(PyInferenceSession("valid_model.om", 0, options), std::runtime_error);

    // log_level=5（高于LOG_ERROR_LEVEL=4）
    options->log_level = 5;
    EXPECT_THROW(PyInferenceSession("valid_model.om", 0, options), std::runtime_error);
}

// 无效模型路径测试
TEST_F(PyInferenceSessionTest, InvalidModelPath) {
    EXPECT_THROW(PyInferenceSession("invalid_model.om", 0, options), std::runtime_error);
}

// 无效ACL配置路径测试（非空时）
TEST_F(PyInferenceSessionTest, InvalidAclJsonPath) {
    options->aclJsonPath = "invalid_config.json";
    EXPECT_THROW(PyInferenceSession("valid_model.om", 0, options), std::runtime_error);
}

// deviceId越界测试
TEST_F(PyInferenceSessionTest, InvalidDeviceId) {
    // deviceId=-1
    EXPECT_THROW(PyInferenceSession("valid_model.om", -1, options), std::runtime_error);

    // deviceId=256（假设DEVICE_ID_MAX=255）
    EXPECT_THROW(PyInferenceSession("valid_model.om", 256, options), std::runtime_error);
}
} // namespace AISBench_test
