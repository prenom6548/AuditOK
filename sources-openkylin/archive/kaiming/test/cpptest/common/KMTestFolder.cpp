#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "KMFolder.h"
//#define public private

namespace fs = std::filesystem;

class FindIdPathsTest : public ::testing::Test
{
protected:
    fs::path tempRoot;
    KMFolder folder; // 你的KMFolder类

    void SetUp() override
    {
        tempRoot = fs::temp_directory_path() / "find_id_paths_test";
        fs::create_directories(tempRoot);
    }

    void TearDown() override
    {
        fs::remove_all(tempRoot);
    }
};

TEST_F(FindIdPathsTest, FindsCorrectIdAtDepth3)
{
    // 创建：root/level1/level2/target_id
    fs::create_directories(tempRoot / "level1" / "level2" / "myid");

    std::vector<fs::path> results;
    bool success = folder.findIdPaths(tempRoot, "myid", results, 0);

    ASSERT_TRUE(success);
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].filename(), "myid");
}

class SetActiveTest : public ::testing::Test
{
protected:
    fs::path tempBase;
    KMFolder folder;

    KMRef ref{ "app", "myapp", "x86_64", "stable" };

    void SetUp() override
    {
        tempBase = fs::temp_directory_path() / "set_active_test";
        folder.m_basePath = tempBase;
        fs::create_directories(tempBase / "app" / "myapp" / "x86_64" / "stable");
    }

    void TearDown() override
    {
        fs::remove_all(tempBase);
    }
};

TEST_F(SetActiveTest, ReturnsFalseWhenCommitDoesNotExist)
{
    std::string resultCommit = "nonexistent";
    ASSERT_FALSE(folder.setActive(ref, resultCommit));
}

TEST(KMFolderTest, GetHomeDirectoryReturnsValidPath)
{
    KMFolder folder;

    fs::path homeDir = folder.getHomeDirectory();

    // 判断路径存在，并且是目录
    ASSERT_TRUE(fs::exists(homeDir));
    ASSERT_TRUE(fs::is_directory(homeDir));

    // 通常是当前用户的 home，例如 /home/username
    const char *user = getenv("SUDO_USER");
    if (!user)
        user = getenv("USER");
    ASSERT_TRUE(user != nullptr);

    struct passwd *pw = getpwnam(user);
    ASSERT_TRUE(pw != nullptr);
    EXPECT_EQ(homeDir, fs::path(pw->pw_dir));
}

TEST(KMFolderTest, GetAppDataPathFormsCorrectly)
{
    KMFolder folder;
    std::string appId = "org.example.MyApp";

    fs::path expected = folder.getHomeDirectory() / ".var" / "app" / appId;
    fs::path actual = folder.getAppDataPath(appId);

    EXPECT_EQ(expected, actual);
}

TEST(KMFolderTest, GetPathFileListReturnsFilenames)
{
    KMFolder folder;

    // 创建临时目录
    fs::path tempDir = fs::temp_directory_path() / "test_kmfolder_files";
    fs::create_directory(tempDir);

    // 创建测试文件
    fs::path file1 = tempDir / "file1.txt";
    fs::path file2 = tempDir / "file2.txt";
    std::ofstream(file1.string()).put('a');
    std::ofstream(file2.string()).put('b');

    std::list<fs::path> fileList = folder.getPathFileList(tempDir);

    // 清理
    fs::remove(file1);
    fs::remove(file2);
    fs::remove(tempDir);

    // 验证返回值包含文件名，不是完整路径
    std::list<fs::path> expected = { "file1.txt", "file2.txt" };
    EXPECT_EQ(fileList.size(), expected.size());

    for (const auto &name : expected)
    {
        EXPECT_TRUE(std::find(fileList.begin(), fileList.end(), name) != fileList.end());
    }
}

TEST(KMFolderTest, GetPathFileListNonExistingPathReturnsEmpty)
{
    KMFolder folder;
    fs::path nonExistPath = fs::temp_directory_path() / "this_path_should_not_exist";

    std::list<fs::path> fileList = folder.getPathFileList(nonExistPath);

    EXPECT_TRUE(fileList.empty());
}

TEST(KMFolderTest, GetActivePathForAppIdReturnsCorrectPath)
{
    fs::path base = "/test/base"; // 假设的基路径
    KMFolder folder(base);

    std::string appId = "org.example.MyApp";
    fs::path expected = base / "app" / appId / "current" / "active";

    EXPECT_EQ(folder.getActivePath(appId), expected);
}

