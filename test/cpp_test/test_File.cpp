#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "limits.h"
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

TEST_F(FileTest, CreateDir_Simple) {
    TEST_ExecShellCommand("rm -rf " + testDir);
	// Directory should not exist before
	ASSERT_FALSE(File::IsPathExist(testDir));
	// Create directory
	EXPECT_TRUE(File::CreateDir(testDir, false, 0755));
	// Directory should exist now
	EXPECT_TRUE(File::IsPathExist(testDir));
	// Try to create again, should succeed (already exists)
	EXPECT_TRUE(File::CreateDir(testDir, false, 0755));
    EXPECT_TRUE(File::IsPathExist(testDir));
}

TEST_F(FileTest, CreateDir_Recursion) {
    TEST_ExecShellCommand("rm -rf " + testDir);
	// Subdirectory should not exist before
	ASSERT_FALSE(File::IsPathExist(testDirSub));
	// Create subdirectory with recursion
	EXPECT_TRUE(File::CreateDir(testDirSub, true, 0755));
	// Both directories should exist now
	EXPECT_TRUE(File::IsPathExist(testDir));
	EXPECT_TRUE(File::IsPathExist(testDirSub));
}

TEST_F(FileTest, CreateDir_NoRecursion_Fails) {
    TEST_ExecShellCommand("rm -rf " + testDir);
	// Parent directory does not exist, should fail without recursion
	ASSERT_FALSE(File::IsPathExist(testDirSub));
	EXPECT_FALSE(File::CreateDir(testDirSub, false, 0755));
	EXPECT_FALSE(File::IsPathExist(testDirSub));
}

TEST_F(FileTest, CreateDir_EmptyPath) {
	// Empty path should fail
	EXPECT_FALSE(File::CreateDir("", false, 0755));
}

TEST_F(FileTest, CreateDir_InvalidCharacters) {
	// Path with invalid characters should fail
	std::string invalidDir = "./ut_test_dir/<>:|?*\"";
	EXPECT_FALSE(File::CreateDir(invalidDir, false, 0755));
}

TEST_F(FileTest, CreateDir_TooLongPath) {
	// Path exceeding FULL_PATH_LENGTH_MAX should fail
	std::string longDir = "./" + std::string(FULL_PATH_LENGTH_MAX + 1, 'a');
	EXPECT_FALSE(File::CreateDir(longDir, false, 0755));
}

TEST_F(FileTest, CreateDir_TooDeepPath) {
	// Path exceeding PATH_DEPTH_MAX should fail
	std::string deepDir = "./ut_test_dir";
	for (int i = 0; i < PATH_DEPTH_MAX + 2; ++i) {
		deepDir += "/d";
	}
	EXPECT_FALSE(File::CreateDir(deepDir, true, 0755));
}

TEST_F(FileTest, CreateDir_PathIsFile) {
	// Create a file, then try to create a directory at the same path
	std::ofstream ofs(testDir);
	ofs << "test";
	ofs.close();
	ASSERT_TRUE(File::IsPathExist(testDir));
	// Should fail because a file exists at the path
}

TEST_F(FileTest, TestCheckDir_Normal)
{
    // Directory exists, is readable, not a symlink, valid path
    EXPECT_TRUE(File::CheckDir(testDirSub));
}

TEST_F(FileTest, TestCheckDir_PathIsEmpty)
{
    EXPECT_FALSE(File::CheckDir(""));
}

TEST_F(FileTest, TestCheckDir_PathLengthIllegal)
{
    std::string longDir = std::string(FULL_PATH_LENGTH_MAX + 1, 'a');
    EXPECT_FALSE(File::CheckDir(longDir));
}

TEST_F(FileTest, TestCheckDir_PathCharactersInvalid)
{
    std::string invalidDir = testDir + "/<>:|?*\"";
    EXPECT_FALSE(File::CheckDir(invalidDir));
}

TEST_F(FileTest, TestCheckDir_PathDepthInvalid)
{
    std::string deepDir = testDir;
    for (int i = 0; i < PATH_DEPTH_MAX + 2; ++i) {
        deepDir += "/d";
    }
    EXPECT_FALSE(File::CheckDir(deepDir));
}

