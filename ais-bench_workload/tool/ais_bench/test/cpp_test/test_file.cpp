#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "test_utils.hpp"
#include "base/include/Base/ModelInfer/File.h"

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
        ASSERT_EQ(symlink(File::GetAbsPath(testRegularFile).c_str(), testLink.c_str()), 0);
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
    EXPECT_TRUE(File::IsPathExist("/"));
    EXPECT_TRUE(File::IsPathExist("."));
    EXPECT_TRUE(File::IsPathExist(testRegularFile));
    EXPECT_FALSE(File::IsPathExist(testNotExistsFile));
}

TEST_F(FileTest, TestGetAbsPath)
{
    std::string pwd = Trim(TEST_ExecShellCommand("pwd"));
    EXPECT_EQ(pwd, File::GetAbsPath("."));
    EXPECT_EQ(pwd + "/testpath", File::GetAbsPath("./testpath"));
    EXPECT_EQ(pwd + "/testpath", File::GetAbsPath("./testpath/"));
    EXPECT_EQ(pwd + "/testpath", File::GetAbsPath("./subdir/../testpath"));
    EXPECT_EQ(pwd + "/testpath", File::GetAbsPath("subdir/subdir/.././../testpath"));
    EXPECT_EQ(pwd + "/subdir/testpath", File::GetAbsPath("./subdir/.././/subdir/testpath"));
}

TEST_F(FileTest, TestIsDir)
{
    EXPECT_TRUE(File::IsDir("/"));
    EXPECT_TRUE(File::IsDir("./"));
    EXPECT_TRUE(File::IsDir(testDirSub));
    EXPECT_FALSE(File::IsDir(testRegularFile));
    EXPECT_FALSE(File::IsDir(testFifo));
}

TEST_F(FileTest, TestIsRegularFile)
{
    EXPECT_TRUE(File::IsRegularFile(testRegularFile));
    EXPECT_FALSE(File::IsRegularFile(testDirSub));
    EXPECT_TRUE(File::IsRegularFile(testLink));
    EXPECT_FALSE(File::IsRegularFile(testFifo));
    EXPECT_FALSE(File::IsRegularFile(testNotExistsFile));
}

TEST_F(FileTest, TestIsSoftLink)
{
    EXPECT_TRUE(File::IsSoftLink(testLink));
    EXPECT_FALSE(File::IsSoftLink(testDirSub));
    EXPECT_FALSE(File::IsSoftLink(testNotExistsFile));
    EXPECT_FALSE(File::IsSoftLink(testRegularFile));
    EXPECT_FALSE(File::IsSoftLink(testFifo));
}

TEST_F(FileTest, TestIsPathCharactersValid)
{
    std::string validPath = "/tmp/FileTest/testfile.txt";
    std::string invalidPath1 = "/tmp/FileTest/<>:|?*\"";
    std::string invalidPath2 = " /tmp/FileTest/testfile.txt";
    EXPECT_TRUE(File::IsPathCharactersValid("123456789"));
    EXPECT_TRUE(File::IsPathCharactersValid(validPath));
    EXPECT_FALSE(File::IsPathCharactersValid(invalidPath1));
    EXPECT_FALSE(File::IsPathCharactersValid(invalidPath2));
}

TEST_F(FileTest, TestIsFileReadable)
{
    TEST_ExecShellCommand("chmod -r " + testRegularFile);
    EXPECT_FALSE(File::IsFileReadable(testRegularFile));
    TEST_ExecShellCommand("chmod +r " + testRegularFile);
    EXPECT_TRUE(File::IsFileReadable(testRegularFile));
    TEST_ExecShellCommand("chmod -r " + testDirSub);
    EXPECT_FALSE(File::IsFileReadable(testDirSub));
    TEST_ExecShellCommand("chmod +r " + testDirSub);
    EXPECT_TRUE(File::IsFileReadable(testDirSub));
}

TEST_F(FileTest, TestIsFileWritable)
{
    TEST_ExecShellCommand("chmod -w " + testRegularFile);
    EXPECT_FALSE(File::IsFileWritable(testRegularFile));
    TEST_ExecShellCommand("chmod +w " + testRegularFile);
    EXPECT_TRUE(File::IsFileWritable(testRegularFile));
    TEST_ExecShellCommand("chmod -w " + testDirSub);
    EXPECT_FALSE(File::IsFileWritable(testDirSub));
    TEST_ExecShellCommand("chmod +w " + testDirSub);
    EXPECT_TRUE(File::IsFileWritable(testDirSub));
}

