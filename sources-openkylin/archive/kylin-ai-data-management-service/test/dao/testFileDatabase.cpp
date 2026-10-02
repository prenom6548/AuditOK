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

#include <filesystem>
#include <iostream>
#include <memory>
#include <numeric>
#include <unordered_set>
#include <vector>

#include "constant.h"
#include "fileDatabase.h"

std::string joinIds(const std::unordered_set<int64_t>& ids) {
    std::string result =
        std::accumulate(ids.begin(), ids.end(), std::string(), [](const std::string& params, int64_t id) {
            return params.empty() ? std::to_string(id) : params + ", " + std::to_string(id);
        });
    return result;
}

class FileDatabaseTest : public testing::Test {
protected:
    static void SetUpTestSuite() {
        std::cout << "FileDatabaseTest SetUpTestSuite..." << std::endl;
        std::string dbPath = FileDatabase::getDbFilePath();
        if (!dbPath.empty()) {
            std::filesystem::remove(dbPath);
        }
    }

    static void TearDownTestSuite() { std::cout << "FileDatabaseTest TearDownTestSuite..." << std::endl; }

    void SetUp() override {
        std::cout << "FileDatabaseTest SetUp..." << std::endl;
        db = std::make_unique<SQLite::Database>(FileDatabase::createDatabase());
    }

    void TearDown() override { std::cout << "FileDatabaseTest TearDown..." << std::endl; }

    std::unique_ptr<SQLite::Database> db;
};

TEST_F(FileDatabaseTest, test01) {
    // 测试表是否存在
    std::string dbPath = FileDatabase::getDbFilePath();
    EXPECT_FALSE(dbPath.empty());
    EXPECT_TRUE(std::filesystem::exists(dbPath));

    EXPECT_TRUE(db->tableExists("t_file_info"));
    EXPECT_TRUE(db->tableExists("t_tag_info"));
    EXPECT_TRUE(db->tableExists("t_file_tag_info"));
    EXPECT_TRUE(db->tableExists("t_ai_index_task"));
}

TEST_F(FileDatabaseTest, test02) {
    // 测试插入文件
    FileInfoDO fileInfo;
    fileInfo.filePath = "/test/path1";
    fileInfo.enable = 1;
    fileInfo.modifyTime = "2025-06-30 12:00:00";

    EXPECT_TRUE(FileDatabase::insertFileInfo(*db, fileInfo));
    EXPECT_GT(fileInfo.fileId, 0);

    FileInfoDO retrieved;
    EXPECT_TRUE(FileDatabase::getSingleFileInfo(*db, "/test/path1", retrieved));
    EXPECT_EQ(retrieved.filePath, "/test/path1");
    EXPECT_EQ(retrieved.fileSummary, "");  // 一开始是空的，因为是闲时任务,insertFileInfo时不会去插入摘要

    FileInfoDO updated = retrieved;
    updated.fileSummary = "updated_summary";
    EXPECT_TRUE(FileDatabase::updateFileInfo(*db, updated));

    FileInfoDO verified;
    EXPECT_TRUE(FileDatabase::getSingleFileInfo(*db, "/test/path1", verified));
    EXPECT_EQ(verified.fileSummary, "updated_summary");

    auto allFiles = FileDatabase::getAllFileInfos(*db);
    EXPECT_GT(allFiles.size(), 0);

    std::string path = FileDatabase::getFilePathById(*db, fileInfo.fileId);
    EXPECT_EQ(path, "/test/path1");
}

TEST_F(FileDatabaseTest, test03) {
    // 测试插入标签
    FileInfoDO fileInfo;
    fileInfo.filePath = "/test/tag_file";
    fileInfo.enable = 1;
    fileInfo.modifyTime = "2025-06-30 12:00:00";

    EXPECT_TRUE(FileDatabase::insertFileInfo(*db, fileInfo));
    EXPECT_GT(fileInfo.fileId, 0);

    int64_t tagId;
    EXPECT_TRUE(FileDatabase::insertNewTagName(*db, fileInfo.fileId, "test_tag", tagId));
    EXPECT_GT(tagId, 0);

    int64_t retrievedTagId;
    EXPECT_TRUE(FileDatabase::getTagIdByTagName(*db, "test_tag", retrievedTagId));
    EXPECT_EQ(retrievedTagId, tagId);

    auto tags = FileDatabase::getFileTags(*db, "/test/tag_file");
    EXPECT_EQ(tags.size(), 1);
    EXPECT_EQ(tags[0], "test_tag");

    auto allTags = FileDatabase::getAllTags(*db);
    EXPECT_GE(allTags.size(), 1);

    EXPECT_TRUE(FileDatabase::isTagInUse(*db, tagId));

    EXPECT_TRUE(FileDatabase::deleteFileTagRelation(*db, fileInfo.fileId, tagId));

    EXPECT_TRUE(FileDatabase::deleteTagInfo(*db, tagId));
    EXPECT_FALSE(FileDatabase::isTagInUse(*db, tagId));
}