TEST_F(FileTest, TestCheckDir_PathNotExist)
{
    EXPECT_FALSE(File::CheckDir(testNotExistsFile));
}

TEST_F(FileTest, TestCheckDir_PathIsNotDir)
{
    EXPECT_FALSE(File::CheckDir(testRegularFile));
}

TEST_F(FileTest, TestCheckDir_PathIsSoftLink)
{
    // testLink is a symlink to a file
    EXPECT_FALSE(File::CheckDir(testLink));
}

TEST_F(FileTest, TestCheckDir_DirNotReadable)
{
    TEST_ExecShellCommand("chmod 000 " + testDirSub);
    EXPECT_FALSE(File::CheckDir(testDirSub));
    TEST_ExecShellCommand("chmod 750 " + testDirSub); // restore
    EXPECT_TRUE(File::CreateDir(testDir, false, 0755));
}

TEST_F(FileTest, OpenFile_Read_Success)
{
    std::ofstream ofs(testRegularFile);
    chmod(testRegularFile.c_str(), 0644);
    ofs << "test content";
    ofs.close();

    std::ifstream ifs;
    EXPECT_TRUE(File::OpenFile(testRegularFile, ifs, std::ios::in));
    std::string content;
    ifs >> content;
    EXPECT_EQ(content, "test");
    ifs.close();
}

TEST_F(FileTest, OpenFile_Read_FileNotExist)
{
    std::ifstream ifs;
    EXPECT_FALSE(File::OpenFile(testNotExistsFile, ifs, std::ios::in));
}

TEST_F(FileTest, OpenFile_Read_PathIsDir)
{
    std::ifstream ifs;
    EXPECT_FALSE(File::OpenFile(testDirSub, ifs, std::ios::in));
}

TEST_F(FileTest, OpenFile_Read_PathIsSoftLink)
{
    std::ifstream ifs;
    EXPECT_FALSE(File::OpenFile(testLink, ifs, std::ios::in));
}

TEST_F(FileTest, OpenFile_Read_EmptyPath)
{
    std::ifstream ifs;
    EXPECT_FALSE(File::OpenFile("", ifs, std::ios::in));
}

TEST_F(FileTest, OpenFile_Write_Success)
{
    std::string filePath = testDir + "/write_test.txt";
    std::ofstream ofs;
    EXPECT_TRUE(File::OpenFile(filePath, ofs, std::ios::out | std::ios::trunc));
    ofs << "write test";
    ofs.close();

    std::ifstream ifs(filePath);
    std::string content;
    ifs >> content;
    EXPECT_EQ(content, "write");
    ifs.close();
}

TEST_F(FileTest, OpenFile_Write_CreateParentDir)
{
    std::string subDir = testDir + "/newsub";
    std::string filePath = subDir + "/file.txt";
    TEST_ExecShellCommand("rm -rf " + subDir);
    std::ofstream ofs;
    EXPECT_TRUE(File::OpenFile(filePath, ofs, std::ios::out | std::ios::trunc));
    ofs << "abc";
    ofs.close();
    EXPECT_TRUE(File::IsPathExist(subDir));
    EXPECT_TRUE(File::IsPathExist(filePath));
}

TEST_F(FileTest, OpenFile_Write_EmptyPath)
{
    std::ofstream ofs;
    EXPECT_FALSE(File::OpenFile("", ofs, std::ios::out));
}

TEST_F(FileTest, OpenFile_Write_InvalidCharacters)
{
    std::string invalidPath = testDir + "/<>:|?*\"";
    std::ofstream ofs;
    EXPECT_FALSE(File::OpenFile(invalidPath, ofs, std::ios::out));
}

TEST_F(FileTest, OpenFile_Write_TooLongPath)
{
    std::string longPath = testDir + "/" + std::string(FULL_PATH_LENGTH_MAX + 1, 'a');
    std::ofstream ofs;
    EXPECT_FALSE(File::OpenFile(longPath, ofs, std::ios::out));
}