TEST_F(FileTest, TestIsFileExecutable)
{
    TEST_ExecShellCommand("chmod -x " + testRegularFile);
    EXPECT_FALSE(File::IsFileExecutable(testRegularFile));
    TEST_ExecShellCommand("chmod +x " + testRegularFile);
    EXPECT_TRUE(File::IsFileExecutable(testRegularFile));
    TEST_ExecShellCommand("chmod -x " + testDirSub);
    EXPECT_FALSE(File::IsFileExecutable(testDirSub));
    TEST_ExecShellCommand("chmod +x " + testDirSub);
    EXPECT_TRUE(File::IsFileExecutable(testDirSub));
}

TEST_F(FileTest, TestIsDirReadable)
{
    EXPECT_TRUE(".");
    EXPECT_TRUE(File::IsDirReadable(testDirSub));
    TEST_ExecShellCommand("chmod 100 " + testDirSub);
    EXPECT_FALSE(File::IsDirReadable(testDirSub));
    TEST_ExecShellCommand("chmod 400 " + testDirSub);
    EXPECT_FALSE(File::IsDirReadable(testDirSub));
    TEST_ExecShellCommand("chmod 500 " + testDirSub);
    EXPECT_TRUE(File::IsDirReadable(testDirSub));
}

TEST_F(FileTest, TestGetParentDir)
{
    EXPECT_EQ("/tmp/FileTest", File::GetParentDir("/tmp/FileTest/dir"));
    EXPECT_EQ("/tmp/FileTest", File::GetParentDir("/tmp/FileTest/"));
    EXPECT_EQ("./FileTest", File::GetParentDir("./FileTest/testfile.txt"));
    EXPECT_EQ(".", File::GetParentDir("testfile.txt"));
    EXPECT_EQ(".", File::GetParentDir(""));
}

TEST_F(FileTest, TestGetFileName)
{
    EXPECT_EQ("dir", File::GetFileName("/tmp/FileTest/dir"));
    EXPECT_EQ("", File::GetFileName("/tmp/FileTest/"));
    EXPECT_EQ("testfile.txt", File::GetFileName("./FileTest/testfile.txt"));
    EXPECT_EQ("testfile.txt", File::GetFileName("testfile.txt"));
    EXPECT_EQ("", File::GetFileName(""));
}

TEST_F(FileTest, TestGetFileSuffix)
{
    EXPECT_EQ("", File::GetFileSuffix("/tmp/FileTest/dir"));
    EXPECT_EQ("", File::GetFileSuffix("/tmp/FileTest/"));
    EXPECT_EQ("txt", File::GetFileSuffix("./FileTest/testfile.txt"));
    EXPECT_EQ("txt", File::GetFileSuffix("testfile.txt"));
    EXPECT_EQ("", File::GetFileSuffix("testfile"));
    EXPECT_EQ("", File::GetFileSuffix("testfile."));
}

TEST_F(FileTest, TestCheckFileRWX)
{
    TEST_ExecShellCommand("chmod 640 " + testRegularFile);
    EXPECT_TRUE(File::CheckFileRWX(testRegularFile, "rw"));
    EXPECT_FALSE(File::CheckFileRWX(testRegularFile, "rx"));
    TEST_ExecShellCommand("chmod 750 " + testDirSub);
    EXPECT_TRUE(File::CheckFileRWX(testDirSub, "rwx"));
}

TEST_F(FileTest, TestIsPathLengthLegal)
{
    std::string maxFile = std::string(FILE_NAME_LENGTH_MAX, 'a');
    std::string longFile = std::string(FILE_NAME_LENGTH_MAX + 1, 'a');
    std::string maxPath(FULL_PATH_LENGTH_MAX, '/');
    std::string longPath = maxPath + "/";
    EXPECT_TRUE(File::IsPathLengthLegal(maxFile));
    EXPECT_TRUE(File::IsPathLengthLegal(maxPath));
    EXPECT_FALSE(File::IsPathLengthLegal(longFile));
    EXPECT_FALSE(File::IsPathLengthLegal(longPath));
    EXPECT_FALSE(File::IsPathLengthLegal(""));
}

TEST_F(FileTest, TestIsPathDepthValid)
{
    EXPECT_TRUE(File::IsPathDepthValid(""));
    EXPECT_TRUE(File::IsPathDepthValid(std::string(PATH_DEPTH_MAX, PATH_SEPARATOR)));
    EXPECT_FALSE(File::IsPathDepthValid(std::string(PATH_DEPTH_MAX + 1, PATH_SEPARATOR)));
}

}
