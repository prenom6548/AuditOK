
#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>

#include "common/KMPermissionConfig.h"
#include "common/KMDirUtils.h"

// 测试用的类，提供一个成员对象
class KMTestPermissionConfig
{
public:
    KMTestPermissionConfig()
        : m_permissionConfig("testapp"){};
    ~KMTestPermissionConfig() = default;

    KMPermissionConfig m_permissionConfig;
};

class KMPermissionTest : public ::testing::Test {
    protected:
        std::string tempFile = "./test_config.json";
    
        void SetUp() override {
            // 每个测试开始前创建文件
            std::ofstream(tempFile) << "{}";
        }
    
        void TearDown() override {
            // 每个测试结束后清理文件
            std::filesystem::remove(tempFile);
        }
    };
    
TEST_F(KMPermissionTest, LoadExistingFileReturnsTrue) 
{
    KMPermissionConfig config("testapp");
    EXPECT_TRUE(config.loadFile(tempFile));
}

//加载文件路径错误
TEST(KMTestPermissionConfig, test_loadFile_failure)
{
    KMTestPermissionConfig testpermissionconfig;
    const std::string invalidFile = "/path/to/nonexistent/file.json";
    EXPECT_FALSE(testpermissionconfig.m_permissionConfig.loadFile(invalidFile));
}

//加载配置成功
TEST(KMTestPermissionConfig, test_loadConfiguration)
{
    std::string home;
    KMDirUtils::getHomeDir(home);
    std::filesystem::path configPath = std::filesystem::path(home) / ".config";
    std::filesystem::path jsonFile = configPath / "kaiming-permission.json";

    KMTestPermissionConfig testpermissionconfig;
    bool result = testpermissionconfig.m_permissionConfig.loadConfiguration();

    AppPermissionConfig config = testpermissionconfig.m_permissionConfig.getPermissionConfig();

    EXPECT_TRUE(config.bluetooth == 0);
}

TEST(KMTestPermissionConfig, test_getpermissionconfig)
{
    std::string home;
    KMDirUtils::getHomeDir(home);

    std::filesystem::path configDir =
        std::filesystem::path(home) / ".config";

    std::filesystem::create_directories(configDir);

    std::filesystem::path jsonFile =
        configDir / "kaiming-permission.json";

    std::ofstream ofs(jsonFile);
    ofs << R"({
        "testapp": {
            "bluetooth": true,
            "camera": true,
            "network": true
        }
    })";
    ofs.close();

    KMPermissionConfig config("testapp");

    AppPermissionConfig perm = config.getPermissionConfig();

    EXPECT_TRUE(perm.bluetooth);
    EXPECT_TRUE(perm.camera);
    EXPECT_TRUE(perm.network);

    std::filesystem::remove(jsonFile);
}
