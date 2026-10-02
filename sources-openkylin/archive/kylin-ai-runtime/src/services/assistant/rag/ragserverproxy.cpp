/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "ragserverproxy.h"

#include <cstdio>
#include <string>

#include "assistant/message.h"
#include "utils/logger.h"
#include "utils/util.h"

namespace {
constexpr double similarityThreshold = 0.60;
constexpr unsigned int topK = 4;
}  // namespace

RagServerProxy &RagServerProxy::instance() {
    static RagServerProxy instance;
    return instance;
}

bool RagServerProxy::isValid() const {
    bool success{false};
    auto status = session_.isSupported(success);
    if (!status.success) {
        LOGE("Call [isSupported()] failed, error info: {}, {}.", static_cast<int>(status.errorCode),
             status.errorMsg);
        return false;
    }
    return success;
}

RagServerProxy::RagServerProxy() {}

std::string RagServerProxy::createKnowledgeBase(RagErrorInfo &error) {
    DialogueData::KnowledgeBaseResult result{};
    std::string knowledgebaseName;
    auto status = session_.createKnowledgebase(knowledgebaseName);

    if (!status.success) {
        LOGE("Create knowledge base failed: {}, {}.", static_cast<int>(status.errorCode), status.errorMsg);
        error.errorCode = status.errorCode;
        error.errorMessage = status.errorMsg;
        return {};
    }
    error.errorCode = 0;
    error.errorMessage = "";
    return knowledgebaseName;
}

void RagServerProxy::deleteKnowledgeBase(const std::string &knowledgeBaseName) {
    auto status = session_.deleteKnowledgebase(knowledgeBaseName);
    if (!status.success) {
        LOGE("Delete knowledge base [{}] failed: {}, {}.", knowledgeBaseName, static_cast<int>(status.errorCode),
             status.errorMsg);
    }
}

RagErrorInfo RagServerProxy::addFiles(const std::string &knowledgeBaseName, const std::vector<std::string> &files) {
    LOGI("Add files to knowledge base: {}.", knowledgeBaseName);
    Json::Value jsonFileInfos;
    for (const auto &file : files) {
        Json::Value fileInfo;
        fileInfo["knowledgebase_name"] = knowledgeBaseName;
        fileInfo["filepath"] = file;
        jsonFileInfos.append(fileInfo);
    }

    std::string fileInfos = util::jsonToString(jsonFileInfos);
    LOGI("File infos: {}", fileInfos);
    auto status = session_.addTextFiles(fileInfos);

    if (!status.success) {
        LOGE("Add files to knowledge base failed: {} {}.", static_cast<int>(status.errorCode), status.errorMsg);
    }
    return {status.errorCode, status.errorMsg};
}

RagServerProxy::SimilaritySearchResults RagServerProxy::similaritySearch(const std::string &knowledgeBaseName,
                                                                         const std::string &searchConditions,
                                                                         RagErrorInfo &error) {
    Json::Value jsonSearchInput;
    jsonSearchInput["knowledgebase_name"] = knowledgeBaseName;
    jsonSearchInput["search_condition"] = searchConditions;

    std::string searchInput = util::jsonToString(jsonSearchInput);
    std::string searchOutput;
    auto status = session_.similaritySearch(searchInput, searchOutput);

    if (!status.success) {
        error.errorCode = status.errorCode;
        error.errorMessage = status.errorMsg;
        LOGE("Similarity search failed: {}, {}", static_cast<int>(status.errorCode), status.errorMsg);
        return {};
    }
    error.errorCode = 0;
    error.errorMessage = "";
    return handleSearchResult(searchOutput);
}

std::string RagServerProxy::getFileContent(const std::string &knowledgeBaseName, const std::string &filePath) {
    if (filePath.empty()) {
        return {};
    }

    Json::Value fileInfos;
    Json::Value fileInfo;
    fileInfo["filepath"] = filePath;
    fileInfos.append(fileInfo);

    std::string input = util::jsonToString(fileInfos);
    std::string output;
    auto status = session_.getTextFileContents(input, output);
    if (!status.success) {
        LOGE("Get file content failed: {} {}", static_cast<int>(status.errorCode), status.errorMsg);
        return {};
    }

    LOGI("Get file content result: {}.", output);

    Json::Value results = util::stringToJson(output);
    if (!results.isArray()) {
        return {};
    }
    if (results.empty()) {
        return {};
    }
    if (!results[0].isMember("document")) {
        return {};
    }
    if (!results[0]["document"].isString()) {
        return {};
    }
    return results[0]["document"].asString();
}

RagServerProxy::SimilaritySearchResults RagServerProxy::handleSearchResult(const std::string &searchResults) {
    // 获取前topK个结果，并筛选相似度大于similarityThreshold的
    Json::Value jsonResults = util::stringToJson(searchResults);
    if (!jsonResults.isArray()) {
        return {};
    }
    SimilaritySearchResults results;
    for (unsigned int i = 0; i < std::min(topK, jsonResults.size()); ++i) {
        const auto &jsonResult = jsonResults[i];
        SimilaritySearchResult result;
        if (jsonResult.isMember("similarity") && jsonResult["similarity"].isDouble()) {
            result.similarity = jsonResult["similarity"].asDouble();
            if (result.similarity < similarityThreshold) {
                continue;
            }
        } else {
            continue;
        }
        if (jsonResult.isMember("document") && jsonResult["document"].isString()) {
            result.fileContent = jsonResult["document"].asString();
        } else {
            continue;
        }
        if (jsonResult.isMember("filepath") && jsonResult["filepath"].isString()) {
            result.filePath = jsonResult["filepath"].asString();
        } else {
            continue;
        }
        results.push_back(result);
    }
    return results;
}
