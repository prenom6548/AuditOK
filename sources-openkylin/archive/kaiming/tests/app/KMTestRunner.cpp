
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <cstdlib> // for setenv/unsetenv
#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

#define private public
#include "app/run/KMRunner.h"
#undef private
#include "app/kill/KMKill.h"
#include "common/KMInstalledAppQuery.h"

namespace fs = std::filesystem;

class KMTestRunner
{
public:
    KMRunner m_runner;
    KMKill m_killer;
};

TEST(KMTestRunner, test_run_with_wayland)
{
    KMTestRunner testRunner;
    KMInstalledAppQuery appQuery;

    // 查询已安装的应用
    std::vector<KMRef> refs = appQuery.kind("app").query(KMInstalledAppQuery::All);
    // 如果没有安装应用，则跳过测试
    if (refs.empty())
    {
        GTEST_SKIP() << "No installed apps found, skip run test";
    }

    std::string appid = refs.front().id;
    const char *argv[]  = {"run",  appid.c_str()};
    const char *argv2[] = {"kill", appid.c_str()};

    // 设置 WAYLAND_DISPLAY 环境变量，触发 mountWayland() 分支
    setenv("WAYLAND_DISPLAY", "wayland-0", 1);

    std::thread runThread([&]() 
    {
        EXPECT_EQ(testRunner.m_runner.dispose(2, const_cast<char**>(argv)), 0);
    });

    std::this_thread::sleep_for(std::chrono::seconds(2));
    testRunner.m_killer.dispose(2, const_cast<char**>(argv));

    runThread.join();

    // 清理环境变量
    unsetenv("WAYLAND_DISPLAY");
}

TEST(KMTestRunner, permDev_file)
{
    KMRunner runner;

    std::string dev = "/tmp/testdevfile";

    // 删除旧文件
    fs::remove(dev);

    // 创建一个普通文件设备模拟
    std::ofstream(dev).close();

    EXPECT_TRUE(runner.permDev(dev));
}

TEST(KMTestRunner, ParseRunCommandTest)
{
    KMRunner runner;

    // 1. 普通命令，不涉及路径前缀
    std::string cmd1 = "echo hello world";
    auto res1 = runner.parseRunCommand(cmd1);
    EXPECT_EQ(res1, " \"echo\" \"hello\" \"world\"");

    // 2. 命令前后有空格
    std::string cmd2 = "   ls -l   ";
    auto res2 = runner.parseRunCommand(cmd2);
    EXPECT_EQ(res2, " \"ls\" \"-l\"");

    // 3. 绝对路径，加前缀
    fs::path tmpFile = fs::temp_directory_path() / "testfile";
    std::ofstream(tmpFile).close();  // 创建文件
    std::string cmd3 = tmpFile.string();  // "/tmp/xxxx"
    auto res3 = runner.parseRunCommand(cmd3);
    EXPECT_EQ(res3, " \"/run/host/rootfs" + tmpFile.string() + "\"");

    // 4. 不加前缀的路径
    std::string cmd4 = "/home/user/app";
    auto res4 = runner.parseRunCommand(cmd4);
    EXPECT_EQ(res4, " \"/home/user/app\"");
}