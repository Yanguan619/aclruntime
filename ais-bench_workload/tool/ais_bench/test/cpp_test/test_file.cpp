#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "test_utils.hpp"
#include "Base/ModelInfer/File.h"

namespace AISBench_test {

class FileTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建目录
        ASSERT_EQ(mkdir(testDir.c_str(), 0750), 0);
        ASSERT_EQ(mkdir(testDirSub.c_str(), 0750), 0);
        // 创建文件
        std::ofstream file(testRegularFile);
        file.close();
        // 创建符号链接
        ASSERT_EQ(symlink(GetAbsPath(testRegularFile).c_str(), testLink.c_str()), 0);
        ASSERT_EQ(mkfifo(testFifo.c_str(), 0640), 0);
    }

    void TearDown() override {
        // 删除测试目录和文件
        TEST_ExecShellCommand("rm -rf " + testDir);
    }

    const std::string testDir = "./FileTest";
    const std::string testDirSub = testDir + "/subdir";
    const std::string testRegularFile = testDir + "/RegularFile.txt";
    const std::string testNotExistsFile = testDir + "/NotExistsFile.txt";
    const std::string testLink = testDir + "/testlink";
    const std::string testFifo = testDir + "/testfifo";
};

TEST_F(FileTest, TestIsPathExist)
{
    EXPECT_TRUE(IsPathExist("/"));
    EXPECT_TRUE(IsPathExist("."));
    EXPECT_TRUE(IsPathExist(testRegularFile));
    EXPECT_FALSE(IsPathExist(testNotExistsFile));
}

TEST_F(FileTest, TestGetAbsPath)
{
    std::string pwd = Trim(TEST_ExecShellCommand("pwd"));
    EXPECT_EQ(pwd, GetAbsPath("."));
    EXPECT_EQ(pwd + "/testpath", GetAbsPath("./testpath"));
    EXPECT_EQ(pwd + "/testpath", GetAbsPath("./testpath/"));
    EXPECT_EQ(pwd + "/testpath", GetAbsPath("./subdir/../testpath"));
    EXPECT_EQ(pwd + "/testpath", GetAbsPath("subdir/subdir/.././../testpath"));
    EXPECT_EQ(pwd + "/subdir/testpath", GetAbsPath("./subdir/.././/subdir/testpath"));
}

TEST_F(FileTest, TestIsDir)
{
    EXPECT_TRUE(IsDir("/"));
    EXPECT_TRUE(IsDir("./"));
    EXPECT_TRUE(IsDir(testDirSub));
    EXPECT_FALSE(IsDir(testRegularFile));
    EXPECT_FALSE(IsDir(testFifo));
}

TEST_F(FileTest, TestIsRegularFile)
{
    EXPECT_TRUE(IsRegularFile(testRegularFile));
    EXPECT_FALSE(IsRegularFile(testDirSub));
    EXPECT_TRUE(IsRegularFile(testLink));
    EXPECT_FALSE(IsRegularFile(testFifo));
    EXPECT_FALSE(IsRegularFile(testNotExistsFile));
}

TEST_F(FileTest, TestIsSoftLink)
{
    EXPECT_TRUE(IsSoftLink(testLink));
    EXPECT_FALSE(IsSoftLink(testDirSub));
    EXPECT_FALSE(IsSoftLink(testNotExistsFile));
    EXPECT_FALSE(IsSoftLink(testRegularFile));
    EXPECT_FALSE(IsSoftLink(testFifo));
}

TEST_F(FileTest, TestIsPathCharactersValid)
{
    std::string validPath = "/tmp/FileTest/testfile.txt";
    std::string invalidPath1 = "/tmp/FileTest/<>:|?*\"";
    std::string invalidPath2 = " /tmp/FileTest/testfile.txt";
    EXPECT_TRUE(IsPathCharactersValid("123456789"));
    EXPECT_TRUE(IsPathCharactersValid(validPath));
    EXPECT_FALSE(IsPathCharactersValid(""));
    EXPECT_FALSE(IsPathCharactersValid(invalidPath1));
    EXPECT_FALSE(IsPathCharactersValid(invalidPath2));
}

TEST_F(FileTest, TestIsFileReadable)
{
    TEST_ExecShellCommand("chmod -r " + testRegularFile);
    EXPECT_FALSE(IsFileReadable(testRegularFile));
    TEST_ExecShellCommand("chmod +r " + testRegularFile);
    EXPECT_TRUE(IsFileReadable(testRegularFile));
    TEST_ExecShellCommand("chmod -r " + testDirSub);
    EXPECT_FALSE(IsFileReadable(testDirSub));
    TEST_ExecShellCommand("chmod +r " + testDirSub);
    EXPECT_TRUE(IsFileReadable(testDirSub));
}

