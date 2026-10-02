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
#include <vector>

#include "documentParse.h"

class DocumentParseTest : public testing::Test {
public:
    static void SetUpTestSuite() { std::cout << "DocumentParseTest SetUpTestSuite..." << std::endl; }

    static void TearDownTestSuite() { std::cout << "DocumentParseTest TearDownTestSuite..." << std::endl; }

protected:
    void SetUp() override {
        std::cout << "DocumentParseTest SetUp..." << std::endl;
        namespace fs = std::filesystem;
        // 获取测试文件路径
        fs::path dir = fs::path(__FILE__).parent_path().parent_path();
        testFilePath_ = (dir / "hand.docx").string();
    }

    void TearDown() override { std::cout << "DocumentParseTest TearDown..." << std::endl; }

    std::string testFilePath_;
};

// TEST_F(DocumentParseTest, test01) {
//     // 测试使用默认分块大小
//     DocumentParse documentParse;
//     nlohmann::json ret = documentParse.syncChunkDocument({testFilePath_});

//     EXPECT_FALSE(ret.empty());
//     EXPECT_TRUE(ret.is_array());

//     // 检查返回的数据结构是否正确
//     for (const auto& item : ret) {
//         EXPECT_TRUE(item.contains("filePath"));
//         EXPECT_TRUE(item.contains("chunks"));
//         EXPECT_TRUE(item["chunks"].is_array());
//     }
// }

// TEST_F(DocumentParseTest, test02) {
//     // 测试使用自定义分块大小
//     DocumentParse documentParse;
//     nlohmann::json ret = documentParse.syncChunkDocument({testFilePath_}, 256);

//     EXPECT_FALSE(ret.empty());
//     EXPECT_TRUE(ret.is_array());

//     // 检查返回的数据结构是否正确
//     for (const auto& item : ret) {
//         EXPECT_TRUE(item.contains("filePath"));
//         EXPECT_TRUE(item.contains("chunks"));
//         EXPECT_TRUE(item["chunks"].is_array());
//     }
// }

// TEST_F(DocumentParseTest, test03) {
//     // 测试处理无效文件路径
//     DocumentParse documentParse;
//     nlohmann::json ret = documentParse.syncChunkDocument({"/invalid/path/file.txt"});

//     std::cout << ret << std::endl;
//     if (!ret.empty()) {
//         // 检查返回的数据结构是否正确
//         for (const auto& item : ret) {
//             EXPECT_TRUE(item.contains("filePath"));
//             EXPECT_TRUE(item.contains("chunks"));
//             EXPECT_TRUE(item["chunks"].is_array());
//             EXPECT_TRUE(item["chunks"].empty()); // 无效路径 虽然会有返回值，但chunks
//         }
//     }
// }

// TEST_F(DocumentParseTest, test04) {
//     // 测试处理多个文件的情况
//     DocumentParse documentParse;
//     const std::vector<std::string>& filePaths = {testFilePath_, testFilePath_, testFilePath_, testFilePath_,
//                                                  testFilePath_, testFilePath_, testFilePath_, testFilePath_};
//     nlohmann::json ret = documentParse.syncChunkDocument(filePaths);

//     EXPECT_FALSE(ret.empty());
//     EXPECT_EQ(ret.size(), filePaths.size());

//     for (const auto& item : ret) {
//         EXPECT_TRUE(item.contains("filePath"));
//         EXPECT_TRUE(item.contains("chunks"));
//     }
// }