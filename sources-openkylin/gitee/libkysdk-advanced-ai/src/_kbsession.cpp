#include "_kbsession.h"

#include "_error.h"
#include "session.h"

KBSession ::KBSession(const char *appName, const char *password,
                      const char *databasePath)
    : appId_(appName), password_(password), databasePath_(databasePath) {
    serviceProxy_.setReconnectedCallback([this] {
        printf("Reconnected callback\n");
        initSession();
    });
}

int KBSession::init() {
    if (!initServer()) {
        return AI_COMMON_SERVER_CONNECTION_FAILED;
    }
    return initSession();
}

bool KBSession::initServer() {
    if (!serviceProxy_.connectToServer()) {
        return serviceProxy_.reconnectServer();
    }
    return true;
}

int KBSession::initSession() {
    return serviceProxy_.init(appId_, password_, databasePath_, &sessionId_);
}

int KBSession::destroy() { return serviceProxy_.destroy(sessionId_); }

char *KBSession::createKnowledgeBase(const KnowledgeBaseInfo *knowledgeBaseInfo,
                                     int *errorCode) {
    if (knowledgeBaseInfo == nullptr) {
        setErrorCode(errorCode, AI_COMMON_INVALID_PARAMETER);
        return nullptr;
    }
    return serviceProxy_.createKnowledgeBase(sessionId_, appId_, password_,
                                             knowledgeBaseInfo, errorCode);
}

int KBSession::deleteKnowledgeBase(const char *knowledgeBaseId) {
    if (knowledgeBaseId == nullptr) {
        return AI_COMMON_INVALID_PARAMETER;
    }
    return serviceProxy_.deleteKnowledgeBase(sessionId_, appId_, password_,
                                             knowledgeBaseId);
}

KnowledgeBaseInfo *KBSession::getKnowledgeBaseInfo(const char *knowledgeBaseId,
                                                   int *errorCode) {
    if (knowledgeBaseId == nullptr) {
        setErrorCode(errorCode, AI_COMMON_INVALID_PARAMETER);
        return nullptr;
    }
    return serviceProxy_.getKnowledgeBaseInfo(sessionId_, appId_, password_,
                                              knowledgeBaseId, errorCode);
}

KnowledgeBaseInfo **KBSession::getKnowledgeBaseInfoList(int *count,
                                                        int *errorCode) {
    if (count == nullptr) {
        setErrorCode(errorCode, AI_COMMON_INVALID_PARAMETER);
        return nullptr;
    }
    return serviceProxy_.getKnowledgeBaseInfoList(sessionId_, appId_, password_,
                                                  count, errorCode);
}

int KBSession::updateKnowledgeBase(const KnowledgeBaseInfo *knowledgeBaseInfo) {
    if (knowledgeBaseInfo == nullptr) {
        return AI_COMMON_INVALID_PARAMETER;
    }
    return serviceProxy_.updateKnowledgeBase(sessionId_, appId_, password_,
                                             knowledgeBaseInfo);
}

int KBSession::addDocument(const char *knowledgeBaseId,
                           const char *documentPath, const char *documentId,
                           void *reserved) {
    if (knowledgeBaseId == nullptr || documentPath == nullptr ||
        documentId == nullptr) {
        return AI_COMMON_INVALID_PARAMETER;
    }
    return serviceProxy_.addDocument(sessionId_, appId_, password_,
                                     knowledgeBaseId, documentPath, documentId);
}

int KBSession::addMultipleDocuments(const char *knowledgeBaseId,
                                    const char **documentPaths,
                                    const char **documentIds, int docsCount,
                                    void *reserved, int **errorCodes) {
    if (knowledgeBaseId == nullptr || documentPaths == nullptr ||
        documentIds == nullptr || docsCount <= 0 || *documentIds == nullptr ||
        *documentPaths == nullptr) {
        return AI_COMMON_INVALID_PARAMETER;
    }
    return serviceProxy_.addMultipleDocuments(
        sessionId_, appId_, password_, knowledgeBaseId, documentPaths,
        documentIds, docsCount, errorCodes);
}

