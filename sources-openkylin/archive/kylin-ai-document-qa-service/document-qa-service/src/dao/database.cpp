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

#include "database.h"

#include <kyai-business-framework/utils.h>
#include <log.h>

using namespace VectorDB;

const int TEXT_DIMENSION = 768;

static const char* DATABASE_FILE_PATH = "filepath";
static const char* DATABASE_DOCUMENT = "document";
static const char* DATABASE_VECTOR = "vector";
static const char* DATABASE_ID = "ids";

std::string getHomeDirectory() {
    // 1. 优先使用 HOME 环境变量
    const char* home_env = std::getenv("HOME");
    if (home_env != nullptr && home_env[0] != '\0') {
        return std::string(home_env);
    }

    // 2. 尝试获取 USER 环境变量
    const char* user_env = std::getenv("USER");
    if (user_env == nullptr || user_env[0] == '\0') {
        // 连 USER 都没有，无法推断，返回当前目录
        return ".";
    }

    std::string username(user_env);

    // 3. 特殊处理 root 用户
    if (username == "root") {
        return "/root";
    }

    // 4. 普通用户：/home/用户名
    return "/home/" + username;
}

KnowledgebaseVectorDatabase::KnowledgebaseVectorDatabase() : isConnected(false), client_(Database::Create()) {}

VectorDatabaseErrorCode KnowledgebaseVectorDatabase::connect() {
    if (isConnected) return VectorDatabaseErrorCode(true);

    VectorDB::ConnectParam param(DATABASE_APPID);
    auto status = client_->Connect(param);
    if (!status.IsOk()) {
        PRINT_ERROR("Failed to connect vector db,code: %d, message: %s\n", static_cast<int>(status.Code()),
                    status.Message().c_str());
        return VectorDatabaseErrorCode(false, status.Code(), status.Message());
    }

    std::string home = getHomeDirectory();
    std::string dbDir = home + "/.local/share/document_qa_service";
    std::filesystem::create_directories(dbDir);
    std::string dbFilePath = dbDir + "/document_qa.db";

    status = client_->LoadDBFile(dbFilePath, true, DATABASE_PASSWORD);
    if (status.IsOk()) {
        isConnected = true;
    }

    isConnected = true;
    return VectorDatabaseErrorCode(true);
}

VectorDatabaseErrorCode KnowledgebaseVectorDatabase::disconnect() {
    if (!isConnected) return VectorDatabaseErrorCode(true);
    auto status = client_->Disconnect();
    if (!status.IsOk()) {
        PRINT_ERROR("Failed to disconnect vector db,code: %d, message: %s\n", static_cast<int>(status.Code()),
                    status.Message().c_str());
        return VectorDatabaseErrorCode(false, status.Code(), status.Message());
    }

    isConnected = false;
    return VectorDatabaseErrorCode(true);
}

VectorDatabaseErrorCode KnowledgebaseVectorDatabase::createCollection(std::string& collectionName) {
    VectorDatabaseErrorCode errorCode = connect();
    if (!errorCode.success) return errorCode;

    std::string knowledgeBaseName = Utils::generateUUID();
    std::replace(knowledgeBaseName.begin(), knowledgeBaseName.end(), '-', '_');
    collectionName = "_" + knowledgeBaseName;

    VectorDB::CollectionSchema collection_schema(collectionName);
    collection_schema.AddField(FieldSchema(DATABASE_FILE_PATH, DataType::VARCHAR, "file path").WithMaxLength(100));
    collection_schema.AddField(
        FieldSchema(DATABASE_DOCUMENT, DataType::VARCHAR, "document contents").WithMaxLength(1024));
    collection_schema.AddField(FieldSchema(DATABASE_ID, DataType::VARCHAR, "ids", true, false).WithMaxLength(40));
    collection_schema.AddField(
        FieldSchema(DATABASE_VECTOR, DataType::FLOAT_VECTOR, "vectors").WithDimension(TEXT_DIMENSION));

    IndexDesc index_desc(DATABASE_VECTOR, "vector-index");

    // 先判断是否存在该collection 不存在在创建
    bool exist;
    auto status = client_->HasCollection(collectionName, exist);
    if (!status.IsOk()) {
        PRINT_ERROR("Failed to check collection %s,code: %d, message: %s\n", collectionName.c_str(),
                    static_cast<int>(status.Code()), status.Message().c_str());
        return VectorDatabaseErrorCode(false, status.Code(), status.Message());
    }

    if (exist) return VectorDatabaseErrorCode(true);

    status = client_->CreateCollection(collection_schema, index_desc);
    if (!status.IsOk()) {
        PRINT_ERROR("Failed to create collection %s,code: %d, message: %s\n", collectionName.c_str(),
                    static_cast<int>(status.Code()), status.Message().c_str());
        return VectorDatabaseErrorCode(false, status.Code(), status.Message());
    }

    return VectorDatabaseErrorCode(true);
}

