#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <fstream>
#include <climits>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include "Base/ModelInfer/utils.h"
#include "Base/ModelInfer/File.h"
#include "Base/Tensor/TensorBase/TensorBase.h"
#include "Base/Log/Log.h"
#include "Base/ModelInfer/cnpy.h"



// gmock方式mock File::OpenFile
class MockFileOpener {
public:
    MOCK_METHOD(bool, OpenFileIfstream, (const std::string&, std::ifstream&, std::ios_base::openmode), ());
    MOCK_METHOD(bool, OpenFileOfstream, (const std::string&, std::ofstream&, std::ios_base::openmode), ());
    MOCK_METHOD(bool, OpenFileOfstreamSimple, (const std::string&, std::ofstream&), ());
};
MockFileOpener* g_fileOpener = nullptr;

// 用函数指针替换法mock File::OpenFile
namespace {
    bool OpenFileIfstreamProxy(const std::string& path, std::ifstream& f, std::ios_base::openmode mode) {
        if (g_fileOpener) return g_fileOpener->OpenFileIfstream(path, f, mode);
        return false;
    }
    bool OpenFileOfstreamProxy(const std::string& path, std::ofstream& f, std::ios_base::openmode mode) {
        if (g_fileOpener) return g_fileOpener->OpenFileOfstream(path, f, mode);
        return false;
    }
    bool OpenFileOfstreamSimpleProxy(const std::string& path, std::ofstream& f) {
        if (g_fileOpener) return g_fileOpener->OpenFileOfstreamSimple(path, f);
        return false;
    }
}

// 替换File::OpenFile为proxy（需在utils.cpp等实现文件中用函数指针定义File::OpenFile，或用链接器替换/weak符号/LD_PRELOAD等方式）

// mock TensorBase
namespace Base {
class MockTensorBase : public TensorBase {
public:
    MOCK_METHOD(std::vector<size_t>, GetShape, (), (const));
    MOCK_METHOD(int, GetDataType, (), (const));
    MOCK_METHOD(void*, GetBuffer, (), (const));
    MOCK_METHOD(size_t, GetByteSize, (), (const));
    MOCK_METHOD(size_t, GetSize, (), (const));
};
}

// mock cnpy
namespace cnpy {
    template<typename T>
    void NpySave(const std::string& fname, T* data, std::vector<size_t>& shape, std::string mode = "w") {
        // 模拟真实行为：data为nullptr时抛异常，否则什么都不做
        if (data == nullptr) {
            throw std::runtime_error("NpySave: origin data is null");
        }
        // 可选：模拟写文件行为（如需要可添加）
    }
}

// mock aclFloat16
typedef uint16_t aclFloat16;

// mock log
#define ERROR_LOG(fmt, ...) do {} while(0)
#define DEBUG_LOG(fmt, ...) do {} while(0)
#define PROMPT_MSG(fmt, ...) do {} while(0)

using namespace UtilsResult;
using namespace std;
using ::testing::Return;
using ::testing::_;

class UtilsTest : public ::testing::Test {
protected:
    MockFileOpener fileOpener;
    void SetUp() override {
        g_fileOpener = &fileOpener;
        ON_CALL(fileOpener, OpenFileIfstream).WillByDefault(Return(true));
        ON_CALL(fileOpener, OpenFileOfstream).WillByDefault(Return(true));
        ON_CALL(fileOpener, OpenFileOfstreamSimple).WillByDefault(Return(true));
    }
    void TearDown() override {
        g_fileOpener = nullptr;
    }
};

TEST_F(UtilsTest, CreateRandomNum_Success) {
    uint8_t num = Utils::CreateRandomNum();
    ASSERT_GE(num, 0);
}

TEST_F(UtilsTest, SplitString_NormalAndEmpty) {
    std::string s = " a ,b, c ";
    std::vector<std::string> v;
    Utils::SplitString(s, v, ',');
    ASSERT_EQ(v.size(), 3);
    s = "";
    v.clear();
    Utils::SplitString(s, v, ',');
    ASSERT_EQ(v.size(), 0);
}

TEST_F(UtilsTest, ModelName_NormalAndIllegal) {
    std::string s = "/path/to/model.om";
    ASSERT_EQ(Utils::modelName(s), "model");
    std::string s2 = "model.om";
    ASSERT_EQ(Utils::modelName(s2), "model");
    std::string s3 = "illegal.om/illegal";
    ASSERT_THROW(Utils::modelName(s3), std::runtime_error);
}

TEST_F(UtilsTest, TimeLine_Format) {
    std::string t = Utils::TimeLine();
    ASSERT_EQ(t.size(), 15);
}

TEST_F(UtilsTest, PrintCurrentTime_Normal) {
    std::string t = Utils::printCurrentTime();
    ASSERT_FALSE(t.empty());
}

TEST_F(UtilsTest, PrintDiffTime_Normal) {
    time_t t1 = 100, t2 = 200;
    double diff = Utils::printDiffTime(t2, t1);
    ASSERT_DOUBLE_EQ(diff, (t2-t1)*1000);
}

TEST_F(UtilsTest, SplitStringSimple_Cover) {
    std::string s = "a:b,c;d:e,f";
    std::vector<std::string> out;
    Utils::SplitStringSimple(s, out, ';', ':', ',');
    ASSERT_EQ(out.size(), 4);
}

TEST_F(UtilsTest, SplitStringWithSemicolonsAndColons_Cover) {
    std::string s = "a:b;c:d";
    std::vector<std::string> out;
    Utils::SplitStringWithSemicolonsAndColons(s, out, ';', ':');
    ASSERT_EQ(out.size(), 2);
}

