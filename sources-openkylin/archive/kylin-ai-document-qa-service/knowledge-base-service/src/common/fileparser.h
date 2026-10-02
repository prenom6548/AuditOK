#pragma once

#include <kylin-ai-document-service/include/client.h>

#include <nlohmann/json.hpp>

struct ChunkResult {
    std::string filePath;
    std::vector<std::string> chunk;
    int errorCode;
    std::string errorMsg;
};

class FileParser {
public:
    FileParser();
    std::vector<ChunkResult> chunkFiles(const std::vector<std::string>& filePaths,
                                        const std::vector<int32_t>& maxDocWordsCount = {});

    ~FileParser() = default;

private:
    nlohmann::json syncChunkDocument(const std::vector<std::string>& filePaths,
                                     const std::vector<int32_t>& maxDocWordsCount = {});
    std::vector<ChunkResult> parseChunkResult(nlohmann::json& data);

private:
    std::shared_ptr<document::DocumentChunkClient> client_;
};
