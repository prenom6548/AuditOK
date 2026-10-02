#include <gtest/gtest.h>

#include "common/KMEnv.h"

// 移除重复的 KMCurl 类定义

class KMTestEnv
{
public:
    KMTestEnv() = default;
    ~KMTestEnv() = default;

public:
    KMEnv m_env;
};

TEST(KMTestEnv, test_env_isSudoOrRoot)
{
    KMTestEnv isSudoOrRoot;
    bool result = isSudoOrRoot.m_env.isSudoOrRoot();
    EXPECT_EQ(result, true);
}

TEST(KMTestEnv, test_env_getXdgDataDirs)
{
    KMTestEnv getXdgDataDirs;
    std::string result = getXdgDataDirs.m_env.getXdgDataDirs();
    EXPECT_FALSE(result.empty());
    // EXPECT_EQ(result, "/usr/share/ukui-wayland:/usr/local/share:/usr/share");
}

TEST(KMTestEnv, test_env_getRepoUrl)
{
    KMTestEnv getRepoUrl;
    std::string result = getRepoUrl.m_env.getRepoUrl();
    EXPECT_FALSE(result.empty());
    // EXPECT_EQ(result, "/usr/share/ukui-wayland:/usr/local/share:/usr/share");
}

TEST(KMTestEnv, test_env_passEnvCheck)
{
    KMTestEnv passEnvCheck;
    bool result = passEnvCheck.m_env.passEnvCheck();
    EXPECT_EQ(result, true);
}

