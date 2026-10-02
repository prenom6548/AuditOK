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

#include "baseTask.h"
#include "nlpBaseTask.h"
#include "summaryTask.h"
#include "tagTask.h"
#include "textEmbeddingTask.h"

class BaseTaskTest : public testing::Test {
public:
    // 所有案例执行之前
    static void SetUpTestSuite();
    // 所有案例执行之后
    static void TearDownTestSuite();

protected:
    // 每个案例之前执行
    void SetUp() override { std::cout << "BaseTaskTest SetUp..." << std::endl; }
    // 每个案例之后执行
    void TearDown() override { std::cout << "BaseTaskTest TearDown..." << std::endl; }

private:
};

void BaseTaskTest::SetUpTestSuite() { std::cout << "BaseTaskTest SetUpTestSuite..." << std::endl; }

void BaseTaskTest::TearDownTestSuite() { std::cout << "BaseTaskTest TearDownTestSuite..." << std::endl; }

// 测试子类继承 BaseTask
namespace {
class TestClass : public DataManagement::BaseTask {
public:
    explicit TestClass() : DataManagement::BaseTask("test01") {}
    void handlerTask() override { std::cout << "TestClass handlerTask..." << std::endl; }

    void stopTask() override { std::cout << "TestClass stopTask..." << std::endl; }

    bool haveTask() override { return true; }
    bool isReadyToStart() override { return true; }
    bool isReadyToStop() override { return true; }
};
}  // namespace name

// 测试基类中的虚函数
TEST_F(BaseTaskTest, test01) {
    // 测试开启任务
    std::shared_ptr<DataManagement::BaseTask> base = std::make_shared<TestClass>();
    EXPECT_TRUE(base->haveTask());
    EXPECT_TRUE(base->isReadyToStart());
    EXPECT_TRUE(base->isReadyToStop());

    base->handlerTask();
    base->stopTask();
}

// 通过基类测试所有子类的公共部分
TEST_F(BaseTaskTest, test02) {
    // 测试开启任务
    using namespace DataManagement;

    const std::map<int, std::shared_ptr<BaseTask>> taskTypeMap = {
        {TASK_TYPE_TEXT_EMBEDDING, std::make_shared<TextEmbeddingTask>()},
        {TASK_TYPE_TAG, std::make_shared<TagTask>()},
        {TASK_TYPE_SUMMARY, std::make_shared<SummaryTask>()},
    };
    std::vector<std::shared_ptr<BaseTask>> taskLists;

    std::vector<int> taskType = {1, 2, 3};  // 1 向量化 2 标签 3 摘要
    for (const auto& item : taskType) {
        const std::map<int, std::shared_ptr<BaseTask>>::const_iterator& it = taskTypeMap.find(item);
        if (it == taskTypeMap.end()) {
            continue;
        }
        taskLists.push_back(it->second);
    }

    for (const auto& worker : taskLists) {
        worker->start();
    }
}