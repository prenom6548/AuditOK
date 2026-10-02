#include "gtest/gtest.h"
#include "KMRunnerOnce.h"
#include <vector>
#include <string>
#include <unistd.h> 

class KMRunnerOnceTest : public ::testing::Test
{
protected:
    KMRunnerOnce runner;
};

TEST_F(KMRunnerOnceTest, AssembleNoEnvNoSock)
{
    std::vector<std::string> args;
    bool ret = runner.assembleRunOnceOptions("", {}, args);

    EXPECT_TRUE(ret);
    ASSERT_GE(args.size(), 2u);
    EXPECT_EQ(args[0], "--detach");

    // 最后一个参数应该是 --user=uid:gid
    std::string userArg = "--user=" + std::to_string(getuid()) + ":" + std::to_string(getgid());
    EXPECT_EQ(args.back(), userArg);
}

TEST_F(KMRunnerOnceTest, AssembleWithEnv)
{
    std::vector<std::string> envs = { "VAR1=1", "VAR2=2" };
    std::vector<std::string> args;
    bool ret = runner.assembleRunOnceOptions("", envs, args);

    EXPECT_TRUE(ret);
    EXPECT_NE(std::find(args.begin(), args.end(), "--env=VAR1=1"), args.end());
    EXPECT_NE(std::find(args.begin(), args.end(), "--env=VAR2=2"), args.end());
}

TEST_F(KMRunnerOnceTest, AssembleWithSock)
{
    std::vector<std::string> envs = { "VAR1=1" };
    std::vector<std::string> args;
    std::string sock = "/tmp/test.sock";

    bool ret = runner.assembleRunOnceOptions(sock, envs, args);

    EXPECT_TRUE(ret);
    EXPECT_NE(std::find(args.begin(), args.end(), "--tty"), args.end());
    EXPECT_NE(std::find(args.begin(), args.end(), "--console-socket=" + sock), args.end());
}

TEST_F(KMRunnerOnceTest, AssembleAllCombined)
{
    std::vector<std::string> envs = { "E1=1", "E2=2" };
    std::vector<std::string> args;
    std::string sock = "/tmp/sock";

    bool ret = runner.assembleRunOnceOptions(sock, envs, args);

    EXPECT_TRUE(ret);
    EXPECT_EQ(args[0], "--detach");
    EXPECT_NE(std::find(args.begin(), args.end(), "--tty"), args.end());
    EXPECT_NE(std::find(args.begin(), args.end(), "--console-socket=" + sock), args.end());

    // 检查环境变量是否添加
    for (const auto &e : envs)
    {
        EXPECT_NE(std::find(args.begin(), args.end(), "--env=" + e), args.end());
    }

    // 检查用户参数
    std::string userArg = "--user=" + std::to_string(getuid()) + ":" + std::to_string(getgid());
    EXPECT_EQ(args.back(), userArg);
}