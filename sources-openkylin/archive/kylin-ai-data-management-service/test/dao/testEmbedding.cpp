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
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "constant.h"
#include "embedding.h"

// 辅助函数（保持不变）
std::vector<uint8_t> readFile(const std::string &filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file");
    }
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char *>(buffer.data()), size)) {
        throw std::runtime_error("Failed to read file");
    }
    return buffer;
}

const std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

std::string base64Encode(const std::vector<uint8_t> &buffer) {
    std::string encodedData;
    int i = 0;
    uint8_t char_array_3[3];
    uint8_t char_array_4[4];

    while (i < buffer.size()) {
        char_array_3[0] = buffer[i++];
        char_array_3[1] = (i < buffer.size()) ? buffer[i++] : 0;
        char_array_3[2] = (i < buffer.size()) ? buffer[i++] : 0;

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = (char_array_3[2] & 0x3f);

        for (int j = 0; j < 4; ++j) {
            encodedData += base64_chars[char_array_4[j]];
        }
    }

    while ((encodedData.size() % 4) != 0) {
        encodedData += '=';
    }
    return encodedData;
}

class EmbeddingTest : public testing::Test {
protected:
    void SetUp() override {
        // 准备测试图片路径
        namespace fs = std::filesystem;
        fs::path dir = fs::path(__FILE__).parent_path().parent_path();
        testImagePath_ = (dir / "20250626.jpg").string();

        // 检查测试文件是否存在
        if (!fs::exists(testImagePath_)) {
            throw std::runtime_error("Test image file not found: " + testImagePath_);
        }
    }

    std::string testImagePath_;
};

// TEST_F(EmbeddingTest, test01) {
//     // 测试有效数据
//     Embedding embedder(DataManagementEmbeddingDataType::Text, scene::TaskPriority::HIGH);
//     std::string testText = "This is a test sentence for embedding.";

//     auto result = embedder.getEmbedding(testText);

//     EXPECT_FALSE(result.empty()) << "Text embedding result should not be empty";
//     EXPECT_GT(result.size(), 0) << "Text embedding vector should have positive size";
// }

// TEST_F(EmbeddingTest, test02) {
//     // 测试空字符串
//     Embedding embedder(DataManagementEmbeddingDataType::Text, scene::TaskPriority::HIGH);
//     auto result = embedder.getEmbedding("");

//     EXPECT_TRUE(result.empty()) << "Empty input should return empty embedding";
// }

// TEST_F(EmbeddingTest, test03) {
//     // 测试有效数据
//     Embedding embedder(DataManagementEmbeddingDataType::ImageText, scene::TaskPriority::HIGH);
//     std::string testText = "Text description of an image";

//     auto result = embedder.getEmbedding(testText);

//     // 虽然实际可能调用文本处理器，但测试接口行为
//     EXPECT_FALSE(result.empty()) << "ImageText embedding result should not be empty";
// }

// TEST_F(EmbeddingTest, test04) {
//     // 测试有效数据
//     Embedding embedder(DataManagementEmbeddingDataType::ImageFlie, scene::TaskPriority::HIGH);

//     auto result = embedder.getEmbedding(testImagePath_);

//     EXPECT_FALSE(result.empty()) << "Image file embedding result should not be empty";
//     EXPECT_GT(result.size(), 0) << "Image embedding vector should have positive size";
// }

// TEST_F(EmbeddingTest, test05) {
//     // 测试无效数据
//     Embedding embedder(DataManagementEmbeddingDataType::ImageFlie, scene::TaskPriority::HIGH);
//     std::string invalidPath = "/nonexistent/path/to/image.jpg";

//     auto result = embedder.getEmbedding(invalidPath);

//     EXPECT_TRUE(result.empty()) << "Invalid image path should return empty embedding";
// }

// TEST_F(EmbeddingTest, test06) {
//     // data-management目前没用到 ImageBase64，虽然接口可以传入这个函数，但是调用的却是ImageFlie
//     Embedding embedder(DataManagementEmbeddingDataType::ImageBase64, scene::TaskPriority::HIGH);

//     auto imageData = readFile(testImagePath_);
//     std::string base64Data = base64Encode(imageData);

//     auto result = embedder.getEmbedding(base64Data);

//     // 这里返回值为空,以后要是实现了base64，这里要改成 EXPECT_FALSE
//     EXPECT_TRUE(result.empty());
// }

// TEST_F(EmbeddingTest, test07) {
//     // 传入一些没有意义的枚举值
//     auto invalidType = static_cast<DataManagementEmbeddingDataType>(99);
//     Embedding embedder(invalidType, scene::TaskPriority::HIGH);

//     auto result = embedder.getEmbedding("any data");

//     EXPECT_TRUE(result.empty()) << "Unknown data type should return empty embedding";
// }

// TEST_F(EmbeddingTest, test08) {
//     // 测试不同优先级是否会影响结果
//     Embedding highPriority(DataManagementEmbeddingDataType::Text, scene::TaskPriority::HIGH);
//     Embedding lowPriority(DataManagementEmbeddingDataType::Text, scene::TaskPriority::LOW);

//     std::string testText = "Testing priority handling";

//     auto highResult = highPriority.getEmbedding(testText);
//     auto lowResult = lowPriority.getEmbedding(testText);

//     EXPECT_EQ(highResult.size(), lowResult.size()) << "Same input should produce same output size";
// }