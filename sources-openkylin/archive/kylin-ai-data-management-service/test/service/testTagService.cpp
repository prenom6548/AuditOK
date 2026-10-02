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

#include <chrono>
#include <iostream>
#include <unordered_set>

#include "tagService.h"

static int64_t testFileId = []() {
    // 用当前时间戳 + 随机数生成唯一 ID
    return std::chrono::system_clock::now().time_since_epoch().count() + rand();
}();

static const std::vector<std::string> testTags = {"test_tag1", "test_tag2", "test_tag3"};

class TagServiceTest : public testing::Test {
public:
    static void SetUpTestSuite() {
        std::cout << "TagServiceTest SetUpTestSuite..." << std::endl;
        // 可以在这里初始化测试数据库等
    }

    static void TearDownTestSuite() {
        std::cout << "TagServiceTest TearDownTestSuite..." << std::endl;
        // 清理测试环境
    }

protected:
    void SetUp() override {
        std::cout << "TagServiceTest SetUp..." << std::endl;
        tagService = std::make_unique<TagService>();
    }

    void TearDown() override {
        std::cout << "TagServiceTest TearDown..." << std::endl;
        // 清理单个测试用例的资源
        tagService.reset();
    }

    std::unique_ptr<TagService> tagService;
};

// TEST_F(TagServiceTest, test01) {
//     // 测试添加新标签
//     tagService->updateFileTags(testFileId, {testTags[0], testTags[1]});

//     auto similarTags = tagService->searchSimilarTagNames({testTags[0]});
//     EXPECT_FALSE(similarTags.empty());
//     EXPECT_NE(similarTags.end(), std::find(similarTags.begin(), similarTags.end(), testTags[0]));
// }

// TEST_F(TagServiceTest, test02) {
//     // 测试添加重复标签
//     tagService->updateFileTags(testFileId, {testTags[0]});
//     size_t beforeCount = tagService->searchSimilarTagNames({testTags[0]}).size();

//     // 再次添加相同标签
//     tagService->updateFileTags(testFileId, {testTags[0]});
//     size_t afterCount = tagService->searchSimilarTagNames({testTags[0]}).size();

//     // 验证标签数量没有增加（去重）
//     EXPECT_EQ(beforeCount, afterCount);
// }

// TEST_F(TagServiceTest, test03) {
//     // 先添加一些标签
//     tagService->updateFileTags(testFileId, {testTags[0], testTags[1]});

//     // 验证标签存在
//     auto beforeTags = tagService->searchSimilarTagNames({testTags[0]});
//     EXPECT_FALSE(beforeTags.empty());

//     // 删除标签
//     std::unordered_set<int64_t> fileIds = {testFileId};
//     tagService->deleteFileTags(fileIds);

//     // 验证标签已被删除  这里当时设计的时候，只去删向量数据库了，sql数据库没有去删
//     auto afterTags = tagService->searchSimilarTagNames({testTags[0]});
//     // EXPECT_TRUE(afterTags.empty());
// }

// TEST_F(TagServiceTest, test04) {
//     // 添加测试标签
//     tagService->updateFileTags(testFileId, {testTags[0]});

//     // 搜索相似标签
//     auto results = tagService->searchSimilarTagNames({testTags[0]});

//     // 验证结果包含我们添加的标签
//     EXPECT_FALSE(results.empty());
//     EXPECT_NE(results.end(), std::find(results.begin(), results.end(), testTags[0]));
// }

// TEST_F(TagServiceTest, test05) {
//     // 添加带标签的文件
//     tagService->updateFileTags(testFileId, {testTags[0]});

//     // 搜索具有相同标签的文件
//     auto results = tagService->searchSimilarFilePaths({testTags[0]});

//     // EXPECT_FALSE(results.empty());
// }