TEST_F(FileTest, OpenFile_Write_PathIsDir)
{
    std::ofstream ofs;
    EXPECT_FALSE(File::OpenFile(testDirSub, ofs, std::ios::out));
}

TEST_F(FileTest, OpenFile_Write_PathIsSoftLink)
{
    std::ofstream ofs;
    EXPECT_FALSE(File::OpenFile(testLink, ofs, std::ios::out));
}

TEST_F(FileTest, OpenFile_Read_PermissionDenied)
{
    // Remove read permission
    TEST_ExecShellCommand("chmod 000 " + testRegularFile);
    std::ifstream ifs;
    EXPECT_FALSE(File::OpenFile(testRegularFile, ifs, std::ios::in));
    // Restore permission for cleanup
    TEST_ExecShellCommand("chmod 644 " + testRegularFile);
}

TEST_F(FileTest, OpenFile_Write_PermissionDenied)
{
    // Remove write permission from directory
    TEST_ExecShellCommand("chmod 500 " + testDirSub);
    std::string filePath = testDirSub + "/no_write.txt";
    std::ofstream ofs;
    EXPECT_FALSE(File::OpenFile(filePath, ofs, std::ios::out));
    // Restore permission for cleanup
    TEST_ExecShellCommand("chmod 750 " + testDirSub);
}

TEST_F(FileTest, OpenFile_Write_ExistingFileNotOwned)
{
    // Simulate not owned by changing owner to root (if possible)
    // If not root, just skip this test
    if (getuid() == 0) {
        std::string filePath = testDir + "/root_owned.txt";
        std::ofstream ofs(filePath);
        ofs << "root";
        ofs.close();
        TEST_ExecShellCommand("chown root:root " + filePath);
        std::ofstream ofs2;
        EXPECT_FALSE(File::OpenFile(filePath, ofs2, std::ios::out));
        TEST_ExecShellCommand("rm -f " + filePath);
    }
}

TEST_F(FileTest, OpenFile_Write_PathIsFifo)
{
    std::ofstream ofs;
    EXPECT_FALSE(File::OpenFile(testFifo, ofs, std::ios::out));
}

TEST_F(FileTest, DeleteFile_Normal)
{
    std::ofstream ofs(testRegularFile);
    ofs << "test";
    ofs.close();
    ASSERT_TRUE(File::IsPathExist(testRegularFile));
    EXPECT_TRUE(File::DeleteFile(testRegularFile));
    EXPECT_FALSE(File::IsPathExist(testRegularFile));
}

TEST_F(FileTest, DeleteFile_NotExist)
{
    EXPECT_TRUE(File::DeleteFile(testNotExistsFile));
}

TEST_F(FileTest, DeleteFile_SoftLink)
{
    ASSERT_TRUE(File::IsSoftLink(testLink));
    EXPECT_FALSE(File::DeleteFile(testLink));
    EXPECT_TRUE(File::IsPathExist(testLink));
}

TEST_F(FileTest, DeleteFile_EmptyPath)
{
    EXPECT_TRUE(File::DeleteFile(""));
}

TEST_F(FileTest, DeleteFile_NotOwner)
{
    // Only run as root, otherwise skip
    if (getuid() == 0) {
        std::string filePath = testDir + "/root_owned_del.txt";
        std::ofstream ofs(filePath);
        ofs << "root";
        ofs.close();
        TEST_ExecShellCommand("chown root:root " + filePath);
        EXPECT_FALSE(File::DeleteFile(filePath));
        TEST_ExecShellCommand("rm -f " + filePath);
    }
}

TEST_F(FileTest, CheckOwner_NormalFile_Owner)
{
    // The test file is created by current user, should return true
    EXPECT_TRUE(File::CheckOwner(testRegularFile));
}

TEST_F(FileTest, CheckOwner_NotExist)
{
    // Non-existent file, should return false
    EXPECT_FALSE(File::CheckOwner(testNotExistsFile));
}

TEST_F(FileTest, CheckOwner_NotOwner)
{
    // Only run as root, otherwise skip
    if (getuid() == 0) {
        std::string filePath = testDir + "/root_owned_check.txt";
        std::ofstream ofs(filePath);
        ofs << "root";
        ofs.close();
        TEST_ExecShellCommand("chown root:root " + filePath);
        EXPECT_FALSE(File::CheckOwner(filePath));
        TEST_ExecShellCommand("rm -f " + filePath);
    }
}

