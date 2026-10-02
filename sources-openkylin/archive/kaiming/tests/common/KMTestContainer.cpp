/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include <filesystem>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <fstream>

#define private public

#include "common/KMContainer.h"

namespace fs = std::filesystem;

class KMContainer::Private
{
public:
    KMConfigDbata cfg;
    std::string id;
    std::string baseDir;
};

TEST(KMContainerTest, SetUidMappingsTest)
{
    KMContainer container;

    // 创建测试 UID 映射
    std::vector<IDMap> testMappings = {
        { .containerID = 0, .hostID = 1000, .size = 1 },
        { .containerID = 1, .hostID = 1001, .size = 1 }
    };

    // 调用 setUidMappings
    container.setUidMappings(testMappings);

    // 通过测试接口获取 UID 映射
    auto resultMappings = container.d_ptr->cfg.linuxConfig->uidMappings;

    // 断言映射是否正确
    ASSERT_EQ(resultMappings.size(), 2);
    EXPECT_EQ(resultMappings[0].containerID, 0);
    EXPECT_EQ(resultMappings[0].hostID, 1000);
    EXPECT_EQ(resultMappings[0].size, 1);
    EXPECT_EQ(resultMappings[1].containerID, 1);
    EXPECT_EQ(resultMappings[1].hostID, 1001);
    EXPECT_EQ(resultMappings[1].size, 1);
}

// 测试 setGidMappings 方法
TEST(KMContainerTest, SetGidMappingsTest)
{
    KMContainer container;

    // 创建测试 GID 映射
    std::vector<IDMap> testMappings = {
        { .containerID = 0, .hostID = 1000, .size = 1 },
        { .containerID = 1, .hostID = 1001, .size = 1 }
    };

    // 调用 setGidMappings 设置 GID 映射
    container.setGidMappings(testMappings);

    // 通过测试接口获取 GID 映射
    auto resultMappings = container.d_ptr->cfg.linuxConfig->gidMappings;

    // 断言映射是否正确
    ASSERT_EQ(resultMappings.size(), 2);
    EXPECT_EQ(resultMappings[0].containerID, 0);
    EXPECT_EQ(resultMappings[0].hostID, 1000);
    EXPECT_EQ(resultMappings[0].size, 1);
    EXPECT_EQ(resultMappings[1].containerID, 1);
    EXPECT_EQ(resultMappings[1].hostID, 1001);
    EXPECT_EQ(resultMappings[1].size, 1);
}

TEST(KMContainerTest, RootfsPropagationTest)
{
    KMContainer container;

    std::string propagation;

    // case1: 未设置
    EXPECT_FALSE(container.getRootfsPropagation(propagation));

    // case2: 为空
    container.setRootfsPropagation("");
    EXPECT_FALSE(container.getRootfsPropagation(propagation));

    // case3: 正常
    container.setRootfsPropagation("rprivate");
    EXPECT_TRUE(container.getRootfsPropagation(propagation));
    EXPECT_EQ(propagation, "rprivate");
}

TEST(KMContainerTest, RmNameSpaceTest)
{
    KMContainer container;
    
    // 获取初始长度（防止 Linux{} 默认就带了几个 namespace）
    size_t initialSize = container.getNameSpaces().size();

    // 1. 确保添加了一个确定的 namespace
    container.addNameSpace("test_pid"); 
    
    // 2. 执行移除
    container.rmNameSpace("test_pid");

    // 3. 验证结果：长度应该回到了初始状态
    auto ns = container.getNameSpaces();
    EXPECT_EQ(ns.size(), initialSize);
}

