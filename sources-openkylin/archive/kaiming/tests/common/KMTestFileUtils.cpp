
/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include <gtest/gtest.h>
#include <filesystem>
#include <fcntl.h>
#include <fstream>
#include <unistd.h>
#include <iostream>
#include <regex>

#include "common/KMFileUtils.h"

namespace fs = std::filesystem;

// 测试夹具类
class KMFileUtilsTest : public ::testing::Test
{
protected:
    void createFile(const std::string &fileName, const std::string &content)
    {
        std::ofstream out(fileName);
        out << content;
        out.close();
    }

    void deleteFile(const std::string &fileName)
    {
        std::remove(fileName.c_str());
    }

protected:
    std::string tempFileName;

    void SetUp() override
    {
        tempFileName = "/tmp/test_file_exists.txt";
        std::ofstream out(tempFileName);
        out << "temporary test file";
        out.close();
    }

    void TearDown() override
    {
        if (std::ifstream(tempFileName))
        {
            std::remove(tempFileName.c_str());
        }
    }
};

class KMFileUtilsRemoveEmptyDirectoryTest : public ::testing::Test
{
protected:
    std::string testDir = "/tmp/km_test_remove_empty_dir";
    std::string subDir1 = "/tmp/km_test_remove_empty_dir/subdir1";
    std::string subDir2 = "/tmp/km_test_remove_empty_dir/subdir2";
    std::string emptyDir = "/tmp/km_test_remove_empty_dir/emptyDir";

    void SetUp() override
    {
        // 创建一个根目录并在其下创建子目录
        fs::create_directories(testDir);
        fs::create_directory(subDir1);
        fs::create_directory(subDir2);
        fs::create_directory(emptyDir); // 创建一个空目录
    }

    void TearDown() override
    {
        // 清理测试数据
        if (fs::exists(testDir))
        {
            fs::remove_all(testDir);
        }
    }
};

class KMFileUtilsMd5sumTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // 设置需要的测试文件和目录
        testDir = "/tmp/testDir";
        std::filesystem::create_directory(testDir);
        createTestFile("file1.txt", "This is a test file.");
        createTestFile("file2.txt", "Another test file.");
    }

    void TearDown() override
    {
        // 清理测试文件和目录
        std::filesystem::remove_all(testDir);
    }

    void createTestFile(const std::string &filename, const std::string &content)
    {
        std::ofstream outFile(testDir + "/" + filename);
        outFile << content;
        outFile.close();
    }

    std::string testDir;
};

class KMFileUtilsIsObsoleteTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // 设置测试文件的路径
        testFile = "/tmp/md5sum.txt";
    }

    void TearDown() override
    {
        // 清理测试文件
        std::remove(testFile.c_str());
    }

    void createTestFile(const std::string &content)
    {
        std::ofstream outFile(testFile);
        outFile << content;
        outFile.close();
    }

    std::string testFile;
};

class KMFileUtilsTest_v11 : public ::testing::Test
{
protected:
    // 临时文件路径
    const std::string testFilePath = "/tmp/test_kylin_osinfo";

    // 测试之前创建临时文件
    void SetUp() override
    {
        std::ofstream ofs(testFilePath);
        ofs << "MajorVersion=V10\nMinorVersion=SP1"; // 默认内容
        ofs.close();
    }

    // 测试结束后清理临时文件
    void TearDown() override
    {
        fs::remove(testFilePath);
    }

    // 帮助函数，用于写入测试内容到临时文件
    void writeToOsinfo(const std::string &content)
    {
        std::ofstream ofs(testFilePath);
        ofs << content;
        ofs.close();
    }

};

// 有效 JSON 文件测试
TEST_F(KMFileUtilsTest, ValidJsonFile)
{
    std::string fileName = "valid.json";
    std::string content = R"({"name": "test", "value": 123})";
    createFile(fileName, content);

    EXPECT_TRUE(KMFileUtils::isJsonFile(fileName));

    deleteFile(fileName);
}

// 无效 JSON 文件测试
TEST_F(KMFileUtilsTest, InvalidJsonFile)
{
    std::string fileName = "/home/xiaobing/invalid.json";
    std::string content = R"({"name": "bad json", )";
    createFile(fileName, content);

    EXPECT_FALSE(KMFileUtils::isJsonFile(fileName));

    deleteFile(fileName);
}