VectorDatabaseErrorCode KnowledgebaseVectorDatabase::deleteCollection(const std::string& collectionName) {
    VectorDatabaseErrorCode errorCode = connect();
    if (!errorCode.success) return errorCode;

    bool exist;
    auto status = client_->HasCollection(collectionName, exist);
    if (!status.IsOk()) {
        PRINT_ERROR("Failed to check collection %s,code: %d, message: %s\n", collectionName.c_str(),
                    static_cast<int>(status.Code()), status.Message().c_str());
        return VectorDatabaseErrorCode(false, status.Code(), status.Message());
    }

    if (!exist) return VectorDatabaseErrorCode(true);

    status = client_->DropCollection(collectionName);
    if (!status.IsOk()) {
        PRINT_ERROR("Failed to delete collection %s,code: %d, message: %s\n", collectionName.c_str(),
                    static_cast<int>(status.Code()), status.Message().c_str());
        return VectorDatabaseErrorCode(false, status.Code(), status.Message());
    }

    return VectorDatabaseErrorCode(true);
}

VectorDatabaseErrorCode KnowledgebaseVectorDatabase::insertVectorData(
    const std::vector<KnowledgeFileInfo>& vectorInfos) {
    VectorDatabaseErrorCode errorCode = connect();
    if (!errorCode.success) return errorCode;

    for (const KnowledgeFileInfo& vectorInfo : vectorInfos) {
        std::vector<std::string> ids;
        std::vector<std::string> filepaths;
        std::vector<std::vector<float>> embeddings = vectorInfo.vectors;
        std::vector<std::string> documents = vectorInfo.documents;

        for (int i = 0; i < embeddings.size(); i++) {
            ids.emplace_back(Utils::generateUUID());
            filepaths.emplace_back(vectorInfo.filepath);
        }

        std::vector<FieldDataPtr> fields_data{std::make_shared<VarCharFieldData>(DATABASE_ID, ids),
                                              std::make_shared<VarCharFieldData>(DATABASE_FILE_PATH, filepaths),
                                              std::make_shared<VarCharFieldData>(DATABASE_DOCUMENT, documents),
                                              std::make_shared<FloatVecFieldData>(DATABASE_VECTOR, embeddings)};
        DmlResults dml_results;
        auto status = client_->Insert(vectorInfo.knowledgeBaseName, fields_data, dml_results);
        if (!status.IsOk()) {
            PRINT_ERROR("Failed to insert,code: %d, message: %s\n", static_cast<int>(status.Code()),
                        status.Message().c_str());
            return VectorDatabaseErrorCode(false, status.Code(), status.Message());
        }
        PRINT_DEBUG("%s vector insert rows: %zu\n", vectorInfo.knowledgeBaseName.c_str(),
                    dml_results.IdArray().StrIDArray().size());
    }

    return VectorDatabaseErrorCode(true);
}

KnowledgebaseSearchResults KnowledgebaseVectorDatabase::transformSearchResults(SearchResults results) const {
    KnowledgebaseSearchResults output;

    for (const auto& result : results.Results()) {
        auto& ids = result.Ids().StrIDArray();
        if (ids.empty()) {
            PRINT_WARN("KnowledgebaseVectorDatabase error: searchMetadata result is empty.\n");
            continue;
        }

        auto similarity = result.Scores();
        if (ids.size() != similarity.size()) {
            PRINT_WARN("KnowledgebaseVectorDatabase error: illegal search result.\n");
            continue;
        }

        auto documentResult = result.OutputField(DATABASE_DOCUMENT);
        auto filepathResult = result.OutputField(DATABASE_FILE_PATH);
        VarCharFieldDataPtr documentResult_ptr = std::static_pointer_cast<VarCharFieldData>(documentResult);
        VarCharFieldDataPtr filepathResult_ptr = std::static_pointer_cast<VarCharFieldData>(filepathResult);
        auto documentResult_data = documentResult_ptr->Data();
        auto filepathResult_data = filepathResult_ptr->Data();

        for (int i = 0; i < ids.size(); i++) {
            output.emplace_back(
                KnowledgebaseSearchResult{documentResult_data[i], std::string(filepathResult_data[i]), similarity[i]});
        }
    }
    return output;
}
VectorDatabaseErrorCode KnowledgebaseVectorDatabase::similaritySearch(const std::string& knowledgeBaseName,
                                                                      const std::vector<float>& vector,
                                                                      KnowledgebaseSearchResults& searchResult) {
    VectorDatabaseErrorCode errorCode = connect();
    if (!errorCode.success) return errorCode;

    SearchArguments arguments{};
    arguments.SetCollectionName(knowledgeBaseName);
    arguments.SetTopK(10);
    arguments.AddOutputField(DATABASE_DOCUMENT);
    arguments.AddOutputField(DATABASE_FILE_PATH);
    arguments.SetGuaranteeTimestamp(GuaranteeStrongTs());

    arguments.AddTargetVector(DATABASE_VECTOR, vector);

    SearchResults results{};

    auto status = client_->Search(arguments, results);
    if (!status.IsOk()) {
        PRINT_ERROR("Failed to search,code: %d, message: %s\n", static_cast<int>(status.Code()),
                    status.Message().c_str());
        return VectorDatabaseErrorCode(false, status.Code(), status.Message());
    }

    searchResult = transformSearchResults(results);
    return VectorDatabaseErrorCode(true);
}

KnowledgebaseVectorDatabase::~KnowledgebaseVectorDatabase() { disconnect(); }
