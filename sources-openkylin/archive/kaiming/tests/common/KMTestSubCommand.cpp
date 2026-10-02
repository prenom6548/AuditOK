/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include <gtest/gtest.h>
#include "common/KMSubCommand.h"
#include "common/KMDynamicCreator.h"

// 测试子命令实现类
class TestSubCommand : public KMSubCommand {
public:
    int dispose(int argc, char **argv) override {
        return 0; // 简单实现
    }
};

// 测试 KMSubCommandFactory 功能
TEST(KMSubCommandFactoryTest, BasicFunctionality) {
    // 准备测试数据 - 根据实际 SubCommandDescription 结构调整
    SubCommandDescription desc1;
    desc1.m_name = "cmd1";
    desc1.m_description = "Description1";
    
    SubCommandDescription desc2;
    desc2.m_name = "cmd2";
    desc2.m_description = "Description2";
    
    std::vector<SubCommandDescription> commands{desc1, desc2};
    
    // 测试设置子命令描述
    KMSubCommandFactory::setSubCommandDescriptions(commands);
    const auto& retrieved = KMSubCommandFactory::subCommandDescriptions();
    
    EXPECT_EQ(retrieved.size(), 2);
    EXPECT_EQ(retrieved[0].m_name, "cmd1");
    EXPECT_EQ(retrieved[1].m_name, "cmd2");
}

// 测试创建子命令
TEST(KMSubCommandFactoryTest, CreateSubCommand) {
    // 注册测试子命令类
    auto constructor = []() -> KMObject* { 
        return new TestSubCommand(); 
    };
    KMDynamicCreator::registerClass("test_cmd", "TestSubCommand", constructor);
    
    // 测试创建存在的子命令
    auto cmd = KMSubCommandFactory::createKMSubCommand("test_cmd");
    EXPECT_NE(cmd.get(), nullptr);
    
    // 测试创建不存在的子命令
    auto invalidCmd = KMSubCommandFactory::createKMSubCommand("invalid_cmd");
    EXPECT_EQ(invalidCmd.get(), nullptr);
}

// 测试空描述列表
TEST(KMSubCommandFactoryTest, EmptyDescriptions) {
    // 设置空描述列表
    KMSubCommandFactory::setSubCommandDescriptions({});
    const auto& retrieved = KMSubCommandFactory::subCommandDescriptions();
    
    EXPECT_TRUE(retrieved.empty());
}
