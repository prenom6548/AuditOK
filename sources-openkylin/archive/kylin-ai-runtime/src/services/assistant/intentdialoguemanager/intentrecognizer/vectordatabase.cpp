/*
 * Copyright 2026 KylinSoft Co., Ltd.
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

#include "vectordatabase.h"

#include <jsoncpp/json/json.h>
#include <unistd.h>

#include <filesystem>
#include <sstream>

#include "utils/logger.h"

namespace intent {

// 辅助函数：Json::Value 转换为 nlohmann::json（通过字符串）
static nlohmann::json jsoncppToNlohmann(const Json::Value& jsonValue) {
    Json::StreamWriterBuilder writerBuilder;
    writerBuilder["indentation"] = "";
    writerBuilder["emitUTF8"] = true;  // 允许 UTF-8 字符，不转义为 \uXXXX
    std::string jsonStr = Json::writeString(writerBuilder, jsonValue);
    return nlohmann::json::parse(jsonStr);
}

// 辅助函数：nlohmann::json 转换为 Json::Value（通过字符串）
static Json::Value nlohmannToJsoncpp(const nlohmann::json& nlohmannJson) {
    std::string jsonStr = nlohmannJson.dump();
    Json::Value jsonValue;
    Json::CharReaderBuilder readerBuilder;
    std::string errors;
    std::istringstream stream(jsonStr);
    Json::parseFromStream(readerBuilder, stream, &jsonValue, &errors);
    return jsonValue;
}

static const char* FIELD_INTENT_TEXT = "intent_text";
static const char* FIELD_INTENT_VECTOR = "intent_vector";
static const char* FIELD_METADATA = "metadata";

// 检测是否创建连接成功的函数
static bool checkStatus(std::string&& prefix, const VectorDB::Status& status) {
    if (!status.IsOk()) {
        LOGE("{} {} {}", prefix, static_cast<int>(status.Code()), status.Message());
        return false;
    }

    return true;
}

VectorDatabase::VectorDatabase() : connectParam_(appId_) {
    vectorDatabase_ = VectorDB::Database::Create();
    if (vectorDatabase_ == nullptr) {
        LOGE("Failed to create vector database.");
        isInitialized_ = false;
        return;
    }

    if (!connectVectorDatabase()) {
        LOGE("Failed to connect to vector database.");
        isInitialized_ = false;
        return;
    }

    isInitialized_ = true;
}

VectorDatabase::~VectorDatabase() {
    if (vectorDatabase_ == nullptr) {
        return;
    }

    VectorDB::Status disconnectStatus = vectorDatabase_->Disconnect();
    checkStatus("Failed to disconnect milvus server:", disconnectStatus);
    vectorDatabase_.reset();
}

bool VectorDatabase::hasCollection() {
    bool existCollection = false;
    auto status = vectorDatabase_->HasCollection(collectionName_, existCollection);
    if (!status.IsOk()) {
        return false;
    }
    return existCollection;
}

bool VectorDatabase::createCollection() {
    VectorDB::CollectionSchema collection_text_schema(collectionName_,
                                                      "This collection stores vectorized data of intent label", true);
    // 设置向量数据库的列名称
    collection_text_schema.AddField(
        VectorDB::FieldSchema{"id", VectorDB::DataType::VARCHAR, "id field", true, true}.WithMaxLength(40));
    collection_text_schema.AddField(
        VectorDB::FieldSchema{FIELD_INTENT_TEXT, VectorDB::DataType::VARCHAR, "intent text field"}.WithMaxLength(200));
    collection_text_schema.AddField(
        VectorDB::FieldSchema{FIELD_INTENT_VECTOR, VectorDB::DataType::FLOAT_VECTOR, "intent vector field"}
            .WithDimension(768));
    collection_text_schema.AddField(
        VectorDB::FieldSchema{FIELD_METADATA, VectorDB::DataType::JSON, "meta field"}.WithMaxLength(1000));

    // 设置索引便于搜索，默认是COS搜索
    VectorDB::IndexDesc index_desc(FIELD_INTENT_VECTOR, "intent_vector_index", VectorDB::IndexType::FLAT,
                                   VectorDB::MetricType::COSINE);

    // 创建数据库表格并返回结果
    VectorDB::Status collectionTextStatus = vectorDatabase_->CreateCollection(collection_text_schema, index_desc);
    bool ret = checkStatus("Failed to create collection:", collectionTextStatus);
    return ret;
}

bool VectorDatabase::dropCollection() {
    auto status = vectorDatabase_->DropCollection(collectionName_);
    if (!status.IsOk()) {
        return false;
    }
    return true;
}

bool VectorDatabase::connectVectorDatabase() {
    bool isConnected = false;
    bool isDBFileLoaded = false;
    VectorDB::Status status = vectorDatabase_->Connect(connectParam_);
    isConnected = checkStatus("Failed to connect milvus server:", status);
    if (!isConnected) {
        LOGE("Failed to connect milvus server");
        return false;
    }

    status = vectorDatabase_->LoadDBFile(intentVectorDBPath_, false, "");
    if (!status.IsOk()) {
        LOGE("Failed to load vector database file");
        return false;
    }

    // 检查文本collection表是否存在
    bool isDatabaseValid = hasCollection();
    if (isDatabaseValid == false) {
        isDatabaseValid = createCollection();
    }
    return isDatabaseValid;
}

bool VectorDatabase::insert(std::vector<IntentRecord>& intentRecords) {
    if (false == isInitialized_) {
        LOGE("向量数据库连接失败，无法保存数据");
        return false;
    }

    std::vector<std::string> intentTextVec;
    std::vector<std::vector<float>> embeddingVec;
    std::vector<nlohmann::json> metaDataVec;
    for (auto& intent : intentRecords) {
        intentTextVec.push_back(intent.intent_text);
        embeddingVec.push_back(intent.intent_vector);
        // 使用 Json::Value 构建数据
        Json::Value metaData;
        metaData["intent_type"] = intent.intent_type;
        metaData["intent_name"] = intent.intent_name;
        metaData["primary_intent_name"] = intent.primary_intent_name;
        // 转换为 nlohmann::json（向量数据库 API 需要）
        metaDataVec.push_back(jsoncppToNlohmann(metaData));
    }

    std::vector<VectorDB::FieldDataPtr> fields_data;
    fields_data.push_back(std::make_shared<VectorDB::VarCharFieldData>(FIELD_INTENT_TEXT, intentTextVec));
    fields_data.push_back(std::make_shared<VectorDB::FloatVecFieldData>(FIELD_INTENT_VECTOR, embeddingVec));
    fields_data.push_back(std::make_shared<VectorDB::JsonFieldData>(FIELD_METADATA, metaDataVec));

    VectorDB::DmlResults dml_results;
    VectorDB::Status status = vectorDatabase_->Insert(collectionName_, fields_data, dml_results);
    if (false == checkStatus("Failed to insert:", status)) {
        return false;
    }
    LOGI("Successfully insert {} rows.", dml_results.IdArray().StrIDArray().size());
    return true;
}

std::vector<IntentResult> VectorDatabase::search(const std::vector<float>& queryEmbeddings) const {
    if (false == isInitialized_) {
        LOGE("向量数据库连接失败，无法保存数据");
        return {};
    }

    if (queryEmbeddings.size() == 0) {
        LOGW("Please ensure that the number of results requested is at least 1.");
        return {};
    }

    std::string embedding_str;
    for (auto& embedding : queryEmbeddings) {
        embedding_str += std::to_string(embedding) + " ";
    }
    // 相似搜索
    VectorDB::SearchArguments arguments;
    arguments.SetCollectionName(collectionName_);
    // topk 最大值是 16384，超过后不合法
    arguments.SetTopK(5);
    // 查询输出哪些列
    arguments.AddOutputField(FIELD_INTENT_TEXT);
    arguments.AddOutputField(FIELD_METADATA);
    // set to strong guarantee so that the search is executed after the inserted data is persisted
    arguments.SetGuaranteeTimestamp(VectorDB::GuaranteeStrongTs());
    // 设置要搜索的向量
    arguments.AddTargetVector(FIELD_INTENT_VECTOR, std::move(queryEmbeddings));

    VectorDB::SearchResults searchResults{};
    VectorDB::Status searchStatus = vectorDatabase_->Search(arguments, searchResults);
    if (false == checkStatus("Failed to search:", searchStatus)) {
        return {};
    }

    std::vector<IntentResult> intentResults;
    for (const auto& result : searchResults.Results()) {
        auto& ids = result.Ids().StrIDArray();
        if (ids.empty()) {
            LOGD("No search result found");
            return {};
        }

        VectorDB::VarCharFieldDataPtr intentText_ptr =
            std::static_pointer_cast<VectorDB::VarCharFieldData>(result.OutputField(FIELD_INTENT_TEXT));
        VectorDB::JsonFieldDataPtr metaData_ptr =
            std::static_pointer_cast<VectorDB::JsonFieldData>(result.OutputField(FIELD_METADATA));
        auto intentText_data = intentText_ptr->Data();
        auto metaData_data = metaData_ptr->Data();

        std::vector<float> distances = result.Scores();
        for (size_t i = 0; i < ids.size(); ++i) {
            IntentResult intentResult;
            // 从 nlohmann::json（向量数据库 API 返回）转换为 Json::Value
            Json::Value metaData = nlohmannToJsoncpp(metaData_data[i]);
            intentResult.intent_name = metaData["intent_name"].asString();
            intentResult.intent_score = distances[i];
            intentResult.intent_type = metaData["intent_type"].asString();
            intentResult.recall_text = intentText_data[i];
            intentResults.push_back(intentResult);
        }
    }

    return intentResults;
}

}  // namespace intent