// 文件不存在测试
TEST_F(KMFileUtilsTest, FileDoesNotExist)
{
    std::string fileName = "nonexistent.json";
    EXPECT_FALSE(KMFileUtils::isJsonFile(fileName));
}

TEST_F(KMFileUtilsTest, FileExists_WhenFileExists_ReturnsTrue)
{
    EXPECT_TRUE(KMFileUtils::fileExists(tempFileName));
}

TEST_F(KMFileUtilsTest, FileExists_WhenFileDoesNotExist_ReturnsFalse)
{
    std::string nonExistentFile = "/tmp/nonexistent_file.txt";
    EXPECT_FALSE(KMFileUtils::fileExists(nonExistentFile));
}

class RemoveEmptyDirectoryTest : public ::testing::Test
{
protected:
    fs::path testRoot;

    void SetUp() override
    {
        testRoot = "/tmp/kmtest_remove_dir";

        // 构造目录结构
        fs::create_directories(testRoot / "a/b/c");
        fs::create_directories(testRoot / "a/b2");
        fs::create_directories(testRoot / "a_non_empty");

        // 创建非空文件
        std::ofstream(testRoot / "a_non_empty/file.txt") << "data";
    }

    void TearDown() override
    {
        fs::remove_all(testRoot);
    }
};

TEST_F(RemoveEmptyDirectoryTest, RemoveOnlyEmptyDirectories)
{
    std::error_code ec;
    bool result = KMFileUtils::removeEmptyDirectory((testRoot / "a").string(), ec);

    EXPECT_TRUE(result);
    EXPECT_FALSE(fs::exists(testRoot / "a/b/c")); // c should be removed
    EXPECT_FALSE(fs::exists(testRoot / "a/b"));   // b should be removed
    EXPECT_FALSE(fs::exists(testRoot / "a/b2"));  // b2 should be removed
    EXPECT_TRUE(fs::exists(testRoot / "a"));      // a should still exist, though now empty

    // Non-empty directory should remain
    EXPECT_TRUE(fs::exists(testRoot / "a_non_empty"));
}

TEST_F(KMFileUtilsTest, IsEmptyDirectory_EmptyDir)
{
    std::string testDir = "/tmp/km_empty_dir_test";
    fs::create_directory(testDir);

    EXPECT_TRUE(KMFileUtils::isEmptyDirectory(testDir));

    fs::remove(testDir);
}

TEST_F(KMFileUtilsTest, IsEmptyDirectory_NonEmptyDir)
{
    std::string testDir = "/tmp/km_nonempty_dir_test";
    std::string filePath = testDir + "/test.txt";

    fs::create_directory(testDir);
    std::ofstream(filePath) << "test";

    EXPECT_FALSE(KMFileUtils::isEmptyDirectory(testDir));

    fs::remove(filePath);
    fs::remove(testDir);
}

TEST_F(KMFileUtilsTest, IsEmptyDir_EmptyRecursive)
{
    std::string rootDir = "/tmp/km_empty_recursive_dir";
    std::string subDir = rootDir + "/subdir";

    fs::create_directories(subDir);

    EXPECT_TRUE(KMFileUtils::isEmptyDir(rootDir));

    fs::remove_all(rootDir);
}

TEST_F(KMFileUtilsTest, IsEmptyDir_ContainsFile)
{
    std::string rootDir = "/tmp/km_nonempty_recursive_dir";
    std::string subDir = rootDir + "/subdir";
    std::string filePath = subDir + "/file.txt";

    fs::create_directories(subDir);
    std::ofstream(filePath) << "data";

    EXPECT_FALSE(KMFileUtils::isEmptyDir(rootDir));

    fs::remove_all(rootDir);
}

