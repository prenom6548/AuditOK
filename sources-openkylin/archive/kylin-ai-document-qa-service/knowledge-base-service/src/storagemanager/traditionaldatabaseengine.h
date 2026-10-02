#pragma once
#include <SQLiteCpp/Database.h>

#include "../common/common.h"
#include "../common/utils.h"

namespace knowledgebase {

class TraditionalDatabaseEngine {
public:
    // 数据库文件由用户自行管理，考虑到单实例每次对各个用户的知识库进行打开关闭操作过于频繁开销较大，此处采用多实例长期持有各个用户的db文件
    TraditionalDatabaseEngine(const std::string dbFilePath);

    ~TraditionalDatabaseEngine() = default;

    // 创建知识库
    bool createKnowledgeBase(const std::string& appId, const std::string& name, const std::string& description,
                             const std::string& collectionName, const std::string& textSearchDbName,
                             std::string& knowledgeBaseId, KBType type, IndexType indexType, KBErrorCode& errCode);

    // 删除知识库
    bool deleteKnowledgeBase(const std::string& knowledgeBaseId, KBErrorCode& errCode);

    // 修改知识库名
    bool changeKnowledgeBaseName(const std::string& knowledgeBaseId, const std::string& name, KBErrorCode& errCode);

    // 修改知识库描述
    bool changeKnowledgeBaseDescription(const std::string& knowledgeBaseId, const std::string& description,
                                        KBErrorCode& errCode);

    // 获取知识库信息
    KnowledgeBaseInfo getKnowledgeBase(const std::string& knowledgeBaseId, KBErrorCode& errCode);

    // 获取一个app的所有知识库信息
    std::vector<KnowledgeBaseInfo> getAllKnowledgeBase(const std::string appId, KBErrorCode& errCode);

    // 获取一个知识库的所有文件
    std::vector<std::string> getFileListInKnowledgeBase(const std::string& knowledgeBaseId, KBErrorCode& errCode);

    // 批量添加文档（失败时回滚并返回）fileId, filePath
    bool addDocumentsBatch(const std::string& knowledgeBaseId,
                           const std::unordered_map<std::string, std::string>& files, KBErrorCode& errCode);

    // 批量删除文档,失败时全部回滚
    bool deleteDocuments(const std::string& knowledgeBaseId, const std::vector<std::string>& fileIds,
                         KBErrorCode& errCode);

    // 获取知识库的数据库名称
    bool getKnowledgeBaseDatabaseName(const std::string& knowledgeBaseId, std::string& collectionName,
                                      std::string& textSearchDbName, KBErrorCode& errCode);

    // 鉴定app是拥有知识库访问权限
    bool hasAccessPermission(const std::string& appId, const std::string& knowledgeBaseId, KBErrorCode& errCode);

    // 鉴定knowledgeBaseId是哪一种知识库：RAG或者FAQ
    KBType knowledgeBaseType(const std::string& appId, const std::string& knowledgeBaseId, KBErrorCode& errCode);

public:
    bool isKnowledgeBaseExisted(const std::string& appId, const std::string& knowledgeBaseName);

    bool hasFileInKnowledgeBase(const std::string& knowledgeBaseId, const std::string& fileId);

private:
    bool initTraditionalDatabase(std::unique_ptr<SQLite::Database>& k_db, KBErrorCode& errCode);
    std::string dbFilePath_;
    std::unique_ptr<SQLite::Database> k_db;
};

}  // namespace knowledgebase