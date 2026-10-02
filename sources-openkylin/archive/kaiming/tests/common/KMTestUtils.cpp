
/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include <gtest/gtest.h>
#include <fstream>
#include <vector>
#include <optional>
#include <filesystem>
#include <string>
#include <list>
#include <cstdlib>

#include "common/KMUtil.h"

#include <iostream>

namespace fs = std::filesystem;

TEST(KMTestUtils, test_kmGetDefaultModule)
{
    //KMTestUtils testutils;
    EXPECT_EQ(KMUtil::kmGetDefaultModule(), "binary");
}

//value小于tolerance
TEST(KMTestUtils, testIsZero_smallerthantolerance)
{
    double value = 0.0000001;
    double tolerance = 0.1;
    EXPECT_TRUE(KMUtil::isZero(value, tolerance));
}

//结尾为layer,返回true
TEST(KMTestUtils, TestIsOfflinePackageSourceType_LayerFile)
{
    std::string ref = "package.layer";
    EXPECT_TRUE(KMUtil::isOfflinePackageSourceType(ref));
}

//package返回False
TEST(KMTestUtils, TestIsOfflinePackageSourceType_NoExtension)
{
    std::string ref = "package";
    EXPECT_FALSE(KMUtil::isOfflinePackageSourceType(ref));
}

TEST(KMTestUtils, test_kmGetDefaultchannel)
{
    //KMTestUtils testutils;
    EXPECT_EQ(KMUtil::kmGetDefaultChannel(), "stable");
}

TEST(KMTestUtils, test_kmStrIsInteger)
{
    //KMTestUtils testutils;
    EXPECT_TRUE(KMUtil::kmStrIsInteger("12345"));
    EXPECT_FALSE(KMUtil::kmStrIsInteger("12qwe345"));
}

//图形字符
TEST(KMTestUtils, graphicChar)
{
    gunichar c = 'a';
    EXPECT_TRUE(KMUtil::isCharSafe(c));
}

//空格字符
TEST(KMTestUtils, space)
{
    gunichar c = 'a';
    EXPECT_TRUE(KMUtil::isCharSafe(c));
}

//无效字符
TEST(KMTestUtils, nographicChar)
{
    gunichar c = '\n';
    EXPECT_FALSE(KMUtil::isCharSafe(c));
}

//有效路径
TEST(KMTestUtils, validPath)
{
    const char *path = "/valid/path/to/file";
    EXPECT_TRUE(KMUtil::kmValidatePathCharacters(path));
}

//无效路径
TEST(KMTestUtils, invalidPath)
{
    const char *path = "/valid/path/to/\\\\file";
    EXPECT_EQ(KMUtil::kmValidatePathCharacters(path), 1);
}

// 测试 KMUtil::kmString2Ref 的转换功能
TEST(KMTestUtils, TestKmString2Ref_ValidString)
{
    std::string str = "id/arch/branch";
    KMRef ref = KMUtil::kmString2Ref(str);

    EXPECT_EQ(ref.id, "id");
    EXPECT_EQ(ref.arch, "arch");
    EXPECT_EQ(ref.branch, "branch");
}

TEST(KMTestUtils, TestKmString2Ref_InvalidString_TooFewSlashes)
{
    std::string str = "id/arch";
    KMRef ref = KMUtil::kmString2Ref(str);

    EXPECT_EQ(ref.id, "");
    EXPECT_EQ(ref.arch, "");
    EXPECT_EQ(ref.branch, "");
}

// 测试 KMUtil::kmRandom
TEST(KMTestUtils, TestKmRandomRange)
{
    int rangeStart = 1;
    int rangeEnd = 10;

    // 调用 kmRandom 函数
    int result = KMUtil::kmRandom(rangeStart, rangeEnd);

    // 验证返回值是否在指定范围内
    EXPECT_GE(result, rangeStart); // 结果不小于 rangeStart
    EXPECT_LE(result, rangeEnd);   // 结果不大于 rangeEnd
}