TEST_F(KMFileUtilsTest, RemoveEmptyDirectory_RemovesOnlyEmptyDirs)
{
    std::error_code ec;

    std::string root = "/tmp/km_remove_test";
    std::string emptySub1 = root + "/empty1";
    std::string emptySub2 = root + "/nested/empty2";
    std::string nonEmptySub = root + "/nested/notempty";
    std::string nonEmptyFile = nonEmptySub + "/file.txt";

    // 构造目录结构
    fs::create_directories(emptySub1);
    fs::create_directories(emptySub2);
    fs::create_directories(nonEmptySub);
    std::ofstream(nonEmptyFile) << "data";

    EXPECT_TRUE(fs::exists(emptySub1));
    EXPECT_TRUE(fs::exists(emptySub2));
    EXPECT_TRUE(fs::exists(nonEmptyFile));

    bool result = KMFileUtils::removeEmptyDirectory(root, ec);

    EXPECT_TRUE(result);
    EXPECT_FALSE(fs::exists(emptySub1));  // 被删除
    EXPECT_FALSE(fs::exists(emptySub2));  // 被删除
    EXPECT_TRUE(fs::exists(nonEmptySub)); // 保留
    EXPECT_TRUE(fs::exists(nonEmptyFile));

    fs::remove_all(root);
}

TEST_F(KMFileUtilsTest, GenTempName_ValidTempName)
{
    std::string tempName = "tempXXXXXX";
    bool result = KMFileUtils::genTempName(tempName);

    EXPECT_TRUE(result);                                             // 预期成功生成临时文件名
    EXPECT_EQ(tempName.length(), 10);                                // 文件名应为 12 个字符
    EXPECT_TRUE(tempName.substr(tempName.length() - 4) != "XXXXXX"); // 最后 6 个字符应已替换
}

TEST_F(KMFileUtilsTest, GenTempName_InvalidPattern)
{
    std::string tempName = "invalidName";
    bool result = KMFileUtils::genTempName(tempName);

    EXPECT_FALSE(result);               // 预期失败，因为文件名没有以 "XXXXXX" 结尾
    EXPECT_EQ(tempName, "invalidName"); // 文件名应该保持原样
}

TEST_F(KMFileUtilsTest, KmGenerateTempName_ValidTempName)
{
    std::string tempName = "tempXXXXXX";
    std::string result = KMFileUtils::kmGenerateTempName(tempName);

    EXPECT_EQ(result.length(), 10);                              // 文件名应为 12 个字符
    EXPECT_TRUE(result.substr(result.length() - 4) != "XXXXXX"); // 最后 6 个字符应已替换
    EXPECT_NE(result, tempName);                                 // 生成的文件名应该与原始文件名不同
}

TEST_F(KMFileUtilsTest, KmGenerateTempName_PatternIntegrity)
{
    std::string tempName = "tempXXXXXX";
    std::string result = KMFileUtils::kmGenerateTempName(tempName);

    // 确保 tempName 中的 "XXXXXX" 被替换成了一个随机的字符串
    EXPECT_TRUE(result.find("XXXXXX") == std::string::npos); // "XXXXXX" 应该不再出现在生成的文件名中
}

TEST_F(KMFileUtilsMd5sumTest, GetDirMd5sum_ValidDirectory)
{
    // 测试 getDirMd5sum 函数，确保它能够正确计算目录中所有文件的 MD5
    std::string result = KMFileUtils::getDirMd5sum(testDir);

    EXPECT_FALSE(result.empty()); // 预期结果应为非空 MD5 值
}

TEST_F(KMFileUtilsMd5sumTest, GetDirMd5sum_EmptyDirectory)
{
    // 测试空目录的情况，应该返回空的 MD5 值
    std::string emptyDir = "/tmp/emptyDir";
    std::filesystem::create_directory(emptyDir);

    std::string result = KMFileUtils::getDirMd5sum(emptyDir);
    EXPECT_TRUE(result.empty()); // 预期返回空字符串，因为目录没有文件

    std::filesystem::remove_all(emptyDir);
}

TEST_F(KMFileUtilsMd5sumTest, GetDirMd5sum_TempFileHandling)
{
    // 测试临时文件的生成和删除
    std::string result = KMFileUtils::getDirMd5sum(testDir);

    // 额外验证临时文件是否已删除
    std::string tmpFile = result.substr(0, result.find_last_of('/'));
    EXPECT_FALSE(std::filesystem::exists(tmpFile)); // 预期临时文件已被删除
}

TEST_F(KMFileUtilsIsObsoleteTest, IsObsolete_FileOpenFailure)
{
    // 测试文件打开失败的情况，文件不存在时应返回 true
    std::string nonexistentFile = "/tmp/nonexistentfile.txt";
    bool result = KMFileUtils::isObsolete(nonexistentFile);

    EXPECT_TRUE(result); // 预期返回 true，因为文件无法打开
}

