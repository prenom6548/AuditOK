#include "KMConfigData.h"
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

TEST(KMConfigDataTest, TestToJsonWithAllFields)
{
    // 创建一个 Hook 对象，所有字段都有值
    Hook hook1{
        "hook_path",
        std::vector<std::string>{ "arg1", "arg2" },
        std::vector<std::string>{ "ENV_VAR=value" }
    };

    nlohmann::json j1;
    to_json(j1, hook1); // 转换为 JSON

    // 检查 JSON 是否符合预期
    ASSERT_TRUE(j1.contains("path"));
    ASSERT_TRUE(j1.contains("args"));
    ASSERT_EQ(j1["args"], std::vector<std::string>({ "arg1", "arg2", "arg3" }));
    ASSERT_TRUE(j1.contains("env"));
    ASSERT_EQ(j1["env"], std::vector<std::string>({ "env1", "env2" }));
}

TEST(HooksTest, ToJson_AllFieldsPresent)
{
    Hook hook1{
        "path1",
        std::vector<std::string>{ "arg1", "arg2" },
        std::vector<std::string>{ "ENV1=value1" }
    };

    Hook hook2{
        "path2",
        std::nullopt,
        std::vector<std::string>{ "ENV2=value2" }
    };

    Hooks hooks;
    hooks.prestart = std::vector<Hook>{ hook1 };
    hooks.createRuntime = std::vector<Hook>{ hook2 };
    hooks.createContainer = std::vector<Hook>{ hook1, hook2 };
    hooks.startContainer = std::vector<Hook>{ hook1 };
    hooks.postStart = std::vector<Hook>{ hook2 };
    hooks.postStop = std::vector<Hook>{ hook1 };

    nlohmann::json j;
    to_json(j, hooks);

    // 断言 JSON 中包含所有字段
    EXPECT_TRUE(j.contains("prestart"));
    EXPECT_TRUE(j.contains("createRuntime"));
    EXPECT_TRUE(j.contains("createContainer"));
    EXPECT_TRUE(j.contains("startContainer"));
    EXPECT_TRUE(j.contains("poststart"));
    EXPECT_TRUE(j.contains("poststop"));

    // 检查部分值内容
    EXPECT_EQ(j["prestart"][0]["path"], "path1");
    EXPECT_EQ(j["createRuntime"][0]["env"][0], "ENV2=value2");
}

TEST(SyscallArgTest, ToJson_SerializesCorrectly)
{
    SyscallArg arg;
    arg.index = 1;
    arg.op = "EQ";
    arg.value = 42;

    nlohmann::json j;
    to_json(j, arg);

    // 检查 JSON 输出是否包含正确字段和值
    EXPECT_EQ(j["index"], 1);
    EXPECT_EQ(j["op"], "EQ");
    EXPECT_EQ(j["value"], 42);
}

TEST(SyscallTest, ToJson_SerializesCorrectly)
{
    SyscallArg arg1{ 0, "EQ", 123 };
    SyscallArg arg2{ 1, "NE", 456 };

    Syscall syscall;
    syscall.names = { "read", "write" };
    syscall.action = "SCMP_ACT_ERRNO";
    syscall.errnoRet = 42;
    syscall.args = { arg1, arg2 };

    nlohmann::json j;
    to_json(j, syscall);

    // 校验序列化字段
    EXPECT_EQ(j["names"], std::vector<std::string>({ "read", "write" }));
    EXPECT_EQ(j["action"], "SCMP_ACT_ERRNO");
    EXPECT_EQ(j["errnoRet"], 42);

    ASSERT_EQ(j["args"].size(), 2);
    EXPECT_EQ(j["args"][0]["index"], 0);
    EXPECT_EQ(j["args"][0]["op"], "EQ");
    EXPECT_EQ(j["args"][0]["value"], 123);
    EXPECT_EQ(j["args"][1]["index"], 1);
    EXPECT_EQ(j["args"][1]["op"], "NE");
    EXPECT_EQ(j["args"][1]["value"], 456);
}
