#include "fileparser.h"

#include <conf.h>
#include <kyai-business-framework/conf.h>
#include <kyai-business-framework/constant.h>
#include <log.h>

#include <random>

#include "../configmanager.h"

const char* DOCUMENT_CHUNK_TEXT_TYPE = "SemanticChunker";
const char* DOCUMENT_RESPONSE_DATA = "responseData";
const char* CHUNKS = "chunks";
const char* FILEPATH = "filePath";

const char* DOCUMENT_CHUNK_SIZE = "document_chunk_size";

FileParser::FileParser() {
    client_ = std::make_shared<document::DocumentChunkClient>();
    if (client_ == nullptr) {
        PRINT_ERROR("FileParser error: failed to create DocumentClient instance.\n");
    }
}
std::vector<ChunkResult> FileParser::chunkFiles(const std::vector<std::string>& filePaths,
                                                const std::vector<int32_t>& maxDocWordsCount) {
    nlohmann::json ret = syncChunkDocument(filePaths, maxDocWordsCount);
    if (ret.empty()) {
        PRINT_ERROR("FileParser error: chunk document server something wrong.\n");
        return std::vector<ChunkResult>();
    }
    return parseChunkResult(ret);
}

std::vector<ChunkResult> FileParser::parseChunkResult(nlohmann::json& data) {
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
        if (item.contains("status") && item["status"].contains("errorCode") &&
            item["status"].contains("errorMessage")) {
            chunkResult.errorCode = item["status"]["errorCode"];
            chunkResult.errorMsg = item["status"]["errorMessage"];
        } else {
            chunkResult.errorCode = 0;
            chunkResult.errorMsg = "";
        }
        res.push_back(chunkResult);
    }

    return res;
}

nlohmann::json FileParser::syncChunkDocument(const std::vector<std::string>& filePaths,
                                             const std::vector<int32_t>& maxDocWordsCount) {
    std::vector<std::string> chunkTypes(filePaths.size(), DOCUMENT_CHUNK_TEXT_TYPE);
    knowledgebase::FileProcessorConfig config = knowledgebase::ConfigManager::getInstance().getFileProcessorConfig();
    std::vector<int32_t> chunkSizes(filePaths.size(), config.chunkMaxSize_);

    auto result = client_->SyncChunkDocument(filePaths, chunkTypes, chunkSizes, maxDocWordsCount);

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