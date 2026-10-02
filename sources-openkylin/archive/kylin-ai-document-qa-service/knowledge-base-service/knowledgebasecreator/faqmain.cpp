#include <rpc/vectorDB/vectorDatabase.h>
#include <unistd.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "../src/common/common.h"
#include "../src/common/embedding.h"
#include "../src/common/utils.h"

using namespace knowledgebase;
using json = nlohmann::json;

const char* DATABASE_APPID = "kylin-ai-knowledge-base-service";
const char* DATABASE_PASSWORD = "k6bf1d_349ecd";
static const char* DATABASE_KNOWLEDGE_ID = "knowledge_id";
static const char* DATABASE_QUESTION = "question";
static const char* DATABASE_QUESTION_DESCRIPTION = "question_description";
static const char* DATABASE_ANSWER = "answer";
static const char* DATABASE_QUESTION_DENSE_VECTOR = "question_dense_vector";

static const char* DATABASE_FAQ_VECTORDB_PATH = "/vector.db";

static uint32_t DIMENSION = 768;

// 辅助函数：将任意JSON值转换为字符串
std::string jsonValueToString(const json& value) {
    if (value.is_string()) {
        return value.get<std::string>();
    } else if (value.is_number()) {
        // 将数字转换为字符串
        std::ostringstream oss;
        if (value.is_number_integer()) {
            oss << value.get<int>();
        } else {
            oss << value.get<double>();
        }
        return oss.str();
    } else if (value.is_null()) {
        return "";
    }
    // 对于其他类型（如布尔值、数组等），可以按需扩展
    return value.dump();
}

std::vector<KnowledgeItem> parseFaqJson(const std::string& jsonFilePath) {
    std::vector<KnowledgeItem> knowledgeItems;

    try {
        // 读取JSON文件
        std::ifstream inputFile(jsonFilePath);
        if (!inputFile.is_open()) {
            throw std::runtime_error("无法打开JSON文件: " + jsonFilePath);
        }

        // 解析JSON
        json jsonData;
        inputFile >> jsonData;

        // 遍历JSON数组
        for (const auto& item : jsonData) {
            std::shared_ptr<FaqItem> itemPtr = std::make_shared<FaqItem>();

            // 使用辅助函数处理可能为数字的字段
            itemPtr->knowledgeId_ = item.contains("问题编号") ? jsonValueToString(item["问题编号"]) : "";
            itemPtr->question_ = item.contains("问题") ? jsonValueToString(item["问题"]) : "";
            itemPtr->description_ = item.contains("问题_new") ? jsonValueToString(item["问题_new"]) : "";
            itemPtr->content_ = item.contains("回答_filter") ? jsonValueToString(item["回答_filter"]) : "";

            KnowledgeItem knowledgeItem(itemPtr, KBType::FAQ);
            knowledgeItems.push_back(knowledgeItem);
        }
    } catch (const std::exception& e) {
        std::cerr << "解析JSON时出错: " << e.what() << std::endl;
        return {};
    }

    return knowledgeItems;
}

bool createVectorDatabase(const std::string& appId, const std::string& password, const std::string& name,
                          const std::string& dataBaseDir) {
    const char* VECTOR_INDEX = NULL;
    VectorDB::CollectionSchema collection_schema(name);

    collection_schema.AddField(
        VectorDB::FieldSchema(DATABASE_KNOWLEDGE_ID, VectorDB::DataType::VARCHAR, "knowledge id", true, true)
            .WithMaxLength(1024));
    collection_schema.AddField(
        VectorDB::FieldSchema(DATABASE_QUESTION, VectorDB::DataType::VARCHAR, "question").WithMaxLength(1024));
    collection_schema.AddField(
        VectorDB::FieldSchema(DATABASE_QUESTION_DESCRIPTION, VectorDB::DataType::VARCHAR, "question description")
            .WithMaxLength(1024));
    collection_schema.AddField(
        VectorDB::FieldSchema(DATABASE_ANSWER, VectorDB::DataType::VARCHAR, "answer").WithMaxLength(1024));
    collection_schema.AddField(
        VectorDB::FieldSchema(DATABASE_QUESTION_DENSE_VECTOR, VectorDB::DataType::FLOAT_VECTOR, "vectors")
            .WithDimension(DIMENSION));
    VECTOR_INDEX = DATABASE_QUESTION_DENSE_VECTOR;

    VectorDB::ConnectParam param(appId);
    VectorDB::IndexDesc index_desc(VECTOR_INDEX, "vector-index");
    std::string absoluteDbFilePath = dataBaseDir;
    std::filesystem::path pathObj(absoluteDbFilePath);
    scene::VectorDatabase vectorDatabase(param, absoluteDbFilePath, true, password);
    VectorDB::Status status = vectorDatabase.createCollection(collection_schema, index_desc);

    return status.IsOk();
}

