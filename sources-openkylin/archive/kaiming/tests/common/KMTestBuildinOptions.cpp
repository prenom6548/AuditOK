/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include <gtest/gtest.h>
#include <sstream>
#include "common/KMBuildinOptions.h"


// 测试 OptionDescription::formatName()
TEST(OptionDescriptionTest, BasicFormatName) {
    // 测试1: 有长名和短名
    KMOption::OptionDescription opt1("long", "s", nullptr, "desc");
    opt1.print();
    EXPECT_EQ(opt1.formatName(), "-s [ --long ]");
    
    // 测试2: 只有长名
    KMOption::OptionDescription opt2("long", "", nullptr, "desc");
    EXPECT_EQ(opt2.formatName(), "--long");
}

// 测试 PositionOptionDescription 基本功能
TEST(PositionOptionDescriptionTest, BasicFunctions) {
    // 基本打印测试
    std::stringstream buffer;
    auto old = std::cout.rdbuf(buffer.rdbuf());
    
    KMOption::PositionOptionDescription opt("name", nullptr, 1, "desc");
    opt.print();  // 确保能调用不报错
    
    std::cout.rdbuf(old);
    EXPECT_FALSE(buffer.str().empty());
}

// 创建 Options 的测试派生类
class TestOptions : public KMOption::Options {
public:
    void preParseHook() override {}
    void postParseHook() override {}
    
    // 将 protected 方法公开以便测试
    using KMOption::Options::showUsage;
    using KMOption::Options::getPositionalOptionIndex;
    using KMOption::Options::addOption;
    using KMOption::Options::addPositionOption;
};

// 测试 showUsage() 函数
TEST(OptionsTest, ShowUsage) {
    TestOptions options;
    
    // 测试空选项的情况
    {
        std::stringstream buffer;
        auto old = std::cout.rdbuf(buffer.rdbuf());
        
        options.showUsage();
        
        std::cout.rdbuf(old);
        EXPECT_TRUE(buffer.str().find("Options:") == std::string::npos);
        EXPECT_TRUE(buffer.str().find("Position Options:") == std::string::npos);
    }
    
    // 添加选项后测试
    options.addOption("long1", "s1", nullptr, "description1");
    options.addPositionOption("pos1", nullptr, 1, "pos desc1");
    
    {
        std::stringstream buffer;
        auto old = std::cout.rdbuf(buffer.rdbuf());
        
        options.showUsage();
        
        std::cout.rdbuf(old);
        EXPECT_TRUE(buffer.str().find("Options:") != std::string::npos);
        EXPECT_TRUE(buffer.str().find("Position Options:") != std::string::npos);
        EXPECT_TRUE(buffer.str().find("long1") != std::string::npos);
        EXPECT_TRUE(buffer.str().find("pos1") != std::string::npos);
    }
}

// 测试 getPositionalOptionIndex() 函数
TEST(OptionsTest, GetPositionalOptionIndex) {
    TestOptions options;
    
    // 测试空位置选项的情况
    EXPECT_EQ(options.getPositionalOptionIndex(1), -1);
    
    // 添加位置选项
    options.addPositionOption("pos1", nullptr, 1, "desc1");
    options.addPositionOption("pos2", nullptr, 1, "desc2");
    
    // 测试无效索引
    EXPECT_EQ(options.getPositionalOptionIndex(0), -1);
    EXPECT_EQ(options.getPositionalOptionIndex(3), -1);
    
    // 测试有效索引（由于没有调用 parseCommandLine，m_argIndex 默认为 -1）
    EXPECT_EQ(options.getPositionalOptionIndex(1), -1);
    EXPECT_EQ(options.getPositionalOptionIndex(2), -1);
}