#pragma once
#include <atomic>
#include <mutex>
#include <set>
#include <thread>
#include <unordered_map>

#include "common/common.h"
#include "hybridretriever/hybridretriever.h"
#include "inferenceengine.h"
#include "storagemanager/storagemanager.h"

namespace knowledgebase {

class KnowledgeBaseService {
public:
    KnowledgeBaseService(const KnowledgeBaseService&) = delete;
    KnowledgeBaseService& operator=(const KnowledgeBaseService&) = delete;
    static KnowledgeBaseService& getInstance() {
        static KnowledgeBaseService instance;
        return instance;
    }

private:
    KnowledgeBaseService();
    ~KnowledgeBaseService() = default;

public:
    /**
     * @brief 初始化知识库会话
     * @param appId 应用唯一ID
     * @param password 数据库密码
     * @param dbFilePath 数据库文件路径
     * @param errCode 输出错误码
     * @return 创建的会话ID
     */
    int initKnowledgeBaseSession(const std::string& appId, const std::string& password, const std::string& dbFilePath,
                                 KBErrorCode& errCode);

    /**
     * @brief 销毁知识库会话
     * @param sessionId 会话ID
     * @param errCode 输出错误码
     */
    void destroyKnowledgeBaseSession(int sessionId, KBErrorCode& errCode);

    /**
     * @brief 创建知识库
     * @param sessionId 会话ID
     * @param name 知识库名称
     * @param description 知识库描述
     * @param type 知识库类型（DOCUMENT或FAQ）
     * @param errCode 输出错误码
     * @return 返回创建的知识库信息对象
     */
    KnowledgeBaseInfo createKnowledgeBase(const int sessionId, const std::string& name, const std::string& description,
                                          KBType type, IndexType indexType, KBErrorCode& errCode);

    /**
     * @brief 删除知识库
     * @param sessionId 会话ID
     * @param knowledgeBaseId 要删除的知识库唯一ID
     * @param errCode 输出错误码
     */
    void deleteKnowledgeBase(const int sessionId, const std::string& knowledgeBaseId, KBErrorCode& errCode);
    /**
     * @brief 获取指定知识库的信息
     * @param sessionId 会话ID
     * @param knowledgeBaseId 知识库唯一ID
     * @param errCode 输出错误码
     * @return 知识库信息对象，如果不存在则错误码指示
     */
    KnowledgeBaseInfo getKnowledgeBase(const int sessionId, const std::string& knowledgeBaseId, KBErrorCode& errCode);

    /**
     * @brief 获取应用下的所有知识库信息
     * @param sessionId 会话ID
     * @param errCode 输出错误码
     * @return 知识库信息列表
     */
    std::vector<KnowledgeBaseInfo> getAllKnowledgeBase(const int sessionId, KBErrorCode& errCode);

    /**
     * @brief 修改知识库名称
     * @param sessionId 会话ID
     * @param knowledgeBaseId 知识库唯一ID
     * @param name 新的知识库名称
     * @param errCode 输出错误码
     * @return 知识库信息对象，如果不存在则错误码指示
     */
    void setKnowledgeBaseName(const int sessionId, const std::string& knowledgeBaseId, const std::string& name,
                              KBErrorCode& errCode);

    /**
     * @brief 修改知识库描述
     * @param sessionId 会话ID
     * @param knowledgeBaseId 知识库唯一ID
     * @param description 新的知识库描述
     * @param errCode 输出错误码
     * @return 知识库信息对象，如果不存在则错误码指示
     */
    void setKnowledgeBaseDescription(const int sessionId, const std::string& knowledgeBaseId,
                                     const std::string& description, KBErrorCode& errCode);

    /**
     * @brief 异步上传单个文档到知识库
     * @param sessionId 会话ID
     * @param knowledgeBaseId 知识库ID
     * @param filePath 本地文件的真实路径
     * @param callback 上传进度回调函数
     * @param config 文档处理配置（文件处理、嵌入、索引等参数）
     * @param errCode 输出错误码
     * @return 知识库文件信息对象
     */
    void uploadDocument(const int sessionId, const std::string& knowledgeBaseId, const std::string& filePath,
                        const std::string& fileId, UploadDocumentProgressCallback callback,
                        const FileProcessorConfig& config,  // 用户需要传吗
                        KBErrorCode& errCode);

