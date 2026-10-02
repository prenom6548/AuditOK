#ifndef ADVAI_SERVICE_PROXY_H
#define ADVAI_SERVICE_PROXY_H

#include <atomic>
#include <functional>
#include <mutex>
#include <queue>
#include <shared_mutex>

#include "_chathistory.h"
#include "_retrievalconfig.h"
#include "_retrievalresponse.h"
#include "core.h"
#include "jsoncpp/json/json.h"
#include "knowledgebaseserver.h"
#include "session.h"

namespace kyai::advancedai::knowledgebase {

struct DocumentStatus {
    std::string knowledgeBaseId;
    std::string fileId;
    core::DocumentUploadStatus status;
    int documentProgress;
};

struct UploadDocumentsStatus {
    int progress;
    std::vector<DocumentStatus> documentStatus;
};

using UploadDocumentProgressCallback =
    std::function<void(std::string knowledgeBaseId, std::string fileId,
                       int progress, core::DocumentUploadStatus status)>;

using ReconnectedCallback = std::function<void()>;

struct ChatCallbackInfo {
    KBChatStreamCallback callback;
    void *userData;
};

struct RetrievalCallbackInfo {
    KBRetrievalCallback callback;
    void *userData;
};

class KnowledgeBaseServiceProxy {
public:
    ~KnowledgeBaseServiceProxy();

    bool connectToServer();
    bool reconnectServer();

    void setReconnectedCallback(const ReconnectedCallback &callback);

    int init(const std::string &appId, const std::string &password,
             const std::string &databasePath, int *sessionId);
    int destroy(int sessionId);

    char *createKnowledgeBase(int sessionId, const std::string &appId,
                              const std::string &password,
                              const KnowledgeBaseInfo *knowledgeBaseInfo,
                              int *errorCode);

    int deleteKnowledgeBase(int sessionId, const std::string &appId,
                            const std::string &password,
                            const std::string &knowledgeBaseId) const;

    KnowledgeBaseInfo *getKnowledgeBaseInfo(int sessionId,
                                            const std::string &appId,
                                            const std::string &password,
                                            const std::string &knowledgeBaseId,
                                            int *errorCode) const;

    KnowledgeBaseInfo **getKnowledgeBaseInfoList(int sessionId,
                                                 const std::string &appId,
                                                 const std::string &password,
                                                 int *count,
                                                 int *errorCode) const;
    int updateKnowledgeBase(int sessionId, const std::string &appId,
                            const std::string &password,
                            const KnowledgeBaseInfo *knowledgeBaseInfo) const;

    int addDocument(int sessionId, const std::string &appId,
                    const std::string &password,
                    const std::string &knowledgeBaseId,
                    const std::string &documentPath,
                    const std::string &documentId);

    int addMultipleDocuments(int sessionId, const std::string &appId,
                             const std::string &password,
                             const std::string &knowledgeBaseId,
                             const char **documentPaths,
                             const char **documentIds, int docsCount,
                             int **errorCode);
    char **listDocuments(int sessionId, const std::string &appId,
                         const std::string &password,
                         const std::string &knowledgeBaseId, int *count,
                         int *errorCode) const;

    int deleteDocument(int sessionId, const std::string &appId,
                       const std::string &password,
                       const std::string &knowledgeBaseId,
                       const std::string &documentId) const;

    int deleteMultipleDocuments(int sessionId, const std::string &appId,
                                const std::string &password,
                                const std::string &knowledgeBaseId,
                                const char **documentIds, int docsCount,
                                int **errorCodes) const;

    int cancelDocumentAddition(int sessionId, const std::string &appId,
                               const std::string &password,
                               const std::string &knowledgeBaseId,
                               const char **documentIds, int docsCount);

    int getDocumentProcessingStatus(int sessionId, const std::string &appId,
                                    const std::string &password,
                                    KnowledgeBaseInfo *knowledgeBase,
                                    const char *documentId, int *status,
                                    int *errorCode);

    int retrieval(int sessionId, const std::string &appId,
                  const std::string &password,
                  const std::string &knowledgeBaseId,
                  const std::string &requestId, const std::string &query,
                  const KBRetrievalConfig *config, KBRetrievalCallback callback,
                  void *userData);

