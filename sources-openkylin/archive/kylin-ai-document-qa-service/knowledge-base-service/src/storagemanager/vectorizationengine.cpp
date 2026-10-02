#include "vectorizationengine.h"

#include <kyai-business-framework/log.h>
#include <kyai-business-framework/rpc/vectorDB/vectorDatabase.h>

#include <filesystem>

#include "../common/performancetimer.h"

namespace knowledgebase {

using namespace VectorDB;

static const char* DATABASE_ID = "ids";
static const char* DATABASE_PARAGRAPH_INDEX = "paragraph_index";
static const char* DATABASE_CONTENT = "content";
static const char* DATABASE_DENSE_VECTOR = "dense_vector";
static const char* DATABASE_FILE_ID = "file_id";
static const char* DATABASE_CHUNK_INDEX = "chunk_index";
static const char* DATABASE_METADATA = "metadata";

static const char* DATABASE_KNOWLEDGE_ID = "knowledge_id";
static const char* DATABASE_QUESTION = "question";
static const char* DATABASE_QUESTION_DESCRIPTION = "question_description";
static const char* DATABASE_ANSWER = "answer";
static const char* DATABASE_QUESTION_DENSE_VECTOR = "question_dense_vector";
static const char* DATABASE_FILE_DEFAULT_NAME = "vector.db";

static uint32_t DIMENSION = 768;

VectorizationEngine::VectorizationEngine() : client_(Database::Create()) {}

bool VectorizationEngine::createVectorDatabase(const std::string& appId, const std::string& password,
                                               const std::string& dbFilePath, const std::string& collectionName,
                                               const std::string& description, const KBType kbType,
                                               const IndexType indexType, KBErrorCode& errCode) {
    errCode = KBErrorCode::SUCCESS;
    std::string absoluteDbFilePath = dbFilePath + "/" + DATABASE_FILE_DEFAULT_NAME;
    if (hasCollection(appId, password, absoluteDbFilePath, collectionName)) {
        errCode = KBErrorCode::KB_ALREADY_EXISTS;
        return false;
    }
    const char* VECTOR_INDEX = NULL;
    CollectionSchema collection_schema(collectionName);
    if (kbType == KBType::RAG) {
        collection_schema.AddField(
            FieldSchema(DATABASE_ID, DataType::VARCHAR, "primary key", true, true).WithMaxLength(1024));
        collection_schema.AddField(
            FieldSchema(DATABASE_PARAGRAPH_INDEX, DataType::JSON, "paragraph index").WithMaxLength(1024));
        collection_schema.AddField(FieldSchema(DATABASE_CONTENT, DataType::VARCHAR, "content").WithMaxLength(1024));
        collection_schema.AddField(FieldSchema(DATABASE_FILE_ID, DataType::VARCHAR, "file id").WithMaxLength(1024));
        collection_schema.AddField(
            FieldSchema(DATABASE_CHUNK_INDEX, DataType::INT32, "chunk index").WithMaxLength(1024));
        collection_schema.AddField(FieldSchema(DATABASE_METADATA, DataType::JSON, "metadata").WithMaxLength(1024));
        collection_schema.AddField(
            FieldSchema(DATABASE_DENSE_VECTOR, DataType::FLOAT_VECTOR, "vectors").WithDimension(DIMENSION));
        VECTOR_INDEX = DATABASE_DENSE_VECTOR;
    } else if (kbType == KBType::FAQ) {
        collection_schema.AddField(
            FieldSchema(DATABASE_KNOWLEDGE_ID, DataType::INT32, "knowledge id", true).WithMaxLength(1024));
        collection_schema.AddField(FieldSchema(DATABASE_QUESTION, DataType::VARCHAR, "question").WithMaxLength(1024));
        collection_schema.AddField(
            FieldSchema(DATABASE_QUESTION_DESCRIPTION, DataType::VARCHAR, "question description").WithMaxLength(1024));
        collection_schema.AddField(FieldSchema(DATABASE_ANSWER, DataType::VARCHAR, "answer").WithMaxLength(1024));
        collection_schema.AddField(
            FieldSchema(DATABASE_QUESTION_DENSE_VECTOR, DataType::FLOAT_VECTOR, "vectors").WithDimension(DIMENSION));
        VECTOR_INDEX = DATABASE_QUESTION_DENSE_VECTOR;
    }
    IndexDesc index_desc(VECTOR_INDEX, "vector-index");
    VectorDB::ConnectParam param(appId);
    std::filesystem::path pathObj(absoluteDbFilePath);
    std::filesystem::path dirPath = pathObj.parent_path();

    if (!dirPath.empty()) {
        std::filesystem::create_directories(dirPath);
    }
    scene::VectorDatabase vectorDatabase(param, absoluteDbFilePath, true, password);
    VectorDB::Status status = vectorDatabase.createCollection(collection_schema, index_desc);
    if (!status.IsOk()) {
        PRINT_ERROR("创建知识库失败: %s errorCode：%d\n", status.Message().c_str(), status.Code());
        errCode = KBErrorCode::KB_CREATE_FAILED;
    }

    return status.IsOk();
}

bool VectorizationEngine::deleteVectorDatabase(const std::string& appId, const std::string& password,
                                               const std::string& dbFilePath, const std::string& collectionName,
                                               KBErrorCode& errCode) {
    errCode = KBErrorCode::SUCCESS;
    std::string absoluteDbFilePath = dbFilePath + "/" + DATABASE_FILE_DEFAULT_NAME;

    if (!hasCollection(appId, password, absoluteDbFilePath, collectionName)) {
        errCode = KBErrorCode::KB_NOT_FOUND;
        return false;
    }

    VectorDB::ConnectParam param(appId);
    scene::VectorDatabase vectorDatabase(param, absoluteDbFilePath, true, password);
    VectorDB::Status status = vectorDatabase.dropCollection(collectionName);
    if (!status.IsOk()) {
        PRINT_ERROR("删除知识库失败: %s errorCode：%d\n", status.Message().c_str(), status.Code());
        errCode = KBErrorCode::KB_CORRUPTED;
    }

    return status.IsOk();
}

bool VectorizationEngine::addVectors(const int& taskId, const std::string& appId, const std::string& password,
                                     const std::string& dbFilePath, const std::string& collectionName,
                                     const std::vector<KnowledgeItem>& knowledgeItems,
                                     std::function<void(float)> callback, KBErrorCode& errCode) {
    auto& timer = PerformanceTimer::getInstance();  // 获取单例
    errCode = KBErrorCode::SUCCESS;
    std::string absoluteDbFilePath = dbFilePath + "/" + DATABASE_FILE_DEFAULT_NAME;
    addTaskId(taskId);

    std::vector<FieldDataPtr> fields_data;
    // 分块 获得向量化结果
    if (knowledgeItems.size() == 0) {
        errCode = KBErrorCode::DOC_UPLOAD_FAILED;
        removeTaskId(taskId);
        return false;
    }
    std::vector<nlohmann::json> sectionTitles;
    std::vector<std::string> content;
    std::vector<std::string> fileId;
    std::vector<int> chunkIndex;
    std::vector<nlohmann::json> metaData;
    std::vector<std::vector<float>> embedding;
    size_t index = 0;
    float totalNum = static_cast<float>(knowledgeItems.size());
    // 在循环开始前预计算所有回调点，避免在循环中重复计算
    std::vector<size_t> callbackPoints;
    callbackPoints.reserve(21);
    for (int i = 1; i <= 20; ++i) {
        size_t point = static_cast<size_t>(totalNum * i / 20.0f);
        if (point == 0) {
            point = 1;
        }
        callbackPoints.push_back(point);
    }
    // 确保最后一个是总数
    callbackPoints.push_back(static_cast<size_t>(totalNum));
    size_t callbackPointIndex = 0;
    FileChunk* firstChunk = static_cast<FileChunk*>(knowledgeItems.at(0).item_.get());
    timer.start("文本向量化: " + firstChunk->fileId_);
    for (const auto& item : knowledgeItems) {
        FileChunk* chunk = static_cast<FileChunk*>(item.item_.get());
        ++index;
        if (callbackPointIndex < callbackPoints.size() && index >= callbackPoints[callbackPointIndex]) {
            float progress = static_cast<float>(index) / totalNum;
            callback(progress);
            ++callbackPointIndex;
        }
        PRINT_INFO("Vector Database chunk index: %d, total size: %f\n", index, totalNum);

        sectionTitles.push_back(nlohmann::json(chunk->sectionTitles_));
        content.push_back(chunk->content_);
        fileId.push_back(chunk->fileId_);
        chunkIndex.push_back(chunk->chunkIndex_);
        metaData.push_back(nlohmann::json(chunk->metaData_));
        if (!hasTaskId(taskId)) {
            removeTaskId(taskId);
            return false;
        }
        if (chunk->embedding_.size() == 0) {
            PRINT_DEBUG("Text Database chunk index: %d, embedding size: %d, we will call embeddingText()\n", index,
                        chunk->embedding_.size());
            embedding.push_back(embeddingText(chunk->content_));
        } else {
            PRINT_DEBUG("Text Database chunk index: %d, embedding size: %d, just save\n", index,
                        chunk->embedding_.size());
            embedding.push_back(chunk->embedding_);
        }
    }
    timer.end("文本向量化: " + firstChunk->fileId_);
    fields_data.push_back(std::make_shared<JsonFieldData>(DATABASE_PARAGRAPH_INDEX, sectionTitles));
    fields_data.push_back(std::make_shared<VarCharFieldData>(DATABASE_CONTENT, content));
    fields_data.push_back(std::make_shared<VarCharFieldData>(DATABASE_FILE_ID, fileId));
    fields_data.push_back(std::make_shared<Int32FieldData>(DATABASE_CHUNK_INDEX, chunkIndex));
    fields_data.push_back(std::make_shared<JsonFieldData>(DATABASE_METADATA, metaData));
    fields_data.push_back(std::make_shared<FloatVecFieldData>(DATABASE_DENSE_VECTOR, embedding));

    DmlResults dml_results;
    std::lock_guard<std::mutex> lock(operationMutex_);
    VectorDB::ConnectParam param(appId);
    scene::VectorDatabase vectorDatabase(param, absoluteDbFilePath, true, password);
    timer.start("向量存储: " + firstChunk->fileId_);
    VectorDB::Status insertStatus = vectorDatabase.insert(collectionName, fields_data, dml_results);
    timer.end("向量存储: " + firstChunk->fileId_);
    if (!insertStatus.IsOk()) {
        PRINT_ERROR("向量存储失败: %s errorCode：%d\n", insertStatus.Message().c_str(), insertStatus.Code());
        errCode = KBErrorCode::VECTOR_DB_DATAUPLOAD_FAILED;
    }
    removeTaskId(taskId);

    return insertStatus.IsOk();
}

bool VectorizationEngine::deleteVectors(const std::string& appId, const std::string& password,
                                        const std::string& dbFilePath, const std::string& vectorDbName,
                                        const std::vector<std::string>& fileIds, KBErrorCode& errCode) {
    auto& timer = PerformanceTimer::getInstance();
    errCode = KBErrorCode::SUCCESS;
    std::string absoluteDbFilePath = dbFilePath + "/" + DATABASE_FILE_DEFAULT_NAME;
    std::string fileList = "[";
    for (size_t i = 0; i < fileIds.size(); ++i) {
        fileList += "\"" + fileIds[i] + "\"";
        if (i != fileIds.size() - 1) {
            fileList += ",";
        }
    }
    fileList += "]";
    std::string expression = std::string(DATABASE_FILE_ID) + " in " + fileList + ";";
    DmlResults dml_results;
    std::lock_guard<std::mutex> lock(operationMutex_);
    VectorDB::ConnectParam param(appId);
    scene::VectorDatabase vectorDatabase(param, absoluteDbFilePath, true, password);
    timer.start("文档删除-向量删除deleteData总耗时" + fileList);
    VectorDB::Status status = vectorDatabase.deleteData(vectorDbName, expression, dml_results);
    timer.end("文档删除-向量删除deleteData总耗时" + fileList);

    if (!status.IsOk()) {
        PRINT_ERROR("向量删除失败: %s errorCode：%d\n", status.Message().c_str(), status.Code());
        errCode = KBErrorCode::DOC_DELETE_FAILED;
        return status.IsOk();
    }
    return true;
}

std::vector<KnowledgeItem> VectorizationEngine::search(const std::string& appId, const std::string& password,
                                                       const std::string& dbFilePath, const std::string& collectionName,
                                                       const std::string& queryText, const RetrievalConfig& config,
                                                       const KBType kbType, KBErrorCode& errCode) {
    auto& timer = PerformanceTimer::getInstance();  // 获取单例
    errCode = KBErrorCode::SUCCESS;
    std::string absoluteDbFilePath = dbFilePath + "/" + DATABASE_FILE_DEFAULT_NAME;
    // 向量化
    timer.start("问题向量化:" + queryText + " " + collectionName);
    std::vector<float> vectors = embeddingText(queryText);
    timer.end("问题向量化:" + queryText + " " + collectionName);
    SearchArguments arguments{};
    arguments.SetCollectionName(collectionName);
    arguments.SetTopK(config.topK);
    if (kbType == KBType::RAG) {
        arguments.AddOutputField(DATABASE_PARAGRAPH_INDEX);
        arguments.AddOutputField(DATABASE_CONTENT);
        arguments.AddOutputField(DATABASE_FILE_ID);
        arguments.AddOutputField(DATABASE_CHUNK_INDEX);
        arguments.AddOutputField(DATABASE_METADATA);
        arguments.AddTargetVector(DATABASE_DENSE_VECTOR, vectors);
    } else if (kbType == KBType::FAQ) {
        arguments.AddOutputField(DATABASE_KNOWLEDGE_ID);
        arguments.AddOutputField(DATABASE_QUESTION);
        arguments.AddOutputField(DATABASE_QUESTION_DESCRIPTION);
        arguments.AddOutputField(DATABASE_ANSWER);
        arguments.AddTargetVector(DATABASE_QUESTION_DENSE_VECTOR, vectors);
    }

    arguments.SetGuaranteeTimestamp(GuaranteeStrongTs());

    SearchResults results{};

    VectorDB::ConnectParam param(appId);
    scene::VectorDatabase vectorDatabase(param, absoluteDbFilePath, true, password);
    timer.start("向量检索:" + queryText + " " + collectionName);
    VectorDB::Status status = vectorDatabase.search(arguments, results);
    timer.end("向量检索:" + queryText + " " + collectionName);
    if (!status.IsOk()) {
        PRINT_ERROR("向量检索失败: %s errorCode：%d\n", status.Message().c_str(), status.Code());
        errCode = KBErrorCode::VECTOR_RETRIEVAL_FAILED;
        return {};
    }

    std::vector<KnowledgeItem> output;

    for (const auto& result : results.Results()) {
        auto& ids = result.Ids().StrIDArray();
        if (ids.empty()) {
            PRINT_ERROR("SearchMetadata result is empty\n");
            continue;
        }

        auto similarity = result.Scores();
        if (ids.size() != similarity.size()) {
            PRINT_ERROR("Illegal search result\n");
            continue;
        }
        if (kbType == KBType::RAG) {
            auto sectionTitles = result.OutputField(DATABASE_PARAGRAPH_INDEX);
            auto content = result.OutputField(DATABASE_CONTENT);
            auto fileId = result.OutputField(DATABASE_FILE_ID);
            auto chunkIndex = result.OutputField(DATABASE_CHUNK_INDEX);
            auto metaData = result.OutputField(DATABASE_METADATA);
            JsonFieldDataPtr sectionTitlesResult_ptr = std::static_pointer_cast<JsonFieldData>(sectionTitles);
            VarCharFieldDataPtr contentResult_ptr = std::static_pointer_cast<VarCharFieldData>(content);
            VarCharFieldDataPtr fileIdResult_ptr = std::static_pointer_cast<VarCharFieldData>(fileId);
            Int32FieldDataPtr chunkIndexResult_ptr = std::static_pointer_cast<Int32FieldData>(chunkIndex);
            JsonFieldDataPtr metaDataResult_ptr = std::static_pointer_cast<JsonFieldData>(metaData);
            auto sectionTitlesResult_data = sectionTitlesResult_ptr->Data();
            auto contentResult_data = contentResult_ptr->Data();
            auto fileIdResult_data = fileIdResult_ptr->Data();
            auto chunkIndexResult_data = chunkIndexResult_ptr->Data();
            auto metaDataResult_data = metaDataResult_ptr->Data();

            for (int i = 0; i < ids.size(); i++) {
                // 根据相似度阈值筛选结果，数据库查询结果是按照相似度倒排的，如果出现相似度小于阈值的，后面的结果都比阈值小
                if (similarity[i] < config.vectorScoreThreshold) {
                    PRINT_INFO("vector search similarity %f less than threshold %f, chunkIndex: %d, fileId: %s\n",
                               similarity[i], config.vectorScoreThreshold, chunkIndexResult_data[i],
                               fileIdResult_data[i].c_str());
                    break;
                }
                PRINT_INFO("Rag vector search similarity %f, chunkIndex: %d, fileId: %s\n", similarity[i],
                           chunkIndexResult_data[i], fileIdResult_data[i].c_str());
                std::shared_ptr<FileChunk> chunkPtr = std::make_shared<FileChunk>();
                chunkPtr->fileId_ = fileIdResult_data[i];
                chunkPtr->content_ = contentResult_data[i];
                chunkPtr->sectionTitles_ = sectionTitlesResult_data[i];
                chunkPtr->chunkIndex_ = chunkIndexResult_data[i];
                chunkPtr->metaData_ = metaDataResult_data[i];
                chunkPtr->similarity_ = similarity[i];
                KnowledgeItem item(chunkPtr, KBType::RAG);
                output.push_back(item);
            }
        } else if (kbType == KBType::FAQ) {
            auto knowledgeId = result.OutputField(DATABASE_KNOWLEDGE_ID);
            auto question = result.OutputField(DATABASE_QUESTION);
            auto description = result.OutputField(DATABASE_QUESTION_DESCRIPTION);
            auto answer = result.OutputField(DATABASE_ANSWER);
            VarCharFieldDataPtr knowledgeId_ptr = std::static_pointer_cast<VarCharFieldData>(knowledgeId);
            VarCharFieldDataPtr question_ptr = std::static_pointer_cast<VarCharFieldData>(question);
            VarCharFieldDataPtr description_ptr = std::static_pointer_cast<VarCharFieldData>(description);
            VarCharFieldDataPtr answer_ptr = std::static_pointer_cast<VarCharFieldData>(answer);
            auto knowledgeId_data = knowledgeId_ptr->Data();
            auto question_data = question_ptr->Data();
            auto description_data = description_ptr->Data();
            auto answer_data = answer_ptr->Data();
            for (int i = 0; i < ids.size(); i++) {
                // 根据相似度阈值筛选结果，数据库查询结果是按照相似度倒排的，如果出现相似度小于阈值的，后面的结果都比阈值小
                if (similarity[i] < config.vectorScoreThreshold) {
                    PRINT_INFO("vector search similarity %f less than threshold %f\n", similarity[i],
                               config.vectorScoreThreshold);
                    break;
                }
                PRINT_INFO("Faq vector search similarity %f, knowledgeId: %s\n", similarity[i],
                           knowledgeId_data[i].c_str());
                std::shared_ptr<FaqItem> faqItemPtr = std::make_shared<FaqItem>();
                faqItemPtr->knowledgeId_ = knowledgeId_data[i];
                faqItemPtr->question_ = question_data[i];
                faqItemPtr->description_ = description_data[i];
                faqItemPtr->content_ = answer_data[i];
                faqItemPtr->similarity_ = similarity[i];
                KnowledgeItem item(faqItemPtr, KBType::FAQ);
                output.push_back(item);
            }
        }
    }

    return output;
}

std::vector<KnowledgeItem> VectorizationEngine::getVectors(const std::string& appId, const std::string& password,
                                                           const std::string& dbFilePath,
                                                           const std::string& collectionName,
                                                           const std::vector<std::string>& fileIds, const KBType kbType,
                                                           KBErrorCode& errCode) {
    if (kbType == KBType::FAQ) {
        PRINT_ERROR("Not supported type : KBType::FAQ\n");
        errCode = KBErrorCode::INVALID_PARAMETER;
        return {};
    }
    auto& timer = PerformanceTimer::getInstance();
    errCode = KBErrorCode::SUCCESS;
    std::string absoluteDbFilePath = dbFilePath + "/" + DATABASE_FILE_DEFAULT_NAME;
    std::string fileList = "[";
    for (size_t i = 0; i < fileIds.size(); ++i) {
        fileList += "\"" + fileIds[i] + "\"";
        if (i != fileIds.size() - 1) {
            fileList += ",";
        }
    }
    fileList += "]";
    std::string expression;
    // 向量化
    std::vector<float> vectors;
    QueryArguments arguments{};
    arguments.SetCollectionName(collectionName);
    expression = std::string(DATABASE_FILE_ID) + " in " + fileList + ";";
    arguments.AddOutputField(DATABASE_PARAGRAPH_INDEX);
    arguments.AddOutputField(DATABASE_CONTENT);
    arguments.AddOutputField(DATABASE_FILE_ID);
    arguments.AddOutputField(DATABASE_CHUNK_INDEX);
    arguments.AddOutputField(DATABASE_METADATA);
    arguments.AddOutputField(DATABASE_DENSE_VECTOR);
    arguments.SetGuaranteeTimestamp(GuaranteeStrongTs());
    arguments.SetExpression(expression);

    QueryResults results{};

    VectorDB::ConnectParam param(appId);
    scene::VectorDatabase vectorDatabase(param, absoluteDbFilePath, true, password);
    timer.start("向量数据库查询:" + collectionName);
    VectorDB::Status status = vectorDatabase.query(arguments, results);
    timer.end("向量数据库查询:" + collectionName);
    if (!status.IsOk()) {
        PRINT_ERROR("vectorDatabase query error : %s errCode: %d\n", status.Message().c_str(), status.Code());
        errCode = KBErrorCode::VECTOR_RETRIEVAL_FAILED;
        return {};
    }

    std::vector<KnowledgeItem> output;

    FieldDataPtr paragraphIndexField = results.GetFieldByName(DATABASE_PARAGRAPH_INDEX);
    FieldDataPtr contentField = results.GetFieldByName(DATABASE_CONTENT);
    FieldDataPtr fileIdField = results.GetFieldByName(DATABASE_FILE_ID);
    FieldDataPtr chunkIndexField = results.GetFieldByName(DATABASE_CHUNK_INDEX);
    FieldDataPtr metadataField = results.GetFieldByName(DATABASE_METADATA);
    FieldDataPtr denseVectorField = results.GetFieldByName(DATABASE_DENSE_VECTOR);
    auto& paragraphIndexData = std::static_pointer_cast<JsonFieldData>(paragraphIndexField)->Data();
    auto& contentData = std::static_pointer_cast<VarCharFieldData>(contentField)->Data();
    auto& fileIdData = std::static_pointer_cast<VarCharFieldData>(fileIdField)->Data();
    auto& chunkIndexData = std::static_pointer_cast<Int32FieldData>(chunkIndexField)->Data();
    auto& metadataData = std::static_pointer_cast<JsonFieldData>(metadataField)->Data();
    auto& denseVectorData = std::static_pointer_cast<FloatVecFieldData>(denseVectorField)->Data();

    for (int i = 0; i < fileIdData.size(); i++) {
        std::shared_ptr<FileChunk> chunkPtr = std::make_shared<FileChunk>();
        chunkPtr->fileId_ = fileIdData[i];
        chunkPtr->content_ = contentData[i];
        chunkPtr->sectionTitles_ = paragraphIndexData[i];
        chunkPtr->chunkIndex_ = chunkIndexData[i];
        chunkPtr->metaData_ = metadataData[i];
        chunkPtr->embedding_ = denseVectorData[i];
        KnowledgeItem item(chunkPtr, KBType::RAG);
        output.push_back(item);
    }

    return output;
}

std::vector<float> VectorizationEngine::embeddingText(const std::string& text) {
    Embedding textEmbedding;
    std::vector<float> res = textEmbedding.getTextEmbedding(text);

    return res;
}

bool VectorizationEngine::checkStatus(std::string&& prefix, const Status& status) {
    if (!status.IsOk()) {
        PRINT_ERROR("%s ,errMsg: %s\n", prefix.c_str(), status.Message().c_str());
        return false;
    }
    return true;
}

bool VectorizationEngine::hasCollection(const std::string& appId, const std::string& password,
                                        const std::string& dbFilePath, const std::string& name) {
    VectorDB::ConnectParam param(appId);
    scene::VectorDatabase vectorDatabase(param, dbFilePath, true, password);
    bool exist = true;
    VectorDB::Status status = vectorDatabase.hasCollection(name, exist);
    if (!exist || !status.IsOk()) {
        PRINT_ERROR("向量数据库探测失败: %s errorCode：%d\n", status.Message().c_str(), status.Code());
        return false;
    }
    return true;
}

void VectorizationEngine::stopVectorsUpload(const std::vector<int>& taskIds, KBErrorCode& errCode) {
    for (auto taskId : taskIds) {
        removeTaskId(taskId);
    }
}

}  // namespace knowledgebase