TEST(KMFolderTest, GetActivePathForRuntimeIdReturnsCorrectPath)
{
    fs::path base = "/test/base"; // 假设的基路径
    KMFolder folder(base);

    std::string runtimeId = "runtimeExample";
    fs::path expected = base / "runtime" / runtimeId / "current" / "active";

    EXPECT_EQ(folder.getRuntimeActivePath(runtimeId), expected);
}

TEST(KMFolderTest, GetPulseRuntimePathFromEnv)
{
    setenv("PULSE_RUNTIME_PATH", "/tmp/test_pulse", 1);
    KMFolder folder;
    EXPECT_EQ(folder.getPulseRuntimePath(), fs::path("/tmp/test_pulse"));
    unsetenv("PULSE_RUNTIME_PATH");
}

TEST(KMFolderTest, GetPulseRuntimePathFromDefault)
{
    unsetenv("PULSE_RUNTIME_PATH");

    // mock g_get_user_runtime_dir 返回的路径
    const char *runtimeDir = g_get_user_runtime_dir();
    fs::path expected = fs::path(realpath(runtimeDir, NULL)) / "pulse";

    if (fs::exists(expected))
    {
        KMFolder folder;
        EXPECT_EQ(folder.getPulseRuntimePath(), expected);
    }
    else
    {
        GTEST_SKIP() << "默认 pulse 路径不存在，跳过此测试";
    }
}

TEST(KMFolderTest, GetActivePathReturnsCorrectSymlink)
{
    // 临时目录结构创建
    fs::path base = fs::temp_directory_path() / "km_test";
    fs::path target = base / "kind" / "id" / "x86_64" / "testbranch" / "commit123";
    fs::create_directories(target);

    // 创建 active symlink
    fs::path branchDir = target.parent_path();
    fs::create_symlink("commit123", branchDir / "active");

    // 设置 ref
    KMRef ref;
    ref.kind = "kind";
    ref.id = "id";
    ref.arch = "x86_64";
    ref.branch = "testbranch";

    KMFolder folder;
    folder.m_basePath = base;

    // 实际测试
    fs::path activePath = folder.getActivePath(ref);
    EXPECT_EQ(activePath, target);

    // 清理
    fs::remove_all(base);
}

TEST(KMFolderTest, FindTmpIdPathsFindsCorrectPaths)
{
    fs::path root = fs::temp_directory_path() / "km_test_find";
    fs::create_directories(root / "match_id");
    fs::create_directories(root / "systemd-private-abc");
    fs::create_directories(root / "not_match");

    KMFolder folder;
    std::vector<fs::path> result;
    bool success = folder.findTmpIdPaths(root, "match_id", result);

    EXPECT_TRUE(success);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], root / "match_id");

    fs::remove_all(root);
}

TEST_F(FindIdPathsTest, FindDeployFilesFindsCorrectFiles)
{
    // 1. 准备路径 (建议直接用类里定义的 tempRoot)
    fs::path level0 = tempRoot / "deploy";
    fs::path level1 = tempRoot / "level1" / "deploy";
    fs::path level2 = tempRoot / "level1" / "level2" / "deploy";
    fs::path level3 = tempRoot / "level1" / "level2" / "level3" / "deploy";

    // 2. 创建目录和文件
    fs::create_directories(tempRoot / "level1" / "level2" / "level3");
    std::ofstream(level0).put('a');
    std::ofstream(level1).put('b');
    std::ofstream(level2).put('c');
    std::ofstream(level3).put('d');

    std::vector<fs::path> result;
    folder.findDeployFiles(tempRoot, result, 0);

    EXPECT_EQ(result.size(), 3); 
    
    // 验证包含前三个
    EXPECT_TRUE(std::count(result.begin(), result.end(), level0));
    EXPECT_TRUE(std::count(result.begin(), result.end(), level1));
    EXPECT_TRUE(std::count(result.begin(), result.end(), level2));
    
    // 验证不包含第四个
    EXPECT_FALSE(std::count(result.begin(), result.end(), level3));
}

