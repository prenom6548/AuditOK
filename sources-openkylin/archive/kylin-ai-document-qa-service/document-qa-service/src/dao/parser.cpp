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

#include "parser.h"

#include <conf.h>
#include <kyai-business-framework/conf.h>
#include <log.h>

#include <random>

#include "constant.h"

const char* DOCUMENT_CHUNK_TEXT_TYPE = "TextChunker";
const char* DOCUMENT_RESPONSE_DATA = "responseData";
const char* CHUNKS = "chunks";
const char* FILEPATH = "filePath";

const char* DOCUMENT_CHUNK_SIZE = "document_chunk_size";

Parser::Parser() {
    client_ = std::make_shared<document::DocumentChunkClient>();
    if (client_ == nullptr) {
        PRINT_ERROR("Parser error: failed to create DocumentClient instance.\n");
    }
}
std::vector<ChunkResult> Parser::chunkFiles(const std::vector<std::string>& filePaths) {
    nlohmann::json ret = syncChunkDocument(filePaths);
    if (ret.empty()) {
        PRINT_ERROR("Parser error: chunk document server something wrong.\n");
        return std::vector<ChunkResult>();
    }
    return parseChunkResult(ret);
}

std::vector<ChunkResult> Parser::parseChunkResult(nlohmann::json& data) {
    std::vector<ChunkResult> res;

    // 解析分块结果
    for (const auto& item : data) {
        ChunkResult chunkResult;
        std::string file_path = item["filePath"];
        chunkResult.filePath = file_path;

        std::vector<std::string> chunks;
        for (const auto& chunk : item["chunks"]) {
            chunks.push_back(chunk);
        }
        chunkResult.chunk = chunks;
        res.push_back(chunkResult);
    }

    return res;
}
nlohmann::json Parser::syncChunkDocument(const std::vector<std::string>& filePaths) {
    std::vector<std::string> chunkTypes(filePaths.size(), DOCUMENT_CHUNK_TEXT_TYPE);
    std::vector<int32_t> chunkSizes(filePaths.size(), Configuration::getInt(DOCUMENT_CHUNK_SIZE));

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
    return data;
}