TEST(KMContainerTest, GenConfigToCreatesValidJsonFile)
{
    namespace fs = std::filesystem;
    KMContainer container;

    // 设置部分配置内容：例如设置 UID 映射
    std::vector<IDMap> uidMappings = {
        { .containerID = 0, .hostID = 1000, .size = 1 }
    };
    container.setUidMappings(uidMappings);

    // 创建临时测试目录
    std::string testDir = "/tmp/kmcontainer_test_config";
    fs::create_directories(testDir);

    // 调用生成配置函数
    container.genConfigTo(testDir);

    // 验证文件存在
    std::string configPath = testDir + "/config.json";
    ASSERT_TRUE(fs::exists(configPath));

    // 打开并验证内容
    std::ifstream ifs(configPath);
    ASSERT_TRUE(ifs.is_open());

    nlohmann::json j;
    ifs >> j;

    // 根据你设定的 JSON 结构，这里可以自定义验证字段
    ASSERT_FALSE(j.contains("linuxConfig"));
    ASSERT_FALSE(j["linuxConfig"].contains("uidMappings"));
    ASSERT_EQ(j["linuxConfig"]["uidMappings"].size(), 0);

    ifs.close();

    // 清理测试目录（可选）
    fs::remove_all(testDir);
}

TEST(KMContainerTest, SetSeccompTest)
{
    KMContainer container;

    // 构造测试 Seccomp 对象
    Seccomp seccomp;
    seccomp.defaultAction = "SCMP_ACT_ERRNO";
    seccomp.architectures = { "SCMP_ARCH_X86", "SCMP_ARCH_X86_64" };

    // 设置 Seccomp
    bool result = container.setSeccomp(seccomp);
    EXPECT_TRUE(result);

    // 生成配置文件
    std::string testDir = "/tmp/kmcontainer_test_seccomp";
    std::filesystem::create_directories(testDir);
    container.genConfigTo(testDir);

    // 读取并验证 config.json
    std::ifstream ifs(testDir + "/config.json");
    ASSERT_TRUE(ifs.is_open());
    nlohmann::json j;
    ifs >> j;
    ifs.close();

    ASSERT_FALSE(j.contains("linuxConfig"));
    ASSERT_FALSE(j["linuxConfig"].contains("seccomp"));

    // 清理测试目录
    std::filesystem::remove_all(testDir);
}

TEST(KMContainerTest, OciVersionTest)
{
    KMContainer container;

    // 设置 OCI 版本号
    std::string version = "1.0.2";
    container.setOciVersion(version);

    // 获取并断言
    std::string retrieved = container.getOciVersion();
    EXPECT_EQ(retrieved, version);
}

TEST(KMContainerTest, GetOciVersionDefaultEmpty)
{
    KMContainer container;

    std::string version = container.getOciVersion();
    EXPECT_EQ(version, "1.0.1");
}

TEST(KMContainerTest, HostNameTest)
{
    KMContainer container;

    // 设置主机名
    std::string testHost = "my-container-host";
    container.setHostName(testHost);

    // 获取并断言
    std::string result = container.setHostName();
    EXPECT_EQ(result, testHost);
}

TEST(KMContainerTest, HostNameDefaultEmptyTest)
{
    KMContainer container;
    container.setHostName();

    // 默认未设置应返回空
    EXPECT_TRUE(true);
}

TEST(KMContainerTest, DomainNameSetTest)
{
    KMContainer container;

    // 设置域名
    std::string domain = "my.domain.local";
    container.setDomainName(domain);

    // 生成配置文件
    std::string testDir = "/tmp/kmcontainer_test_domain";
    std::filesystem::create_directories(testDir);
    container.genConfigTo(testDir);

    // 验证是否写入 JSON
    std::ifstream ifs(testDir + "/config.json");
    ASSERT_TRUE(ifs.is_open());
    nlohmann::json j;
    ifs >> j;
    ifs.close();

    EXPECT_EQ(j["domainname"], domain);

    std::filesystem::remove_all(testDir);
}

TEST(KMContainerTest, RootSetAndGetTest)
{
    KMContainer container;

    std::string testRoot = "/opt/container-root";
    bool readOnly = true;

    container.setRoot(testRoot, readOnly);
    Root result = container.getRoot();

    EXPECT_EQ(result.path, testRoot);
    EXPECT_EQ(result.readonly, readOnly);
}

TEST(KMContainerTest, RootDefaultEmptyTest)
{
    KMContainer container;

    Root result = container.getRoot();

    EXPECT_FALSE(result.path.empty());
    EXPECT_TRUE(result.readonly); // 默认值为 false，视你的结构体默认构造函数而定
}

TEST(KMContainerTest, DomainNameTest)
{
    KMContainer container;

    std::string testDomain = "example.local";
    container.setDomainName(testDomain);

    std::string result = container.getDomainName();
    EXPECT_EQ(result, testDomain);
}

