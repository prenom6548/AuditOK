#include "knowledgebaseservice.h"

#include <Python.h>
#include <kyai-business-framework/log.h>

#include "common/errno.h"
#include "common/performancetimer.h"
#include "storagemanager/storagemanager.h"

namespace knowledgebase {
using StreamingChatCallback =
    std::function<void(StreamingChatResponse& response, KBErrorCode& errCode, std::string& errMsg)>;

using RetrievalCallback = std::function<void(RetrievalResult& response, KBErrorCode& errCode, std::string& errMsg)>;

KnowledgeBaseService::KnowledgeBaseService() {
    if (!Py_IsInitialized()) {
        Py_Initialize();
    }
    PyEval_SaveThread();  // 相当于释放 GIL，并保存线程状态
}

int KnowledgeBaseService::initKnowledgeBaseSession(const std::string& appId, const std::string& password,
                                                   const std::string& dbFilePath, KBErrorCode& errCode) {
    int sessionId = generateSessionId();
    sessionIds_.insert(sessionId);
    inferenceEngines_[sessionId] = std::make_shared<InferenceEngine>();
    hybridRetrievers_[sessionId] = std::make_shared<HybridRetriever>(appId, password, dbFilePath);
    StorageManager_[sessionId] = std::make_shared<StorageManager>(appId, password, dbFilePath);
    taskMutex_[sessionId] = std::make_shared<std::mutex>();
    errCode = KBErrorCode::SUCCESS;
    return sessionId;
}

void KnowledgeBaseService::destroyKnowledgeBaseSession(int sessionId, KBErrorCode& errCode) {
    if (sessionIds_.find(sessionId) == sessionIds_.end()) {
        errCode = KBErrorCode::SUCCESS;
        return;
    }

    sessionIds_.erase(sessionId);
    stopFlags_.erase(sessionId);
    hybridRetrievers_.erase(sessionId);
    inferenceEngines_.erase(sessionId);
    StorageManager_.erase(sessionId);
    taskMutex_.erase(sessionId);
    errCode = KBErrorCode::SUCCESS;
}

KnowledgeBaseInfo KnowledgeBaseService::createKnowledgeBase(const int sessionId, const std::string& name,
                                                            const std::string& description, KBType type,
                                                            IndexType indexType, KBErrorCode& errCode) {
    std::shared_ptr<StorageManager> storageManager = StorageManager_[sessionId];
    bool isKnowledgeBaseExisted = storageManager->isKnowledgeBaseExisted(name);
    if (isKnowledgeBaseExisted) {
        errCode = KBErrorCode::KB_ALREADY_EXISTS;
        return KnowledgeBaseInfo();
    }

    std::string knowledgeBaseId;
    bool ret = storageManager->createKnowledgeBase(name, description, knowledgeBaseId, type, indexType, errCode);
    if (!ret) {
        return KnowledgeBaseInfo();
    }

    KnowledgeBaseInfo knowledgeBaseInfo = storageManager->getKnowledgeBase(knowledgeBaseId, errCode);
    return knowledgeBaseInfo;
}

void KnowledgeBaseService::deleteKnowledgeBase(const int sessionId, const std::string& knowledgeBaseId,
                                               KBErrorCode& errCode) {
    std::shared_ptr<StorageManager> storageManager = StorageManager_[sessionId];
    storageManager->deleteKnowledgeBase(knowledgeBaseId, errCode);
}

KnowledgeBaseInfo KnowledgeBaseService::getKnowledgeBase(const int sessionId, const std::string& knowledgeBaseId,
                                                         KBErrorCode& errCode) {
    std::shared_ptr<StorageManager> storageManager = StorageManager_[sessionId];
    KnowledgeBaseInfo knowledgeBaseInfo = storageManager->getKnowledgeBase(knowledgeBaseId, errCode);

    return knowledgeBaseInfo;
}

std::vector<KnowledgeBaseInfo> KnowledgeBaseService::getAllKnowledgeBase(const int sessionId, KBErrorCode& errCode) {
    std::shared_ptr<StorageManager> storageManager = StorageManager_[sessionId];
    std::vector<KnowledgeBaseInfo> knowledgeBaseInfoList = storageManager->getAllKnowledgeBase(errCode);

    return knowledgeBaseInfoList;
}

void KnowledgeBaseService::setKnowledgeBaseName(const int sessionId, const std::string& knowledgeBaseId,
                                                const std::string& name, KBErrorCode& errCode) {
    std::shared_ptr<StorageManager> storageManager = StorageManager_[sessionId];
    bool isKnowledgeBaseExisted = storageManager->isKnowledgeBaseExisted(name);
    if (isKnowledgeBaseExisted) {
        errCode = KBErrorCode::KB_ALREADY_EXISTS;
        return;
    }
    storageManager->changeKnowledgeBaseName(knowledgeBaseId, name, errCode);

    return;
}

void KnowledgeBaseService::setKnowledgeBaseDescription(const int sessionId, const std::string& knowledgeBaseId,
                                                       const std::string& description, KBErrorCode& errCode) {
    std::shared_ptr<StorageManager> storageManager = StorageManager_[sessionId];
    storageManager->changeKnowledgeBaseDescription(knowledgeBaseId, description, errCode);

    return;
}

void KnowledgeBaseService::uploadDocument(const int sessionId, const std::string& knowledgeBaseId,
                                          const std::string& filePath, const std::string& fileId,
                                          UploadDocumentProgressCallback callback,
                                          const FileProcessorConfig& config,  // 用户需要传吗
                                          KBErrorCode& errCode) {
    std::shared_ptr<StorageManager> storageManager = StorageManager_[sessionId];
    storageManager->uploadDocumentsBatch(knowledgeBaseId, {{filePath, fileId}}, callback, config, errCode);

    return;
}

void KnowledgeBaseService::uploadDocuments(const int sessionId, const std::string& knowledgeBaseId,
                                           const std::vector<std::pair<std::string, std::string>>& files,
                                           UploadDocumentProgressCallback callback, const FileProcessorConfig& config,
                                           KBErrorCode& errCode) {
    auto& timer = PerformanceTimer::getInstance();  // 获取单例
    timer.start("文档上传总耗时");
    std::shared_ptr<StorageManager> storageManager = StorageManager_[sessionId];
    storageManager->uploadDocumentsBatch(knowledgeBaseId, files, callback, config, errCode);
    timer.end("文档上传总耗时");
    timer.printReport();
    timer.clear();

    return;
}

std::vector<std::string> KnowledgeBaseService::getFileListInKnowledgeBase(const int sessionId,
                                                                          const std::string& knowledgeBaseId,
                                                                          KBErrorCode& errCode) {
    std::shared_ptr<StorageManager> storageManager = StorageManager_[sessionId];
    std::vector<std::string> fileList = storageManager->getFileListInKnowledgeBase(knowledgeBaseId, errCode);

    return fileList;
}

void KnowledgeBaseService::deleteFiles(const int sessionId, const std::string& knowledgeBaseId,
                                       const std::vector<std::string>& fileIds, KBErrorCode& errCode) {
    auto& timer = PerformanceTimer::getInstance();
    timer.start("文档删除总耗时");
    std::shared_ptr<StorageManager> storageManager = StorageManager_[sessionId];
    std::vector<std::string> failedFile;
    storageManager->deleteDocuments(knowledgeBaseId, fileIds, failedFile, errCode);
    timer.end("文档删除总耗时");
    timer.printReport();
    timer.clear();

    return;
}

void KnowledgeBaseService::getFileProcessingProgress(const int sessionId, const std::string& knowledgeBappseId,
                                                     const std::string& fileId, int& progress, FileUploadStatus& status,
                                                     KBErrorCode& errCode) {
    std::shared_ptr<StorageManager> storageManager = StorageManager_[sessionId];
    storageManager->getFileProcessingProgress(knowledgeBappseId, fileId, progress, status, errCode);

    return;
}

void KnowledgeBaseService::retrievalKnowledgeBase(const int sessionId, const std::string& knowledgeBaseId,
                                                  const std::string& requestId, const std::string& query,
                                                  const RetrievalConfig& config, RetrievalCallback callback,
                                                  KBErrorCode& errCode) {
    std::lock_guard<std::mutex> lock(*taskMutex_[sessionId]);
    stopFlags_[sessionId][requestId].store(0, std::memory_order_release);  // 设置为 false
    std::string errMsg = "";

    if (isSessionIdValid(sessionId) == false) {
        PRINT_ERROR("Invalid session id\n");
        errCode = KBErrorCode::INVALID_SESSION_ID;
        return;
    }

    std::shared_ptr<HybridRetriever> hybridRetriever = hybridRetrievers_[sessionId];
    RetrievalResult retrievalResult;
    // 可插入中断点
    if (stopFlags_[sessionId][requestId].load(std::memory_order_acquire)) {
        return;
    }
    retrievalResult = hybridRetriever->retrieveAndRerank(knowledgeBaseId, query, config, errCode, errMsg);
    // 可插入中断点
    if (stopFlags_[sessionId][requestId].load(std::memory_order_acquire)) {
        return;
    }
    callback(retrievalResult, errCode, errMsg);

    return;
}

void KnowledgeBaseService::chatWithKnowledgeBase(const int sessionId, const std::string& knowledgeBaseId,
                                                 const std::string& requestId, const std::string& query,
                                                 const std::string& chatHistory, const ChatConfig& config,
                                                 StreamingChatCallback callback, KBErrorCode& errCode) {
    auto& timer = PerformanceTimer::getInstance();  // 获取单例
    timer.start("文档问答总耗时");
    std::lock_guard<std::mutex> lock(*taskMutex_[sessionId]);
    std::string chatId = "";  // TODO
    std::string errMsg = "";

    if (isSessionIdValid(sessionId) == false) {
        PRINT_ERROR("Invalid session id\n");
        errCode = KBErrorCode::INVALID_SESSION_ID;
        timer.end("文档问答总耗时");
        timer.printReport();
        timer.clear();
        return;
    }

    std::shared_ptr<HybridRetriever> hybridRetriever = hybridRetrievers_[sessionId];
    std::shared_ptr<InferenceEngine> inferenceEngine = inferenceEngines_[sessionId];

    if (stopFlags_[sessionId][requestId].load(std::memory_order_acquire)) {
        errCode = KBErrorCode::SUCCESS;
        StreamingChatResponse response{chatId, requestId, config.modelName, true};
        callback(response, errCode, errMsg);
        timer.end("文档问答总耗时");
        timer.printReport();
        timer.clear();
        return;
    }
    RetrievalResult retrievalResult =
        hybridRetriever->retrieveAndRerank(knowledgeBaseId, query, config.retrievalConfig, errCode, errMsg);

    StreamingChatCallback callbackWrapper = [=](StreamingChatResponse& response, KBErrorCode& errCode,
                                                std::string& errMsg) {
        response.chatId = chatId;
        response.requestId = requestId;
        if (response.isFinal) {
            stopFlags_[sessionId].erase(requestId);
        }

        callback(response, errCode, errMsg);
    };
    // 问题扩展结束后优先检查是否被终止查询，是则忽略扩展和检索异常，优先返回用户终止指令的响应
    if (stopFlags_[sessionId][requestId].load(std::memory_order_acquire)) {
        stopFlags_[sessionId][requestId].store(false, std::memory_order_release);
        StreamingChatResponse response;
        response.isFinal = true;
        errCode = KBErrorCode::SUCCESS;
        callbackWrapper(response, errCode, errMsg);
        timer.end("文档问答总耗时");
        timer.printReport();
        timer.clear();
        return;
    }
    // 查询未被终止的情况下再判断检扩展和检索是否存在异常
    if (errCode != KBErrorCode::SUCCESS) {
        PRINT_ERROR("问题扩展或检索异常：errCode:%d\n", errCode);
        StreamingChatResponse response{chatId, requestId, config.modelName, true};
        callback(response, errCode, errMsg);
        timer.end("文档问答总耗时");
        timer.printReport();
        timer.clear();
        return;
    }
    PRINT_INFO("开始总结对话\n");
    timer.start("总结对话");
    inferenceEngine->generateAnswerStreaming(query, retrievalResult, chatHistory, config, callbackWrapper);
    timer.end("总结对话");
    timer.end("文档问答总耗时");
    timer.printReport();
    timer.clear();

    return;
}

void KnowledgeBaseService::stopChat(int sessionId, const std::string& requestId, KBErrorCode& errCode) {
    if (isSessionIdValid(sessionId) == false) {
        PRINT_ERROR("Invalid session id\n");
        errCode = KBErrorCode::INVALID_SESSION_ID;
        return;
    }

    if (stopFlags_[sessionId].count(requestId)) {
        // 设置为 true（停止标志）
        stopFlags_[sessionId][requestId].store(true, std::memory_order_release);
        // 注意：线程一般不能强行停止，只能通知它退出
        if (isSessionIdValid(sessionId)) {
            hybridRetrievers_[sessionId]->stopRetrieve();
            inferenceEngines_[sessionId]->stopChat();
        }
    } else {
        // 对话请求还未处理就收到停止请求
        stopFlags_[sessionId][requestId].store(true, std::memory_order_release);
    }

    return;
}

void KnowledgeBaseService::stopDocumentsUpload(int sessionId, const std::string& knowledgeBaseId,
                                               const std::vector<std::string>& fileIds, KBErrorCode& errCode) {
    if (isSessionIdValid(sessionId) == false) {
        PRINT_ERROR("Invalid session id\n");
        errCode = KBErrorCode::INVALID_SESSION_ID;
        return;
    }

    std::shared_ptr<StorageManager> storageManager = StorageManager_[sessionId];

    storageManager->stopDocumentsUpload(knowledgeBaseId, fileIds, errCode);
    return;
}

int KnowledgeBaseService::generateSessionId() {
    int sessionId = -1;
    do {
        sessionId = std::rand();
    } while (sessionIds_.find(sessionId) != sessionIds_.end());

    return sessionId;
}

bool KnowledgeBaseService::isSessionIdValid(int sessionId) { return sessionIds_.find(sessionId) != sessionIds_.end(); }

}  // namespace knowledgebase
