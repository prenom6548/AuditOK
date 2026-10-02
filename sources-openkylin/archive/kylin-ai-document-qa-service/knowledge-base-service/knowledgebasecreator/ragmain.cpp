#include <rpc/vectorDB/vectorDatabase.h>
#include <unistd.h>

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "../src/common/common.h"
#include "../src/common/embedding.h"
#include "../src/common/fileparser.h"
#include "../src/common/utils.h"
#include "../src/configmanager.h"

using namespace knowledgebase;

const char* DATABASE_APPID = "kylin-ai-knowledge-base-service";
const char* DATABASE_PASSWORD = "k6bf1d_349ecd";

static const char* DATABASE_ID = "ids";
static const char* DATABASE_PARAGRAPH_INDEX = "paragraph_index";
static const char* DATABASE_CONTENT = "content";
static const char* DATABASE_DENSE_VECTOR = "dense_vector";
static const char* DATABASE_FILE_ID = "file_id";
static const char* DATABASE_CHUNK_INDEX = "chunk_index";
static const char* DATABASE_METDATA = "metadata";

static const char* DATABASE_RAG_VECTORDB_PATH = "/vector.db";

static uint32_t DIMENSION = 768;

bool createVectorDatabase(const std::string& appId, const std::string& password, const std::string& name,
                          const std::string& dataBaseDir) {
    const char* VECTOR_INDEX = NULL;
    VectorDB::CollectionSchema collection_schema(name);
    collection_schema.AddField(
        VectorDB::FieldSchema(DATABASE_ID, VectorDB::DataType::VARCHAR, "primary key", true, true).WithMaxLength(1024));
    collection_schema.AddField(
        VectorDB::FieldSchema(DATABASE_PARAGRAPH_INDEX, VectorDB::DataType::JSON, "paragraph index")
            .WithMaxLength(1024));
    collection_schema.AddField(
        VectorDB::FieldSchema(DATABASE_CONTENT, VectorDB::DataType::VARCHAR, "content").WithMaxLength(1024));
    collection_schema.AddField(
        VectorDB::FieldSchema(DATABASE_FILE_ID, VectorDB::DataType::VARCHAR, "file id").WithMaxLength(1024));
    collection_schema.AddField(
        VectorDB::FieldSchema(DATABASE_CHUNK_INDEX, VectorDB::DataType::INT32, "chunk index").WithMaxLength(1024));
    collection_schema.AddField(
        VectorDB::FieldSchema(DATABASE_METDATA, VectorDB::DataType::JSON, "metadata").WithMaxLength(1024));
    collection_schema.AddField(VectorDB::FieldSchema(DATABASE_DENSE_VECTOR, VectorDB::DataType::FLOAT_VECTOR, "vectors")
                                   .WithDimension(DIMENSION));
    VECTOR_INDEX = DATABASE_DENSE_VECTOR;
    VectorDB::IndexDesc index_desc(VECTOR_INDEX, "vector-index");
    VectorDB::ConnectParam param(appId);
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

bool addFilesToVector(const std::vector<std::string>& filepaths, const std::string& appId, const std::string& password,
                      const std::string& name, const std::string& dataBaseDir) {
    // 分块 获得向量化结果

    FileParser parser;
    std::vector<int> maxDocWordsCountList;
    for (const auto& filepath : filepaths) {
        printf("filepath:%s\n", filepath.c_str());
        maxDocWordsCountList.push_back(1000 * 1000);
    }
    std::vector<ChunkResult> chunkResults = parser.chunkFiles(filepaths, maxDocWordsCountList);
    printf("chunkResults.size():%ld\n", chunkResults.size());
    if (chunkResults.empty()) {
        std::cerr << "Failed to add files, chunkResult is empty." << std::endl;
        return false;
    }

    Embedding textEmbedding;
    printf("filepaths.size():%ld\n", filepaths.size());
    for (size_t i = 0; i < filepaths.size(); i++) {
        std::vector<KnowledgeItem> knowledgeItems;
        const std::string& fileId = filepaths[i];

        KnowledgeItem knowledgeItem;
        knowledgeItem.type_ = KBType::RAG;

        int chunkIndex = 0;
        const std::vector<std::string>& chunks = chunkResults[i].chunk;
        printf("---------------TextEmbedding---------------filepaths[%ld/%ld]:%s\n", i, filepaths.size(),
               filepaths[i].c_str());
        printf("chunks.size():%ld\n", chunks.size());
        if (chunks.size() < 1) {
            continue;
        }
        for (const auto& chunk : chunks) {
            try {
                // 解析为 JSON 对象
                nlohmann::json j = nlohmann::json::parse(chunk);
                nlohmann::json metadata;
                metadata["filePath"] = fileId;

                std::shared_ptr<FileChunk> chunkPtr = std::make_shared<FileChunk>();
                chunkPtr->knowledgeBaseId_ = "";
                chunkPtr->fileId_ = fileId;
                chunkPtr->chunkIndex_ = chunkIndex++;               // 从0自增
                chunkPtr->sectionTitles_ = j["title_path"].dump();  // 将 vector 转换为 JSON 字符串
                chunkPtr->content_ = j["content"];
                chunkPtr->embedding_ = textEmbedding.getTextEmbedding(chunkPtr->content_);
                chunkPtr->metaData_ = metadata.dump();  // 将 metadata 对象转换为 JSON 字符串
                chunkPtr->similarity_ = 0.0f;           // 默认值为 0.0f，查询时使用

                KnowledgeItem item(chunkPtr, KBType::RAG);
                knowledgeItems.push_back(item);
            } catch (const nlohmann::json::exception& e) {
                printf("JSON error: %s\n", e.what());
            }
        }
        std::vector<VectorDB::FieldDataPtr> fields_data;
        std::vector<nlohmann::json> sectionTitles;
        std::vector<std::string> content;
        std::vector<std::string> fileIds;
        std::vector<int> chunkIndexs;
        std::vector<nlohmann::json> metaData;
        std::vector<std::vector<float>> embedding;
        for (auto& item : knowledgeItems) {
            FileChunk* chunk = static_cast<FileChunk*>(item.item_.get());
            sectionTitles.push_back(nlohmann::json(chunk->sectionTitles_));
            content.push_back(chunk->content_);
            fileIds.push_back(chunk->fileId_);
            chunkIndexs.push_back(chunk->chunkIndex_);
            metaData.push_back(nlohmann::json(chunk->metaData_));
            embedding.push_back(chunk->embedding_);
            printf("item.fileId_:%s\n", chunk->fileId_.c_str());
        }
        fields_data.push_back(std::make_shared<VectorDB::JsonFieldData>(DATABASE_PARAGRAPH_INDEX, sectionTitles));
        fields_data.push_back(std::make_shared<VectorDB::VarCharFieldData>(DATABASE_CONTENT, content));
        fields_data.push_back(std::make_shared<VectorDB::VarCharFieldData>(DATABASE_FILE_ID, fileIds));
        fields_data.push_back(std::make_shared<VectorDB::Int32FieldData>(DATABASE_CHUNK_INDEX, chunkIndexs));
        fields_data.push_back(std::make_shared<VectorDB::JsonFieldData>(DATABASE_METDATA, metaData));
        fields_data.push_back(std::make_shared<VectorDB::FloatVecFieldData>(DATABASE_DENSE_VECTOR, embedding));
        VectorDB::DmlResults dml_results;
        VectorDB::ConnectParam param(appId);
        std::string absoluteDbFilePath = dataBaseDir;
        std::filesystem::path pathObj(absoluteDbFilePath);
        scene::VectorDatabase vectorDatabase(param, absoluteDbFilePath, true, password);
        VectorDB::Status insertStatus = vectorDatabase.insert(name, fields_data, dml_results);
        printf("---------------inserting---------------\n");
        if (!insertStatus.IsOk()) {
            printf("Failed to insert data into vector db.");
            return false;
        }
    }

    return true;
}

std::vector<std::string> collectAllFiles(const std::string& path) {
    std::vector<std::string> files;

    // 检查路径是否存在
    if (!std::filesystem::exists(path)) {
        std::cerr << "Path does not exist." << std::endl;
        return files;
    }

    // 根据路径类型执行不同逻辑
    if (std::filesystem::is_regular_file(path)) {  // 输入是文件
        std::cout << "input is filepath." << std::endl;
        files.push_back(path);
    } else if (std::filesystem::is_directory(path)) {  // 输入是目录
        // 递归遍历目录下的所有文件
        std::cout << "input is directory." << std::endl;
        std::cout << "start to collect all filepahts." << std::endl;
        for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
            if (entry.is_regular_file()) {
                files.push_back(entry.path().string());
            }
        }
    }
    std::cout << "collect all filepahts sucess." << std::endl;
    return files;
}

