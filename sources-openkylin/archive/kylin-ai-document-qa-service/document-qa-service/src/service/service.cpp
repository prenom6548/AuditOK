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

#include "service.h"

#include <kyai-business-framework/conf.h>
#include <log.h>

#include <nlohmann/json.hpp>
#include <sstream>

#include "database.h"
#include "embedding.h"
#include "parser.h"

const char* SUPPORT_CHUNK_COUNT = "support_chunk_count";

ERROR_CODE KnowledgebaseService::addTexts(const std::string& collectionName, const std::string& filepath) {
    std::vector<KnowledgeFileInfo> infos;
    // 分块 并获得向量化结果
    Parser parser;
    std::vector<ChunkResult> chunkResult = parser.chunkFiles({filepath});
    if (chunkResult.empty()) {
        PRINT_ERROR("KnowledgebaseService: document parse result is empty.\n");
        return CHUNK_FILE_INFO_FAILED;
    }

    Embedding textEmbedding;
    for (const auto& item : chunkResult) {
        if (item.chunk.empty()) {
            PRINT_ERROR("KnowledgebaseService: filepath:%s chunk result is empty.\n", item.filePath.c_str());
            return CHUNK_FILE_INFO_FAILED;
        }

        if (item.chunk.size() > Configuration::getInt(SUPPORT_CHUNK_COUNT)) {
            PRINT_ERROR("KnowledgebaseService: unsupport chunk counts %d.\n", item.chunk.size());
            return TEXT_CHUNKS_UNSUPPORT;
        }
        std::vector<std::string> chunkText;
        for (const auto& chunk : item.chunk) {
            nlohmann::json chunkJson = nlohmann::json::parse(chunk);
            chunkText.push_back(chunkJson["content"]);
        }
        std::vector<std::vector<float>> embeddingResult = textEmbedding.getTextEmbeddings(chunkText);
        if (embeddingResult.empty()) {
            PRINT_ERROR("KnowledgebaseService: embedding result is empty.\n");
            return EMBEDDING_FAILED;
        }
        KnowledgeFileInfo info{item.filePath, embeddingResult, item.chunk, collectionName};
        infos.push_back(info);
    }

    // insert vector data
    KnowledgebaseVectorDatabase database;

    auto status = database.insertVectorData(infos);
    if (!status.success) {
        if (status.code == VectorDB::StatusCode::DATABASE_INIT_FAILED) {
            PRINT_ERROR("KnowledgebaseService:insert vector data failed, Database init error - %s\n",
                        status.message.c_str());
            return VECTOR_DATABASE_DATA_UNAVAILABLE;
        }
        PRINT_ERROR("KnowledgebaseService:insert vector data failed, Unknown database insert error - %s\n",
                    status.message.c_str());
        return INSERT_VECTOR_DATABASE_FAILED;
    }

    return SUCCESS;
}

ERROR_CODE KnowledgebaseService::getFileContent(const std::vector<std::string>& filepaths, std::string& output) {
    // 分块
    Parser parser;
    std::vector<ChunkResult> chunkResult = parser.chunkFiles(filepaths);
    if (chunkResult.empty()) {
        PRINT_ERROR("KnowledgebaseService: document parse result is empty.\n");
        return CHUNK_FILE_INFO_FAILED;
    }

    // 拼接
    std::unordered_map<std::string, std::string> fileContentMap;
    for (const auto& item : chunkResult) {
        std::ostringstream contentBuilder;
        for (const auto& str : item.chunk) {
            nlohmann::json strJson = nlohmann::json::parse(str);
            contentBuilder << strJson["content"];
        }
        fileContentMap[item.filePath] = contentBuilder.str();
    }

    output = getFileContentJson(fileContentMap);

    return SUCCESS;
}

std::string KnowledgebaseService::getFileContentJson(std::unordered_map<std::string, std::string>& fileContents) const {
    nlohmann::json root = nlohmann::json::array();

    for (const auto& result : fileContents) {
        nlohmann::json item = {{"filepath", result.first}, {"document", result.second}};
        root.push_back(item);
    }

    return root.dump();
}
std::string KnowledgebaseService::getSearchResultJson(std::vector<KnowledgebaseSearchResult>& searchResult) const {
    nlohmann::json root = nlohmann::json::array();

    for (const auto& result : searchResult) {
        nlohmann::json item = {
            {"filepath", result.source}, {"document", result.documents}, {"similarity", result.similarity}};
        root.push_back(item);
    }

    return root.dump();
}
ERROR_CODE KnowledgebaseService::similaritySearch(const std::string& collectionName, const std::string& searchCondition,
                                                  std::string& output) const {
    // 向量化
    Embedding textEmbedding;
    std::vector<float> res = textEmbedding.getTextEmbedding(searchCondition);
    if (res.empty()) {
        PRINT_ERROR("KnowledgebaseService: embedding result is empty.\n");
        return EMBEDDING_FAILED;
    }
    // 搜索
    KnowledgebaseVectorDatabase database;
    KnowledgebaseSearchResults searchResult;
    auto status = database.similaritySearch(collectionName, res, searchResult);
    if (!status.success) {
        if (status.code == VectorDB::StatusCode::DATABASE_INIT_FAILED) {
            PRINT_ERROR("KnowledgebaseService: similarity search failed,, Database init error - %s\n",
                        status.message.c_str());
            return VECTOR_DATABASE_DATA_UNAVAILABLE;
        }
        PRINT_ERROR("KnowledgebaseService: similarity search failed, Unknown database search error - %s\n",
                    status.message.c_str());
        return SEARCH_FILE_INFO_FAILED;
    }

    if (searchResult.empty()) {
        PRINT_ERROR("KnowledgebaseService: searchResult is empty.\n");
        return SEARCH_FILE_INFO_FAILED;
    }
    // 处理成json
    output = getSearchResultJson(searchResult);

    return SUCCESS;
}