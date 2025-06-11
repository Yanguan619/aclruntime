#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include "PyInferenceSession/PyInferenceSession.h"
#include "Base/ModelInfer/File.h"

using namespace Base;
using ::testing::_;
using ::testing::Return;

// 由于File类使用静态方法，这里通过宏替换实现Mock
#undef File
class MockFile {
public:
    MOCK_METHOD2(CheckFileBeforeRead, bool(const std::string&, FileType));
};

// 全局变量用于存储Mock实例
static MockFile mockFile;

// 替换原File类的静态方法为Mock方法
bool File::CheckFileBeforeRead(const std::string& path, FileType type) {
    return mockFile.CheckFileBeforeRead(path, type);
}

namespace AISBench_test {
class PyInferenceSessionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 默认有效参数
        options = std::make_shared<SessionOptions>();
        options->loop = 1;
        options->log_level = LOG_INFO_LEVEL; // 2
        options->aclJsonPath = "";

        // 默认Mock行为：有效模型和配置返回true
        testing::Mock::VerifyAndClearExpectations(&mockFile);
        ON_CALL(mockFile, CheckFileBeforeRead("valid_model.om", FileType::OM))
            .WillByDefault(Return(true));
        ON_CALL(mockFile, CheckFileBeforeRead("valid_config.json", FileType::JSON))
            .WillByDefault(Return(true));
    }


    std::shared_ptr<SessionOptions> options;
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
    EXPECT_CALL(mockFile, CheckFileBeforeRead("invalid_model.om", FileType::OM))
        .Times(1)
        .WillOnce(Return(false));
    EXPECT_THROW(PyInferenceSession("invalid_model.om", 0, options), std::runtime_error);
}

// 无效ACL配置路径测试（非空时）
TEST_F(PyInferenceSessionTest, InvalidAclJsonPath) {
    options->aclJsonPath = "invalid_config.json";
    EXPECT_CALL(mockFile, CheckFileBeforeRead("valid_model.om", FileType::OM))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(mockFile, CheckFileBeforeRead("invalid_config.json", FileType::JSON))
        .Times(1)
        .WillOnce(Return(false));
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