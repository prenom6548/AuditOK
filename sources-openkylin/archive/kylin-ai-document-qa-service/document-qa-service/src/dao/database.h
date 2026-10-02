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

#pragma once

#include <Database.h>

#include <filesystem>
#include <nlohmann/json.hpp>

struct KnowledgebaseSearchResult {
    std::string documents;
    std::string source;
    double similarity;
};

using KnowledgebaseSearchResults = std::vector<KnowledgebaseSearchResult>;

struct KnowledgeFileInfo {
    std::string filepath;
    std::vector<std::vector<float>> vectors;
    std::vector<std::string> documents;
    std::string knowledgeBaseName;
};

struct InputParameters {
    std::string filepath;
    std::string knowledgeBaseName;
};

struct VectorDatabaseErrorCode {
    VectorDatabaseErrorCode(bool success, VectorDB::StatusCode code = VectorDB::StatusCode::OK,
                            std::string message = "Success")
        : success(success), code(code), message(message) {}

    bool success;
    VectorDB::StatusCode code;
    std::string message;
};

class KnowledgebaseVectorDatabase {
public:
    KnowledgebaseVectorDatabase();
    VectorDatabaseErrorCode createCollection(std::string& collectionName);
    VectorDatabaseErrorCode deleteCollection(const std::string& collectionName);

    VectorDatabaseErrorCode insertVectorData(const std::vector<KnowledgeFileInfo>& vectorInfos);

    VectorDatabaseErrorCode similaritySearch(const std::string& knowledgeBaseName, const std::vector<float>& vector,
                                             KnowledgebaseSearchResults& searchResult);
    ~KnowledgebaseVectorDatabase();

private:
    KnowledgebaseSearchResults transformSearchResults(VectorDB::SearchResults results) const;
    VectorDatabaseErrorCode connect();
    VectorDatabaseErrorCode disconnect();

private:
    std::shared_ptr<VectorDB::Database> client_;
    bool isConnected;
    static constexpr const char* DATABASE_APPID = "kylin-ai-document-qa-service";
    static constexpr const char* DATABASE_PASSWORD = "k38e1a_b1ec77a";
};