    int chat(int sessionId, const std::string &appId,
             const std::string &password, const std::string &requestId,
             const std::string &knowledgeBaseId, const std::string &question,
             const KBChatHistory *history, const KBChatConfig *config,
             KBChatStreamCallback callback, void *userData);

    int stopChat(int sessionId, const std::string &requestId) const;

private:
    int processInitResult(const std::string &jsonStr, int *sessionId);
    static int processDestroyResult(const std::string &jsonStr);
    static char *processCreateKnowledgeBaseResult(const std::string &jsonStr,
                                                  int *errorCode);
    static int processDeleteKnowledgeBaseResult(const std::string &jsonStr);
    static int processGetDocumentProcessingStatusResult(
        const std::string &jsonStr, int *status, int *errorCode);
    static int processUpdateKnowledgeBaseResult(
        const std::string &nameStr, const std::string &descriptionStr);
    static int processStopResult(const std::string &jsonStr);
    static KnowledgeBaseInfo *processKnowledgeBaseInfoResult(
        const std::string &jsonStr, int *errorCode);
    static KnowledgeBaseInfo **processKnowledgeBaseInfoListResult(
        const std::string &jsonStr, int *count, int *errorCode);
    static char **processListDocumentsResult(const std::string &jsonStr,
                                             int *count, int *errorCode);
    static int processDeleteDocumentResult(const std::string &jsonStr);
    static int processDeleteDocumentsResult(const std::string &jsonStr,
                                            int **errorCodes);
    static KBRetrievalResponse *processRetrievalResult(
        const std::string &jsonStr, int *errorCode);

    static int parseErrorCode(const Json::Value &json);
    static KnowledgeBaseInfo *parseKnowledgeBaseInfo(const Json::Value &json);

    static void setErrorCode(int *errorCode, int code);
    static void onUploadDocumentCallback(
        GDBusConnection *connection, const gchar *senderName,
        const gchar *objectPath, const gchar *interfaceName,
        const gchar *signalName, GVariant *parameters, gpointer userData);

    static void onChatResultCallback(GDBusConnection *connection,
                                     const gchar *senderName,
                                     const gchar *objectPath,
                                     const gchar *interfaceName,
                                     const gchar *signalName,
                                     GVariant *parameters, gpointer userData);
    static void onKBRetrievalFinishedCallback(GObject *sourceObject,
                                              GAsyncResult *res,
                                              gpointer userData);

    void handleErrorOccurred(int errorCode);
    void handleServerClosed();
    void handleResultTimeout();
    void handleGeneralError();

    void reconnectServerAsync();
    void removeCallbackTimerIfExists();
    void removeChatCallback(const std::string &requestId);
    void removeRetrievalCallback(const std::string &requestId);

    static void onKBChatStreamFinishedCallback(GObject *sourceObject,
                                               GAsyncResult *res,
                                               gpointer userData);

    void connectUploadDocumentResultSignals();
    void connectChatResultSignals(int sessionId);
    std::string buildChatHistoryJsonString(
        const std::vector<ChatTurn> &chatTurns);

    static bool isConnectionActive();

private:
    mutable std::shared_mutex proxyMutex_;
    AiBusinessKnowledgeBaseService *proxy_{nullptr};
    KnowledgeBaseServer *advancedAiServer_{nullptr};
    guint chatSubscriptionId_{0};
    guint uploadSubscriptionId_{0};

    std::mutex chatCallbackMutex_;
    std::unordered_map<std::string, ChatCallbackInfo> chatCallbacks_;
    std::unordered_map<std::string, RetrievalCallbackInfo> retrievalCallbacks_;
    ReconnectedCallback reconnectedCallback_{nullptr};
    std::string knowledgeBaseId_;
    std::queue<std::string> pendingChatRequests_;
    std::string currentChatRequestId_;
    std::queue<std::string> pendingRetrievalRequests_;
    std::string currentRetrievalRequestId_;
    int currentSessionId_{0};

    std::atomic<int> reconnectAttempts_{0};
    static constexpr int maxReconnectAttempts = 10;
    std::atomic<guint> reconnectTimerId_{0};
};

}  // namespace kyai::advancedai::knowledgebase

#endif