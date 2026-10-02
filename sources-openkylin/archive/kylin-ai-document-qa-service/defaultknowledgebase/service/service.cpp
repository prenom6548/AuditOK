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

#include "service.h"

#include <kylin-ai-document-service/include/client.h>
#include <kylin-ai/coreai/embedding/embedding.h>
#include <kylin-ai/coreai/embedding/error.h>
#include <unistd.h>
#include <uuid/uuid.h>

#include <iostream>
#include <memory>
#include <set>

namespace knowledgebase_tool {

static const char* DATABASE_FILE_PATH = "filepath";
static const char* DATABASE_DOCUMENT = "document";
static const char* DATABASE_ID = "ids";
static const char* DATABASE_VECTOR = "vector";
static uint32_t DIMENSION = 768;

const char* DOCUMENT_CHUNK_TEXT_TYPE = "TextChunker";
const char* DOCUMENT_RESPONSE_DATA = "responseData";
const int DOCUMENT_CHUNK_TEXT_SIZE = 800;
const char* CHUNKS = "chunks";
const char* FILEPATH = "filePath";

using namespace VectorDB;

std::string generateUUID() {
    uuid_t uuid;
    char uuid_str[37];

    uuid_generate_random(uuid);
    uuid_unparse(uuid, uuid_str);
    std::string id = uuid_str;

    return id;
}

bool Service::checkStatus(std::string&& prefix, const Status& status) {
    if (!status.IsOk()) {
        std::cerr << prefix << ",errMsg: " << status.Message() << std::endl;
        return false;
    }
    return true;
}
Service::Service() { client_ = Database::Create(); }
Service::~Service() { disconnect(); }

bool Service::disconnect() {
    auto status = client_->Disconnect();
    if (!checkStatus("Failed to disconnect vector db ", status)) {
        return false;
    }
    return true;
}
bool Service::hasCollection(const std::string& name) {
    bool exist;
    auto status = client_->HasCollection(name, exist);
    // 数据库接口有问题，临时兼容下，如果返回报错也认为不存在，Failed to has TEST collection: 1002: collection not found
    if (!status.IsOk()) {
        return false;
    }
    return exist;
}
bool Service::createCollection(const std::string& name) {
    ConnectParam connect_param("localhost:19530");  // 19539 milvus-lite官方推荐端口
    auto status = client_->Connect(connect_param);
    if (!checkStatus("Failed to connect vector db ", status)) {
        return false;
    }
    if (hasCollection(name)) {
        return true;
    }

    CollectionSchema collection_schema(name);
    collection_schema.AddField(FieldSchema(DATABASE_FILE_PATH, DataType::VARCHAR, "file path").WithMaxLength(100));
    collection_schema.AddField(
        FieldSchema(DATABASE_DOCUMENT, DataType::VARCHAR, "document contents").WithMaxLength(1024));
    collection_schema.AddField(FieldSchema(DATABASE_ID, DataType::VARCHAR, "ids", true, false).WithMaxLength(40));
    collection_schema.AddField(
        FieldSchema(DATABASE_VECTOR, DataType::FLOAT_VECTOR, "vectors").WithDimension(DIMENSION));

    IndexDesc index_desc(DATABASE_VECTOR, "vector-index", 0);

    status = client_->CreateCollection(collection_schema, index_desc);
    if (!checkStatus("Failed to create collection " + name, status)) {
        return false;
    }

    return true;
}

bool Service::addFiles(const std::string& name, const std::vector<std::string>& filepaths) {
    ConnectParam connect_param("localhost:19530");  // 19539 milvus-lite官方推荐端口
    auto status = client_->Connect(connect_param);

    if (!checkStatus("Failed to connect vector db ", status)) {
        return false;
    }

    // 分块 获得向量化结果
    std::vector<ChunkResult> chunkResult = getChunkResult(filepaths, DOCUMENT_CHUNK_TEXT_SIZE);
    if (chunkResult.empty()) {
        std::cerr << "Failed to add files, chunkResult is empty." << std::endl;
        return false;
    }
    std::vector<KnowledgeFileInfo> infos;
    for (const auto& item : chunkResult) {
        std::vector<std::vector<float>> embeddingResult = embedding(item.chunk);
        KnowledgeFileInfo info{name, item.filePath, item.chunk, embeddingResult};
        infos.push_back(info);
    }

    // 结果存入向量数据库
    for (const KnowledgeFileInfo& fileinfo : infos) {
        std::vector<std::string> ids;
        std::vector<std::string> filepaths;
        std::vector<std::vector<float>> embeddings = fileinfo.vectors;
        std::vector<std::string> documents = fileinfo.documents;

        for (int i = 0; i < embeddings.size(); i++) {
            ids.emplace_back(generateUUID());
            filepaths.emplace_back(fileinfo.filePath);
        }

        std::vector<FieldDataPtr> fields_data{std::make_shared<VarCharFieldData>(DATABASE_ID, ids),
                                              std::make_shared<VarCharFieldData>(DATABASE_FILE_PATH, filepaths),
                                              std::make_shared<VarCharFieldData>(DATABASE_DOCUMENT, documents),
                                              std::make_shared<FloatVecFieldData>(DATABASE_VECTOR, embeddings)};
        DmlResults dml_results;

        auto status = client_->Insert(fileinfo.collectionName, fields_data, dml_results);
        if (!checkStatus("Failed to insert files", status)) {
            return false;
        }
    }
    return true;
}

bool Service::deleteFiles(const std::string& name, const std::vector<std::string>& filepaths) {
    // std::cout << port << " 1111111111111" << std::endl;
    ConnectParam connect_param("localhost:19530");  // 19530 milvus-lite官方推荐端口
    auto status = client_->Connect(connect_param);
    if (!checkStatus("Failed to connect vector db ", status)) {
        return false;
    }
    for (const auto& filepath : filepaths) {
        std::string expression = std::string(DATABASE_FILE_PATH) + " == \"" + filepath + "\"";
        DmlResults results;
        auto status = client_->Delete(name, expression, results);
        if (!checkStatus("Failed to delete files: " + filepath, status)) {
            return false;
        }
    }
    // std::cout << port << " 22222222222222" << std::endl;
    return true;
}
bool Service::updateFiles(const std::string& name, const std::vector<std::string>& filepaths) {
    // 先删在加
    if (!deleteFiles(name, filepaths)) return false;
    if (!addFiles(name, filepaths)) return false;
    return true;
}
bool Service::search(const std::string& name, const std::string& text, float threshold) {
    ConnectParam connect_param("localhost:19530");
    auto status = client_->Connect(connect_param);
    if (!checkStatus("Failed to connect vector db ", status)) {
        return false;
    }
    // 向量化
    std::vector<std::vector<float>> vectors = embedding(std::vector<std::string>{text});

    SearchArguments arguments{};
    arguments.SetCollectionName(name);
    arguments.SetTopK(10);
    arguments.AddOutputField(DATABASE_DOCUMENT);
    arguments.AddOutputField(DATABASE_FILE_PATH);
    arguments.SetGuaranteeTimestamp(GuaranteeStrongTs());

    arguments.AddTargetVector(DATABASE_VECTOR, vectors[0]);

    SearchResults results{};
    status = client_->Search(arguments, results);
    if (!checkStatus("Failed to search", status)) {
        return false;
    }

    KnowledgebaseSearchResults output;

    for (const auto& result : results.Results()) {
        auto& ids = result.Ids().StrIDArray();
        if (ids.empty()) {
            std::cerr << "SearchMetadata result is empty" << std::endl;
            continue;
        }

        auto similarity = result.Scores();
        if (ids.size() != similarity.size()) {
            std::cerr << "Illegal search result" << std::endl;
            continue;
        }

        auto documentResult = result.OutputField(DATABASE_DOCUMENT);
        auto filepathResult = result.OutputField(DATABASE_FILE_PATH);
        VarCharFieldDataPtr documentResult_ptr = std::static_pointer_cast<VarCharFieldData>(documentResult);
        VarCharFieldDataPtr filepathResult_ptr = std::static_pointer_cast<VarCharFieldData>(filepathResult);
        auto documentResult_data = documentResult_ptr->Data();
        auto filepathResult_data = filepathResult_ptr->Data();

        for (int i = 0; i < ids.size(); i++) {
            // 根据相似度阈值筛选结果，数据库查询结果是按照相似度倒排的，如果出现相似度小于阈值的，后面的结果都比阈值小
            if (similarity[i] < threshold) {
                std::cout << "SearchMetadata similarity " << similarity[i] << " less than threshold " << threshold
                          << std::endl;
                break;
            }
            output.emplace_back(
                KnowledgebaseSearchResult{documentResult_data[i], std::string(filepathResult_data[i]), similarity[i]});
        }
    }

    if (output.empty()) {
        std::cerr << "Search result is empty." << std::endl;
        return false;
    }
    std::cout << "Start print search result." << std::endl;
    for (const auto& item : output) {
        std::cout << "[filepath] :" << item.source << std::endl;
        std::cout << "[similarity] :" << item.similarity << std::endl;
        std::cout << "[document]: " << item.documents << std::endl;
        std::cout << std::endl;
    }
    std::cout << "Print search result end." << std::endl;
    return true;
}
bool Service::dropCollection(const std::string& name) {
    ConnectParam connect_param("localhost:19530");
    auto status = client_->Connect(connect_param);
    if (!checkStatus("Failed to connect vector db ", status)) {
        return false;
    }

    if (!hasCollection(name)) {
        return true;
    }

    status = client_->DropCollection(name);
    return checkStatus("Failed to drop collection " + name, status);
}
bool Service::query(const std::string& name) {
    ConnectParam connect_param("localhost:19530");
    auto status = client_->Connect(connect_param);
    if (!checkStatus("Failed to connect vector db ", status)) {
        return false;
    }
    if (!hasCollection(name)) {
        return false;
    }

    QueryArguments arguments;
    arguments.SetCollectionName(name);
    // 查询输出哪些列
    arguments.AddOutputField(DATABASE_FILE_PATH);
    // arguments.AddOutputField(DATABASE_DOCUMENT);
    // arguments.AddOutputField(DATABASE_ID);
    // arguments.AddOutputField(DATABASE_VECTOR);

    // 分两次查询
    arguments.SetExpression("filepath LIKE '/%'");

    std::cout << "First query" << std::endl;
    QueryResults queryResults;
    status = client_->Query(arguments, queryResults);
    checkStatus("Failed to query:", status);
    std::cout << "Query success. output field num: " << queryResults.OutputFields().size() << std::endl;

    FieldDataPtr filepath = queryResults.GetFieldByName(DATABASE_FILE_PATH);
    std::shared_ptr<VarCharFieldData> filepathFieldPtr = std::static_pointer_cast<VarCharFieldData>(filepath);
    auto& filepathData = filepathFieldPtr->Data();
    std::cout << "Query filepath rows: " << filepathData.size() << std::endl;

    // FieldDataPtr ids = queryResults.GetFieldByName(DATABASE_ID);
    // std::shared_ptr<VarCharFieldData> idsFieldPtr = std::static_pointer_cast<VarCharFieldData>(ids);
    // auto& idsData = idsFieldPtr->Data();
    // std::cout << "Query ids rows: " << idsData.size() << std::endl;

    // FieldDataPtr document = queryResults.GetFieldByName(DATABASE_DOCUMENT);
    // std::shared_ptr<VarCharFieldData> docFieldPtr = std::static_pointer_cast<VarCharFieldData>(document);
    // auto& docData = docFieldPtr->Data();
    // std::cout << "Query document rows: " << docData.size() << std::endl;

    // FieldDataPtr vec = queryResults.GetFieldByName(DATABASE_VECTOR);
    // std::shared_ptr<FloatVecFieldData> vecFieldPtr = std::static_pointer_cast<FloatVecFieldData>(vec);
    // auto& vecData = vecFieldPtr->Data();
    // std::cout << "Query vec rows: " << vecData.size() << std::endl;

    if (filepathData.empty()) {
        std::cout << "First query result is empty" << std::endl;
        return false;
    }
    std::unordered_set<std::string> filepaths;  // 过滤重复
    for (int i = 0; i < filepathData.size(); ++i) {
        // std::cout << filepathFieldPtr->Name() << ": " << filepathData[i] << std::endl;
        filepaths.insert(filepathData[i]);
    }
    std::cout << "filepath 过滤后的结果：" << std::endl;
    // 输出所有路径
    for (const auto& path : filepaths) {
        std::cout << "filepath: " << path << std::endl;
    }

    QueryArguments argumentsSec;
    argumentsSec.SetCollectionName(name);
    // 查询输出哪些列
    argumentsSec.AddOutputField(DATABASE_FILE_PATH);
    std::cout << "Second query" << std::endl;

    // 分两次查询
    argumentsSec.SetExpression("filepath LIKE 'F%'");

    QueryResults queryResultsSecond;
    status = client_->Query(argumentsSec, queryResultsSecond);
    checkStatus("Failed to query:", status);
    std::cout << "Query success. output field num: " << queryResultsSecond.OutputFields().size() << std::endl;

    FieldDataPtr filepathSec = queryResultsSecond.GetFieldByName(DATABASE_FILE_PATH);
    std::shared_ptr<VarCharFieldData> filepathFieldPtrSec = std::static_pointer_cast<VarCharFieldData>(filepathSec);
    auto& filepathDataSec = filepathFieldPtrSec->Data();
    std::cout << "Query filepath rows: " << filepathDataSec.size() << std::endl;

    if (filepathDataSec.empty()) {
        std::cout << "Second query result is empty" << std::endl;
        return false;
    }
    std::unordered_set<std::string> filepathsSec;  // 过滤重复
    for (int i = 0; i < filepathDataSec.size(); ++i) {
        filepathsSec.insert(filepathDataSec[i]);
    }
    std::cout << "filepath 过滤后的结果：" << std::endl;
    // 输出所有路径
    for (const auto& path : filepathsSec) {
        std::cout << "filepath: " << path << std::endl;
    }

    // std::vector<std::vector<FieldDataPtr>> fields_datas;
    // for (int i = 0; i < idsData.size(); ++i) {
    //     std::cout << idsFieldPtr->Name() << ": " << idsData[i] << "\t" << filepathFieldPtr->Name() << ": "
    //               << filepathData[i] << "\t" << docFieldPtr->Name() << ": " << docData[i] << vecFieldPtr->Name()
    //               << ": [";
    //     for (const auto& item : vecData[i]) {
    //         std::cout << item << ", ";
    //     }
    //     std::cout << "]" << std::endl;

    //     // fields_datas.emplace_back(std::vector<FieldDataPtr>{
    //     //     std::make_shared<VarCharFieldData>(DATABASE_ID, std::vector<std::string>{idsData[i]}),
    //     //     std::make_shared<VarCharFieldData>(DATABASE_FILE_PATH, std::vector<std::string>{filepathData[i]}),
    //     //     std::make_shared<VarCharFieldData>(DATABASE_DOCUMENT, std::vector<std::string>{docData[i]}),
    //     //     std::make_shared<FloatVecFieldData>(DATABASE_ID, std::vector<std::vector<float>>{vecData[i]})});
    // }

    return true;
}
std::vector<std::vector<float>> Service::embedding(const std::vector<std::string>& texts) {
    // 调用向量化服务，向量化文本

    TextEmbeddingSession* session = text_embedding_create_session();
    int initSession = text_embedding_init_session(session);
    if (initSession != COREAI_EMBEDDING_SUCESS) {
        std::cerr << "init embedding session failed" << std::endl;
        return std::vector<std::vector<float>>();
    }
    std::vector<std::vector<float>> res;

    for (const auto& item : texts) {
        EmbeddingResult* result = nullptr;
        bool sucess = text_embedding(session, item.c_str(), &result);

        if (!result) {
            std::cerr << "text embedding result is nullptr" << std::endl;
            continue;
        }

        if (embedding_result_get_error_code(result) != COREAI_EMBEDDING_SUCESS) {
            const char* error_message = embedding_result_get_error_message(result);
            std::cout << "error_message :" << error_message << std::endl;
            continue;
        }

        int vectorLength = embedding_result_get_vector_length(result);
        float* vectorData = embedding_result_get_vector_data(result);
        std::vector<float> vec = std::vector<float>(vectorData, vectorData + vectorLength);
        embedding_result_destroy(&result);
        res.emplace_back(vec);
    }

    if (session) {
        text_embedding_destroy_session(&session);
    }

    return res;
}

std::vector<ChunkResult> Service::getChunkResult(const std::vector<std::string>& filePaths, int32_t size) {
    nlohmann::json ret = syncChunkDocument(filePaths, size);
    if (ret.empty()) {
        std::cerr << "chunk document server something wrong" << std::endl;
        return std::vector<ChunkResult>();
    }
    return parseChunkResult(ret);
}

nlohmann::json Service::syncChunkDocument(const std::vector<std::string>& filePaths, int32_t size) {
    // 调用分块服务 获得分块文本

    std::shared_ptr<document::DocumentChunkClient> client = std::make_shared<document::DocumentChunkClient>();

    std::vector<std::string> chunkTypes(filePaths.size(), DOCUMENT_CHUNK_TEXT_TYPE);
    std::vector<int32_t> chunkSizes(filePaths.size(), size);

    auto result = client->SyncChunkDocument(filePaths, chunkTypes, chunkSizes);
    // grpc 通信异常
    if (result.errorCode != 0) {
        std::cerr << "DocumentClient connect failed, err: " << result.errorCode << ",msg: " << result.errorMsg
                  << std::endl;
        return nlohmann::json::array();
    }

    nlohmann::json root = nlohmann::json::parse(result.data);
    if (root.empty() || !root.contains(DOCUMENT_RESPONSE_DATA) || !root[DOCUMENT_RESPONSE_DATA].is_array() ||
        root[DOCUMENT_RESPONSE_DATA].empty()) {
        std::cerr << "Failed to parse DocumentClient result: " << result.data << std::endl;
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
std::vector<ChunkResult> Service::parseChunkResult(nlohmann::json& data) {
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
        res.push_back(chunkResult);
    }

    return res;
}

}  // namespace knowledgebase_tool