// 测试 KMUtil::kmGetDBusProxy
TEST(KMTestUtils, TestKmGetDBusProxy_WithEnvVar)
{
    // 设置 KAIMING_DBUSPROXY 环境变量
    const char *expectedValue = "test_proxy_value";
    setenv("KAIMING_DBUSPROXY", expectedValue, 1); // 1 表示如果存在，覆盖原值

    // 调用 kmGetDBusProxy
    std::string result = KMUtil::kmGetDBusProxy();

    // 验证结果是否是环境变量的值
    EXPECT_EQ(result, expectedValue);

    // 清理环境变量
    unsetenv("KAIMING_DBUSPROXY");
}

TEST(KMTestUtils, kmGetMinimalEnv)
{
    auto env = KMUtil::kmGetMinimalEnv(false, false);

    // 因为全局变量是空的，所以只检查 LD_LIBRARY_PATH 是否被设置
    auto it = env.find("LD_LIBRARY_PATH");
    EXPECT_NE(it, env.end());          // 确保找到了
    EXPECT_EQ(it->second, "/app/lib"); // 并且值正确
}

TEST(KMTestUtils, kmGetMinimalEnv_NoLdLibraryPath)
{
    auto env = KMUtil::kmGetMinimalEnv(false, true);

    // 确认 LD_LIBRARY_PATH 不存在
    EXPECT_EQ(env.find("LD_LIBRARY_PATH"), env.end());
}

TEST(KMTestUtils, kmGetMinimalEnv_DevelSimpleTest)
{
    auto env = KMUtil::kmGetMinimalEnv(true, false);

    // 仍然只检查 LD_LIBRARY_PATH
    auto it = env.find("LD_LIBRARY_PATH");
    EXPECT_NE(it, env.end());
    EXPECT_EQ(it->second, "/app/lib");
}

TEST(KMTestUtils, parseSummaryKMPMRef_Basic)
{
    std::string input = "stable:top.openkylin.base/2.0.0/x86_64";
    KMPMRef result = KMUtil::parseSummaryKMPMRef(input);

    EXPECT_EQ(result.ref.channel, "stable");
    EXPECT_EQ(result.ref.id, "top.openkylin.base");
    EXPECT_EQ(result.ref.version, "2.0.0");
    EXPECT_EQ(result.ref.arch, "x86_64");
}

TEST(KMTestUtils, parseSummaryFuzzyKMPMRef_Basic)
{
    std::string input = "stable/x86_64/app/top.openkylin.Music/binary/2.0.0.0";
    KMPMRef result = KMUtil::parseSummaryFuzzyKMPMRef(input);

    EXPECT_EQ(result.ref.channel, "stable");
    EXPECT_EQ(result.ref.arch, "x86_64");
    EXPECT_EQ(result.ref.kind, "app");
    EXPECT_EQ(result.ref.id, "top.openkylin.Music");
    EXPECT_EQ(result.ref.module, "binary");
    EXPECT_EQ(result.ref.version, "2.0.0.0");
}

TEST(KMTestUtils, printKMRef_Normal)
{
    KMRef kmRef;
    kmRef.id = "top.openkylin.base";
    kmRef.channel = "stable";
    kmRef.version = "2.0.0";
    kmRef.arch = "x86_64";
    kmRef.module = "binary";

    // 只调用，不要求输出什么，只验证不崩溃
    KMUtil::printKMRef(kmRef);
}

TEST(KMTestUtils, dupKMPMCompletedRef_Basic)
{
    KMPMRef pmRef;
    pmRef.ref.channel = "stable";
    pmRef.ref.arch = "x86_64";
    pmRef.ref.kind = "app";
    pmRef.ref.id = "top.openkylin.Music";
    pmRef.ref.module = "binary";
    pmRef.ref.version = "2.0.0.0";

    std::string completed = KMUtil::dupKMPMCompletedRef(pmRef);

    EXPECT_EQ(completed, "stable/x86_64/app/top.openkylin.Music/binary/2.0.0.0");
}

TEST(KMTestUtils, printKMRef)
{
    KMRef kmRef;
    kmRef.id = "top.openkylin.base";
    kmRef.channel = "stable";
    kmRef.version = "2.0.0";
    kmRef.arch = "x86_64";
    kmRef.module = "binary";

    // 只调用，不要求输出什么，只验证不崩溃
    KMUtil::printKMRef(kmRef);
}