bool deleteVectorDatabase(const std::string& appId, const std::string& password, const std::string& name,
                          const std::string& dataBaseDir) {
    std::string absoluteDbFilePath = dataBaseDir;

    VectorDB::ConnectParam param(appId);
    scene::VectorDatabase vectorDatabase(param, absoluteDbFilePath, true, password);
    VectorDB::Status status = vectorDatabase.dropCollection(name);
    if (!status.IsOk()) {
        printf("deleteVectorDatabase error: %s\n", status.Message().c_str());
    }

    return status.IsOk();
}

bool addVectors(const std::string& appId, const std::string& password, const std::string& vectorDbName,
                const std::vector<KnowledgeItem>& knowledgeItems, const std::string& dataBaseDir) {
    Embedding textEmbedding;
    std::vector<VectorDB::FieldDataPtr> fields_data;
    // 分块 获得向量化结果
    if (knowledgeItems.size() == 0) {
        printf("knowledgeItems data empty\n");
        return false;
    }
    std::vector<std::string> knowledgeId;
    std::vector<std::string> question;
    std::vector<std::string> description;
    std::vector<std::string> answer;
    std::vector<std::vector<float>> embedding;
    int index = 0;
    for (auto& knowledgeItem : knowledgeItems) {
        FaqItem* item = static_cast<FaqItem*>(knowledgeItem.item_.get());
        std::string itemContent = item->description_;
        printf("faq data embedding: (%d/%ld)\n", index++, knowledgeItems.size());
        std::vector<float> embeddingResult = textEmbedding.getTextEmbedding(itemContent);
        knowledgeId.push_back(item->knowledgeId_);
        question.push_back(item->question_);
        description.push_back(item->description_);
        answer.push_back(item->content_);
        embedding.push_back(embeddingResult);
    }
    fields_data.push_back(std::make_shared<VectorDB::VarCharFieldData>(DATABASE_KNOWLEDGE_ID, knowledgeId));
    fields_data.push_back(std::make_shared<VectorDB::VarCharFieldData>(DATABASE_QUESTION, question));
    fields_data.push_back(std::make_shared<VectorDB::VarCharFieldData>(DATABASE_QUESTION_DESCRIPTION, description));
    fields_data.push_back(std::make_shared<VectorDB::VarCharFieldData>(DATABASE_ANSWER, answer));
    fields_data.push_back(std::make_shared<VectorDB::FloatVecFieldData>(DATABASE_QUESTION_DENSE_VECTOR, embedding));

    VectorDB::DmlResults dml_results;
    VectorDB::ConnectParam param(appId);
    std::string absoluteDbFilePath = dataBaseDir;
    std::filesystem::path pathObj(absoluteDbFilePath);
    scene::VectorDatabase vectorDatabase(param, absoluteDbFilePath, true, password);
    VectorDB::Status insertStatus = vectorDatabase.insert(vectorDbName, fields_data, dml_results);

    return insertStatus.IsOk();
}

int main() {
    std::string vectorDbName = "kakbs_kylin_om_faq";
    std::string jsonFile = "./output.json";
    std::string home = getHomeDirectory();
    std::string dataBaseDir = home + DATABASE_FAQ_VECTORDB_PATH;
    bool succ = deleteVectorDatabase(DATABASE_APPID, DATABASE_PASSWORD, vectorDbName, dataBaseDir);
    printf("deleteVectorDatabase: %d\n", succ);
    succ = createVectorDatabase(DATABASE_APPID, DATABASE_PASSWORD, vectorDbName, dataBaseDir);
    printf("createVectorDatabase: %d\n", succ);
    std::vector<KnowledgeItem> knowledgeItems = parseFaqJson(jsonFile);

    // 打印解析结果
    for (const auto& knowledgeItem : knowledgeItems) {
        FaqItem* item = static_cast<FaqItem*>(knowledgeItem.item_.get());
        std::cout << "知识编号: " << item->knowledgeId_ << "\n"
                  << "问题: " << item->question_ << "\n"
                  << "描述: " << item->description_ << "\n"
                  << "回答: " << item->content_ << "\n\n";
    }

    addVectors(DATABASE_APPID, DATABASE_PASSWORD, vectorDbName, knowledgeItems, dataBaseDir);

    return 0;
}