TEST_F(KMFileUtilsIsObsoleteTest, IsObsolete_ValidFile)
{
    // 测试文件格式正确的情况
    std::string validContent = "d41d8cd98f00b204e9800998ecf8427e  somefile.txt";
    createTestFile(validContent);

    bool result = KMFileUtils::isObsolete(testFile);

    EXPECT_FALSE(result); // 预期返回 false，因为文件内容符合预期格式
}

TEST_F(KMFileUtilsIsObsoleteTest, IsObsolete_InvalidFileContent)
{
    // 测试文件内容格式不正确的情况
    std::string invalidContent = "d41d8cd98f00b204e9800998ecf8427e";
    createTestFile(invalidContent);

    bool result = KMFileUtils::isObsolete(testFile);

    EXPECT_TRUE(result); // 预期返回 true，因为文件内容格式不符合预期
}

class KMFileCopeTest : public ::testing::Test
{
protected:
    const std::string sourceDir = "/tmp/sourceDir";
    const std::string destDir = "/tmp/destDir";
    const std::string skipPattern = ".*\\.log"; // 例如跳过 .log 文件
    const std::string testFile = "/tmp/sourceDir/testFile.txt";
    const std::string skipFile = "/tmp/sourceDir/skippedFile.log";

    void SetUp() override
    {
        // 创建源目录和文件
        fs::create_directory(sourceDir);
        std::ofstream ofs(testFile);
        ofs << "Test content" << std::endl;
        ofs.close();

        std::ofstream skipOfs(skipFile);
        skipOfs << "Log content" << std::endl;
        skipOfs.close();
    }

    void TearDown() override
    {
        // 删除源目录和目标目录
        fs::remove_all(sourceDir);
        fs::remove_all(destDir);
    }
};

class KMpathExistsTest : public ::testing::Test
{
protected:
    fs::path testDir = "/tmp/km_test_dir";
    fs::path testFile = testDir / "test_file.txt";

    void SetUp() override
    {
        fs::create_directories(testDir);
        std::ofstream(testFile) << "Hello World";
    }

    void TearDown() override
    {
        fs::remove_all(testDir);
    }
};

TEST_F(KMpathExistsTest, PathExistsReturnsTrueForExistingPath)
{
    EXPECT_TRUE(KMFileUtils::pathExists(testFile.string()));
}

TEST_F(KMpathExistsTest, PathExistsReturnsFalseForNonexistentPath)
{
    EXPECT_FALSE(KMFileUtils::pathExists("/tmp/nonexistent_path_abc123"));
}

TEST_F(KMpathExistsTest, GetDirectorySizeReturnsNonZeroForNonEmptyDir)
{
    uint64_t size = KMFileUtils::getDirectorySize(testDir);
    ASSERT_GT(size, 0);
}

TEST_F(KMpathExistsTest, GetDirectorySizeReturnsZeroForNonexistentDir)
{
    uint64_t size = KMFileUtils::getDirectorySize("/tmp/nonexistent_dir_abc123");
    ASSERT_EQ(size, 0);
}

class KMmkpathTest : public ::testing::Test
{
protected:
    fs::path testDir = "/tmp/km_test_dir";
    fs::path nestedDir = testDir / "nested" / "dir";

    void TearDown() override
    {
        if (fs::exists(testDir))
        {
            fs::remove_all(testDir);
        }
    }
};

TEST_F(KMmkpathTest, MkpathCreatesDirectoriesRecursively)
{
    EXPECT_FALSE(fs::exists(nestedDir)); // 确保目录不存在

    bool result = KMFileUtils::mkpath(nestedDir.string());

    EXPECT_TRUE(result);                // 检查创建目录是否成功
    EXPECT_TRUE(fs::exists(nestedDir)); // 确保目录存在
}

TEST_F(KMmkpathTest, MkpathDoesNotFailIfDirectoryExists)
{
    // 首先创建该目录
    KMFileUtils::mkpath(nestedDir.string());

    // 再次调用mkpath应该成功并且不抛出错误
    bool result = KMFileUtils::mkpath(nestedDir.string());

    EXPECT_TRUE(result); // 已存在的目录应该返回true
}

