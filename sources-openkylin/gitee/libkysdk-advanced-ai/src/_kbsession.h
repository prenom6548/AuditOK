#ifndef _KB_SESSION_H
#define _KB_SESSION_H

#include "_retrievalconfig.h"
#include "_retrievalresponse.h"
#include "knowledgebaseserviceproxy.h"

class KBSession {
public:
    KBSession(const char *appName, const char *password,
              const char *databasePath);

    int init();

    int destroy();

    char *createKnowledgeBase(const KnowledgeBaseInfo *knowledgeBaseInfo,
                              int *errorCode);

    int deleteKnowledgeBase(const char *knowledgeBaseId);

    KnowledgeBaseInfo *getKnowledgeBaseInfo(const char *knowledgeBaseId,
                                            int *errorCode);

    KnowledgeBaseInfo **getKnowledgeBaseInfoList(int *count, int *errorCode);

    int updateKnowledgeBase(const KnowledgeBaseInfo *knowledgeBaseInfo);

    int addDocument(const char *knowledgeBaseId, const char *documentPath,
                    const char *documentId, void *reserved);

    int addMultipleDocuments(const char *knowledgeBaseId,
                             const char **documentPaths,
                             const char **documentIds, int docsCount,
                             void *reserved, int **errorCodes);

    char **listDocuments(const char *knowledgeBaseId, int *count,
                         int *errorCode);

    int deleteDocument(const char *knowledgeBaseId, const char *documentId);

    int deleteMultipleDocument(const char *knowledgeBaseId,
                               const char **documentIds, int docsCount,
                               int **errorCodes);

    int cancelDocumentAddition(const char *knowledgeBaseId,
                               const char **documentIds, int docsCount);

    int getDocumentProcessingStatus(KnowledgeBaseInfo *knowledgeBase,
                                    const char *documentId, int *status,
                                    int *errorCode);

    int retrieval(const char *knowledgeBaseId, const char *requestId,
                  const char *query, const KBRetrievalConfig *config,
                  KBRetrievalCallback callback, void *userData);

    int chat(const char *requestId, const char *knowledgeBaseId,
             const char *question, const KBChatHistory *history,
             const KBChatConfig *config, KBChatStreamCallback callback,
             void *userData);

    int stopChat(const char *requestId);

private:
    bool initServer();
    int initSession();
    void setErrorCode(int *errorCode, int code);

private:
    kyai::advancedai::knowledgebase::KnowledgeBaseServiceProxy serviceProxy_;
    std::string chatRequestId_;
    std::string retrievalRequestId_;
    int sessionId_{0};
    std::string appId_;
    std::string password_;
    std::string databasePath_;
};

#endif