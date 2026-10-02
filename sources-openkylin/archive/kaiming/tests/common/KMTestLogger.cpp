/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include "gtest/gtest.h"
#include <sstream>
#include <iostream>
#include <functional>
#include <cstdio>

#include "common/KMLogger.h"

// 捕获 std::cout 和 std::cerr 输出
std::string captureOutput(std::function<void()> func)
{
    std::stringstream buffer;
    auto* oldCout = std::cout.rdbuf(buffer.rdbuf());
    auto* oldCerr = std::cerr.rdbuf(buffer.rdbuf());
    func();
    std::cout.rdbuf(oldCout);
    std::cerr.rdbuf(oldCerr);
    return buffer.str();
}

class KMLoggerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // 重置日志等级为最低，保证所有日志级别都输出
        kmlogger.setLogLevel(KMLogLevel::trace);
    }
};

// 测试日志等级设置（枚举）
TEST_F(KMLoggerTest, SetLogLevelEnum)
{
    kmlogger.setLogLevel(KMLogLevel::debug);
    std::string out = captureOutput([&]() {
        kmlogger.debugLog("debug msg");
    });
    EXPECT_NE(out.find("debug msg"), std::string::npos);
}

// 测试日志等级设置（字符串）
TEST_F(KMLoggerTest, SetLogLevelString)
{
    kmlogger.setLogLevel("info");
    std::string out = captureOutput([&]() {
        kmlogger.infoLog("info message");
        kmlogger.warnLog("warn message");
    });
    EXPECT_NE(out.find("info message"), std::string::npos);
    EXPECT_NE(out.find("warn message"), std::string::npos);
}

// 测试 pinfo 和 perror 宏
TEST_F(KMLoggerTest, PinfoPerror)
{
    std::string out = captureOutput([&]() {
        kmlogger.pinfo("pinfo msg");
        kmlogger.perror("perror msg");
    });
    EXPECT_NE(out.find("pinfo msg"), std::string::npos);
    EXPECT_NE(out.find("perror msg"), std::string::npos);
}

// // 测试格式化输出
// TEST_F(KMLoggerTest, FormatString)
// {
//     std::string out = captureOutput([&]() {
//         kmlogger.trace("str %d", 123);
//         kmlogger.debug("str %d", 123);
//         kmlogger.info("str %d", 123);
//         kmlogger.warn("str %d", 123);
//         kmlogger.error("str %d", 123);
//         kmlogger.critical("str %d", 123);
//     });

//     EXPECT_NE(out.find("str 123"), std::string::npos);
// }

// 测试未知字符串日志等级
TEST_F(KMLoggerTest, SetLogLevelUnknownString)
{
    kmlogger.setLogLevel("unknown_level");
    // 不崩溃即可
    SUCCEED();
}

// 测试 switch 枚举日志等级
TEST_F(KMLoggerTest, SetLogLevelEnumSwitch)
{
    kmlogger.setLogLevel(KMLogLevel::warn);
    // 不崩溃即可
    SUCCEED();
}