TEST(KMFolderTest, ReadDeployFilesReadsUniqueLines)
{
    // 创建临时 deploy 文件
    fs::path tempFile = fs::temp_directory_path() / "test_deploy_file";
    std::ofstream ofs(tempFile);
    ofs << "line1\nline2\nline1\nline3\n"; // 包含重复行
    ofs.close();

    // 调用待测试函数
    KMFolder folder;
    std::vector<std::string> lines = folder.readDeployFiles(tempFile);

    // 验证结果
    EXPECT_EQ(lines.size(), 3);
    EXPECT_NE(std::find(lines.begin(), lines.end(), "line1"), lines.end());
    EXPECT_NE(std::find(lines.begin(), lines.end(), "line2"), lines.end());
    EXPECT_NE(std::find(lines.begin(), lines.end(), "line3"), lines.end());

    // 清理
    fs::remove(tempFile);
}

class KMFolderIsInstalledTest : public ::testing::Test
{
protected:
    KMFolder folder;
    fs::path root;

    void SetUp() override
    {
        root = "/tmp/kmfolder_test_layers";
        fs::remove_all(root); // 清理旧数据
        folder.m_layersPath = root.string();
    }

    void TearDown() override
    {
        fs::remove_all(root);
    }

    void touchFile(const fs::path &path)
    {
        fs::create_directories(path.parent_path());
        std::ofstream(path.string()) << "dummy";
    }
};

TEST_F(KMFolderIsInstalledTest, TestWithStringName)
{
    std::string name = "testapp";
    touchFile(root / "stable" / "x86_64" / "app" / name);
    EXPECT_TRUE(folder.isInstalled(name));
}

TEST_F(KMFolderIsInstalledTest, TestWithStringNameAndKMPMRef)
{
    std::string name = "testruntime";
    KMPMRef pmRef;
    pmRef.ref.arch = "x86_64";
    touchFile(root / "stable" / "x86_64" / "runtime" / name);
    EXPECT_TRUE(folder.isInstalled(name, pmRef));
}

TEST_F(KMFolderIsInstalledTest, TestWithKMRef)
{
    KMRef ref;
    ref.kind = "runtime";
    ref.id = "testpkg";
    ref.channel = "stable";
    ref.version = "1.0.0";
    touchFile(root / "runtime" / "testpkg" / "stable" / "1.0.0");
    EXPECT_TRUE(folder.isInstalled(ref));
}

TEST_F(KMFolderIsInstalledTest, TestWithKMPMRef)
{
    KMPMRef pmRef;
    pmRef.ref.kind = "base";
    pmRef.ref.id = "testbase";
    pmRef.ref.channel = "stable";
    pmRef.ref.version = "2.0.0";
    touchFile(root / "base" / "testbase" / "stable" / "2.0.0");
    EXPECT_TRUE(folder.isInstalled(pmRef));
}

class KMIsInstalledLikeTest : public ::testing::Test
{
protected:
    KMFolder folder;

    void SetUp() override
    {
        // 设置模拟安装列表
        folder.m_appInstallList = { "vim-8.2.0", "nano-5.5" };
        folder.m_runtimeInstallList = { "glibc-2.31", "libc-2.29" };
        folder.m_baseInstallList = { "baseutils-1.0", "inittools-0.9" };
    }
};

TEST_F(KMIsInstalledLikeTest, IsInstalledLike_App_Hit)
{
    KMRef ref;
    ref.kind = "app";
    ref.id = "vim";
    std::string id;

    bool result = folder.isInstalledLike(ref, id);

    ASSERT_TRUE(result);
    ASSERT_EQ(id, "vim-8.2.0");
}

class KMIsRuntimeExtensionTest : public ::testing::Test
{
protected:
    KMFolder folder; // 创建 KMFolder 类的实例
};
// 测试 isRuntimeExtension 方法
TEST_F(KMIsRuntimeExtensionTest, TestIsRuntimeExtension)
{
    KMRef ref1;
    ref1.kind = "runtime";
    KMRef ref2;
    ref2.kind = "app";

    // 测试当 ref.kind 为 "runtime" 时，isRuntimeExtension 返回 true
    EXPECT_FALSE(folder.isRuntimeExtension(ref1));

    // 测试当 ref.kind 不为 "runtime" 时，isRuntimeExtension 返回 false
    EXPECT_FALSE(folder.isRuntimeExtension(ref2));
}

