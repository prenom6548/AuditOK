#pragma once

#include <kysdk-vector-engine-client/Database.h>
#include <kysdk-vector-engine-client/types/Status.h>

#include <mutex>

#include "../common/common.h"
#include "../common/embedding.h"
#include "../common/utils.h"

namespace knowledgebase {

class VectorizationEngine {
public:
    VectorizationEngine(const VectorizationEngine&) = delete;
    VectorizationEngine& operator=(const VectorizationEngine&) = delete;
    static VectorizationEngine& getInstance() {
        static VectorizationEngine instance;
        return instance;
    }

public:
    // 创建一个知识库的向量存储
    bool createVectorDatabase(const std::string& appId, const std::string& password, const std::string& dbFilePath,
                              const std::string& collectionName, const std::string& description, const KBType kbType,
                              const IndexType indexType, KBErrorCode& errCode);

    // 删除一个知识库的向量存储
    bool deleteVectorDatabase(const std::string& appId, const std::string& password, const std::string& dbFilePath,
                              const std::string& collectionName, KBErrorCode& errCode);

    // 向指定知识库添加一组文本Chunk的向量或faq的Knowledge条目
    bool addVectors(const int& taskId, const std::string& appId, const std::string& password,
                    const std::string& dbFilePath, const std::string& vectorDbName,
                    const std::vector<KnowledgeItem>& knowledgeItems, std::function<void(float)> callback,
                    KBErrorCode& errCode);

    // 从指定知识库删除一组文件的向量或faq的Knowledge条目
    bool deleteVectors(const std::string& appId, const std::string& password, const std::string& dbFilePath,
                       const std::string& collectionName, const std::vector<std::string>& fileIds,
                       KBErrorCode& errCode);

    // 从指定知识库获取一组文件的向量或faq的Knowledge条目
    std::vector<KnowledgeItem> getVectors(const std::string& appId, const std::string& password,
                                          const std::string& dbFilePath, const std::string& collectionName,
                                          const std::vector<std::string>& fileIds, const KBType kbType,
                                          KBErrorCode& errCode);

    // 给定查询向量，返回最相似的 Top-K 文档Chunks对象和相似度
    std::vector<KnowledgeItem> search(const std::string& appId, const std::string& password,
                                      const std::string& dbFilePath, const std::string& collectionName,
                                      const std::string& queryText, const RetrievalConfig& config, const KBType kbType,
                                      KBErrorCode& errCode);

    // 停止指定的向量上传任务
    void stopVectorsUpload(const std::vector<int>& taskIds, KBErrorCode& errCode);

private:
    // 私有构造函数
    VectorizationEngine();
    // 将文本转为向量
    std::vector<float> embeddingText(const std::string& text);
    static bool checkStatus(std::string&& prefix, const VectorDB::Status& status);
    bool hasCollection(const std::string& appId, const std::string& password, const std::string& dbFilePath,
                       const std::string& name);
    bool disconnect();

    void addTaskId(int taskId) { auto result = vectorUploadTaskList_.insert(taskId); }
    void removeTaskId(int taskId) { size_t erased = vectorUploadTaskList_.erase(taskId); }
    bool hasTaskId(int taskId) const { return vectorUploadTaskList_.count(taskId) > 0; }

private:
    std::shared_ptr<VectorDB::Database> client_;
    // VectorizationEngine:向量上传任务列表，<taskId>
    std::set<int> vectorUploadTaskList_;
    mutable std::mutex operationMutex_;
};

}  // namespace knowledgebase
