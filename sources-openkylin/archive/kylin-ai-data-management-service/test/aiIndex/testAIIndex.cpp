/*
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include <iostream>

#include "aiIndex.h"
#include "systemCallback.h"

class AIIndexTest : public testing::Test {
public:
    // 所有案例执行之前
    static void SetUpTestSuite();
    // 所有案例执行之后
    static void TearDownTestSuite();

protected:
    // 每个案例之前执行
    void SetUp() override { std::cout << "AIIndexTest SetUp..." << std::endl; }
    // 每个案例之后执行
    void TearDown() override { std::cout << "AIIndexTest TearDown..." << std::endl; }

private:
};

void AIIndexTest::SetUpTestSuite() { std::cout << "AIIndexTest SetUpTestSuite..." << std::endl; }

void AIIndexTest::TearDownTestSuite() { std::cout << "AIIndexTest TearDownTestSuite..." << std::endl; }

TEST_F(AIIndexTest, test01) {
    // 测试AIIndex 启动
    EXPECT_EQ(DataManagement::AIIndex::getInstance().exec(), 0);
    EXPECT_FALSE(DataManagement::AIIndex::getInstance().isIdle());  // 由于当前在测试肯定为非空闲
}
TEST_F(AIIndexTest, test02) {
    // 测试状态机
    /*
     * system-signal    idle-status
     *      0               0       忙碌
     *      3               1       空闲
     *  在收到系统空闲信号后，等待一分钟进行真正的空闲  idle-status 被设置成2
     *  状态机的切换逻辑：单向
     */

    // 由于获得不了 IdleState的状态，所以这个函数先不测
    DataManagement::AIIndex::getInstance().setIdleState(DataManagement::IdleState::IDLE_STATE_BUSY);
}

TEST_F(AIIndexTest, test03) {
    // 测试索引开关
    DataManagement::AIIndex::getInstance().isEnable();
    DataManagement::AIIndex::getInstance().setAIIndexEnable(false);
}