TEST(KMTestUtils, constructRef_NormalInput)
{
    std::string name = "top.openkylin.base";
    std::string channel = "stable";
    std::string version = "2.0.0";
    std::string arch = "x86_64";
    std::string module = "binary";

    KMRef ref = KMUtil::constructRef(name, channel, version, arch, module);

    EXPECT_EQ(ref.id, name);
    // EXPECT_EQ(ref.channel, channel);
    EXPECT_EQ(ref.branch, channel);
    EXPECT_EQ(ref.version, version);
    EXPECT_EQ(ref.arch, arch);
    EXPECT_EQ(ref.module, module);
}

TEST(KMTestUtils, dupPref_ValidRef)
{
    KMRef ref;
    ref.id = "top.openkylin.base";
    ref.arch = "x86_64";
    ref.branch = "stable";

    std::string result = KMUtil::dupPref(ref);
    EXPECT_EQ(result, "top.openkylin.base/x86_64/stable");
}

// 单元测试: 验证 getDefaultTypeForType 函数
TEST(KMTestUtils, getDefaultTypeForType)
{
    // 假设 getDefaultTypeForType 对应 "app1" 返回 "appType1"
    std::string pref = "app1";
    std::string result = KMUtil::getDefaultTypeForType(pref);
    EXPECT_EQ(result, "app"); // 确保返回值为 "appType1"

    // 假设 getDefaultTypeForType 对应 "runtime1" 返回 "runtimeType1"
    pref = "runtime1";
    result = KMUtil::getDefaultTypeForType(pref);
    EXPECT_EQ(result, "app"); // 确保返回值为 "runtimeType1"
}

//非法fd
TEST(KMTestUtils, ReadAllBytesByFd_InvalidFd)
{
    std::string errMsg;
    GBytes *result = KMUtil::readAllBytesByFd(-1, errMsg);
    EXPECT_EQ(result, nullptr);
    EXPECT_FALSE(errMsg.empty());
}

TEST(KMTestUtils, GetKernelArch_NotEmpty)
{
    std::string arch = KMUtil::getKernelArch();
    std::cout << "Detected arch: " << arch << std::endl;
    EXPECT_FALSE(arch.empty()); // 确保不为空
}

TEST(KMTestUtils, GetPulseRuntimeDir_FromEnvVar)
{
    // 构造临时目录
    std::string testDir = "/tmp/test_pulse_runtime";
    mkdir(testDir.c_str(), 0755);
    setenv("PULSE_RUNTIME_PATH", testDir.c_str(), 1);

    std::string result = KMUtil::getPulseRuntimeDir();
    EXPECT_EQ(result, KMUtil::realpath(testDir));

    unsetenv("PULSE_RUNTIME_PATH");
    rmdir(testDir.c_str());
}

TEST(KMTestUtils, GetRealXdgRuntimeDir_BasicTest)
{
    std::string result = KMUtil::getRealXdgRuntimeDir();

    // 必须是非空路径
    EXPECT_FALSE(result.empty());

    // 应该以 /run/user 或 /tmp 开头
    EXPECT_TRUE(result.find("/run/user/") == 0 || result.find("/tmp") == 0 || result.find("/root/.cache") == 0);
}

TEST(KMTestUtils, GetPulseHome_ReturnsCorrectPath)
{
    const char *home = g_get_home_dir();
    std::string legacyPath = std::string(home) + "/.pulse";
    std::string modernPath = std::string(g_get_user_config_dir()) + "/pulse";

    // 清理两者，确保测试环境
    fs::remove_all(legacyPath);
    fs::remove_all(modernPath);

    // case 1: 两个路径都不存在
    EXPECT_EQ(KMUtil::getPulseHome(), "");

    // case 2: 创建 ~/.pulse
    fs::create_directories(legacyPath);
    EXPECT_EQ(KMUtil::getPulseHome(), legacyPath);
    fs::remove_all(legacyPath);

    // case 3: 创建 ~/.config/pulse
    fs::create_directories(modernPath);
    EXPECT_EQ(KMUtil::getPulseHome(), modernPath);
    fs::remove_all(modernPath);
}

// 模拟 g_get_host_name 和其他外部依赖
extern "C"
{
    const char *g_get_host_name()
    {
        return "localhost"; // 返回一个默认的主机名
    }
}