TEST_F(KMmkpathTest, MkpathSetsCorrectPermissions)
{
    // 创建目录并检查权限
    KMFileUtils::mkpath(nestedDir.string(), 0755);
    fs::perms perms = fs::status(nestedDir).permissions();

    ASSERT_EQ(perms, fs::perms::owner_read | fs::perms::owner_write | fs::perms::owner_exec |
                         fs::perms::group_read | fs::perms::group_exec |
                         fs::perms::others_read | fs::perms::others_exec); // 检查目录权限
}

TEST_F(KMmkpathTest, MkpathWithErrorCodeReturnsFalseOnFailure)
{
    std::error_code ec;
    bool result = KMFileUtils::mkpath("", ec);

    EXPECT_FALSE(result); 
    EXPECT_TRUE(ec);     
}

class KMrealpathTest : public ::testing::Test
{
protected:
    fs::path baseDir = "/tmp/km_test_rm_rf";
    fs::path testDir;
    fs::path nestedDir;
    fs::path symlinkPath;

    void SetUp() override
    {
        // 创建测试文件和目录
        if(!fs::exists(baseDir)){
            fs::create_directories(baseDir / "nested" / "dir");
        }

        testDir = fs::canonical(baseDir);
        nestedDir = testDir / "nested" /  "dir";
        symlinkPath = testDir / "symlink";

        fs::path targetFile = testDir / "file.txt";

        if (fs::exists(targetFile)) fs::remove(targetFile);
        if (fs::exists(symlinkPath)) fs::remove(symlinkPath);    

        {
            std::ofstream out(targetFile);
        }
        
        fs::create_symlink(targetFile, symlinkPath); // 创建软链接
    }

    void TearDown() override
    {
        if (fs::exists(testDir))
        {
            fs::remove_all(testDir); // 清理测试目录
        }
    }
};

TEST_F(KMrealpathTest, RmRfAllRemovesDirectoryRecursively)
{
    // 确保目录存在
    EXPECT_TRUE(fs::exists(nestedDir));

    bool result = KMFileUtils::rmRfAll(testDir.string());

    EXPECT_TRUE(result);               // 检查删除是否成功
    EXPECT_FALSE(fs::exists(testDir)); // 确保目录被删除
}

TEST_F(KMrealpathTest, RealpathReturnsCorrectRealPathForSymlink)
{
    std::string realPath = KMFileUtils::realpath(symlinkPath.string());
    
    ASSERT_EQ(realPath, (testDir / "file.txt").string()); // 检查软链接指向的真实路径
}


TEST_F(KMrealpathTest, RealpathReturnsEmptyStringForNonExistentPath)
{
    std::string realPath = KMFileUtils::realpath("/non/existent/path");

    EXPECT_TRUE(realPath.empty()); // 应该返回空字符串，因为路径不存在
}

TEST_F(KMrealpathTest, RealpathHandlesNonSymlinkCorrectly)
{
    std::string realPath = KMFileUtils::realpath(testDir.string());

    ASSERT_EQ(realPath, testDir.string()); // 非软链接路径应该返回原路径
}

class KMgetFileSha256sumTest : public ::testing::Test
{
protected:
    fs::path testFile = "/tmp/testfile.txt";

    void SetUp() override
    {
        // 创建一个测试文件并写入内容
        std::ofstream ofs(testFile);
        ofs << "Hello, World!";
        ofs.close();
    }

    void TearDown() override
    {
        if (fs::exists(testFile))
        {
            fs::remove(testFile); // 清理测试文件
        }
    }
};

TEST_F(KMgetFileSha256sumTest, GetFileSha256sumReturnsCorrectHash)
{
    // 假设 "Hello, World!" 的 SHA256 校验和为一个已知值
    std::string expectedSha256 = "dffd6021bb2bd5b0af676290809ec3a53191dd81c7f70a4b28688a362182986f";

    std::string sha256 = KMFileUtils::getFileSha256sum(testFile.string());

    ASSERT_EQ(sha256, expectedSha256); // 确保返回的校验和正确
}

TEST_F(KMgetFileSha256sumTest, GetFileSha256sumReturnsEmptyForNonExistentFile)
{
    std::string sha256 = KMFileUtils::getFileSha256sum("/tmp/non_existent_file.txt");

    ASSERT_EQ(sha256, ""); // 如果文件不存在，返回应为空字符串
}

