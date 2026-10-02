#pragma once

#include <mutex>

#include "../common/common.h"
#include "../common/fileparser.h"
#include "../common/threadpool.h"
#include "traditionaldatabaseengine.h"

namespace knowledgebase {

class StorageManager {
public:
    StorageManager(const std::string appId, const std::string password, const std::string dbFilePath);

    ~StorageManager() = default;

    // 创建知识库
    bool createKnowledgeBase(const std::string& name, const std::string& description, std::string& knowledgeBaseId,
                             KBType type, IndexType indexType, KBErrorCode& errCode);
    // 删除知识库
    bool deleteKnowledgeBase(const std::string& knowledgeBaseId, KBErrorCode& errCode);
    // 批量添加接口（失败时记录失败文件index和原因并返回）
    bool uploadDocumentsBatch(const std::string& knowledgeBaseId,
                              const std::vector<std::pair<std::string, std::string>>& files,  // (filePath, fileId)
                              UploadDocumentProgressCallback callback, const FileProcessorConfig& config,
                              KBErrorCode& errCode);
    // 批量删除文档,需要支持单个文件的失败回滚
    bool deleteDocuments(const std::string& knowledgeBaseId, const std::vector<std::string>& fileIds,
                         std::vector<std::string>& failedFile, KBErrorCode& errCode);

    // 修改知识库名
    bool changeKnowledgeBaseName(const std::string& knowledgeBaseId, const std::string& name, KBErrorCode& errCode);
    // 修改知识库描述
    bool changeKnowledgeBaseDescription(const std::string& knowledgeBaseId, const std::string& description,
                                        KBErrorCode& errCode);
    // 获取知识库信息
    KnowledgeBaseInfo getKnowledgeBase(const std::string& knowledgeBaseId, KBErrorCode& errCode);
    // 获取一个app的所有知识库信息
    std::vector<KnowledgeBaseInfo> getAllKnowledgeBase(KBErrorCode& errCode);

    // 获取一个知识库的所有文件
    std::vector<std::string> getFileListInKnowledgeBase(const std::string& knowledgeBaseId, KBErrorCode& errCode);

    // 获取文件上传进展
    void getFileProcessingProgress(const std::string& knowledgeBaseId, const std::string& fileId, int& progress,
                                   FileUploadStatus& status, KBErrorCode& errCode);

    bool isKnowledgeBaseExisted(const std::string& knowledgeBaseName);

    void stopDocumentsUpload(const std::string& knowledgeBaseId, const std::vector<std::string>& fileIds,
                             KBErrorCode& errCode);

private:
    // 鉴定app是拥有知识库访问权限
    bool hasAccessPermission(const std::string& knowledgeBaseId, KBErrorCode& errCode);

    bool checkUploadPermission(const std::string& knowledgeBaseId, UploadDocumentProgressCallback callback,
                               KBErrorCode& errCode);
    void storageSingleFile(const std::string& knowledgeBaseId, const std::string& filePath, const std::string& fileId,
                           UploadDocumentProgressCallback& callback, const FileProcessorConfig& config);

    void storageProcessedItems(const std::string& knowledgeBaseId, const std::string& filePath,
                               const std::string& fileId, const std::vector<KnowledgeItem>& items,
                               UploadDocumentProgressCallback& callback);
    // 内部：执行存储（batchSize个文件一组进行存储,失败时整组回滚，记录条目编号，返回失败条目编号）
    bool storeWithConsistency(const std::string& knowledgeBaseId, const std::vector<KnowledgeItem>& knowledgeItems,
                              UploadDocumentProgressCallback callback, KBErrorCode& errCode);
    // 更新文件处理进度
    void updateFileProcessingProgress(const std::string& knowledgeBaseId, const std::string& fileId,
                                      const int& progress, const FileUploadStatus& status, const KBErrorCode& errCode);

    // 文件上传是否失败
    bool isFileUploadFailed(const std::string& knowledgeBaseId, const std::string& fileId);

    void removeFailedFile(std::vector<std::string>& fileIds, const std::string& fileId);

    void notifyProgress(const std::string& knowledgeBaseId, const std::string& fileId, int progress,
                        FileUploadStatus status, UploadDocumentProgressCallback& callback,
                        KBErrorCode errCode = KBErrorCode::SUCCESS);

    void addUploadTask(const std::string& fileId);
    void removeUploadTask(const std::string& fileId);
    bool isFileIdInTaskList(const std::string& fileId) const;
    int getTaskIdByFileId(const std::string& fileId) const;

    // 回滚向量数据库操作
    bool rollbackVectorDatabase(const std::string& collectionName, const std::vector<std::string>& fileIds,
                                KBErrorCode& errCode);
    // 回滚文本搜索数据库操作
    bool rollbackTextSearchDatabase(const std::string& textSearchDbPath, const std::vector<std::string>& fileIds,
                                    KBErrorCode& errCode);
    // 恢复向量数据库数据（用于删除操作的回滚）
    bool restoreVectorDatabase(const std::string& collectionName, const std::vector<KnowledgeItem>& items,
                               const int& tempTaskId, KBErrorCode& errCode);
    // 恢复文本搜索数据库数据（用于删除操作的回滚）
    bool restoreTextSearchDatabase(const std::string& textSearchDbPath, const std::vector<KnowledgeItem>& items,
                                   KBErrorCode& errCode);

private:
    // 所有storagemanager的文档上传状态公用一个uploadFileStatus_，避免切换session后无法获取的问题
    static std::vector<FileUploadResponse> uploadFileStatus_;
    std::unique_ptr<TraditionalDatabaseEngine> traditionalDatabaseEngine_;
    mutable std::mutex statusMutex_;
    mutable std::mutex fileIdsMutex_;
    std::unique_ptr<ThreadPool> threadPool_;
    std::string appId_;
    std::string password_;
    std::string dbFilePath_;
    // StorageManager:文档上传任务列表，<fileId, taskId>
    std::map<std::string, int> documentUploadTaskList_;
};

}  // namespace knowledgebase
