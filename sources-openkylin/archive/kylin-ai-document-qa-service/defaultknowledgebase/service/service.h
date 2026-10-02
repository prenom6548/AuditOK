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

#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
namespace knowledgebase_tool {

struct ChunkResult {
    std::string filePath;
    std::vector<std::string> chunk;
};

struct KnowledgeFileInfo {
    const std::string collectionName;
    std::string filePath;
    std::vector<std::string> documents;
    std::vector<std::vector<float>> vectors;
};

struct KnowledgebaseSearchResult {
    std::string documents;
    std::string source;
    double similarity;
};

using KnowledgebaseSearchResults = std::vector<KnowledgebaseSearchResult>;

class Service {
public:
    static Service* getInstance() {
        static Service service;
        return &service;
    }
    bool createCollection(const std::string& name);
    bool addFiles(const std::string& name, const std::vector<std::string>& filepaths);
    bool deleteFiles(const std::string& name, const std::vector<std::string>& filepaths);
    bool updateFiles(const std::string& name, const std::vector<std::string>& filepaths);
    bool search(const std::string& name, const std::string& text, float threshold);
    bool dropCollection(const std::string& name);
    bool query(const std::string& name);

private:
    Service();
    ~Service();
    static bool checkStatus(std::string&& prefix, const VectorDB::Status& status);
    bool hasCollection(const std::string& name);
    bool disconnect();

private:
    std::vector<std::vector<float>> embedding(const std::vector<std::string>& texts);

    std::vector<ChunkResult> getChunkResult(const std::vector<std::string>& filePaths, int32_t size);
    nlohmann::json syncChunkDocument(const std::vector<std::string>& filePaths, int32_t size);
    std::vector<ChunkResult> parseChunkResult(nlohmann::json& data);

private:
    std::shared_ptr<VectorDB::Database> client_;
};

}  // namespace knowledgebase_tool