void addFiles(const std::string& filepath, const std::string& appId, const std::string& password,
              const std::string& name, const std::string& dataBaseDir) {
    std::vector<std::string> input = collectAllFiles(filepath);
    if (input.empty()) {
        std::cerr << "Error: no file found in the directory." << std::endl;
        return;
    }
    std::cout << "addFiles start." << std::endl;
    bool ret = addFilesToVector(input, appId, password, name, dataBaseDir);
    if (!ret) {
        std::cerr << "addFiles failed." << std::endl;
    }
    std::cout << "addFiles sucess." << std::endl;
}

int main(int argc, char* argv[]) {
    std::string dbName = "kakbs_user_manual";
    std::string filePath = "/usr/share/kylin-user-guide/data/guide/";
    std::string fileKaimingPath = "/opt/system/resource/kylin/kylin-user-guide/data/guide/";
    Configuration::parseConfFile("/usr/share/kylin-ai/kyai-business-framework/", "knowledgebaseservice.json");
    std::string home = getHomeDirectory();
    std::string dataBaseDir = home + DATABASE_RAG_VECTORDB_PATH;

    bool succ = deleteVectorDatabase(DATABASE_APPID, DATABASE_PASSWORD, dbName, dataBaseDir);
    printf("deleteVectorDatabase: %d\n", succ);
    succ = createVectorDatabase(DATABASE_APPID, DATABASE_PASSWORD, dbName, dataBaseDir);
    printf("createVectorDatabase: %d\n", succ);

    addFiles(filePath, DATABASE_APPID, DATABASE_PASSWORD, dbName, dataBaseDir);
    addFiles(fileKaimingPath, DATABASE_APPID, DATABASE_PASSWORD, dbName, dataBaseDir);

    return 0;
}