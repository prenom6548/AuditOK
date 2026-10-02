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

#include <kysdk-vector-engine-client/Database.h>

#include <memory>
#include <nlohmann/json.hpp>

#include "constant.h"

typedef struct metadataResult {
    std::string id;
    float similarity;
    nlohmann::json metadata;
} MetadataResult;

using SimilaritySearchMetadataResult = std::vector<metadataResult>;

typedef struct dataManagementVectorInfo {
    std::string id;
    std::vector<float> embedding;
    nlohmann::json metadata;
} DataManagementVectorInfo;

class DataManagementVectorDatabase {
public:
    static DataManagementVectorDatabase& getInstance();

    bool createCollection(const std::string& collectionName, int dimension = TEXT_DIMENSION);

    bool insertVectorData(const std::string& collectionName, const std::vector<DataManagementVectorInfo>& vectorInfos);

    bool deleteVectorData(const std::string& collectionName, const std::string& expression);

    bool dropCollection(const std::string& collectionName);

    SimilaritySearchMetadataResult searchMetadata(const std::string& collectionName,
                                                  const std::vector<float>& embedding, int topK = 10,
                                                  float threshold = 0.0);

    std::vector<std::string> showCollection();

    std::string getDbFilePath();

private:
    DataManagementVectorDatabase();
    ~DataManagementVectorDatabase();

    bool connectAndLoadDB(VectorDB::ConnectParam param, std::string dbFile, bool encrypt = false, std::string key = "");
    bool hasCollection(const std::string& collectionName);

private:
    std::shared_ptr<VectorDB::Database> client_;
};