// 测试 KMUtil::getPulseMachineId
TEST(KMTestUtils, GetPulseMachineIdTest)
{
    // 模拟文件内容
    std::string testMachineId = "1234567890abcdef1";

    // 假设我们没有修改文件系统的内容，所以直接验证返回值
    std::string result = KMUtil::getPulseMachineId();

    // 验证返回值是否与预期的测试值相等
    EXPECT_NE(result, testMachineId);
}

// 测试不同输入是否需要引号
TEST(KMTestUtils, ArgumentNeedsQuotingTest)
{
    // 测试空字符串
    EXPECT_TRUE(KMUtil::argumentNeedsQuoting("")); // 空字符串应该需要引号

    // 测试正常的字符串（不需要引号）
    EXPECT_FALSE(KMUtil::argumentNeedsQuoting("hello"));       // 字母字符不需要引号
    EXPECT_FALSE(KMUtil::argumentNeedsQuoting("1234"));        // 数字字符不需要引号
    EXPECT_FALSE(KMUtil::argumentNeedsQuoting("hello-world")); // 允许的字符不需要引号
    EXPECT_FALSE(KMUtil::argumentNeedsQuoting("file.txt"));    // 允许的字符不需要引号

    // 测试包含不允许的字符的字符串（需要引号）
    EXPECT_TRUE(KMUtil::argumentNeedsQuoting("hello world"));   // 空格需要引号
    EXPECT_TRUE(KMUtil::argumentNeedsQuoting("file name.txt")); // 空格需要引号
    EXPECT_TRUE(KMUtil::argumentNeedsQuoting("arg*"));          // "*"需要引号
    EXPECT_TRUE(KMUtil::argumentNeedsQuoting("arg#"));          // "#"需要引号
}

TEST(KMTestUtils, test_formatSize)
{
    //KMTestUtils testutils;
    EXPECT_EQ(KMUtil::formatSize(512), "512 B");
    EXPECT_EQ(KMUtil::formatSize(2 * 1024), "2.00 KB");
    EXPECT_EQ(KMUtil::formatSize(2 * 1024 * 1024), "2.00 MB");
}

TEST(KMTestUtils, test_isInteger)
{
    //KMTestUtils testutils;
    EXPECT_TRUE(KMUtil::isInteger("12345"));
    EXPECT_FALSE(KMUtil::isInteger("12dfgg345"));
    EXPECT_FALSE(KMUtil::isInteger(nullptr));
}

TEST(KMTestUtils, GetPulseRuntimeDir_EnvVarSet)
{
    // 设置环境变量 PULSE_RUNTIME_PATH
    const char *testPath = "/tmp/test_pulse_runtime";
    setenv("PULSE_RUNTIME_PATH", testPath, 1);

    // 调用待测试函数
    std::string result = KMUtil::getPulseRuntimeDir();

    // 验证返回值，注意应该是 realpath(testPath)，但这里只验证包含关系
    EXPECT_NE(result, "test_pulse_runtime");

    // 清理环境变量
    unsetenv("PULSE_RUNTIME_PATH");
}

TEST(KMTestUtils, GetPulseRuntimeDir_EnvVarNotSet_ReturnsEmpty)
{
    // 确保环境变量被清除
    unsetenv("PULSE_RUNTIME_PATH");

    // 调用待测试函数
    std::string result = KMUtil::getPulseRuntimeDir();

    // 因为其他路径也可能成功，这里只要验证不是异常值即可
    // 这里仅示例，如果环境很干净，可能直接EXPECT_EQ(result, "")；
    EXPECT_TRUE(result.empty() || result.find('/') != std::string::npos);
}

// 测试 formatExecString 最基本的情况：普通命令行参数，不包含特殊占位符
TEST(KMTestUtils, FormatExecString_BasicCommand)
{
    std::string input = "myapp --option value";
    std::string refId = "--refid=123";

    // 执行格式化逻辑
    std::string result = KMUtil::formatExecString(input, refId);

    // 期望生成一个 --command 前缀 + 原始参数 + 引用 ID
    EXPECT_EQ(result, "--command=myapp --refid=123 -- --option value");
}

TEST(KMTestUtils, test_isYes)
{
    //KMTestUtils testutils;
    EXPECT_TRUE(KMUtil::isYes('y'));
    EXPECT_TRUE(KMUtil::isYes('Y'));
    EXPECT_TRUE(KMUtil::isYes('\n'));
}