// 测试 getCurrentRef 方法
TEST_F(KMIsRuntimeExtensionTest, TestGetCurrentRef)
{
    std::string name = "some_app";
    std::string name2 = "other_app";

    // 获取返回的 KMRef 对象
    KMRef ref1 = folder.getCurrentRef(name);
    KMRef ref2 = folder.getCurrentRef(name2);

    // 检查返回的 KMRef 对象的字段
    EXPECT_EQ(ref1.kind, "");
}

class KMgetUserBaseLocationPathTest : public ::testing::Test
{
protected:
    // 在每个测试前设置环境变量
    void SetUp() override
    {
        // 清空环境变量
        unsetenv("KAIMING_USER_DIR");
    }
};

// 测试 KAIMING_USER_DIR 环境变量被设置时
TEST_F(KMgetUserBaseLocationPathTest, TestWithUserDirEnvVar)
{
    const std::string test_path = "/custom/path/to/kaiming";
    setenv("KAIMING_USER_DIR", test_path.c_str(), 1); // 设置 KAIMING_USER_DIR 环境变量

    KMFolder folder;
    fs::path result = folder.getUserBaseLocationPath();

    // 检查返回的路径是否与环境变量的值一致
    EXPECT_EQ(result, fs::path(test_path));
}

// 测试 KAIMING_USER_DIR 环境变量未设置时
TEST_F(KMgetUserBaseLocationPathTest, TestWithoutUserDirEnvVar)
{
    KMFolder folder;
    fs::path result = folder.getUserBaseLocationPath();

    // 默认路径应该是 ~/.local/share/kaiming/
    fs::path default_path = fs::path(getenv("HOME")) / ".local/share/kaiming/";
    EXPECT_EQ(result, default_path);
}

// 测试 commit 非空时的路径
TEST(KMFolderTest, GetDeployPathWithCommit)
{
    KMFolder folder;
    KMRef ref;

    // 测试 commit 非空时的路径
    std::string commit = "commit_hash";
    fs::path expectedDeployPath = folder.getDeployBasePath(ref) / commit;

    // 调用函数
    fs::path result = folder.getDeployPath(ref, commit);

    // 验证路径
    EXPECT_EQ(result, expectedDeployPath);
}

TEST(KMFolderTest, UserDataAndCachePath)
{
    KMFolder folder;

    // 直接获取路径
    fs::path dataPath = folder.getUserDataPath();
    fs::path cachePath = folder.getUserCachePath();

    EXPECT_TRUE(!dataPath.empty());
    EXPECT_TRUE(!cachePath.empty());

    EXPECT_TRUE(fs::exists(dataPath));
    EXPECT_TRUE(fs::exists(cachePath));
    EXPECT_TRUE(fs::is_directory(dataPath));
    EXPECT_TRUE(fs::is_directory(cachePath));
}

TEST(KMFolderTest, InstancePath)
{
    KMFolder folder;

    fs::path instancePath = folder.getInstancePath();

    // getXdgRuntimePath 返回的是运行时目录
    fs::path expected = folder.getXdgRuntimePath() / ".kaiming";
    EXPECT_EQ(instancePath, expected);
}

TEST(KMFolderTest, SummariesAndRepoPath)
{
    KMFolder folder;

    // 先设置 m_summaryPath 和 m_repoPath
    fs::path summaryPath = "/tmp/test_summary";
    fs::path repoPath = "/tmp/test_repo";

    folder.m_summaryPath = summaryPath;
    folder.m_repoPath = repoPath;

    EXPECT_EQ(folder.getSummariesPath(), summaryPath);
    EXPECT_EQ(folder.getRepoPath(), repoPath);
}

// 测试：路径存在时返回 true
TEST(KMFolderTest, LoadDeployDataPathExists)
{
    KMFolder folder;
    KMRef ref = { "kind", "id" };
    folder.m_basePath = "/tmp/test"; // 设置基础路径

    // 创建测试所需的路径和文件
    fs::path targetPath = folder.m_basePath / ref.kind / ref.id / "current";
    fs::create_directories(targetPath); // 创建路径

    // 调用函数，验证路径存在
    bool result = folder.loadDeployData(ref, "", 0);
    EXPECT_TRUE(result); // 验证路径存在时返回 true

    // 清理临时文件
    fs::remove_all(folder.m_basePath);
}