TEST_F(KMgetFileSha256sumTest, ReadFileReturnsContent)
{
    std::string content;
    bool success = KMFileUtils::readFile(testFile.string(), content);

    EXPECT_TRUE(success);                // 应该成功读取文件
    ASSERT_EQ(content, "Hello, World!"); // 确保文件内容正确
}

TEST_F(KMgetFileSha256sumTest, ReadFileReturnsFalseForNonExistentFile)
{
    std::string content;
    bool success = KMFileUtils::readFile("/tmp/non_existent_file.txt", content);

    EXPECT_FALSE(success);  // 文件不存在，应返回 false
    ASSERT_EQ(content, ""); // 如果读取失败，返回的内容应为空
}

class FindFileInTreeTest : public ::testing::Test
{
protected:
    fs::path rootDir = "/tmp/km_findfile_test";
    fs::path subDir;
    fs::path targetFile;

    void SetUp() override
    {
        subDir = rootDir / "subdir";
        targetFile = subDir / "target.txt";

        fs::create_directories(subDir);
        std::ofstream ofs(targetFile); // 创建目标文件
        ofs << "test";
        ofs.close();
    }

    void TearDown() override
    {
        fs::remove_all(rootDir); // 清理测试数据
    }
};

TEST_F(FindFileInTreeTest, FileExistsInSubDirectory)
{
    bool found = KMFileUtils::findFileInTree(rootDir.string(), "target.txt");
    EXPECT_TRUE(found); // 应该能找到
}

TEST_F(FindFileInTreeTest, FileDoesNotExist)
{
    bool found = KMFileUtils::findFileInTree(rootDir.string(), "nonexistent.txt");
    EXPECT_FALSE(found); // 不存在的文件应返回 false
}

TEST_F(FindFileInTreeTest, BaseDirectoryDoesNotExist)
{
    bool found = KMFileUtils::findFileInTree("/tmp/nonexistent_dir", "target.txt");
    EXPECT_FALSE(found); // 根目录不存在也应返回 false
}

class OpenAnonymousTmpfileFullTest : public ::testing::Test
{
protected:
    std::string testDir = "/tmp/km_anontest";

    void SetUp() override
    {
        fs::create_directories(testDir);
    }

    void TearDown() override
    {
        fs::remove_all(testDir);
    }
};

class KMreplaceFileContentsWithPermsAtTest : public ::testing::Test
{
protected:
    std::string testDir = "/tmp/kmfileutils_test";
    std::string testFile;
    int dfd = -1;

    void SetUp() override
    {
        ::mkdir(testDir.c_str(), 0755);
        testFile = testDir + "/file.txt";
        std::ofstream(testFile) << "old content";
        dfd = ::open(testDir.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);
        ASSERT_GE(dfd, 0);
    }

    void TearDown() override
    {
        ::unlink(testFile.c_str());
        ::close(dfd);
        ::rmdir(testDir.c_str());
    }
};

class KMFileUtilsCopyTest : public ::testing::Test
{
protected:
    std::string srcDir = "/tmp/km_cp_test_src";
    std::string dstDir = "/tmp/km_cp_test_dst";

    void SetUp() override
    {
        fs::create_directory(srcDir);
        std::ofstream(srcDir + "/file1.txt") << "Hello World";
        fs::create_directory(dstDir);
    }

    void TearDown() override
    {
        fs::remove_all(srcDir);
        fs::remove_all(dstDir);
    }
};

TEST_F(KMFileUtilsCopyTest, CpRfAllCopiesFiles)
{
    std::string src = srcDir + "/file1.txt";
    std::string dest = dstDir + "/file1.txt";
    EXPECT_TRUE(KMFileUtils::cpRfAll(src, dest));
    EXPECT_TRUE(fs::exists(dest));
    std::ifstream fin(dest);
    std::string content((std::istreambuf_iterator<char>(fin)), {});
    EXPECT_EQ(content, "Hello World");
}

TEST_F(KMFileUtilsCopyTest, CpRfAllStarCopiesDirContents)
{
    EXPECT_TRUE(KMFileUtils::cpRfAllStar(srcDir + "/*", dstDir));
    EXPECT_TRUE(fs::exists(dstDir + "/file1.txt"));
    std::ifstream fin(dstDir + "/file1.txt");
    std::string content((std::istreambuf_iterator<char>(fin)), {});
    EXPECT_EQ(content, "Hello World");
}