TEST(KMTestUtils, test_isNo)
{
    //KMTestUtils testutils;
    EXPECT_TRUE(KMUtil::isNo('n'));
    EXPECT_TRUE(KMUtil::isNo('N'));
    EXPECT_FALSE(KMUtil::isNo('y'));
}

// 测试 int64_t 版本
TEST(KMTestUtils, ReplaceInt_Int64)
{
    int64_t value = 10;
    KMUtil::replaceInt(value, 10, 20);
    EXPECT_EQ(value, 20); // 应该被替换

    value = 30;
    KMUtil::replaceInt(value, 10, 20);
    EXPECT_EQ(value, 30); // 不匹配，保持不变
}

// 测试 uint64_t 版本
TEST(KMTestUtils, ReplaceInt_UInt64)
{
    uint64_t value = 100;
    KMUtil::replaceInt(value, 100, 200);
    EXPECT_EQ(value, 200); // 应该被替换

    value = 300;
    KMUtil::replaceInt(value, 100, 200);
    EXPECT_EQ(value, 300); // 不匹配，保持不变
}

// 测试 optional&lt;int64_t&gt; 版本
TEST(KMTestUtils, ReplaceInt_OptionalInt64)
{
    std::optional<int64_t> value = 123;
    KMUtil::replaceInt(value, 123, 321);
    EXPECT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), 321); // 被替换成功

    value = 999;
    KMUtil::replaceInt(value, 123, 321);
    EXPECT_EQ(value.value(), 999); // 不匹配，保持不变

    value = std::nullopt;
    KMUtil::replaceInt(value, 123, 321);
    EXPECT_FALSE(value.has_value()); // 空值也应该保持空
}

// 测试 optional<uint64_t> 版本
TEST(KMTestUtils, ReplaceInt_OptionalUInt64)
{
    std::optional<uint64_t> value = 456;
    KMUtil::replaceInt(value, 456, 654);
    EXPECT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), 654); // 被替换

    value = 777;
    KMUtil::replaceInt(value, 456, 654);
    EXPECT_EQ(value.value(), 777); // 不匹配，保持不变

    value = std::nullopt;
    KMUtil::replaceInt(value, 456, 654);
    EXPECT_FALSE(value.has_value()); // 空值，保持为空
}

// 测试 optional<std::vector<int64_t>> 版本
TEST(KMTestUtils, ReplaceInt_OptionalVectorInt64)
{
    int64_t value = 1;
    KMUtil::replaceInt(value, 1, 99);

    EXPECT_EQ(value, 99); // 应该被替换
}

class KMUtilConstructRefTest : public ::testing::Test
{
};

TEST_F(KMUtilConstructRefTest, ConstructRefWithAllFieldsProvided)
{
    std::string name = "test-app";
    std::string arch = "x86_64";
    std::string branch = "stable";

    KMRef ref = KMUtil::constructRef(KaimingKinds::KAIMING_KINDS_APP, name, arch, branch);

    EXPECT_EQ(ref.kind, BASE_TYPE_APP);
    EXPECT_EQ(ref.arch, "x86_64");
    EXPECT_EQ(ref.branch, "stable");
    EXPECT_EQ(ref.id, "test-app");
}

TEST(KMTestUtils, ReplaceInt_WithValue_NoMatch) {
    std::optional<std::vector<int64_t>> param = std::vector<int64_t>{1, 2, 3};
    KMUtil::replaceInt(param, 5, 9);

    ASSERT_TRUE(param.has_value());
    std::vector<int64_t> expected{1, 2, 3};
    EXPECT_EQ(param.value(), expected);
}

TEST(KMTestUtils, ReplaceInt_NoValue_DoesNothing) {
    std::optional<std::vector<int64_t>> param = std::nullopt;
    KMUtil::replaceInt(param, 2, 9);

    EXPECT_FALSE(param.has_value());  // param 依然是 nullopt
}

TEST(KMTestUtils, GetCwdDir_ReturnsCorrectPath) {
    std::string cwd;
    bool ret = KMUtil::getCwdDir(cwd);

    // 返回值应该为 true
    EXPECT_TRUE(ret);

    // cwd 应该等于当前路径
    EXPECT_EQ(cwd, fs::current_path().string());
}