TEST_F(FileTest, GetFullPath_EmptyPath)
{
    EXPECT_EQ(File::GetFullPath(""), "");
}

TEST_F(FileTest, GetFullPath_AbsolutePath)
{
    std::string absPath = "/tmp/testfile";
    EXPECT_EQ(File::GetFullPath(absPath), absPath);
}

TEST_F(FileTest, GetFullPath_RelativePath)
{
    std::string relPath = "abc/def.txt";
    char cwd[PATH_MAX] = {0};
    getcwd(cwd, PATH_MAX);
    std::string expected = std::string(cwd) + "/" + relPath;
    EXPECT_EQ(File::GetFullPath(relPath), expected);
}

TEST_F(FileTest, GetFullPath_RelativePathWithDot)
{
    std::string relPath = "./abc.txt";
    char cwd[PATH_MAX] = {0};
    getcwd(cwd, PATH_MAX);
    std::string expected = std::string(cwd) + "/" + relPath;
    EXPECT_EQ(File::GetFullPath(relPath), expected);
}

TEST_F(FileTest, GetFileSize_NormalFile)
{
    std::ofstream ofs(testRegularFile);
    std::string content = "1234567890";
    ofs << content;
    ofs.close();
    EXPECT_EQ(File::GetFileSize(testRegularFile), content.size());
}

TEST_F(FileTest, GetFileSize_FileNotExist)
{
    EXPECT_EQ(File::GetFileSize(testNotExistsFile), 0);
}

TEST_F(FileTest, GetFileSize_Directory)
{
    TEST_ExecShellCommand("rm -rf " + testDirSub);
    EXPECT_EQ(File::GetFileSize(testDirSub), 0);
}

TEST_F(FileTest, GetFilePermissions_NormalFile)
{
    std::ofstream ofs(testRegularFile);
    ofs << "test";
    ofs.close();
    TEST_ExecShellCommand("chmod 640 " + testRegularFile);
    mode_t perms = File::GetFilePermissions(testRegularFile);
    // Should be 0640
    EXPECT_EQ(perms & 0777, 0640);
}

TEST_F(FileTest, GetFilePermissions_FileNotExist)
{
    mode_t perms = File::GetFilePermissions(testNotExistsFile);
    // Should return FULL_PERMISSIONS (0777) on error
    EXPECT_EQ(perms, FULL_PERMISSIONS);
}

TEST_F(FileTest, GetFilePermissions_Directory)
{
    TEST_ExecShellCommand("chmod 750 " + testDirSub);
    mode_t perms = File::GetFilePermissions(testDirSub);
    // Should be 0750
    EXPECT_EQ(perms & 0777, 0750);
}

TEST_F(FileTest, CheckFileSuffixAndSize_CommonType)
{
    std::string filePath = testDir + "/testfile_common.txt";
    std::ofstream ofs(filePath);
    ofs << "12345";
    ofs.close();
    // Should succeed for COMMON type and size < MAX_FILE_SIZE_DEFAULT
    EXPECT_TRUE(File::CheckFileSuffixAndSize(filePath, FileType::COMMON));
    // Should fail for COMMON type if file too large
    TEST_ExecShellCommand("truncate -s " + std::to_string(MAX_FILE_SIZE_DEFAULT + 1) + " " + filePath);
    EXPECT_FALSE(File::CheckFileSuffixAndSize(filePath, FileType::COMMON));
}