    /**
     * @brief 异步上传多个文档到知识库
     * @param sessionId 会话ID
     * @param knowledgeBaseId 知识库ID
     * @param files (filePath, fileId)本地文件的真实路径和文件ID
     * @param callback 上传进度回调函数
     * @param config 文档处理配置（文件处理、嵌入、索引等参数）
     * @param errCode 输出错误码
     * @return 知识库文件信息对象
     */
    void uploadDocuments(const int sessionId, const std::string& knowledgeBaseId,
                         const std::vector<std::pair<std::string, std::string>>& files,
                         UploadDocumentProgressCallback callback, const FileProcessorConfig& config,
                         KBErrorCode& errCode);

    /**
     * @brief 获取知识库中的所有文件列表
     * @param sessionId 会话ID
     * @param knowledgeBaseId 知识库ID
     * @param errCode 输出错误码
     * @return 知识库内的文件ID列表
     */
    std::vector<std::string> getFileListInKnowledgeBase(const int sessionId, const std::string& knowledgeBaseId,
                                                        KBErrorCode& errCode);

    /**
     * @brief 删除知识库中的文件
     * @param sessionId 会话ID
     * @param knowledgeBaseId 知识库ID
     * @param fileIds 要删除的文件ID列表
     * @param errCode 输出错误码
     */
    void deleteFiles(const int sessionId, const std::string& knowledgeBaseId, const std::vector<std::string>& fileIds,
                     KBErrorCode& errCode);

    /**
     * @brief 获取文件的处理进度（如上传/嵌入进度）
     * @param sessionId 会话ID
     * @param knowledgeBaseId 知识库ID
     * @param fileId 文件ID
     * @param progress 处理进度
     * @param status 上传状态
     * @param errCode 输出错误码
     */
    void getFileProcessingProgress(const int sessionId, const std::string& knowledgeBaseId, const std::string& fileId,
                                   int& progress, FileUploadStatus& status, KBErrorCode& errCode);

    /**
     * @brief 在知识库中检索与查询相关的内容
     * @param sessionId 会话ID
     * @param knowledgeBaseId 知识库ID
     * @param requestId 请求ID（用于追踪）
     * @param query 查询文本
     * @param config 检索配置（如TopK、作用域等）
     * @param callback 检索结果回调
     * @param errCode 输出错误码
     */
    void retrievalKnowledgeBase(const int sessionId, const std::string& knowledgeBaseId, const std::string& requestId,
                                const std::string& query, const RetrievalConfig& config, RetrievalCallback callback,
                                KBErrorCode& errCode);
    /**
     * @brief 对指定知识库进行问答对话
     * @param sessionId 会话ID
     * @param knowledgeBaseId 知识库ID
     * @param requestId 请求ID
     * @param query 用户的问题
     * @param config 聊天配置（包含chatId、模型名、是否创建新聊天、是否保存历史等）
     * @param callback 流式聊天回调，用于接收逐步生成的回答
     * @param errCode 输出错误码
     * @return 完整的回答结果字符串
     */
    void chatWithKnowledgeBase(const int sessionId, const std::string& knowledgeBaseId, const std::string& requestId,
                               const std::string& query, const std::string& chatHistory, const ChatConfig& config,
                               StreamingChatCallback callback, KBErrorCode& errCode);
    /**
     * @brief 终止指定对话
     * @param sessionId 会话ID
     * @param requestId 请求ID
     * @param errCode 输出错误码
     * @return 无
     */
    void stopChat(int sessionId, const std::string& requestId, KBErrorCode& errCode);

    /**
     * @brief 终止文档上传
     * @param sessionId 会话ID
     * @param fileId 请求ID
     * @param errCode 输出错误码
     * @return 无
     */
    void stopDocumentsUpload(int sessionId, const std::string& knowledgeBaseId, const std::vector<std::string>& fileIds,
                             KBErrorCode& errCode);

private:
    int generateSessionId();
    bool isSessionIdValid(int sessionId);

private:
    std::set<int> sessionIds_;
    std::unordered_map<std::string, std::thread> taskThreads_;
    std::unordered_map<int, std::unordered_map<std::string, std::atomic<bool>>> stopFlags_;
    std::unordered_map<int, std::shared_ptr<HybridRetriever>> hybridRetrievers_;
    std::unordered_map<int, std::shared_ptr<InferenceEngine>> inferenceEngines_;
    std::unordered_map<int, std::shared_ptr<StorageManager>> StorageManager_;
    // 由于nlp sdk通过session管理，同时给nlp sdk设置不同的call back会发生输出混乱的问题，故增加session的锁
    std::unordered_map<int, std::shared_ptr<std::mutex>> taskMutex_;
};

}  // namespace knowledgebase