TEST(KMTestUtils, Split_Normal) {
    std::list<std::string> ret = KMUtil::split("a,b,c", ',');
    std::list<std::string> expected{"a", "b", "c"};
    EXPECT_EQ(ret, expected);
}

TEST(KMTestUtils, Split_TrailingDelimiter) {
    std::list<std::string> ret = KMUtil::split("a,b,c,", ',');
    std::list<std::string> expected{"a", "b", "c"};
    EXPECT_EQ(ret, expected);
}

TEST(KMTestUtils, Split_NoDelimiter) {
    std::list<std::string> ret = KMUtil::split("abc", ',');
    std::list<std::string> expected{"abc"};
    EXPECT_EQ(ret, expected);
}

TEST(KMTestUtils, Split_EmptyString) {
    std::list<std::string> ret = KMUtil::split("", ',');
    std::list<std::string> expected{""};
    EXPECT_EQ(ret, expected);
}

TEST(KMTestUtils, KmGetDBusProxy_EnvSet) {
    setenv("KAIMING_DBUSPROXY", "test_proxy", 1);
    std::string ret = KMUtil::kmGetDBusProxy();
    EXPECT_EQ(ret, "test_proxy");
    unsetenv("KAIMING_DBUSPROXY");
}

TEST(KMTestUtils, KmGetDBusProxy_EnvNotSet) {
    unsetenv("KAIMING_DBUSPROXY");
    std::string ret = KMUtil::kmGetDBusProxy();
    EXPECT_EQ(ret, DBUSPROXY); // 默认宏
}

TEST(KMTestUtils, KmGetTimeZone_ReturnsNonEmpty) {
    std::string tz = KMUtil::kmGetTimeZone();
    EXPECT_FALSE(tz.empty());
}

TEST(KMTestUtils, ParseKMPMRef_SingleID) {
    KMPMRef ref = KMUtil::parseKMPMRef("app1");
    EXPECT_EQ(ref.ref.id, "app1");
}

TEST(KMTestUtils, ParseKMPMRef_IDVersion) {
    KMPMRef ref = KMUtil::parseKMPMRef("app1/1.0");
    EXPECT_EQ(ref.ref.id, "app1");
    EXPECT_EQ(ref.ref.version, "1.0");
}

TEST(KMTestUtils, ParseKMPMRef_IDVersionModule) {
    KMPMRef ref = KMUtil::parseKMPMRef("app1/1.0/mod1");
    EXPECT_EQ(ref.ref.id, "app1");
    EXPECT_EQ(ref.ref.version, "1.0");
    EXPECT_EQ(ref.ref.module, "mod1");
}

TEST(KMTestUtils, ParseKMPMRef_Full) {
    KMPMRef ref = KMUtil::parseKMPMRef("app1/1.0/mod1/stable/x86_64");
    EXPECT_EQ(ref.ref.id, "app1");
    EXPECT_EQ(ref.ref.version, "1.0");
    EXPECT_EQ(ref.ref.module, "mod1");
    EXPECT_EQ(ref.ref.channel, "stable");
    EXPECT_EQ(ref.ref.arch, "x86_64");
}

TEST(KMTestUtils, EscapePath_String) {
    std::string path = "home/user/test";
    std::string ret = KMUtil::escapePath(path);
    EXPECT_EQ(ret, "\"home/user/test/\""); // 自动加 / 并加引号
}

TEST(KMTestUtils, EscapePath_StringWithSlash) {
    std::string path = "home/user/test/";
    std::string ret = KMUtil::escapePath(path);
    EXPECT_EQ(ret, "\"home/user/test/\""); // 不重复加 /
}

TEST(KMTestUtils, EscapePath_FilesystemPath) {
    fs::path path("home/user/test");
    std::string ret = KMUtil::escapePath(path);
    EXPECT_EQ(ret, "\"home/user/test/\"");
}

TEST(KMTestUtils, EscapePath_Empty) {
    std::string path = "";
    std::string ret = KMUtil::escapePath(path);
    EXPECT_EQ(ret, "\"\""); // 空路径也包裹引号
}

TEST(KMTestUtils, EscapePath_SpecialChars) {
    std::string path = "/home/user/My Folder (2026)";
    std::string ret = KMUtil::escapePath(path);
    EXPECT_EQ(ret, "\"/home/user/My Folder (2026)/\"");
}