TEST_F(FileTest, TestIsFileWritable)
{
    TEST_ExecShellCommand("chmod -w " + testRegularFile);
    EXPECT_FALSE(IsFileWritable(testRegularFile));
    TEST_ExecShellCommand("chmod +w " + testRegularFile);
    EXPECT_TRUE(IsFileWritable(testRegularFile));
    TEST_ExecShellCommand("chmod -w " + testDirSub);
    EXPECT_FALSE(IsFileWritable(testDirSub));
    TEST_ExecShellCommand("chmod +w " + testDirSub);
    EXPECT_TRUE(IsFileWritable(testDirSub));
}

TEST_F(FileTest, TestIsFileExecutable)
{
    TEST_ExecShellCommand("chmod -x " + testRegularFile);
    EXPECT_FALSE(IsFileExecutable(testRegularFile));
    TEST_ExecShellCommand("chmod +x " + testRegularFile);
    EXPECT_TRUE(IsFileExecutable(testRegularFile));
    TEST_ExecShellCommand("chmod -x " + testDirSub);
    EXPECT_FALSE(IsFileExecutable(testDirSub));
    TEST_ExecShellCommand("chmod +x " + testDirSub);
    EXPECT_TRUE(IsFileExecutable(testDirSub));
}

TEST_F(FileTest, TestIsDirReadable)
{
    EXPECT_TRUE(".");
    EXPECT_TRUE(IsDirReadable(testDirSub));
    TEST_ExecShellCommand("chmod 100 " + testDirSub);
    EXPECT_FALSE(IsDirReadable(testDirSub));
    TEST_ExecShellCommand("chmod 400 " + testDirSub);
    EXPECT_FALSE(IsDirReadable(testDirSub));
    TEST_ExecShellCommand("chmod 500 " + testDirSub);
    EXPECT_TRUE(IsDirReadable(testDirSub));
}

TEST_F(FileTest, TestGetParentDir)
{
    EXPECT_EQ("/tmp/FileTest", GetParentDir("/tmp/FileTest/dir"));
    EXPECT_EQ("/tmp/FileTest", GetParentDir("/tmp/FileTest/"));
    EXPECT_EQ("./FileTest", GetParentDir("./FileTest/testfile.txt"));
    EXPECT_EQ(".", GetParentDir("testfile.txt"));
    EXPECT_EQ(".", GetParentDir(""));
}

TEST_F(FileTest, TestGetFileName)
{
    EXPECT_EQ("dir", GetFileName("/tmp/FileTest/dir"));
    EXPECT_EQ("", GetFileName("/tmp/FileTest/"));
    EXPECT_EQ("testfile.txt", GetFileName("./FileTest/testfile.txt"));
    EXPECT_EQ("testfile.txt", GetFileName("testfile.txt"));
    EXPECT_EQ("", GetFileName(""));
}

TEST_F(FileTest, TestGetFileSuffix)
{
    EXPECT_EQ("", GetFileSuffix("/tmp/FileTest/dir"));
    EXPECT_EQ("", GetFileSuffix("/tmp/FileTest/"));
    EXPECT_EQ("txt", GetFileSuffix("./FileTest/testfile.txt"));
    EXPECT_EQ("txt", GetFileSuffix("testfile.txt"));
    EXPECT_EQ("", GetFileSuffix("testfile"));
    EXPECT_EQ("", GetFileSuffix("testfile."));
}

TEST_F(FileTest, TestCheckFileRWX)
{
    TEST_ExecShellCommand("chmod 640 " + testRegularFile);
    EXPECT_TRUE(CheckFileRWX(testRegularFile, "rw"));
    EXPECT_FALSE(CheckFileRWX(testRegularFile, "rx"));
    TEST_ExecShellCommand("chmod 750 " + testDirSub);
    EXPECT_TRUE(CheckFileRWX(testDirSub, "rwx"));
}

TEST_F(FileTest, TestIsPathLengthLegal)
{
    std::string maxFile = std::string(FILE_NAME_LENGTH_MAX, 'a');
    std::string longFile = std::string(FILE_NAME_LENGTH_MAX + 1, 'a');
    std::string maxPath(FULL_PATH_LENGTH_MAX, '/');
    std::string longPath = maxPath + "/";
    EXPECT_TRUE(IsPathLengthLegal(maxFile));
    EXPECT_TRUE(IsPathLengthLegal(maxPath));
    EXPECT_FALSE(IsPathLengthLegal(longFile));
    EXPECT_FALSE(IsPathLengthLegal(longPath));
    EXPECT_FALSE(IsPathLengthLegal(""));
}

TEST_F(FileTest, TestIsPathDepthValid)
{
    EXPECT_TRUE(IsPathDepthValid(""));
    EXPECT_TRUE(IsPathDepthValid(std::string(PATH_DEPTH_MAX, PATH_SEPARATOR)));
    EXPECT_FALSE(IsPathDepthValid(std::string(PATH_DEPTH_MAX + 1, PATH_SEPARATOR)));
}

}
