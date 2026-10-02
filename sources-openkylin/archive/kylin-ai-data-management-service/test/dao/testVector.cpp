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
#include <kyai-business-framework/scene.h>

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "dataManagementVectorDatabase.h"

using namespace std;
using json = nlohmann::json;

class DataManagementVectorDatabaseTest : public ::testing::Test {
protected:
    void SetUp() override { db = &DataManagementVectorDatabase::getInstance(); }

    void TearDown() override {
        // 自动清理测试数据
        db->dropCollection(TEST_COLLECTION_NAME);
    }

    static constexpr int TEST_DIMENSION = 128;  // 测试专用维度
    const string TEST_COLLECTION_NAME = "test_vector_collection";
    DataManagementVectorDatabase* db;
};

// 测试向量数据操作
TEST_F(DataManagementVectorDatabaseTest, VectorDataCRUDTest) {
    db->createCollection(TEST_COLLECTION_NAME, TEST_DIMENSION);

    // 准备测试数据
    std::vector<float> embedding = std::vector<float>(TEST_DIMENSION, 0.1f);

    // 存入向量数据库
    std::string uuid = Utils::generateUUID();
    nlohmann::json metadata;
    metadata["file_id"] = 1;
    // 插入数据验证
    EXPECT_TRUE(db->insertVectorData(TEST_COLLECTION_NAME, {DataManagementVectorInfo{uuid, embedding, metadata}}));

    // 搜索验证
    SimilaritySearchMetadataResult results = db->searchMetadata(TEST_COLLECTION_NAME, embedding, 10, 0.0);
    EXPECT_EQ(results[0].metadata["file_id"], 1);
    // 删除验证
    std::string expression = "file_id == " + std::to_string(1);
    EXPECT_TRUE(db->deleteVectorData(TEST_COLLECTION_NAME, expression));
    results = db->searchMetadata(TEST_COLLECTION_NAME, embedding, 10, 0.0);
    EXPECT_EQ(results.size(), 0);  // 仅剩vec2
}