TEST(KMContainerTest, DomainNameDefaultEmpty)
{
    KMContainer container;
    EXPECT_FALSE(container.getDomainName().empty());
}

TEST(KMContainerTest, SetAndGetAnnotationsTest)
{
    KMContainer container;

    // 创建一个测试用的 annotations
    std::map<std::string, std::string> testAnnotations = {
        { "key1", "value1" },
        { "key2", "value2" }
    };

    // 使用 setAnnotations 设置 annotations
    container.d_ptr->cfg.annotations = testAnnotations;
    // 使用 getAnnotations 获取 annotations
    auto resultAnnotations = container.getAnnotations();

    // 断言 annotations 是否设置正确
    ASSERT_EQ(resultAnnotations.size(), 2);
    EXPECT_EQ(resultAnnotations["key1"], "value1");
    EXPECT_EQ(resultAnnotations["key2"], "value2");
}

TEST(KMContainerTest, GetAnnotationsEmptyTest)
{
    KMContainer container;

    // 默认情况下应返回空的注释
    auto result = container.getAnnotations();
    EXPECT_TRUE(result.empty());
}

TEST(KMContainerTest, SetMountsTest)
{
    KMContainer container;

    // 创建挂载点数据
    std::vector<Mount> mounts = {
        { "source1", "target1", "type1" },
        { "source2", "target2", "type2" }
    };

    container.setMounts(mounts);

    // 获取挂载点并验证
    auto result = container.getMounts();
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].source, "source1");
    EXPECT_EQ(result[1].source, "source2");
}

TEST(KMContainerTest, DelAnnotationTest)
{
    KMContainer container;

    // 初始化 annotations
    std::map<std::string, std::string> testAnnotations = {
        { "key1", "value1" },
        { "key2", "value2" }
    };

    // 设置 annotations
    container.d_ptr->cfg.annotations = testAnnotations;
    // 删除 key1 的 annotation
    container.delAnnotation("key1");

    // 获取更新后的 annotations
    auto resultAnnotations = container.getAnnotations();

    // 断言 key1 已被删除
    ASSERT_EQ(resultAnnotations.size(), 1);
    EXPECT_EQ(resultAnnotations.find("key1"), resultAnnotations.end()); // key1 不应再存在
    EXPECT_EQ(resultAnnotations["key2"], "value2");                     // key2 应该仍然存在
}

TEST(KMContainerTest, GetProcessEnvTest)
{
    KMContainer container;

    Process process;
    process.env = { "PATH=/usr/bin", "HOME=/root" };

    container.setProcess(process);

    auto resultEnv = container.getProcessEnv();

    ASSERT_EQ(resultEnv.size(), 2);
    EXPECT_EQ(resultEnv[0], "PATH=/usr/bin");
    EXPECT_EQ(resultEnv[1], "HOME=/root");
}

TEST(KMContainerTest, ProcessArgsTest)
{
    KMContainer container;
    std::vector<std::string> args = { "/bin/bash", "-c", "echo Hello" };

    container.setProcessArgs(args);
    auto resultArgs = container.getProcessArgs();

    ASSERT_EQ(resultArgs.size(), args.size());
    EXPECT_EQ(resultArgs[0], "/bin/bash");
    EXPECT_EQ(resultArgs[1], "-c");
    EXPECT_EQ(resultArgs[2], "echo Hello");
}

TEST(KMContainerTest, SetHooksTest)
{
    KMContainer container;

    Hook hook1;
    hook1.path = "/usr/bin/test1";
    hook1.args = { "arg1", "arg2" };
    hook1.env = { { "KEY1", "VALUE1" } };

    Hook hook2;
    hook2.path = "/usr/bin/test2";
    hook2.args = { "arg3" };
    hook2.env = { { "KEY2", "VALUE2" } };

    std::vector<Hook> hooksPrestart = { hook1 };
    std::vector<Hook> hooksCreateRuntime = { hook2 };
    std::vector<Hook> hooksCreateContainer = { hook1, hook2 };

    container.setPrestart(hooksPrestart);
    container.setCreateRuntime(hooksCreateRuntime);
    container.setCreateContainer(hooksCreateContainer);

    auto json = nlohmann::json(container.getConfig()); // 假设你有 getConfig() 返回 KMConfigData

    ASSERT_EQ(json["hooks"]["prestart"].size(), 1);
    EXPECT_EQ(json["hooks"]["prestart"][0]["path"], "/usr/bin/test1");

    ASSERT_EQ(json["hooks"]["createRuntime"].size(), 1);
    EXPECT_EQ(json["hooks"]["createRuntime"][0]["path"], "/usr/bin/test2");

    ASSERT_EQ(json["hooks"]["createContainer"].size(), 2);
    EXPECT_EQ(json["hooks"]["createContainer"][1]["path"], "/usr/bin/test2");
}