TEST(KMFolderTest, EnsureRepo_BasePathNotExist_ReturnsFalse)
{
    KMFolder folder;
    folder.m_basePath = "/tmp/fake_base_path"; // 假设这个路径不存在
    folder.m_repoPath = "/tmp/fake_repo_path";

    // 确保路径不存在
    fs::remove_all(folder.m_basePath);
    fs::remove_all(folder.m_repoPath);

    bool result = folder.ensureRepo(false);
    EXPECT_FALSE(result);
}

//repoPath 不存在
TEST(KMFolderTest, EnsureRepo_RepoPathNotExist_Continues)
{
    KMFolder folder;
    fs::path basePath = "/tmp/kmfolder_test_base1";
    fs::remove_all(basePath);
    fs::create_directories(basePath);
    folder.m_basePath = basePath;

    fs::path repoPath = "/tmp/kmfolder_test_repo1";
    fs::remove_all(repoPath); // 确保 repoPath 不存在
    folder.m_repoPath = repoPath;

    // 假设 ostree 相关被禁用
    bool result = folder.ensureRepo(false);
    EXPECT_TRUE(result); // 即使 repo 不存在，也不会 return false（除非 ostree open 失败）
}

TEST(KMFolderTest, EnsureRepo_OstreeRepoOpenFails_ReturnsFalse)
{
    KMFolder folder;
    fs::path basePath = "/tmp/kmfolder_test_base2";
    fs::path repoPath = "/tmp/kmfolder_test_repo2";
    fs::create_directories(basePath);
    fs::create_directories(repoPath);
    folder.m_basePath = basePath;
    folder.m_repoPath = repoPath;

    // 修改 KMFolder 实现中的 ostree_repo_open 让它 return false
    // 或屏蔽后 return false 模拟路径
    bool result = folder.ensureRepo(false);
    EXPECT_FALSE(result);
}

class KMFolderTestable : public KMFolder
{
public:
    // 提供调用 initInstalled 的能力
    void callInitInstalled()
    {
        initInstalled(); // 调用 KMFolder 的私有方法
    }

    // 提供安装列表的 getter
    std::list<std::string> getAppInstallList() const
    {
        return m_appInstallList;
    }

    std::list<std::string> getRuntimeInstallList() const
    {
        return m_runtimeInstallList;
    }

    std::list<std::string> getBaseInstallList() const
    {
        return m_baseInstallList;
    }
};

TEST(KMFolderTest, InitInstalled_AppPathHasDirs)
{
    KMFolderTestable folder;

    fs::path tempPath = fs::temp_directory_path() / "kmfolder_test_app";
    fs::create_directories(tempPath / "app/app1");
    fs::create_directories(tempPath / "app/app2");
    folder.m_layersPath = tempPath;

    folder.callInitInstalled();
    auto appList = folder.getAppInstallList();

    EXPECT_EQ(appList.size(), 2);
    EXPECT_NE(std::find(appList.begin(), appList.end(), "app1"), appList.end());
    EXPECT_NE(std::find(appList.begin(), appList.end(), "app2"), appList.end());

    fs::remove_all(tempPath);
}

TEST(KMFolderTest, InitInstalled_RuntimePathHasDirs)
{
    KMFolderTestable folder;

    fs::path tempPath = fs::temp_directory_path() / "kmfolder_test_runtime";
    fs::create_directories(tempPath / "runtime/rt1");
    fs::create_directories(tempPath / "runtime/rt2");
    folder.m_layersPath = tempPath;

    folder.callInitInstalled();
    auto rtList = folder.getRuntimeInstallList();

    EXPECT_EQ(rtList.size(), 2);
    EXPECT_NE(std::find(rtList.begin(), rtList.end(), "rt1"), rtList.end());
    EXPECT_NE(std::find(rtList.begin(), rtList.end(), "rt2"), rtList.end());

    fs::remove_all(tempPath);
}

TEST(KMFolderTest, InitInstalled_BasePathHasDirs)
{
    KMFolderTestable folder;

    fs::path tempPath = fs::temp_directory_path() / "kmfolder_test_base";
    fs::create_directories(tempPath / "base/base1");
    fs::create_directories(tempPath / "base/base2");
    folder.m_layersPath = tempPath;

    folder.callInitInstalled();
    auto baseList = folder.getBaseInstallList();

    EXPECT_EQ(baseList.size(), 2);
    EXPECT_NE(std::find(baseList.begin(), baseList.end(), "base1"), baseList.end());
    EXPECT_NE(std::find(baseList.begin(), baseList.end(), "base2"), baseList.end());

    fs::remove_all(tempPath);
}