TEST_F(FileDatabaseTest, test04) {
    // 测试文件和标签关系
    FileInfoDO fileInfo;
    fileInfo.filePath = "/test/relation_file";
    fileInfo.enable = 1;
    fileInfo.modifyTime = "2025-06-30 12:00:00";
    EXPECT_TRUE(FileDatabase::insertFileInfo(*db, fileInfo));

    int64_t tagId1, tagId2;
    EXPECT_TRUE(FileDatabase::insertNewTagName(*db, fileInfo.fileId, "tag1", tagId1));
    EXPECT_TRUE(FileDatabase::insertNewTagName(*db, fileInfo.fileId, "tag2", tagId2));

    auto tagIds = FileDatabase::getTagIdsByFileId(*db, fileInfo.fileId);
    EXPECT_EQ(tagIds.size(), 2);

    std::unordered_set<int64_t> ids{tagId1, tagId2};
    auto filePaths = FileDatabase::getFilePathsByTagIds(*db, ids);
    EXPECT_EQ(filePaths.size(), 2);
    EXPECT_EQ(filePaths[0], "/test/relation_file");

    auto tagNames = FileDatabase::getTagNamesByTagIds(*db, ids);
    EXPECT_EQ(tagNames.size(), 2);
    EXPECT_TRUE(std::find(tagNames.begin(), tagNames.end(), "tag1") != tagNames.end());
    EXPECT_TRUE(std::find(tagNames.begin(), tagNames.end(), "tag2") != tagNames.end());
}

TEST_F(FileDatabaseTest, test05) {
    // 测试删除文件

    FileInfoDO fileInfo1;
    fileInfo1.filePath = "/test/transaction1";
    fileInfo1.enable = 1;
    fileInfo1.modifyTime = "2025-06-30 12:00:00";

    FileInfoDO fileInfo2;
    fileInfo2.filePath = "/test/transaction2";
    fileInfo2.enable = 1;
    fileInfo2.modifyTime = "2025-06-30 12:00:00";

    EXPECT_TRUE(FileDatabase::insertFileInfo(*db, fileInfo1));
    EXPECT_TRUE(FileDatabase::insertFileInfo(*db, fileInfo2));

    auto filesBefore = FileDatabase::getAllFileInfos(*db);
    EXPECT_GE(filesBefore.size(), 2);

    std::unordered_set<int64_t> idsToDelete;
    for (const auto& f : filesBefore) {
        if (f.filePath == "/test/transaction1" || f.filePath == "/test/transaction2") {
            idsToDelete.insert(f.fileId);
        }
    }

    std::string expression = joinIds(idsToDelete);
    EXPECT_TRUE(FileDatabase::deleteByFileIds(*db, expression));
    auto filesAfter = FileDatabase::getAllFileInfos(*db);
    for (const auto& id : idsToDelete) {
        EXPECT_TRUE(FileDatabase::getFilePathById(*db, id).empty());  // 删除成功，这里为空
    }
}
TEST_F(FileDatabaseTest, test06) {
    // 测试不存在的数据

    // 测试查询不存在的文件
    FileInfoDO nonExistent;
    EXPECT_FALSE(FileDatabase::getSingleFileInfo(*db, "/non/existent/path", nonExistent));

    // 测试获取不存在的文件路径
    EXPECT_TRUE(FileDatabase::getFilePathById(*db, 999999).empty());

    // 测试获取不存在的标签
    int64_t dummyTagId;
    EXPECT_FALSE(FileDatabase::getTagIdByTagName(*db, "non_existent_tag", dummyTagId));

    // 测试检查未使用的标签
    EXPECT_FALSE(FileDatabase::isTagInUse(*db, 999999));
}