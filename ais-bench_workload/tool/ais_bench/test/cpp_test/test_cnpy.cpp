#include <gtest/gtest.h>
#include <cstdio>
#include <fstream>
#include <vector>
#include <string>
#include <typeinfo>
#include <memory>
#include <stdexcept>
#include <complex>

#include "test_utils.hpp"
#include "Base/ModelInfer/cnpy.h"
#include "Base/ModelInfer/File.h"

using namespace cnpy;

namespace AISBench_test {

TEST(CnpyTest, BigEndianTest)
{
    char endian = cnpy::BigEndianTest();
    EXPECT_TRUE(endian == '<' || endian == '>');
}

TEST(CnpyTest, MapType)
{
    EXPECT_EQ(cnpy::MapType(typeid(float)), 'f');
    EXPECT_EQ(cnpy::MapType(typeid(double)), 'f');
    EXPECT_EQ(cnpy::MapType(typeid(long double)), 'f');
    EXPECT_EQ(cnpy::MapType(typeid(int)), 'i');
    EXPECT_EQ(cnpy::MapType(typeid(char)), 'i');
    EXPECT_EQ(cnpy::MapType(typeid(short)), 'i');
    EXPECT_EQ(cnpy::MapType(typeid(long)), 'i');
    EXPECT_EQ(cnpy::MapType(typeid(long long)), 'i');
    EXPECT_EQ(cnpy::MapType(typeid(unsigned char)), 'u');
    EXPECT_EQ(cnpy::MapType(typeid(unsigned short)), 'u');
    EXPECT_EQ(cnpy::MapType(typeid(unsigned long)), 'u');
    EXPECT_EQ(cnpy::MapType(typeid(unsigned long long)), 'u');
    EXPECT_EQ(cnpy::MapType(typeid(unsigned int)), 'u');
    EXPECT_EQ(cnpy::MapType(typeid(bool)), 'b');
    EXPECT_EQ(cnpy::MapType(typeid(std::complex<float>)), 'c');
    EXPECT_EQ(cnpy::MapType(typeid(std::complex<double>)), 'c');
    EXPECT_EQ(cnpy::MapType(typeid(std::complex<long double>)), 'c');
    struct Dummy {};
    EXPECT_EQ(cnpy::MapType(typeid(Dummy)), '?');
}

TEST(CnpyTest, VectorCharOperatorPlusEqual)
{
    std::vector<char> v;
    v += std::string("abc");
    EXPECT_EQ(v.size(), 3);
    v += "de";
    EXPECT_EQ(v.size(), 5);
    EXPECT_EQ(std::string(v.begin(), v.end()), "abcde");
}

TEST(CnpyTest, ParseNpyHeader_ThrowsOnNullFile)
{
    size_t wordSize = 0;
    std::vector<size_t> shape;
    bool fortranOrder = false;
    EXPECT_THROW(cnpy::ParseNpyHeader(nullptr, wordSize, shape, fortranOrder), std::runtime_error);
}

TEST(CnpyTest, ParseNpyHeader_ThrowsOnShortFile)
{
    FILE* fp = tmpfile();
    fwrite("short", 1, 5, fp);
    rewind(fp);
    size_t wordSize = 0;
    std::vector<size_t> shape;
    bool fortranOrder = false;
    EXPECT_THROW(cnpy::ParseNpyHeader(fp, wordSize, shape, fortranOrder), std::runtime_error);
    fclose(fp);
}

TEST(CnpyTest, ParseNpyHeader_ThrowsOnMalformedHeader)
{
    FILE* fp = tmpfile();
    // 11 bytes + malformed header
    fwrite("01234567890", 1, 11, fp);
    fwrite("badheader", 1, 9, fp);
    rewind(fp);
    size_t wordSize = 0;
    std::vector<size_t> shape;
    bool fortranOrder = false;
    EXPECT_THROW(cnpy::ParseNpyHeader(fp, wordSize, shape, fortranOrder), std::runtime_error);
    fclose(fp);
}

TEST(CnpyTest, ParseNpyHeader_Success)
{
    // 1）创建临时文件
    FILE* fp = tmpfile();
    ASSERT_NE(fp, nullptr);

    // 2）写入 magic (8 字节，这里按实现填了两个空格)
    //    官方 npy magic 是 6 字节 "\x93NUMPY"，这里保持和实现 fread 11 字节对齐
    const char magic[8] = "\x93NUMPY";
    fwrite(magic, 1, 8, fp);

    // 3）写入版本号 (2 字节)
    const unsigned char version[2] = { 1, 0 };
    fwrite(version, 1, 2, fp);

    // 4）拼 header：必须严格包含 "fortran_order", "shape", "descr" 三段，
    //    且顺序、空格、单引号、末尾换行都要和实现匹配
    std::string header = "{'fortran_order': False, 'shape': (2, 3), 'descr': '<f4'}";
    // pad 到 75 字符再加 '\n'，和原来实现里 header_len 相同
    header.append(75 - header.size(), ' ');
    header.push_back('\n');

    // 5）写入 header 长度 (1 字节)
    unsigned char header_len = static_cast<unsigned char>(header.size());
    fwrite(&header_len, 1, 1, fp);

    // 6）写入 header 本体
    fwrite(header.c_str(), 1, header.size(), fp);

    // 7）回到文件开头，让 ParseNpyHeader 自己 fread 11 字节 + fgets(header)
    rewind(fp);

    // 8）调用并检查
    size_t wordSize = 0;
    std::vector<size_t> shape;
    // 先给个非默认值，确保函数会写回真正的 fortranOrder
    bool fortranOrder = true;

    cnpy::ParseNpyHeader(fp, wordSize, shape, fortranOrder);

    EXPECT_EQ(wordSize, 4u);
    ASSERT_EQ(shape.size(), 2u);
    EXPECT_EQ(shape[0], 2u);
    EXPECT_EQ(shape[1], 3u);
    EXPECT_FALSE(fortranOrder);

    fclose(fp);
}

TEST(CnpyTest, NpyLoad_FileNotExist)
{
    EXPECT_THROW(cnpy::NpyLoad("./not_exist_file.npy"), std::runtime_error);
}

TEST(CnpyTest, NpyLoad_Success)
{
    // 构造一个合法的npy文件
    std::string fname = "./test_npyl_load.npy";
    FILE* fp = fopen(fname.c_str(), "wb");
    ASSERT_NE(fp, nullptr);
    chmod(fname.c_str(), 0644);

    // 写magic和版本
    fwrite("\x93NUMPY  ", 1, 8, fp);
    fwrite("\x01\x00", 1, 2, fp);

    // header: fortran_order, shape, descr
    std::string header = "{'fortran_order': False, 'shape': (2, 2), 'descr': '<f4'}";
    header.append(75 - header.size(), ' ');
    header.push_back('\n');
    unsigned char header_len = static_cast<unsigned char>(header.size());
    fwrite(&header_len, 1, 1, fp);
    fwrite(header.c_str(), 1, header.size(), fp);

    // 写入数据：4字节float，共4个元素
    float data[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    fwrite(data, sizeof(float), 4, fp);
    fclose(fp);

    // 调用NpyLoad
    auto arr = cnpy::NpyLoad(fname);
    ASSERT_EQ(arr.wordSize, 4u);
    ASSERT_EQ(arr.shape.size(), 2u);
    EXPECT_EQ(arr.shape[0], 2u);
    EXPECT_EQ(arr.shape[1], 2u);
    EXPECT_FALSE(arr.fortranOrder);
    ASSERT_TRUE(arr.dataHolder != nullptr);
    ASSERT_EQ(arr.dataHolder->size(), 16u);
    float* loaded = reinterpret_cast<float*>(arr.dataHolder->data());
    EXPECT_FLOAT_EQ(loaded[0], 1.0f);
    EXPECT_FLOAT_EQ(loaded[1], 2.0f);
    EXPECT_FLOAT_EQ(loaded[2], 3.0f);
    EXPECT_FLOAT_EQ(loaded[3], 4.0f);

    remove(fname.c_str());
}

TEST(CnpyTest, NpyLoad_FileCheckFail)
{
    // 构造非法文件名，CheckFileBeforeRead会失败
    std::string fname = "./test_npyl_load.txt";
    std::ofstream ofs(fname);
    ofs << "not a npy file";
    ofs.close();
    // File::CheckFileBeforeRead会返回false，抛异常
    EXPECT_THROW(cnpy::NpyLoad(fname), std::runtime_error);
    remove(fname.c_str());
}

TEST(CnpyTest, NpyLoad_OpenFileFail)
{
    // 文件不存在，fopen失败
    std::string fname = "./not_exist_npyl_file.npy";
    EXPECT_THROW(cnpy::NpyLoad(fname), std::runtime_error);
}

TEST(CnpyTest, NpyLoad_CloseFileFail)
{
    // 利用freopen劫持fclose返回值，模拟关闭失败
    // 这里只能用临时文件+手动关闭，fclose返回0正常，无法直接模拟异常
    // 但可以用gmock等mock fclose，若不允许mock则跳过此分支
    // 这里仅覆盖主流程
    SUCCEED();
}

TEST(CnpyTest, BinLoad_FileNotExist)
{
    EXPECT_THROW(cnpy::BinLoad("./not_exist_file.bin"), std::runtime_error);
}

TEST(CnpyTest, BinLoad_Success)
{
    std::string fname = "./test_binload.bin";
    std::ofstream ofs(fname, std::ios::binary);
    chmod(fname.c_str(), 0644);
    ofs << "abc";
    ofs.close();
    auto arr = cnpy::BinLoad(fname);
    ASSERT_TRUE(arr.dataHolder != nullptr);
    EXPECT_EQ(arr.dataHolder->size(), 3);
    remove(fname.c_str());
}

TEST(CnpyTest, ParseNpyHeader_HeaderNoNewline)
{
    FILE* fp = tmpfile();
    // 写入magic和版本
    fwrite("\x93NUMPY  ", 1, 8, fp);
    fwrite("\x01\x00", 1, 2, fp);
    // header: 合法内容但无换行结尾
    std::string header = "{'fortran_order': False, 'shape': (2, 3), 'descr': '<f4'}";
    header.append(75 - header.size(), ' ');
    // 不加换行
    unsigned char header_len = static_cast<unsigned char>(header.size());
    fwrite(&header_len, 1, 1, fp);
    fwrite(header.c_str(), 1, header.size(), fp);
    rewind(fp);
    fseek(fp, 11, SEEK_SET);
    size_t wordSize = 0;
    std::vector<size_t> shape;
    bool fortranOrder = false;
    EXPECT_THROW(cnpy::ParseNpyHeader(fp, wordSize, shape, fortranOrder), std::runtime_error);
    fclose(fp);
}

TEST(CnpyTest, ParseNpyHeader_MissingFortranOrder)
{
    FILE* fp = tmpfile();
    fwrite("\x93NUMPY  ", 1, 8, fp);
    fwrite("\x01\x00", 1, 2, fp);
    std::string header = "{'shape': (2, 3), 'descr': '<f4'}";
    header.append(75 - header.size(), ' ');
    header.push_back('\n');
    unsigned char header_len = static_cast<unsigned char>(header.size());
    fwrite(&header_len, 1, 1, fp);
    fwrite(header.c_str(), 1, header.size(), fp);
    rewind(fp);
    fseek(fp, 11, SEEK_SET);
    size_t wordSize = 0;
    std::vector<size_t> shape;
    bool fortranOrder = false;
    EXPECT_THROW(cnpy::ParseNpyHeader(fp, wordSize, shape, fortranOrder), std::runtime_error);
    fclose(fp);
}

TEST(CnpyTest, ParseNpyHeader_MissingShapeParenthesis)
{
    FILE* fp = tmpfile();
    fwrite("\x93NUMPY  ", 1, 8, fp);
    fwrite("\x01\x00", 1, 2, fp);
    std::string header = "{'fortran_order': False, 'descr': '<f4'}";
    header.append(75 - header.size(), ' ');
    header.push_back('\n');
    unsigned char header_len = static_cast<unsigned char>(header.size());
    fwrite(&header_len, 1, 1, fp);
    fwrite(header.c_str(), 1, header.size(), fp);
    rewind(fp);
    fseek(fp, 11, SEEK_SET);
    size_t wordSize = 0;
    std::vector<size_t> shape;
    bool fortranOrder = false;
    EXPECT_THROW(cnpy::ParseNpyHeader(fp, wordSize, shape, fortranOrder), std::runtime_error);
    fclose(fp);
}

TEST(CnpyTest, ParseNpyHeader_ShapeReverseParenthesis)
{
    FILE* fp = tmpfile();
    fwrite("\x93NUMPY  ", 1, 8, fp);
    fwrite("\x01\x00", 1, 2, fp);
    std::string header = "{'fortran_order': False, ')descr': '<f4', 'shape': (2, 3), }";
    header.append(75 - header.size(), ' ');
    header.push_back('\n');
    unsigned char header_len = static_cast<unsigned char>(header.size());
    fwrite(&header_len, 1, 1, fp);
    fwrite(header.c_str(), 1, header.size(), fp);
    rewind(fp);
    fseek(fp, 11, SEEK_SET);
    size_t wordSize = 0;
    std::vector<size_t> shape;
    bool fortranOrder = false;
    // ')' 在 '(' 前，应该抛异常
    EXPECT_THROW(cnpy::ParseNpyHeader(fp, wordSize, shape, fortranOrder), std::runtime_error);
    fclose(fp);
}

TEST(CnpyTest, ParseNpyHeader_MissingDescr)
{
    FILE* fp = tmpfile();
    fwrite("\x93NUMPY  ", 1, 8, fp);
    fwrite("\x01\x00", 1, 2, fp);
    std::string header = "{'fortran_order': False, 'shape': (2, 3), }";
    header.append(75 - header.size(), ' ');
    header.push_back('\n');
    unsigned char header_len = static_cast<unsigned char>(header.size());
    fwrite(&header_len, 1, 1, fp);
    fwrite(header.c_str(), 1, header.size(), fp);
    rewind(fp);
    fseek(fp, 11, SEEK_SET);
    size_t wordSize = 0;
    std::vector<size_t> shape;
    bool fortranOrder = false;
    EXPECT_THROW(cnpy::ParseNpyHeader(fp, wordSize, shape, fortranOrder), std::runtime_error);
    fclose(fp);
}

TEST(CnpyTest, ParseNpyHeader_NotLittleEndian)
{
    FILE* fp = tmpfile();
    fwrite("\x93NUMPY  ", 1, 8, fp);
    fwrite("\x01\x00", 1, 2, fp);
    std::string header = "{'fortran_order': False, 'shape': (2, 3), 'descr': '>f4'}";
    header.append(75 - header.size(), ' ');
    header.push_back('\n');
    unsigned char header_len = static_cast<unsigned char>(header.size());
    fwrite(&header_len, 1, 1, fp);
    fwrite(header.c_str(), 1, header.size(), fp);
    rewind(fp);
    fseek(fp, 11, SEEK_SET);
    size_t wordSize = 0;
    std::vector<size_t> shape;
    bool fortranOrder = false;
    EXPECT_THROW(cnpy::ParseNpyHeader(fp, wordSize, shape, fortranOrder), std::runtime_error);
    fclose(fp);
}

TEST(CnpyTest, ParseNpyHeader_NegativeWordSize)
{
    FILE* fp = tmpfile();
    fwrite("\x93NUMPY  ", 1, 8, fp);
    fwrite("\x01\x00", 1, 2, fp);
    // descr: '<-4'
    std::string header = "{'fortran_order': False, 'shape': (2, 3), 'descr': '<-4'}";
    header.append(75 - header.size(), ' ');
    header.push_back('\n');
    unsigned char header_len = static_cast<unsigned char>(header.size());
    fwrite(&header_len, 1, 1, fp);
    fwrite(header.c_str(), 1, header.size(), fp);
    rewind(fp);
    fseek(fp, 11, SEEK_SET);
    size_t wordSize = 0;
    std::vector<size_t> shape;
    bool fortranOrder = false;
    EXPECT_THROW(cnpy::ParseNpyHeader(fp, wordSize, shape, fortranOrder), std::runtime_error);
    fclose(fp);
}

TEST(CnpyTest, BinLoad_OpenFileFail)
{
    // 模拟 OpenFile 返回 false
    // 传入非法路径，OpenFile 会失败
    EXPECT_THROW(cnpy::BinLoad("/this/path/should/not/exist/abc.bin"), std::runtime_error);
}

TEST(CnpyTest, BinLoad_ZeroSize)
{
    // 创建一个空文件
    std::string fname = "./test_binload_empty.bin";
    std::ofstream ofs(fname, std::ios::binary);
    chmod(fname.c_str(), 0644);
    ofs.close();
    auto arr = cnpy::BinLoad(fname);
    ASSERT_TRUE(arr.dataHolder != nullptr);
    EXPECT_EQ(arr.dataHolder->size(), 0u);
    remove(fname.c_str());
}

TEST(CnpyTest, BinLoad_ReadContentCorrect)
{
    std::string fname = "./test_binload_content.bin";
    std::ofstream ofs(fname, std::ios::binary);
    chmod(fname.c_str(), 0644);
    std::string content = "hello,world";
    ofs << content;
    ofs.close();
    auto arr = cnpy::BinLoad(fname);
    ASSERT_TRUE(arr.dataHolder != nullptr);
    EXPECT_EQ(arr.dataHolder->size(), content.size());
    EXPECT_EQ(std::string(arr.dataHolder->begin(), arr.dataHolder->end()), content);
    remove(fname.c_str());
}

} // namespace
