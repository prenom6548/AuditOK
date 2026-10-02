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

#include "dataManagementVectorDatabase.h"

#include <log.h>

DataManagementVectorDatabase& DataManagementVectorDatabase::getInstance() {
    static DataManagementVectorDatabase vectorDB;
    return vectorDB;
}

DataManagementVectorDatabase::DataManagementVectorDatabase() : client_(VectorDB::Database::Create()) {
    std::string dbPath = getDbFilePath();
    VectorDB::ConnectParam param("kyai-data-management-service");
    if (!connectAndLoadDB(param, dbPath)) {
        PRINT_ERROR("Connect vector engine failed.\n");
        return;
    }

    PRINT_INFO("Connect vector engine Success.\n");

    // 单例对象 第一次构造的时候把所有的表都构建完成
    createCollection(TEXT_FILE_CONTENT_COLLECTION_NAME);
    createCollection(VISION_FILE_CONTENT_COLLECTION_NAME, IMAGE_DIMENSION);
    createCollection(FILE_TAGS_COLLECTION_NAME);
}

DataManagementVectorDatabase::~DataManagementVectorDatabase() { client_->Disconnect(); }

bool DataManagementVectorDatabase::connectAndLoadDB(VectorDB::ConnectParam param, std::string dbFile, bool encrypt,
                                                    std::string key) {
    auto status = client_->Connect(param);
    if (!status.IsOk()) {
        return false;
    }

    status = client_->LoadDBFile(dbFile, encrypt, key);
    if (!status.IsOk()) {
        return false;
    }

    return true;
}

bool DataManagementVectorDatabase::createCollection(const std::string& collectionName, int dimension) {
    if (hasCollection(collectionName)) {
        PRINT_DEBUG("DataManagementVectorDatabase collection: %s is exist.\n", collectionName.c_str());
        return true;
    }

    VectorDB::CollectionSchema schema(collectionName, collectionName, true);
    schema.AddField(
        VectorDB::FieldSchema(DEFAULT_ID_FIELD_NAME, VectorDB::DataType::VARCHAR, "id", true).WithMaxLength(50));
    schema.AddField(VectorDB::FieldSchema(DEFAULT_VECTOR_FIELD_NAME, VectorDB::DataType::FLOAT_VECTOR, "vector")
                        .WithDimension(dimension));
    // create index
    VectorDB::IndexDesc desc(DEFAULT_VECTOR_FIELD_NAME, collectionName + "_vector_index");
    bool ret = client_->CreateCollection(schema, desc).IsOk();

    if (!ret) {
        PRINT_ERROR("createCollection %s failed.\n", collectionName.c_str());
        return false;
    }

    PRINT_DEBUG("DataManagementVectorDatabase create collection: %s.\n", collectionName.c_str());
    return true;
}

bool DataManagementVectorDatabase::dropCollection(const std::string& collectionName) {
    if (!hasCollection(collectionName)) {
        PRINT_DEBUG("DataManagementVectorDatabase is not exist collection: %s.\n", collectionName.c_str());
        return true;
    }

    if (!client_->DropCollection(collectionName).IsOk()) {
        PRINT_ERROR("dropCollection %s failed.\n", collectionName.c_str());
        return false;
    }
    PRINT_DEBUG("DataManagementVectorDatabase drop collection: %s.\n", collectionName.c_str());
    return true;
}

bool DataManagementVectorDatabase::insertVectorData(const std::string& collectionName,
                                                    const std::vector<DataManagementVectorInfo>& vectorInfos) {
    std::vector<std::string> idVec;
    idVec.reserve(vectorInfos.size());
    std::vector<std::vector<float>> floatVec;
    floatVec.reserve(vectorInfos.size());
    std::vector<nlohmann::json> metaVec;
    metaVec.reserve(vectorInfos.size());

    for (const auto& info : vectorInfos) {
        idVec.emplace_back(info.id);
        floatVec.emplace_back(info.embedding);
        metaVec.emplace_back(info.metadata);
    }

    std::vector<VectorDB::FieldDataPtr> data{
        std::make_shared<VectorDB::VarCharFieldData>(DEFAULT_ID_FIELD_NAME, idVec),
        std::make_shared<VectorDB::FloatVecFieldData>(DEFAULT_VECTOR_FIELD_NAME, floatVec),
        std::make_shared<VectorDB::JsonFieldData>(DYNAMIC_FIELD_NAME, metaVec)};
    VectorDB::DmlResults results;
    int ret = client_->Insert(collectionName, data, results).IsOk();
    PRINT_DEBUG("%s vector insert rows: %zu\n", collectionName.c_str(), results.IdArray().StrIDArray().size());
    return ret;
}