char **KBSession::listDocuments(const char *knowledgeBaseId, int *count,
                                int *errorCode) {
    if (knowledgeBaseId == nullptr) {
        setErrorCode(errorCode, AI_COMMON_INVALID_PARAMETER);
        return nullptr;
    }
    return serviceProxy_.listDocuments(sessionId_, appId_, password_,
                                       knowledgeBaseId, count, errorCode);
}

int KBSession::deleteDocument(const char *knowledgeBaseId,
                              const char *documentId) {
    if (knowledgeBaseId == nullptr || documentId == nullptr) {
        return AI_COMMON_INVALID_PARAMETER;
    }
    return serviceProxy_.deleteDocument(sessionId_, appId_, password_,
                                        knowledgeBaseId, documentId);
}

int KBSession::deleteMultipleDocument(const char *knowledgeBaseId,
                                      const char **documentIds, int docsCount,
                                      int **errorCodes) {
    if (knowledgeBaseId == nullptr || documentIds == nullptr ||
        docsCount <= 0 || *documentIds == nullptr) {
        return AI_COMMON_INVALID_PARAMETER;
    }
    return serviceProxy_.deleteMultipleDocuments(sessionId_, appId_, password_,
                                                 knowledgeBaseId, documentIds,
                                                 docsCount, errorCodes);
}

int KBSession::cancelDocumentAddition(const char *knowledgeBaseId,
                                      const char **documentIds, int docsCount) {
    if (knowledgeBaseId == nullptr || documentIds == nullptr ||
        docsCount <= 0) {
        return AI_COMMON_INVALID_PARAMETER;
    }
    return serviceProxy_.cancelDocumentAddition(
        sessionId_, appId_, password_, knowledgeBaseId, documentIds, docsCount);
}

int KBSession::getDocumentProcessingStatus(KnowledgeBaseInfo *knowledgeBase,
                                           const char *documentId, int *status,
                                           int *errorCode) {
    if (knowledgeBase == nullptr || documentId == nullptr) {
        setErrorCode(errorCode, AI_COMMON_INVALID_PARAMETER);
        return -1;
    }
    return serviceProxy_.getDocumentProcessingStatus(
        sessionId_, appId_, password_, knowledgeBase, documentId, status,
        errorCode);
}

int KBSession::retrieval(const char *knowledgeBaseId, const char *requestId,
                         const char *query, const KBRetrievalConfig *config,
                         KBRetrievalCallback callback, void *userData) {
    if (knowledgeBaseId == nullptr || query == nullptr || strlen(query) == 0 ||
        callback == nullptr) {
        return AI_COMMON_INVALID_PARAMETER;
    }
    if (requestId == nullptr || strlen(requestId) == 0) {
        retrievalRequestId_ = std::to_string(std::rand());
    }
    return serviceProxy_.retrieval(sessionId_, appId_, password_,
                                   knowledgeBaseId, retrievalRequestId_, query,
                                   config, callback, userData);
}

int KBSession::chat(const char *requestId, const char *knowledgeBaseId,
                    const char *question, const KBChatHistory *history,
                    const KBChatConfig *config, KBChatStreamCallback callback,
                    void *userData) {
    if (knowledgeBaseId == nullptr || question == nullptr ||
        callback == nullptr || strlen(question) == 0) {
        return AI_COMMON_INVALID_PARAMETER;
    }
    if (requestId == nullptr || strlen(requestId) == 0) {
        chatRequestId_ = std::to_string(std::rand());
    }
    return serviceProxy_.chat(sessionId_, appId_, password_, chatRequestId_,
                              knowledgeBaseId, question, history, config,
                              callback, userData);
}

int KBSession::stopChat(const char *requestId) {
    std::string reqId =
        (requestId && strlen(requestId) > 0) ? requestId : chatRequestId_;
    return serviceProxy_.stopChat(sessionId_, reqId);
}

void KBSession::setErrorCode(int *errorCode, int code) {
    if (errorCode) {
        *errorCode = code;
    }
}