class KMFparseTxtTest : public ::testing::Test
{
protected:
    std::string testFilePath = "/tmp/testfile.txt";

    void SetUp() override
    {
        // 在每个测试之前创建一个示例文件
        std::ofstream testFile(testFilePath);
        testFile << "Hello, world!" << std::endl;
        testFile << "This is a test file." << std::endl;
        testFile << "It contains multiple lines." << std::endl;
        testFile.close();
    }

    void TearDown() override
    {
        // 清理测试文件
        if (fs::exists(testFilePath))
        {
            fs::remove(testFilePath);
        }
    }
};

TEST_F(KMFileUtilsTest, ParseTxtFileNotFound)
{
    // 测试文件不存在的情况
    std::string nonexistentFilePath = "/tmp/nonexistentfile.txt";
    std::vector<std::string> lines = KMFileUtils::parseTxt(nonexistentFilePath);

    // 验证返回值为空
    EXPECT_TRUE(lines.empty());
}

TEST_F(KMFileUtilsTest, GetCurrentTime_Format)
{
    std::string t = KMFileUtils::getCurrentTime();

    EXPECT_FALSE(t.empty());
    EXPECT_EQ(t.size(), 19);

    // 正则匹配时间格式
    std::regex pattern(R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})");
    EXPECT_TRUE(std::regex_match(t, pattern));
}

TEST_F(KMFileUtilsTest, CreateFile_Success)
{
    std::string file = "/tmp/km_test_file.txt";

    bool ret = KMFileUtils::createFileIfNotExist(file);

    EXPECT_TRUE(ret);

    std::ifstream in(file);
    EXPECT_TRUE(in.is_open());

    std::string content;
    std::getline(in, content);

    EXPECT_NE(content.find("Install time:"), std::string::npos);

    in.close();

    remove(file.c_str());
}

TEST_F(KMFileUtilsTest, RsyncDir_Success)
{
    std::string src = "/tmp/src";
    std::string dest = "/tmp/dest";

    std::vector<std::string> excludes = {"*.log", "cache"};

    bool ret = KMFileUtils::rsyncDir(src, dest, excludes);

    // 只验证函数返回值
    EXPECT_TRUE(ret);
}

TEST_F(KMFileUtilsTest, IsImmutableSystem)
{
    bool ret = KMFileUtils::isImmutableSystem();

    if (std::filesystem::exists("/etc/ostree-release"))
        EXPECT_TRUE(ret);
    else
        EXPECT_FALSE(ret);
}

TEST_F(KMFileUtilsTest, IsExecutable_ShellScript)
{
    std::string file="/tmp/test.sh";

    std::ofstream f(file);
    f<<"#!/bin/bash\necho test\n";
    f.close();

    EXPECT_TRUE(KMFileUtils::isExecutableByFileCmd(file));

    std::filesystem::remove(file);
}

TEST_F(KMFileUtilsTest, IsShellScript)
{
    std::string file="/tmp/test_script.sh";

    std::ofstream f(file);
    f<<"#!/bin/bash\necho hello\n";
    f.close();

    EXPECT_TRUE(KMFileUtils::isShellScriptByFileCmd(file));

    std::filesystem::remove(file);
}

TEST_F(KMFileUtilsTest, NonExecExt_Text)
{
    EXPECT_TRUE(KMFileUtils::isNonExecExt("test.txt"));
}

TEST_F(KMFileUtilsTest, CopyFilesByExtension)
{
    std::filesystem::create_directories("/tmp/src");
    std::filesystem::create_directories("/tmp/dst");

    std::ofstream("/tmp/src/a.so");
    std::ofstream("/tmp/src/b.txt");

    std::set<std::string> files;

    std::string ret = KMFileUtils::copyFilesByExtension(
        "/tmp/src",
        "/tmp/dst",
        ".so",
        "plugin",
        files
    );

    EXPECT_TRUE(std::filesystem::exists("/tmp/dst/a.so"));
    EXPECT_FALSE(std::filesystem::exists("/tmp/dst/b.txt"));

    std::filesystem::remove_all("/tmp/src");
    std::filesystem::remove_all("/tmp/dst");
}
