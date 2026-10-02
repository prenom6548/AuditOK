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

#include "documentParse.h"

#include <conf.h>
#include <log.h>

#include <random>

#include "constant.h"

const char* DOCUMENT_CHUNK_TEXT_TYPE = "TextChunker";
const char* DOCUMENT_CHUNK_SIZE = "chunk_size";
const char* DOCUMENT_RESPONSE_DATA = "responseData";
const int DOCUMENT_CHUNK_LIMIT = 90;

DocumentParse::DocumentParse() {
    client_ = std::make_shared<document::DocumentChunkClient>();
    if (client_ == nullptr) {
        PRINT_ERROR("Failed to create DocumentClient instance.");
    }
}

nlohmann::json DocumentParse::syncChunkDocument(const std::vector<std::string>& filePaths, int32_t size) {
    std::vector<std::string> chunkTypes(filePaths.size(), DOCUMENT_CHUNK_TEXT_TYPE);
    std::vector<int32_t> chunkSizes(filePaths.size(), size);

    auto result = client_->SyncChunkDocument(filePaths, chunkTypes, chunkSizes);
    // grpc 通信异常
    if (result.errorCode != 0) {
        PRINT_ERROR("DocumentClient connect failed, err: %d, msg: %s.\n", result.errorCode, result.errorMsg.c_str());
        return nlohmann::json::array();
    }

    nlohmann::json root = nlohmann::json::parse(result.data);
    if (root.empty() || !root.contains(DOCUMENT_RESPONSE_DATA) || !root[DOCUMENT_RESPONSE_DATA].is_array() ||
        root[DOCUMENT_RESPONSE_DATA].empty()) {
        PRINT_ERROR("Failed to parse DocumentClient result: %s\n", result.data.c_str());
        return nlohmann::json::array();
    }

    nlohmann::json& data = root[DOCUMENT_RESPONSE_DATA];
    // 过滤掉 没有 filePath 或 chunks 的结果
    data.erase(std::remove_if(
                   data.begin(), data.end(),
                   [](const nlohmann::json& value) { return !value.contains(FILEPATH) || !value.contains(CHUNKS); }),
               data.end());

    for (auto& item : data) {
        if (item[CHUNKS].size() > DOCUMENT_CHUNK_LIMIT) {
            auto value = item[CHUNKS].get<std::vector<std::string>>();
            auto res = divideChunks(value);
            item[CHUNKS] = res;
        }
    }
    return data;
}

nlohmann::json DocumentParse::syncChunkDocument(const std::vector<std::string>& filePaths) {
    return syncChunkDocument(filePaths, Configuration::getInt(DOCUMENT_CHUNK_SIZE, 512));
}

std::vector<std::string> DocumentParse::divideChunks(const std::vector<std::string>& chunks) {
    int n = chunks.size();
    // 当DOCUMENT_CHUNK_LIMIT长度大于90时，采取分割方案：三个分块，第一个分块取前30，后两个分块随机取30
    int part1_len = n / 3;
    int part2_len = n / 3;
    int part3_len = n - part1_len - part2_len;

    // 划分三个区段
    std::vector<std::string> part1(chunks.begin(), chunks.begin() + part1_len);
    std::vector<std::string> part2(chunks.begin() + part1_len, chunks.begin() + part1_len + part2_len);
    std::vector<std::string> part3(chunks.begin() + part1_len + part2_len, chunks.end());

    // 随机数生成器
    std::random_device rd;
    std::mt19937 rng(rd());

    // 打乱后两个分块
    shuffle(part2.begin(), part2.end(), rng);
    shuffle(part3.begin(), part3.end(), rng);

    // 合并结果（确保每个区段取30个）
    std::vector<std::string> result;
    result.insert(result.end(), part1.begin(), part1.begin() + 30);
    result.insert(result.end(), part2.begin(), part2.begin() + 30);
    result.insert(result.end(), part3.begin(), part3.begin() + 30);

    return result;
}