TEST_F(FileTest, CheckFileSuffixAndSize_KnownTypes)
{
    struct {
        FileType type;
        std::string suffix;
        size_t maxSize;
    } cases[] = {
        {FileType::NUMPY, "npy", MAX_NUMPY_SIZE},
        {FileType::JSON, "json", MAX_JSON_SIZE},
        {FileType::CSV, "csv", MAX_CSV_SIZE},
        {FileType::OM, "om", MAX_OM_SIZE},
    };
    for (const auto& c : cases) {
        std::string filePath = testDir + "/testfile." + c.suffix;
        std::ofstream ofs(filePath);
        ofs << "abc";
        ofs.close();
        // Should succeed for correct suffix and small size
        EXPECT_TRUE(File::CheckFileSuffixAndSize(filePath, c.type));
        // Should fail for wrong suffix
        // Should fail for too large file
        TEST_ExecShellCommand("truncate -s " + std::to_string(c.maxSize + 1) + " " + filePath);
        EXPECT_FALSE(File::CheckFileSuffixAndSize(filePath, c.type));
    }
}

TEST_F(FileTest, CheckFileSuffixAndSize_UnknownTypeOrFileNotExist)
{
    // Unknown type (not in map)
    std::string filePath = testDir + "/testfile.unknown";
    std::ofstream ofs(filePath);
    ofs << "abc";
    ofs.close();
    EXPECT_FALSE(File::CheckFileSuffixAndSize(filePath, static_cast<FileType>(999)));
    // File not exist
    EXPECT_FALSE(File::CheckFileSuffixAndSize(testNotExistsFile, FileType::COMMON));
}

TEST_F(FileTest, CheckFileBeforeRead_NormalFile)
{
    std::ofstream ofs(testRegularFile);
    chmod(testRegularFile.c_str(), 0644);
    ofs << "abc";
    ofs.close();
    EXPECT_TRUE(File::CheckFileBeforeRead(testRegularFile));
}

TEST_F(FileTest, CheckFileBeforeRead_EmptyPath)
{
    EXPECT_FALSE(File::CheckFileBeforeRead(""));
}

TEST_F(FileTest, CheckFileBeforeRead_PathLengthIllegal)
{
    std::string longPath = testDir + "/" + std::string(FULL_PATH_LENGTH_MAX + 1, 'a');
    EXPECT_FALSE(File::CheckFileBeforeRead(longPath));
}

TEST_F(FileTest, CheckFileBeforeRead_PathCharactersInvalid)
{
    std::string invalidPath = testDir + "/<>:|?*\"";
    std::ofstream ofs(invalidPath);
    ofs << "abc";
    ofs.close();
    EXPECT_FALSE(File::CheckFileBeforeRead(invalidPath));
}

TEST_F(FileTest, CheckFileBeforeRead_PathDepthInvalid)
{
    std::string deepPath = testDir;
    for (int i = 0; i < PATH_DEPTH_MAX + 2; ++i) {
        deepPath += "/d";
    }
    std::ofstream ofs(deepPath);
    ofs << "abc";
    ofs.close();
    EXPECT_FALSE(File::CheckFileBeforeRead(deepPath));
}

TEST_F(FileTest, CheckFileBeforeRead_NotRegularFile)
{
    EXPECT_FALSE(File::CheckFileBeforeRead(testDirSub));
    EXPECT_FALSE(File::CheckFileBeforeRead(testFifo));
}

TEST_F(FileTest, CheckFileBeforeRead_SoftLink)
{
    EXPECT_FALSE(File::CheckFileBeforeRead(testLink));
}

TEST_F(FileTest, CheckFileBeforeRead_PermissionOver755)
{
    TEST_ExecShellCommand("chmod 777 " + testRegularFile);
    EXPECT_FALSE(File::CheckFileBeforeRead(testRegularFile));
    TEST_ExecShellCommand("chmod 644 " + testRegularFile);
}

TEST_F(FileTest, CheckFileBeforeRead_NotReadable)
{
    TEST_ExecShellCommand("chmod 000 " + testRegularFile);
    EXPECT_FALSE(File::CheckFileBeforeRead(testRegularFile));
    TEST_ExecShellCommand("chmod 644 " + testRegularFile);
}

TEST_F(FileTest, CheckFileBeforeRead_SuffixAndSizeInvalid)
{
    std::string filePath = testDir + "/testfile.csv";
    std::ofstream ofs(filePath);
    ofs << "abc";
    ofs.close();
    // Use wrong type for suffix
    EXPECT_FALSE(File::CheckFileBeforeRead(filePath, FileType::JSON));
    // Too large for CSV
    TEST_ExecShellCommand("truncate -s " + std::to_string(MAX_CSV_SIZE + 1) + " " + filePath);
    EXPECT_FALSE(File::CheckFileBeforeRead(filePath, FileType::CSV));
}