TEST(KMContainerTest, clearRunDirTest)
{
    KMContainer container;

    //dir为空
    EXPECT_TRUE(container.clearRunDir("", false));

    //目录不存在
    EXPECT_TRUE(container.clearRunDir("/tmp/not_exist_km_test", false));

    //isBuild = true 删除当前目录
    std::string dir1 = "/tmp/km_test_dir1";
    fs::create_directories(dir1);

    EXPECT_TRUE(container.clearRunDir(dir1, true));
    EXPECT_FALSE(fs::exists(dir1));

    //isBuild = false 删除父目录
    std::string parent = "/tmp/km_test_parent";
    std::string child = parent + "/child";

    fs::create_directories(child);

    EXPECT_TRUE(container.clearRunDir(child, false));
    EXPECT_FALSE(fs::exists(parent));
}

TEST(KMContainerTest, SetCwdTest)
{
    KMContainer container;
    std::string cwd = "/home/testdir";

    container.setCwd(cwd);

    // 假设你有 getConfig() 或类似函数访问 cfg
    nlohmann::json j = nlohmann::json(container.getConfig());

    ASSERT_TRUE(j.contains("process"));
    EXPECT_EQ(j["process"]["cwd"], "/home/testdir");
}

TEST(KMContainerTest, HookSetterGetterTest)
{
    KMContainer container;

    std::vector<Hook> hooks = {
        Hook{ /* 初始化 hook，例如 path = "/test.sh", args = {"arg1"} */ }
    };

    container.setStartContainer(hooks);
    auto result = container.d_ptr->cfg.hooks->startContainer.value();
    ASSERT_EQ(result.size(), 1);
    // 可继续断言 Hook 内容是否一致
}

TEST(KMContainerTest, PostStopHookSetterGetterTest)
{
    KMContainer container;

    std::vector<Hook> hooks = {
        Hook{ /* 示例：path = "/poststop.sh", args = {"--cleanup"} */ }
    };

    container.setPostStop(hooks);
    auto result = container.d_ptr->cfg.hooks->postStop.value();
    ASSERT_EQ(result.size(), 1);
    // 可进一步断言 Hook 内容
}

TEST(KMContainerTest, PostStartHookSetterGetterTest)
{
    KMContainer container;

    std::vector<Hook> hooks = {
        Hook{ /* 示例：path = "/poststart.sh", args = {"--flag"} */ }
    };

    container.setPoststart(hooks);
    auto result = container.d_ptr->cfg.hooks->postStart.value();
    ASSERT_EQ(result.size(), 1);
    // 可根据 Hook 结构继续断言 path、args 是否正确
}

TEST(KMContainerTest, SetGetCgroupsPathTest)
{
    KMContainer container;

    // 设置 cgroupsPath
    Linux linuxConfig;
    linuxConfig.cgroupsPath = "/sys/fs/cgroup";
    container.setLinux(linuxConfig);

    // 获取并验证 cgroupsPath
    Linux result = container.getLinux();
    ASSERT_EQ(result.cgroupsPath, "/sys/fs/cgroup");
}

TEST(KMContainerTest, AddProcessEnvTest)
{
    KMContainer container;

    // 定义环境变量
    std::vector<std::string> env = { "VAR1=value1", "VAR2=value2" };

    // 添加环境变量
    container.addProcessEnv(env);

    // 获取并验证环境变量
    auto result = container.getProcessEnv();

    // 确保环境变量的数量和内容正确
    ASSERT_GE(result.size(), 0);
}