bool DataManagementVectorDatabase::deleteVectorData(const std::string& collectionName, const std::string& expression) {
    if (!hasCollection(collectionName)) {
        PRINT_DEBUG("DataManagementVectorDatabase is not exist collection: %s.\n", collectionName.c_str());
        return true;
    }

    VectorDB::DmlResults results;
    bool ret = client_->Delete(collectionName, expression, results).IsOk();
    if (!ret) {
        PRINT_ERROR("%s delete vector db failed. expression: %s.\n", collectionName.c_str(), expression.c_str());
        return false;
    }
    PRINT_DEBUG("%s vector delete result: %zu\n", collectionName.c_str(), results.IdArray().StrIDArray().size());
    return true;
}

SimilaritySearchMetadataResult DataManagementVectorDatabase::searchMetadata(const std::string& collectionName,
                                                                            const std::vector<float>& embedding,
                                                                            int topK, float threshold) {
    if (!hasCollection(collectionName)) {
        PRINT_DEBUG("DataManagementVectorDatabase is not exist collection: %s.\n", collectionName.c_str());
        return SimilaritySearchMetadataResult();
    }

    VectorDB::SearchArguments arguments(collectionName, topK);
    arguments.AddOutputField(DYNAMIC_FIELD_NAME);  // 查询只输出 metadata 列
    arguments.SetGuaranteeTimestamp(VectorDB::GuaranteeStrongTs());
    arguments.AddTargetVector(DEFAULT_VECTOR_FIELD_NAME, embedding);

    VectorDB::SearchResults results;
    bool ret = client_->Search(arguments, results).IsOk();
    if (!ret) {
        PRINT_ERROR("SearchMetadata %s failed. embedding size: %zu\n", collectionName.c_str(), embedding.size());
        return SimilaritySearchMetadataResult();
    }

    SimilaritySearchMetadataResult output;
    for (const auto& result : results.Results()) {
        auto& ids = result.Ids().StrIDArray();
        if (ids.empty()) {
            PRINT_INFO("SearchMetadata %s result is empty.\n", collectionName.c_str());
            continue;
        }

        auto similarity = result.Scores();
        if (ids.size() != similarity.size()) {
            PRINT_ERROR("Illegal %s result! %zu vs %zu\n", collectionName.c_str(), ids.size(), similarity.size());
            continue;
        }

        auto dynamicField = result.OutputField(DYNAMIC_FIELD_NAME);
        VectorDB::JsonFieldDataPtr dynamicFieldPtr = std::static_pointer_cast<VectorDB::JsonFieldData>(dynamicField);
        auto& dynamicFieldData = dynamicFieldPtr->Data();

        for (size_t i = 0; i < ids.size(); ++i) {
            // 根据相似度阈值筛选结果，数据库查询结果是按照相似度倒排的，如果出现相似度小于阈值的，后面的结果都比阈值小
            if (similarity[i] < threshold) {
                PRINT_INFO("SearchMetadata similarity %f less than threshold %f.\n", similarity[i], threshold);
                break;
            }

            PRINT_DEBUG("SearchMetadata id: %s, dynamic data: %s, similarity: %f.\n", ids[i].c_str(),
                        dynamicFieldData[i].dump().c_str(), similarity[i]);
            output.emplace_back(MetadataResult{ids[i], similarity[i], dynamicFieldData[i]});
        }
    }
    return output;
}

bool DataManagementVectorDatabase::hasCollection(const std::string& collectionName) {
    bool exist = false;
    bool ret = client_->HasCollection(collectionName, exist).IsOk();
    if (!ret) {
        return false;
    }
    return exist;
}

std::vector<std::string> DataManagementVectorDatabase::showCollection() {
    std::vector<std::string> collection_names;

    auto status = client_->ShowCollections(collection_names);
    if (!status.IsOk()) {
        return std::vector<std::string>();
    }
    return collection_names;
}

std::string DataManagementVectorDatabase::getDbFilePath() {
    std::string homePath = std::getenv("HOME") ? std::string(std::getenv("HOME")) : "";
    std::string databasePath = homePath + "/.local/kylin-ai-business-framework/datamanagement/database/";

    if (!std::filesystem::exists(databasePath)) {
        // 创建目录
        bool ret = std::filesystem::create_directories(databasePath);
        if (!ret) {
            PRINT_ERROR("Create db path failed.\n");
            return "";
        }
        PRINT_INFO("Create db path success.\n");
    }

    return databasePath + "default.db";
}