TEST_F(UtilsTest, SplitStringWithPunctuation_Cover) {
    std::string s = "a,b,c";
    std::vector<std::string> out;
    Utils::SplitStringWithPunctuation(s, out, ',');
    ASSERT_EQ(out.size(), 3);
}

TEST_F(UtilsTest, SplitStingGetNameDimsMulMap_SuccessAndFail) {
    std::vector<std::string> in = {"input:1,2,3"};
    std::map<string, int64_t> out;
    ASSERT_EQ(Utils::SplitStingGetNameDimsMulMap(in, out), SUCCESS);
    ASSERT_EQ(out["input"], 6);
    in = {"input:1,abc"};
    ASSERT_EQ(Utils::SplitStingGetNameDimsMulMap(in, out), FAILED);
    in = {"input"};
    ASSERT_EQ(Utils::SplitStingGetNameDimsMulMap(in, out), FAILED);
}

TEST_F(UtilsTest, ReadBinFileToMemory_SuccessAndFail) {
    char buf[100] = {0};
    size_t offset = 0;
    std::ofstream f("test.bin", std::ios::binary);
    f.write("1234", 4); f.close();
    chmod("test.bin", 0750); // 设置权限为750
    ASSERT_EQ(Utils::ReadBinFileToMemory("./test.bin", buf, 100, offset), SUCCESS);
    offset = 0;
    ASSERT_EQ(Utils::ReadBinFileToMemory("not_exist.bin", buf, 100, offset), FAILED);
    remove("test.bin");
}

TEST_F(UtilsTest, FillFileContentToMemory_SuccessAndFail) {
    char buf[100] = {0};
    size_t offset = 0;
    std::ofstream f("test.bin", std::ios::binary);
    f.write("1234", 4); f.close();
    chmod("test.bin", 0750); // 设置权限为750
    ASSERT_EQ(Utils::FillFileContentToMemory("./test.bin", buf, 100, offset), SUCCESS);
    offset = 0;
    ASSERT_EQ(Utils::FillFileContentToMemory("not_exist.bin", buf, 100, offset), FAILED);
    remove("test.bin");
}

TEST_F(UtilsTest, MergeStr_Cover) {
    std::vector<std::string> v = {"a", "b", "c"};
    ASSERT_EQ(Utils::MergeStr(v, ","), "a,b,c");
    v.clear();
    ASSERT_EQ(Utils::MergeStr(v, ","), "");
}

TEST_F(UtilsTest, GetPrefix_Cover) {
    ASSERT_EQ(Utils::GetPrefix("dir", "a/b/c.npy", ".npy"), "dir/c_");
    ASSERT_EQ(Utils::GetPrefix("dir", "c.bin", ".bin"), "dir/c_");
}

TEST_F(UtilsTest, RemoveSlash_Cover) {
    ASSERT_EQ(Utils::RemoveSlash("/a/b/c"), "abc");
    ASSERT_EQ(Utils::RemoveSlash("abc"), "abc");
}

TEST_F(UtilsTest, CreateDynamicShapeDims_Cover) {
    std::vector<size_t> shapes = {1,2,3};
    ASSERT_EQ(Utils::CreateDynamicShapeDims("input", shapes), "input:1,2,3");
}


TEST_F(UtilsTest, TailContain_TrueAndFalse) {
    ASSERT_TRUE(Utils::TailContain("abc.txt", ".txt"));
    ASSERT_FALSE(Utils::TailContain("abc.txt", ".bin"));
}

TEST_F(UtilsTest, IsValidInteger_TrueAndFalse) {
    ASSERT_TRUE(Utils::IsValidInteger("123"));
    ASSERT_FALSE(Utils::IsValidInteger("abc"));
    ASSERT_FALSE(Utils::IsValidInteger(""));
    ASSERT_FALSE(Utils::IsValidInteger("123abc"));
    ASSERT_FALSE(Utils::IsValidInteger(std::to_string(LONG_MAX)));
}

TEST_F(UtilsTest, SplitStringByComma_NormalAndException) {
    ASSERT_EQ(Utils::SplitStringByComma("1,2,3").size(), 3);
    std::string s(40, 'a');
    ASSERT_THROW(Utils::SplitStringByComma(s), std::runtime_error);
}

TEST_F(UtilsTest, IsDymShapeValid_TrueAndFalse) {
    ASSERT_TRUE(Utils::IsDymShapeValid("1,2,3"));
    ASSERT_FALSE(Utils::IsDymShapeValid(""));
    ASSERT_FALSE(Utils::IsDymShapeValid("0,2,3"));
    ASSERT_FALSE(Utils::IsDymShapeValid("1,2,3,4,5,6,7"));
    ASSERT_FALSE(Utils::IsDymShapeValid("1,2222222222"));
}

TEST_F(UtilsTest, IsInputNameValidChar_TrueAndFalse) {
    ASSERT_TRUE(Utils::IsInputNameValidChar("abc_./-"));
    ASSERT_FALSE(Utils::IsInputNameValidChar("abc$"));
}

TEST_F(UtilsTest, IsLegalDymString_TrueAndFalse) {
    ASSERT_TRUE(Utils::IsLegalDymString("input1:1,2,3;input2:4,5,6"));
    std::string s(5000, 'a');
    ASSERT_FALSE(Utils::IsLegalDymString(s));
    ASSERT_FALSE(Utils::IsLegalDymString("input1:"));
    ASSERT_FALSE(Utils::IsLegalDymString("input1:abc"));
    ASSERT_FALSE(Utils::IsLegalDymString("input1:1,2,3;input2:"));
    ASSERT_FALSE(Utils::IsLegalDymString("input1:1,2,3;input2:0,2,3"));
    ASSERT_FALSE(Utils::IsLegalDymString("input1$:1,2,3"));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();

}