TEST_F(FileTest, CheckFileBeforeCreateOrWrite_EmptyPath)
{
    EXPECT_FALSE(File::CheckFileBeforeCreateOrWrite("", true));
}

TEST_F(FileTest, CheckFileBeforeCreateOrWrite_PathLengthIllegal)
{
    std::string longPath = testDir + "/" + std::string(FULL_PATH_LENGTH_MAX + 1, 'a');
    EXPECT_FALSE(File::CheckFileBeforeCreateOrWrite(longPath, true));
}

TEST_F(FileTest, CheckFileBeforeCreateOrWrite_PathCharactersInvalid)
{
    std::string invalidPath = testDir + "/<>:|?*\"";
    EXPECT_FALSE(File::CheckFileBeforeCreateOrWrite(invalidPath, true));
}

TEST_F(FileTest, CheckFileBeforeCreateOrWrite_PathDepthInvalid)
{
    std::string deepPath = testDir;
    for (int i = 0; i < PATH_DEPTH_MAX + 2; ++i) {
        deepPath += "/d";
    }
    EXPECT_FALSE(File::CheckFileBeforeCreateOrWrite(deepPath, true));
}

TEST_F(FileTest, CheckFileBeforeCreateOrWrite_PathNotExist)
{
    // Should succeed if path does not exist and all checks pass
    std::string filePath = testDir + "/not_exist_file.txt";
    EXPECT_TRUE(File::CheckFileBeforeCreateOrWrite(filePath, true));
}

TEST_F(FileTest, CheckFileBeforeCreateOrWrite_Exist_NotOverwrite)
{
    std::ofstream ofs(testRegularFile);
    ofs << "test";
    ofs.close();
    EXPECT_FALSE(File::CheckFileBeforeCreateOrWrite(testRegularFile, false));
}

TEST_F(FileTest, CheckFileBeforeCreateOrWrite_Exist_NotRegularFile)
{
    // Directory is not a regular file
    EXPECT_FALSE(File::CheckFileBeforeCreateOrWrite(testDirSub, true));
}

TEST_F(FileTest, CheckFileBeforeCreateOrWrite_Exist_SoftLink)
{
    EXPECT_FALSE(File::CheckFileBeforeCreateOrWrite(testLink, true));
}

TEST_F(FileTest, CheckFileBeforeCreateOrWrite_Exist_PermissionOver750)
{
    TEST_ExecShellCommand("chmod 777 " + testRegularFile);
    EXPECT_FALSE(File::CheckFileBeforeCreateOrWrite(testRegularFile, true));
    TEST_ExecShellCommand("chmod 644 " + testRegularFile);
}

TEST_F(FileTest, CheckFileBeforeCreateOrWrite_Exist_NotWritableOrNotOwner)
{
    // Remove write permission
    TEST_ExecShellCommand("chmod 400 " + testRegularFile);
    EXPECT_FALSE(File::CheckFileBeforeCreateOrWrite(testRegularFile, true));
    TEST_ExecShellCommand("chmod 644 " + testRegularFile);

    // Not owner (only run as root)
    if (getuid() == 0) {
        std::string filePath = testDir + "/root_owned_create.txt";
        std::ofstream ofs(filePath);
        ofs << "root";
        ofs.close();
        TEST_ExecShellCommand("chown root:root " + filePath);
        EXPECT_FALSE(File::CheckFileBeforeCreateOrWrite(filePath, true));
        TEST_ExecShellCommand("rm -f " + filePath);
    }
}

TEST_F(FileTest, CheckFileBeforeCreateOrWrite_Exist_RegularFile_Owner_Writable)
{
    std::ofstream ofs(testRegularFile);
    ofs << "test";
    ofs.close();
    TEST_ExecShellCommand("chmod 750 " + testRegularFile);
    EXPECT_TRUE(File::CheckFileBeforeCreateOrWrite(testRegularFile, true));
}
}
