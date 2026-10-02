#include "knowledgebaseserviceproxy.h"

#include <thread>

#include "_chatconfig.h"
#include "_chatresponse.h"
#include "_chatresult.h"
#include "_error.h"
#include "_knowledgebaseinfo.h"
#include "_retrievalconfig.h"
#include "_retrievalresponse.h"
#include "utils.h"

const char *objectPath = "/com/kylin/AiBusiness/KnowledgeBaseService";
const char *interface = "com.kylinos.aisdk.business.KnowledgeBaseService";

const char *uploadDocumentResultSignal = "DocumentUploadProgress";
const char *chatResultSignal = "StreamingChatResponse";

namespace kyai::advancedai::knowledgebase {

namespace {
std::string toRetrievalMethodString(core::RetrievalMethod method) {
    switch (method) {
        case core::RetrievalMethod::SEMANTIC:
            return "SEMANTIC";
        case core::RetrievalMethod::TEXT:
            return "TEXT";
        case core::RetrievalMethod::KEYWORD:
            return "KEYWORD";
        case core::RetrievalMethod::HYBRID:
            return "HYBRID";
        default:
            return "";
    }
}

std::string toRerankMethodString(core::RerankMethod method) {
    switch (method) {
        case core::RerankMethod::RRF:
            return "RRF";
        case core::RerankMethod::WEIGHT:
            return "WEIGHT";
        default:
            return "";
    }
}
}  // namespace

DocumentStatus dissectUploadDocumentResult(GVariant *parameters) {
    GVariantIter iter;
    g_variant_iter_init(&iter, parameters);

    GVariant *response = g_variant_iter_next_value(&iter);

    if (response == nullptr) {
        printf("upload document result error: result is nullptr!\n");
        g_variant_unref(response);
        return {};
    }

    const std::string responseStr = g_variant_get_string(response, nullptr);
    Json::Value responseJson = util::stringToJson(responseStr);

    DocumentStatus result;
    if (!responseJson.isMember("knowledgeBaseId") ||
        !responseJson.isMember("fileId") ||
        !responseJson.isMember("progress") ||
        !responseJson.isMember("status")) {
        printf("No available result %s\n", responseStr.c_str());
        g_variant_unref(response);
        return {};
    }
    result.knowledgeBaseId = responseJson["knowledgeBaseId"].asString();
    result.fileId = responseJson["fileId"].asString();
    result.documentProgress = responseJson["progress"].asInt();
    result.status =
        static_cast<core::DocumentUploadStatus>(responseJson["status"].asInt());

    // 释放资源
    g_variant_unref(response);

    return result;
}

KBChatResponse dissectChatResult(GVariant *parameters) {
    KBChatResponse result{};
    GVariantIter iter;
    g_variant_iter_init(&iter, parameters);

    GVariant *response = g_variant_iter_next_value(&iter);

    if (response == nullptr) {
        printf("chat result error: result is nullptr!\n");
        g_variant_unref(response);
        return result;
    }

    const std::string responseStr = g_variant_get_string(response, nullptr);
    printf("chat result %s\n", responseStr.c_str());
    Json::Value responseJson = util::stringToJson(responseStr);

    if (!responseJson.isMember("data") || !responseJson.isMember("err_no") ||
        !responseJson["data"].isMember("chatId") ||
        !responseJson["data"].isMember("requestId") ||
        !responseJson["data"].isMember("isFinal") ||
        !responseJson["data"].isMember("result") ||
        !responseJson["data"].isMember("queryText") ||
        !responseJson["data"].isMember("chatModel")) {
        printf("No available result %s\n", responseStr.c_str());
        g_variant_unref(response);
        return result;
    }
    result.chatId = responseJson["data"]["chatId"].asString();
    result.requestId = responseJson["data"]["requestId"].asString();
    result.errorCode =
        getErrorCodeFromKnowledgebase(responseJson["err_no"].asInt());
    result.errorMessage = responseJson["err_msg"].asString();
    if (!result.errorMessage.empty()) {
        result.publicErrorMessage = result.errorMessage;
    } else {
        result.publicErrorMessage =
            getErrorMessageFromKnowledgebase(result.errorCode);
    }

    result.result = new KBChatResult;
    result.result->isFinished = responseJson["data"]["isFinal"].asBool();
    result.result->question = responseJson["data"]["queryText"].asString();
    result.result->response = responseJson["data"]["result"].asString();
    result.result->reasoningContent =
        responseJson["data"]["reasoningMsg"].asString();
    result.result->model = responseJson["data"]["chatModel"].asString();

    if (responseJson["data"].isMember("reference")) {
        Json::Value referencesJson = responseJson["data"]["reference"];
        for (auto &dataJson : referencesJson) {
            auto referenceItem = new KBChatReference;
            referenceItem->fileNumber = dataJson["fileNumber"].asInt();
            referenceItem->chunkIndex = dataJson["chunkIndex"].asInt();
            result.result->reference.push_back(referenceItem);
        }
    }

    if (result.result->isFinished) {
        if (!responseJson["data"].isMember("chunks")) {
            printf("No available chunks %s\n", responseStr.c_str());
            g_variant_unref(response);
            return result;
        }
        Json::Value chunksJson = responseJson["data"]["chunks"];
        for (auto &dataJson : chunksJson) {
            auto sourceItem = new KBChatSource;
            sourceItem->fileId = dataJson["fileId"].asString();
            sourceItem->content = dataJson["content"].asString();
            sourceItem->confidence =
                static_cast<double>(dataJson["similarity"].asFloat());
            result.result->source.push_back(sourceItem);
        }
        if (!responseJson["data"].isMember("referenceFileIds") ||
            !responseJson["data"]["referenceFileIds"].isArray()) {
            printf("No available referenceFileIds %s\n", responseStr.c_str());
            g_variant_unref(response);
        }
        Json::Value referenceFileIdsJson =
            responseJson["data"]["referenceFileIds"];
        for (auto &fileIdJson : referenceFileIdsJson) {
            char *fileId = new char[fileIdJson.asString().size() + 1];
            strcpy(fileId, fileIdJson.asString().c_str());
            result.result->referenceFileIds.push_back(fileId);
        }
    }

    g_variant_unref(response);

    return result;
}

KnowledgeBaseServiceProxy::~KnowledgeBaseServiceProxy() {
    if (not KnowledgeBaseServer::getInstance().available()) {
        g_printerr(
            "Error creating core advanced ai server proxy: Server proxy "
            "connection is unavailable.\n");
        return;
    }

    auto *connection = KnowledgeBaseServer::getInstance().getConnection();
    if (uploadSubscriptionId_ != 0) {
        // 取消订阅
        g_dbus_connection_signal_unsubscribe(connection, uploadSubscriptionId_);
    }
    if (chatSubscriptionId_ != 0) {
        // 取消订阅
        g_dbus_connection_signal_unsubscribe(connection, chatSubscriptionId_);
    }
    {
        std::unique_lock lock(proxyMutex_);
        if (proxy_) {
            g_object_unref(proxy_);
            proxy_ = nullptr;
        }
    }
    removeCallbackTimerIfExists();
    while (!pendingChatRequests_.empty()) {
        pendingChatRequests_.pop();
    }
    while (!pendingRetrievalRequests_.empty()) {
        pendingRetrievalRequests_.pop();
    }
    chatCallbacks_.clear();
    retrievalCallbacks_.clear();
}

int KnowledgeBaseServiceProxy::init(const std::string &appId,
                                    const std::string &password,
                                    const std::string &databasePath,
                                    int *sessionId) {
    const std::map<std::string, std::string> fields = {
        {"appId", appId}, {"dbFilePath", databasePath}, {"password", password}};
    const std::string paramString =
        util::buildNestedJsonString(fields, {"appId", "password"});
    GError *error = nullptr;
    gchar *result;
    gboolean success;
    {
        std::shared_lock lock(proxyMutex_);
        success =
            ai_business_knowledge_base_service_call_init_knowledge_base_session_sync(
                proxy_, paramString.c_str(), &result, nullptr, &error);
    }
    if (!success) {
        if (error) {
            g_printerr("Error init service: %s\n", error->message);
            g_error_free(error);
        }
        return AI_COMMON_SERVER_CONNECTION_FAILED;
    }
    reconnectAttempts_ = 0;
    return processInitResult(result, sessionId);
}

int KnowledgeBaseServiceProxy::destroy(int sessionId) {
    if (sessionId <= 0) {
        printf("Error destroy service: Invalid sessionId %d\n", sessionId);
        return AI_COMMON_INVALID_SESSION;
    }
    const std::map<std::string, std::string> fields = {
        {"sessionId", std::to_string(sessionId)}};
    const std::string paramString = util::buildNestedJsonString(fields);
    reconnectAttempts_ = 0;
    GError *error = nullptr;
    gchar *result;
    gboolean success;
    {
        std::shared_lock lock(proxyMutex_);
        success =
            ai_business_knowledge_base_service_call_destroy_knowledge_base_session_sync(
                proxy_, paramString.c_str(), &result, nullptr, &error);
    }
    if (!success) {
        if (error) {
            g_printerr("Error init service: %s\n", error->message);
            g_error_free(error);
        }
        return AI_COMMON_SERVER_CONNECTION_FAILED;
    }
    return processDestroyResult(result);
}

bool KnowledgeBaseServiceProxy::connectToServer() {
    KnowledgeBaseServer::getInstance().init();
    if (not KnowledgeBaseServer::getInstance().available()) {
        g_printerr(
            "Error creating core advanced ai server proxy: Server proxy "
            "connection is unavailable.\n");
        return false;
    }

    // 保护proxy修改
    {
        std::unique_lock<std::shared_mutex> lock(proxyMutex_);
        if (proxy_) {
            g_object_unref(proxy_);
            proxy_ = nullptr;
        }

        GError *error{nullptr};
        auto *connection = KnowledgeBaseServer::getInstance().getConnection();
        proxy_ = ai_business_knowledge_base_service_proxy_new_sync(
            connection, G_DBUS_PROXY_FLAGS_NONE, nullptr, objectPath, nullptr,
            &error);
        if (proxy_ == nullptr) {
            if (error) {
                g_printerr("Error creating text processor proxy %s: %s\n",
                           objectPath, error->message);
                g_error_free(error);
            }
            return false;
        }
        g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(proxy_),
                                         2 * 60 * 60 * 1000);
    }

    return true;
}

bool KnowledgeBaseServiceProxy::reconnectServer() {
    if (reconnectAttempts_ >= maxReconnectAttempts) {
        printf("Max reconnect attempts reached\n");
        return false;
    }

    reconnectAttempts_++;
    printf("Reconnect attempt %d.\n", reconnectAttempts_.load());

    if (!connectToServer()) {
        printf("Failed to reconnect to server\n");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return reconnectServer();
    }

    printf("Successfully reconnected to server.\n");

    return true;
}

void KnowledgeBaseServiceProxy::reconnectServerAsync() {
    removeCallbackTimerIfExists();
    reconnectTimerId_ = g_timeout_add(
        1000,
        [](gpointer data) -> gboolean {
            auto *proxy = static_cast<KnowledgeBaseServiceProxy *>(data);
            if (proxy->reconnectAttempts_ >= maxReconnectAttempts) {
                printf("Max reconnect attempts reached\n");
                proxy->removeCallbackTimerIfExists();
                return G_SOURCE_REMOVE;
            }
            proxy->reconnectAttempts_++;
            printf("Async reconnect attempt %d.\n",
                   proxy->reconnectAttempts_.load());
            if (proxy->connectToServer()) {
                if (proxy->reconnectedCallback_) {
                    proxy->reconnectedCallback_();
                }
                printf("Successfully reconnected to server.\n");
                return G_SOURCE_REMOVE;
            }
            printf("Failed to reconnect to server\n");
            return G_SOURCE_CONTINUE;
        },
        this);
}

void KnowledgeBaseServiceProxy::removeCallbackTimerIfExists() {
    if (reconnectTimerId_ > 0) {
        g_source_remove(reconnectTimerId_);
        reconnectTimerId_ = 0;
    }
}

void KnowledgeBaseServiceProxy::removeChatCallback(
    const std::string &requestId) {
    auto callback = chatCallbacks_.find(requestId);
    if (callback != chatCallbacks_.end()) {
        chatCallbacks_.erase(callback);
    }
}

void KnowledgeBaseServiceProxy::removeRetrievalCallback(
    const std::string &requestId) {
    auto callback = retrievalCallbacks_.find(requestId);
    if (callback != retrievalCallbacks_.end()) {
        retrievalCallbacks_.erase(callback);
    }
}

void KnowledgeBaseServiceProxy::setReconnectedCallback(
    const ReconnectedCallback &callback) {
    reconnectedCallback_ = std::move(callback);
}

void KnowledgeBaseServiceProxy::onKBChatStreamFinishedCallback(
    GObject *sourceObject, GAsyncResult *res, gpointer userData) {
    GError *error{nullptr};
    auto *proxy = static_cast<KnowledgeBaseServiceProxy *>(userData);
    printf("Knowledgebase chat finished \n");
    proxy->removeCallbackTimerIfExists();
    bool result =
        ai_business_knowledge_base_service_call_chat_with_knowledge_base_finish(
            (AiBusinessKnowledgeBaseService *)sourceObject, nullptr, res,
            &error);
    if (!result) {
        if (error == nullptr) {
            g_printerr("Error calling knowledgebase chat: Unknown error\n");
        } else {
            g_printerr("Error calling knowledgebase chat: %s\n",
                       error->message);
            proxy->handleErrorOccurred(error->code);
            g_error_free(error);
        }
        {
            std::lock_guard lock(proxy->chatCallbackMutex_);
            proxy->removeChatCallback(proxy->currentChatRequestId_);
        }
        if (!proxy->pendingChatRequests_.empty()) {
            proxy->pendingChatRequests_.pop();
            // 设置下一个待处理请求
            if (!proxy->pendingChatRequests_.empty()) {
                proxy->currentChatRequestId_ =
                    proxy->pendingChatRequests_.front();
            } else {
                proxy->currentChatRequestId_.clear();
            }
        }
    }
}

void KnowledgeBaseServiceProxy::handleErrorOccurred(int errorCode) {
    printf("Error occurred with code: %d\n", errorCode);
    switch (errorCode) {
        case G_IO_ERROR_CLOSED:
            handleServerClosed();
            break;
        case G_IO_ERROR_TIMED_OUT:
            handleResultTimeout();
            break;
        default:
            handleGeneralError();
            break;
    }
}

void KnowledgeBaseServiceProxy::handleServerClosed() {
    printf("Server closed unexpectedly.\n");
    if (!currentChatRequestId_.empty()) {
        auto it = chatCallbacks_.find(currentChatRequestId_);
        if (it != chatCallbacks_.end()) {
            auto &callbackInfo = it->second;
            KBChatResponse result{};
            result.errorCode = AI_COMMON_SERVER_CONNECTION_FAILED;
            result.result = new KBChatResult();
            result.result->isFinished = true;
            callbackInfo.callback(&result, callbackInfo.userData);
        }
    }
    reconnectServerAsync();
}

void KnowledgeBaseServiceProxy::handleResultTimeout() {
    if (!currentChatRequestId_.empty()) {
        auto it = chatCallbacks_.find(currentChatRequestId_);
        if (it != chatCallbacks_.end()) {
            auto &callbackInfo = it->second;
            KBChatResponse result{};
            result.errorCode = AI_COMMON_SERVICE_TIMEOUT;
            result.result = new KBChatResult();
            result.result->isFinished = true;
            callbackInfo.callback(&result, callbackInfo.userData);
        }
    }

    stopChat(currentSessionId_, currentChatRequestId_);
}

void KnowledgeBaseServiceProxy::handleGeneralError() {
    printf("General error occurred\n");
    if (!currentChatRequestId_.empty()) {
        auto it = chatCallbacks_.find(currentChatRequestId_);
        if (it != chatCallbacks_.end()) {
            auto &callbackInfo = it->second;
            KBChatResponse result{};
            result.errorCode = AI_COMMON_SERVER_CONNECTION_FAILED;
            result.result = new KBChatResult();
            result.result->isFinished = true;
            callbackInfo.callback(&result, callbackInfo.userData);
        }
    }
    removeChatCallback(currentChatRequestId_);
}

char *KnowledgeBaseServiceProxy::createKnowledgeBase(
    int sessionId, const std::string &appId, const std::string &password,
    const KnowledgeBaseInfo *knowledgeBaseInfo, int *errorCode) {
    if (sessionId <= 0) {
        setErrorCode(errorCode, AI_COMMON_INVALID_SESSION);
        return nullptr;
    }
    if (knowledgeBaseInfo == nullptr || knowledgeBaseInfo->name.empty()) {
        printf("Invalid knowledge base info\n");
        setErrorCode(errorCode, AI_COMMON_INVALID_PARAMETER);
        return nullptr;
    }
    const std::map<std::string, std::string> fields = {
        {"sessionId", std::to_string(sessionId)},
        {"appId", appId},
        {"password", password},
        {"name", knowledgeBaseInfo->name},
        {"description", knowledgeBaseInfo->description},
        {"type", std::string(knowledgeBaseInfo->kbType)},
        {"indexType", std::string(knowledgeBaseInfo->indexType)}};
    const std::string paramString = util::buildNestedJsonString(
        fields, {"appId", "password", "name", "description"});

    GError *error = nullptr;
    gchar *result;
    gboolean success;
    {
        std::shared_lock lock(proxyMutex_);
        success =
            ai_business_knowledge_base_service_call_create_knowledge_base_sync(
                proxy_, paramString.c_str(), &result, nullptr, &error);
    }
    if (!success) {
        if (error) {
            g_printerr("Error creating knowledge base: %s\n", error->message);
            g_error_free(error);
        }
        setErrorCode(errorCode, AI_COMMON_SERVER_CONNECTION_FAILED);
        return nullptr;
    }
    setErrorCode(errorCode, AI_COMMON_SUCCESS);
    return processCreateKnowledgeBaseResult(result, errorCode);
}

int KnowledgeBaseServiceProxy::deleteKnowledgeBase(
    int sessionId, const std::string &appId, const std::string &password,
    const std::string &knowledgeBaseId) const {
    if (sessionId <= 0) {
        return AI_COMMON_INVALID_SESSION;
    }
    const std::map<std::string, std::string> fields = {
        {"sessionId", std::to_string(sessionId)},
        {"appId", appId},
        {"password", password},
        {"knowledgeBaseId", knowledgeBaseId},
    };
    const std::string paramString =
        util::buildNestedJsonString(fields, {"appId", "password"});

    GError *error = nullptr;
    gchar *result;
    gboolean success;
    {
        std::shared_lock lock(proxyMutex_);
        success =
            ai_business_knowledge_base_service_call_delete_knowledge_base_sync(
                proxy_, paramString.c_str(), &result, nullptr, &error);
    }

    if (!success) {
        if (error) {
            g_printerr("Error deleting knowledge base: %s\n", error->message);
            g_error_free(error);
        }
        return AI_COMMON_SERVER_CONNECTION_FAILED;
    }
    return processDeleteKnowledgeBaseResult(result);
}

KnowledgeBaseInfo *KnowledgeBaseServiceProxy::getKnowledgeBaseInfo(
    int sessionId, const std::string &appId, const std::string &password,
    const std::string &knowledgeBaseId, int *errorCode) const {
    if (sessionId <= 0) {
        setErrorCode(errorCode, AI_COMMON_INVALID_SESSION);
        return nullptr;
    }
    const std::map<std::string, std::string> fields = {
        {"sessionId", std::to_string(sessionId)},
        {"appId", appId},
        {"password", password},
        {"knowledgeBaseId", knowledgeBaseId},
    };
    const std::string paramString =
        util::buildNestedJsonString(fields, {"appId", "password"});

    GError *error = nullptr;
    gchar *result;
    gboolean success;
    {
        std::shared_lock lock(proxyMutex_);
        success =
            ai_business_knowledge_base_service_call_get_knowledge_base_sync(
                proxy_, paramString.c_str(), &result, nullptr, &error);
    }
    if (!success) {
        if (error) {
            g_printerr("Error getting knowledge base info: %s\n",
                       error->message);
            g_error_free(error);
        }
        setErrorCode(errorCode, AI_COMMON_SERVER_CONNECTION_FAILED);
        return nullptr;
    }
    setErrorCode(errorCode, AI_COMMON_SUCCESS);
    return processKnowledgeBaseInfoResult(result, errorCode);
}

KnowledgeBaseInfo **KnowledgeBaseServiceProxy::getKnowledgeBaseInfoList(
    int sessionId, const std::string &appId, const std::string &password,
    int *count, int *errorCode) const {
    if (sessionId <= 0) {
        setErrorCode(errorCode, AI_COMMON_INVALID_SESSION);
        return nullptr;
    }
    const std::map<std::string, std::string> fields = {
        {"sessionId", std::to_string(sessionId)},
        {"appId", appId},
        {"password", password},
    };

    const std::string paramString =
        util::buildNestedJsonString(fields, {"appId", "password"});
    GError *error = nullptr;
    gchar *result;
    gboolean success;
    {
        std::shared_lock lock(proxyMutex_);
        success =
            ai_business_knowledge_base_service_call_get_all_knowledge_base_sync(
                proxy_, paramString.c_str(), &result, nullptr, &error);
    }
    if (!success) {
        if (error) {
            g_printerr("Error getting knowledge base info list: %s\n",
                       error->message);
            g_error_free(error);
        }
        setErrorCode(errorCode, AI_COMMON_SERVER_CONNECTION_FAILED);
        return nullptr;
    }
    setErrorCode(errorCode, AI_COMMON_SUCCESS);
    return processKnowledgeBaseInfoListResult(result, count, errorCode);
}

int KnowledgeBaseServiceProxy::updateKnowledgeBase(
    int sessionId, const std::string &appId, const std::string &password,
    const KnowledgeBaseInfo *knowledgeBaseInfo) const {
    if (sessionId <= 0) {
        return AI_COMMON_INVALID_SESSION;
    }
    std::map<std::string, std::string> fields = {
        {"sessionId", std::to_string(sessionId)},
        {"appId", appId},
        {"password", password},
        {"knowledgeBaseId", knowledgeBaseInfo->knowledgeBaseId},
        {"name", knowledgeBaseInfo->name}};
    std::string paramString =
        util::buildNestedJsonString(fields, {"appId", "password", "name"});

    GError *error = nullptr;
    gchar *nameResult;
    gboolean success;
    {
        std::shared_lock lock(proxyMutex_);
        success =
            ai_business_knowledge_base_service_call_set_knowledge_base_name_sync(
                proxy_, paramString.c_str(), &nameResult, nullptr, &error);
    }
    if (!success) {
        if (error) {
            g_printerr("Error updating knowledge base name: %s\n",
                       error->message);
            g_error_free(error);
        }
        return AI_COMMON_SERVER_CONNECTION_FAILED;
    }

    gchar *descriptionResult;
    fields = {{"sessionId", std::to_string(sessionId)},
              {"appId", appId},
              {"password", password},
              {"knowledgeBaseId", knowledgeBaseInfo->knowledgeBaseId},
              {"description", knowledgeBaseInfo->description}};
    paramString = util::buildNestedJsonString(
        fields, {"appId", "password", "description"});
    {
        std::shared_lock lock(proxyMutex_);
        success =
            ai_business_knowledge_base_service_call_set_knowledge_base_description_sync(
                proxy_, paramString.c_str(), &descriptionResult, nullptr,
                &error);
    }
    if (!success) {
        if (error) {
            g_printerr("Error updating knowledge base description: %s\n",
                       error->message);
            g_error_free(error);
        }
        return AI_COMMON_SERVER_CONNECTION_FAILED;
    }
    return processUpdateKnowledgeBaseResult(nameResult, descriptionResult);
}

int KnowledgeBaseServiceProxy::addDocument(int sessionId,
                                           const std::string &appId,
                                           const std::string &password,
                                           const std::string &knowledgeBaseId,
                                           const std::string &documentPath,
                                           const std::string &documentId) {
    if (sessionId <= 0) {
        return AI_COMMON_INVALID_SESSION;
    }
    if (!isConnectionActive()) {
        printf("Error adding document, not connected to server\n");
        return AI_COMMON_SERVER_CONNECTION_FAILED;
    }
    knowledgeBaseId_ = knowledgeBaseId;
    const std::map<std::string, std::string> config = {
        {"chunkSize", "1024"},
        {"embeddingModel", "text-embedding-ada"},
        {"enhanceParse", "false"}};
    const std::string configString = util::buildJsonString(config);

    // 服务仅提供上传多文档接口
    std::vector<std::map<std::string, std::string>> files;
    std::map<std::string, std::string> fileEntry = {{"filePath", documentPath},
                                                    {"fileId", documentId}};
    files.push_back(fileEntry);
    std::string filesJson = util::buildJsonArray(files);

    const std::map<std::string, std::string> fields = {
        {"sessionId", std::to_string(sessionId)},
        {"appId", appId},
        {"password", password},
        {"knowledgeBaseId", knowledgeBaseId},
        {"files", filesJson},
        {"config", configString}};
    const std::string paramString =
        util::buildNestedJsonString(fields, {"appId", "password"});

    //    connectUploadDocumentResultSignals();
    {
        std::shared_lock lock(proxyMutex_);
        ai_business_knowledge_base_service_call_upload_documents(
            proxy_, paramString.c_str(), nullptr, nullptr, this);
    }
    return AI_COMMON_SUCCESS;
}

int KnowledgeBaseServiceProxy::addMultipleDocuments(
    int sessionId, const std::string &appId, const std::string &password,
    const std::string &knowledgeBaseId, const char **documentPaths,
    const char **documentIds, int docsCount, int **errorCode) {
    if (sessionId <= 0) {
        return AI_COMMON_INVALID_SESSION;
    }
    if (!isConnectionActive()) {
        printf("Error adding documents, not connected to server\n");
        return AI_COMMON_SERVER_CONNECTION_FAILED;
    }
    knowledgeBaseId_ = knowledgeBaseId;
    const std::map<std::string, std::string> config = {
        {"chunkMaxSize", "1024"},
        {"embeddingModel", "text-embedding-ada"},
        {"enhanceParse", "false"}};
    const std::string configString = util::buildJsonString(config);
    // 构建 files 数组中的每个文件项
    std::vector<std::map<std::string, std::string>> files;
    for (int i = 0; i < docsCount; i++) {
        std::map<std::string, std::string> fileEntry = {
            {"filePath", documentPaths[i]}, {"fileId", documentIds[i]}};
        files.push_back(fileEntry);
    }
    std::string filesJson = util::buildJsonArray(files);
    // 构建完整的 JSON 参数对象
    const std::map<std::string, std::string> fields = {
        {"sessionId", std::to_string(sessionId)},
        {"appId", appId},
        {"password", password},
        {"knowledgeBaseId", knowledgeBaseId},
        {"files", filesJson},
        {"config", configString}};

    const std::string paramString =
        util::buildNestedJsonString(fields, {"appId", "password"});

    //    connectUploadDocumentResultSignals();
    {
        std::shared_lock lock(proxyMutex_);
        ai_business_knowledge_base_service_call_upload_documents(
            proxy_, paramString.c_str(), nullptr, nullptr, this);
    }
    return AI_COMMON_SUCCESS;
}

char **KnowledgeBaseServiceProxy::listDocuments(
    int sessionId, const std::string &appId, const std::string &password,
    const std::string &knowledgeBaseId, int *count, int *errorCode) const {
    if (sessionId <= 0) {
        setErrorCode(errorCode, AI_COMMON_INVALID_SESSION);
        return nullptr;
    }
    const std::map<std::string, std::string> fields = {
        {"sessionId", std::to_string(sessionId)},
        {"appId", appId},
        {"password", password},
        {"knowledgeBaseId", knowledgeBaseId},
    };
    const std::string paramString =
        util::buildNestedJsonString(fields, {"appId", "password"});

    GError *error = nullptr;
    gchar *result;
    gboolean success;
    {
        std::shared_lock lock(proxyMutex_);
        success =
            ai_business_knowledge_base_service_call_get_file_list_in_knowledge_base_sync(
                proxy_, paramString.c_str(), &result, nullptr, &error);
    }
    if (!success) {
        if (error) {
            g_printerr("Error listing documents: %s\n", error->message);
            g_error_free(error);
        }
        setErrorCode(errorCode, AI_COMMON_SERVER_CONNECTION_FAILED);
        return nullptr;
    }
    setErrorCode(errorCode, AI_COMMON_SUCCESS);
    return processListDocumentsResult(result, count, errorCode);
}

int KnowledgeBaseServiceProxy::deleteDocument(
    int sessionId, const std::string &appId, const std::string &password,
    const std::string &knowledgeBaseId, const std::string &documentId) const {
    if (sessionId <= 0) {
        return AI_COMMON_INVALID_SESSION;
    }
    std::vector<std::string> filesVector;
    filesVector.push_back(documentId);
    auto files = util::buildJsonArray(filesVector);
    const std::map<std::string, std::string> fields = {
        {"sessionId", std::to_string(sessionId)},
        {"appId", appId},
        {"password", password},
        {"knowledgeBaseId", knowledgeBaseId},
        {"fileIds", files}};
    const std::string paramString =
        util::buildNestedJsonString(fields, {"appId", "password"});

    GError *error = nullptr;
    gchar *result;
    gboolean success;
    {
        std::shared_lock lock(proxyMutex_);
        success = ai_business_knowledge_base_service_call_delete_files_sync(
            proxy_, paramString.c_str(), &result, nullptr, &error);
    }
    if (!success) {
        if (error) {
            g_printerr("Error deleting document: %s\n", error->message);
            g_error_free(error);
        }
        return AI_COMMON_SERVER_CONNECTION_FAILED;
    }
    return processDeleteDocumentResult(result);
}

int KnowledgeBaseServiceProxy::deleteMultipleDocuments(
    int sessionId, const std::string &appId, const std::string &password,
    const std::string &knowledgeBaseId, const char **documentIds,
    const int docsCount, int **errorCodes) const {
    if (sessionId <= 0) {
        return AI_COMMON_INVALID_SESSION;
    }
    std::vector<std::string> filesVector;
    filesVector.reserve(docsCount);
    for (int i = 0; i < docsCount; i++) {
        filesVector.emplace_back(documentIds[i]);
    }
    auto files = util::buildJsonArray(filesVector);
    const std::map<std::string, std::string> fields = {
        {"sessionId", std::to_string(sessionId)},
        {"appId", appId},
        {"password", password},
        {"knowledgeBaseId", knowledgeBaseId},
        {"fileIds", files}};
    const std::string paramString =
        util::buildNestedJsonString(fields, {"appId", "password"});

    GError *error = nullptr;
    gchar *result;
    gboolean success;
    {
        std::shared_lock lock(proxyMutex_);
        success = ai_business_knowledge_base_service_call_delete_files_sync(
            proxy_, paramString.c_str(), &result, nullptr, &error);
    }
    if (!success) {
        if (error) {
            g_printerr("Error deleting documents: %s\n", error->message);
            g_error_free(error);
        }
        return AI_COMMON_SERVER_CONNECTION_FAILED;
    }
    return processDeleteDocumentsResult(result, errorCodes);
}

int KnowledgeBaseServiceProxy::cancelDocumentAddition(
    int sessionId, const std::string &appId, const std::string &password,
    const std::string &knowledgeBaseId, const char **documentIds,
    int docsCount) {
    if (sessionId <= 0) {
        return AI_COMMON_INVALID_SESSION;
    }
    std::vector<std::string> filesVector;
    filesVector.reserve(docsCount);
    for (int i = 0; i < docsCount; i++) {
        filesVector.emplace_back(documentIds[i]);
    }
    auto files = util::buildJsonArray(filesVector);
    const std::map<std::string, std::string> fields = {
        {"sessionId", std::to_string(sessionId)},
        {"appId", appId},
        {"password", password},
        {"knowledgeBaseId", knowledgeBaseId},
        {"fileIds", files}};
    const std::string paramString =
        util::buildNestedJsonString(fields, {"appId", "password"});

    GError *error = nullptr;
    gchar *result;
    gboolean success;
    {
        std::shared_lock lock(proxyMutex_);
        success = ai_business_knowledge_base_service_call_stop_upload_sync(
            proxy_, paramString.c_str(), &result, nullptr, &error);
    }
    if (!success) {
        if (error) {
            g_printerr("Error cancel document addition: %s\n", error->message);
            g_error_free(error);
        }
        return AI_COMMON_SERVER_CONNECTION_FAILED;
    }
    return AI_COMMON_SUCCESS;
}

int KnowledgeBaseServiceProxy::getDocumentProcessingStatus(
    int sessionId, const std::string &appId, const std::string &password,
    KnowledgeBaseInfo *knowledgeBase, const char *documentId, int *status,
    int *errorCode) {
    if (sessionId <= 0) {
        setErrorCode(errorCode, AI_COMMON_INVALID_SESSION);
        return -1;
    }
    const std::map<std::string, std::string> fields = {
        {"sessionId", std::to_string(sessionId)},
        {"appId", appId},
        {"password", password},
        {"knowledgeBaseId", knowledgeBase->knowledgeBaseId},
        {"fileId", documentId}};
    const std::string paramString =
        util::buildNestedJsonString(fields, {"appId", "password", "fileId"});
    GError *error = nullptr;
    gchar *result;
    gboolean success;
    {
        std::shared_lock lock(proxyMutex_);
        success =
            ai_business_knowledge_base_service_call_get_file_processing_progress_sync(
                proxy_, paramString.c_str(), &result, nullptr, &error);
    }
    if (!success) {
        if (error) {
            g_printerr("Error getting document processing status: %s\n",
                       error->message);
            g_error_free(error);
        }
        setErrorCode(errorCode, AI_COMMON_SERVER_CONNECTION_FAILED);
        return -1;
    }
    setErrorCode(errorCode, AI_COMMON_SUCCESS);
    return processGetDocumentProcessingStatusResult(result, status, errorCode);
}

int KnowledgeBaseServiceProxy::retrieval(
    int sessionId, const std::string &appId, const std::string &password,
    const std::string &knowledgeBaseId, const std::string &requestId,
    const std::string &query, const KBRetrievalConfig *config,
    KBRetrievalCallback callback, void *userData) {
    if (sessionId <= 0) {
        return AI_COMMON_INVALID_SESSION;
    }
    retrievalCallbacks_[requestId] = {callback, userData};

    pendingRetrievalRequests_.push(requestId);
    if (currentRetrievalRequestId_.empty()) {
        currentRetrievalRequestId_ = requestId;
    }
    std::map<std::string, std::string> fields = {
        {"sessionId", std::to_string(sessionId)},
        {"appId", appId},
        {"password", password},
        {"knowledgeBaseId", knowledgeBaseId},
        {"requestId", requestId},
        {"query", query}};

    if (config != nullptr) {
        std::map<std::string, std::string> configFields;
        if (!config->docIds.empty()) {
            configFields["fileIds"] = util::buildJsonArray(config->docIds);
        }

        const std::string method =
            toRetrievalMethodString(config->retrievalMethod);
        if (!method.empty()) {
            configFields["retrievalMethod"] = method;
        }

        const std::string rerank = toRerankMethodString(config->rerankMethod);
        if (!rerank.empty()) {
            configFields["rerankMethod"] = rerank;
        }

        if (config->topKSet && config->topK > 0) {
            configFields["topK"] = std::to_string(config->topK);
        }
        if (!configFields.empty()) {
            fields["config"] = util::buildNestedJsonString(
                configFields, {"retrievalMethod", "rerankMethod"});
        }
    }

    const std::string paramString = util::buildNestedJsonString(
        fields, {"appId", "password", "requestId", "query"});

    {
        std::shared_lock lock(proxyMutex_);
        ai_business_knowledge_base_service_call_retrieval_knowledge_base(
            proxy_, paramString.c_str(), nullptr, onKBRetrievalFinishedCallback,
            this);
    }
    return AI_COMMON_SUCCESS;
}

int KnowledgeBaseServiceProxy::chat(
    int sessionId, const std::string &appId, const std::string &password,
    const std::string &requestId, const std::string &knowledgeBaseId,
    const std::string &question, const KBChatHistory *history,
    const KBChatConfig *config, KBChatStreamCallback callback, void *userData) {
    if (sessionId <= 0) {
        return AI_COMMON_INVALID_SESSION;
    }
    chatCallbacks_[requestId] = {callback, userData};

    reconnectAttempts_ = 0;
    pendingChatRequests_.push(requestId);
    if (currentChatRequestId_.empty()) {
        currentChatRequestId_ = requestId;
    }
    currentSessionId_ = sessionId;

    std::vector<std::string> documentsVector;
    documentsVector.reserve(config->docIds.size());
    for (const auto &docId : config->docIds) {
        documentsVector.push_back(docId);
    }
    auto documents = util::buildJsonArray(documentsVector);

    const std::map<std::string, std::string> configFields = {
        {"modelName", config->model},
        {"expandContext", "true"},
        {"queryDiversifyEnable",
         config->enableDiversifyQuery ? "true" : "false"},
        {"newChat", config->newChat ? "true" : "false"},
        {"fileIds", documents},
    };
    std::string configString =
        util::buildNestedJsonString(configFields, {"modelName"});
    std::string historyString;
    if (history) {
        historyString = buildChatHistoryJsonString(history->chatTurns);
    } else {
        historyString = "[]";  // 应为空数组
    }

    const std::map<std::string, std::string> fields = {
        {"sessionId", std::to_string(sessionId)},
        {"appId", appId},
        {"password", password},
        {"knowledgeBaseId", knowledgeBaseId},
        {"requestId", requestId},
        {"message", historyString},
        {"query", question},
        {"config", configString}};
    const std::string paramString = util::buildNestedJsonString(
        fields, {"appId", "password", "requestId", "query"});

    {
        std::shared_lock lock(proxyMutex_);
        ai_business_knowledge_base_service_call_chat_with_knowledge_base(
            proxy_, paramString.c_str(), nullptr,
            onKBChatStreamFinishedCallback, this);
    }
    return AI_COMMON_SUCCESS;
}

int KnowledgeBaseServiceProxy::stopChat(int sessionId,
                                        const std::string &requestId) const {
    if (sessionId <= 0) {
        return AI_COMMON_INVALID_SESSION;
    }
    const std::map<std::string, std::string> fields = {
        {"sessionId", std::to_string(sessionId)},
        {"requestId", requestId},
    };
    const std::string paramString =
        util::buildNestedJsonString(fields, {"requestId"});

    GError *error = nullptr;
    gchar *result;
    gboolean success;
    {
        std::shared_lock lock(proxyMutex_);
        success = ai_business_knowledge_base_service_call_stop_chat_sync(
            proxy_, paramString.c_str(), &result, nullptr, &error);
    }
    printf("Stop chat \n");
    if (!success) {
        if (error) {
            g_printerr("Error stopping chat: %s\n", error->message);
            g_error_free(error);
        }
        return AI_COMMON_SERVER_CONNECTION_FAILED;
    }

    return processStopResult(result);
}

int KnowledgeBaseServiceProxy::processInitResult(const std::string &jsonStr,
                                                 int *sessionId) {
    Json::Value root = util::stringToJson(jsonStr);
    const int code = parseErrorCode(root);
    if (code != AI_COMMON_SUCCESS) {
        *sessionId = -1;
        return code;
    }
    if (root.isMember("data") && root["data"].isMember("sessionId")) {
        *sessionId = root["data"]["sessionId"].asInt();
        printf("Session id: %d\n", *sessionId);
        connectChatResultSignals(*sessionId);
        return AI_COMMON_SUCCESS;
    }
    return AI_COMMON_SERVICE_ERROR;
}

int KnowledgeBaseServiceProxy::processDestroyResult(
    const std::string &jsonStr) {
    const Json::Value root = util::stringToJson(jsonStr);
    return parseErrorCode(root);
}

char *KnowledgeBaseServiceProxy::processCreateKnowledgeBaseResult(
    const std::string &jsonStr, int *errorCode) {
    std::string errorMessage;

    Json::Value root = util::stringToJson(jsonStr);
    const int code = parseErrorCode(root);
    if (code != AI_COMMON_SUCCESS) {
        setErrorCode(errorCode, code);
        return nullptr;
    }
    if (root.isMember("data") && root["data"].isMember("knowledgeBaseId")) {
        const std::string kbId = root["data"]["knowledgeBaseId"].asString();
        const auto kbIdCopy = new char[kbId.size() + 1];
        strcpy(kbIdCopy, kbId.c_str());
        return kbIdCopy;
    }
    return nullptr;
}

int KnowledgeBaseServiceProxy::processDeleteKnowledgeBaseResult(
    const std::string &jsonStr) {
    const Json::Value root = util::stringToJson(jsonStr);
    return parseErrorCode(root);
}

int KnowledgeBaseServiceProxy::processGetDocumentProcessingStatusResult(
    const std::string &jsonStr, int *status, int *errorCode) {
    const Json::Value root = util::stringToJson(jsonStr);
    const int code = parseErrorCode(root);
    if (code != AI_COMMON_SUCCESS) {
        setErrorCode(errorCode, code);
        return -1;
    }
    if (!root.isMember("data") || !root["data"].isMember("status") ||
        !root["data"].isMember("progress")) {
        setErrorCode(errorCode, AI_COMMON_SERVICE_ERROR);
        return -1;
    }
    *status = getDocumentStatus(root["data"]["status"].asString());
    int progress = root["data"]["progress"].asInt();
    return progress;
}

int KnowledgeBaseServiceProxy::processDeleteDocumentResult(
    const std::string &jsonStr) {
    const Json::Value root = util::stringToJson(jsonStr);
    return parseErrorCode(root);
}

int KnowledgeBaseServiceProxy::processDeleteDocumentsResult(
    const std::string &jsonStr, int **errorCodes) {
    int errorCount = 0;
    const Json::Value root = util::stringToJson(jsonStr);
    if (!root.isMember("errors") || !root["errors"].isArray()) {
        return parseErrorCode(root);
    }
    for (int i = 0; i < root["errors"].size(); i++) {
        int errorCode = parseErrorCode(root["errors"][i]);
        if (errorCode != AI_COMMON_SUCCESS) {
            errorCount += 1;
        }
        if (errorCodes != nullptr && errorCodes[i] != nullptr) {
            *errorCodes[i] = errorCode;
        }
    }
    if (errorCount > 0) {
        if (errorCount == root["errors"].size()) {
            return KB_ERROR_DOC_BATCH_ALL_FAILED;
        }
        return KB_ERROR_DOC_BATCH_PARTIAL_FAILED;
    }
    return AI_COMMON_SUCCESS;
}

KBRetrievalResponse *KnowledgeBaseServiceProxy::processRetrievalResult(
    const std::string &jsonStr, int *errorCode) {
    printf("Retrieval result: %s\n", jsonStr.c_str());
    Json::Value root = util::stringToJson(jsonStr);
    const int code = parseErrorCode(root);
    setErrorCode(errorCode, code);
    if (code != AI_COMMON_SUCCESS) {
        return nullptr;
    }
    if (!root.isMember("data") || !root["data"].isObject()) {
        setErrorCode(errorCode, AI_COMMON_SERVICE_ERROR);
        return nullptr;
    }
    const Json::Value &data = root["data"];
    auto *response = new KBRetrievalResponse();
    response->errorCode = code;

    if (data.isMember("requestId")) {
        response->requestId = data["requestId"].asString();
    }
    if (data.isMember("query")) {
        response->query = data["query"].asString();
    } else if (data.isMember("queryText")) {
        response->query = data["queryText"].asString();
    }

    auto appendSources = [response](const Json::Value &array) {
        for (const auto &item : array) {
            auto *source = new KBChatSource();
            if (item.isMember("knowledgeBaseId")) {
                response->knowledgeBaseId = item["knowledgeBaseId"].asString();
            }
            if (item.isMember("fileId")) {
                source->fileId = item["fileId"].asString();
            }
            if (item.isMember("content")) {
                source->content = item["content"].asString();
            }
            if (item.isMember("similarity")) {
                source->confidence = item["similarity"].asDouble();
            } else if (item.isMember("score")) {
                source->confidence = item["score"].asDouble();
            }
            response->source.push_back(source);
        }
    };

    if (data.isMember("chunks") && data["chunks"].isArray()) {
        appendSources(data["chunks"]);
    } else if (data.isMember("results") && data["results"].isArray()) {
        appendSources(data["results"]);
    }

    return response;
}

int KnowledgeBaseServiceProxy::processStopResult(const std::string &jsonStr) {
    const Json::Value root = util::stringToJson(jsonStr);
    return parseErrorCode(root);
}
KnowledgeBaseInfo *KnowledgeBaseServiceProxy::processKnowledgeBaseInfoResult(
    const std::string &jsonStr, int *errorCode) {
    Json::Value root = util::stringToJson(jsonStr);
    const int code = parseErrorCode(root);
    if (code != AI_COMMON_SUCCESS || !root.isMember("data") ||
        !root["data"].isObject()) {
        setErrorCode(errorCode, code != AI_COMMON_SUCCESS
                                    ? code
                                    : AI_COMMON_SERVICE_ERROR);
        return nullptr;
    }

    return parseKnowledgeBaseInfo(root["data"]);
}

KnowledgeBaseInfo **
KnowledgeBaseServiceProxy::processKnowledgeBaseInfoListResult(
    const std::string &jsonStr, int *count, int *errorCode) {
    Json::Value root = util::stringToJson(jsonStr);
    int code = parseErrorCode(root);
    if (code != AI_COMMON_SUCCESS) {
        setErrorCode(errorCode, code);
        return nullptr;
    }
    if (!root.isMember("data") || !root["data"].isArray() ||
        root["data"].empty()) {
        setErrorCode(errorCode, AI_COMMON_SERVICE_ERROR);
        return nullptr;
    }
    *count = root["data"].size();
    auto knowledgeBaseInfoList = new KnowledgeBaseInfo *[*count];
    for (int i = 0; i < *count; i++) {
        knowledgeBaseInfoList[i] = parseKnowledgeBaseInfo(root["data"][i]);
    }
    return knowledgeBaseInfoList;
}

int KnowledgeBaseServiceProxy::processUpdateKnowledgeBaseResult(
    const std::string &nameStr, const std::string &descriptionStr) {
    // 应该返回哪个错误码
    Json::Value root = util::stringToJson(nameStr);
    int code = parseErrorCode(root);
    if (code != AI_COMMON_SUCCESS) {
        return code;
    }
    root = util::stringToJson(descriptionStr);
    code = parseErrorCode(root);
    if (code != AI_COMMON_SUCCESS) {
        return code;
    }
    return AI_COMMON_SUCCESS;
}

char **KnowledgeBaseServiceProxy::processListDocumentsResult(
    const std::string &jsonStr, int *count, int *errorCode) {
    Json::Value root = util::stringToJson(jsonStr);
    int code = parseErrorCode(root);
    if (code != AI_COMMON_SUCCESS) {
        setErrorCode(errorCode, code);
        return nullptr;
    }
    if (!root.isMember("data") || !root["data"].isMember("fileIds") ||
        !root["data"]["fileIds"].isArray()) {
        setErrorCode(errorCode, AI_COMMON_SERVICE_ERROR);
        return nullptr;
    }
    Json::Value &fileIds = root["data"]["fileIds"];
    *count = root["data"]["fileIds"].size();

    const auto fileIdArray = new char *[*count + 1];
    for (int i = 0; i < *count; ++i) {
        std::string fileId = fileIds[i].asString();
        fileIdArray[i] = new char[fileId.size() + 1];
        strcpy(fileIdArray[i], fileId.c_str());
    }
    fileIdArray[*count] = nullptr;

    setErrorCode(errorCode, AI_COMMON_SUCCESS);
    return fileIdArray;
}

int KnowledgeBaseServiceProxy::parseErrorCode(const Json::Value &json) {
    if (!json.isMember("err_no")) {
        printf("Missing 'err_no' in response\n");
        return AI_COMMON_SERVICE_ERROR;
    }
    const int errCode = json["err_no"].asInt();
    return getErrorCodeFromKnowledgebase(errCode);
}

void KnowledgeBaseServiceProxy::setErrorCode(int *errorCode, int code) {
    if (errorCode) {
        *errorCode = code;
    }
}

KnowledgeBaseInfo *KnowledgeBaseServiceProxy::parseKnowledgeBaseInfo(
    const Json::Value &json) {
    auto *info = new KnowledgeBaseInfo();
    if (json.isMember("name")) {
        info->name = json["name"].asString();
    }
    if (json.isMember("description")) {
        info->description = json["description"].asString();
    }
    if (json.isMember("knowledgeBaseId")) {
        info->knowledgeBaseId = json["knowledgeBaseId"].asString();
    }
    if (json.isMember("indexType")) {
        info->indexType = json["indexType"].asString();
    }
    if (json.isMember("type")) {
        info->kbType = json["type"].asString();
    }
    return info;
}

void KnowledgeBaseServiceProxy::onUploadDocumentCallback(
    GDBusConnection *connection, const gchar *senderName,
    const gchar *objectPath, const gchar *interfaceName,
    const gchar *signalName, GVariant *parameters, gpointer userData) {
    auto *proxy = static_cast<KnowledgeBaseServiceProxy *>(userData);

    const auto result = dissectUploadDocumentResult(parameters);
}

void KnowledgeBaseServiceProxy::onKBRetrievalFinishedCallback(
    GObject *sourceObject, GAsyncResult *res, gpointer userData) {
    auto *proxy = static_cast<KnowledgeBaseServiceProxy *>(userData);

    GError *error = nullptr;
    gchar *result = nullptr;
    gboolean success =
        ai_business_knowledge_base_service_call_retrieval_knowledge_base_finish(
            AI_BUSINESS_KNOWLEDGE_BASE_SERVICE(sourceObject), &result, res,
            &error);

    if (!success) {
        if (error) {
            g_printerr("Error finishing retrieval: %s\n", error->message);
            g_error_free(error);
        }
        if (result) {
            g_free(result);
        }
        KBRetrievalResponse response;
        response.errorCode = AI_COMMON_SERVER_CONNECTION_FAILED;
        auto it =
            proxy->retrievalCallbacks_.find(proxy->currentRetrievalRequestId_);
        if (it == proxy->retrievalCallbacks_.end()) {
            return;
        }
        auto callbackInfo = it->second;
        callbackInfo.callback(&response, callbackInfo.userData);
        proxy->removeRetrievalCallback(proxy->currentRetrievalRequestId_);
        return;
    }

    int parseError = AI_COMMON_SUCCESS;
    std::unique_ptr<KBRetrievalResponse> responsePtr(
        proxy->processRetrievalResult(result, &parseError));
    g_free(result);

    if (responsePtr == nullptr) {
        KBRetrievalResponse response;
        response.errorCode = parseError;
        auto it =
            proxy->retrievalCallbacks_.find(proxy->currentRetrievalRequestId_);
        if (it == proxy->retrievalCallbacks_.end()) {
            return;
        }
        auto callbackInfo = it->second;
        callbackInfo.callback(&response, callbackInfo.userData);
    } else {
        auto it = proxy->retrievalCallbacks_.find(responsePtr.get()->requestId);
        if (it == proxy->retrievalCallbacks_.end()) {
            return;
        }
        auto callbackInfo = it->second;
        callbackInfo.callback(responsePtr.get(), callbackInfo.userData);
        proxy->currentRetrievalRequestId_ = responsePtr.get()->requestId;
    }
    proxy->removeRetrievalCallback(proxy->currentRetrievalRequestId_);
    if (!proxy->pendingRetrievalRequests_.empty()) {
        proxy->pendingRetrievalRequests_.pop();
    }

    // 设置下一个待处理请求
    if (!proxy->pendingRetrievalRequests_.empty()) {
        proxy->currentRetrievalRequestId_ =
            proxy->pendingRetrievalRequests_.front();
    } else {
        proxy->currentRetrievalRequestId_.clear();
    }
}

void KnowledgeBaseServiceProxy::onChatResultCallback(
    GDBusConnection *connection, const gchar *senderName,
    const gchar *objectPath, const gchar *interfaceName,
    const gchar *signalName, GVariant *parameters, gpointer userData) {
    auto *proxy = static_cast<KnowledgeBaseServiceProxy *>(userData);
    std::lock_guard lock(proxy->chatCallbackMutex_);
    auto result = dissectChatResult(parameters);
    // 根据 requestId 查找对应的回调和用户数据
    auto it = proxy->chatCallbacks_.find(result.requestId);
    if (it != proxy->chatCallbacks_.end()) {
        auto &callbackInfo = it->second;
        callbackInfo.callback(&result, callbackInfo.userData);
        proxy->currentChatRequestId_ = result.requestId;

        if (result.result && result.result->isFinished) {
            printf("Chat finished by result, removeChatCallback\n");
            proxy->removeChatCallback(result.requestId);

            if (!proxy->pendingChatRequests_.empty()) {
                proxy->pendingChatRequests_.pop();
                if (!proxy->pendingChatRequests_.empty()) {
                    proxy->currentChatRequestId_ =
                        proxy->pendingChatRequests_.front();
                } else {
                    proxy->currentChatRequestId_.clear();
                }
            }
        }
    } else {
        printf("No callback found for request ID: %s\n",
               result.requestId.c_str());
    }
}

void KnowledgeBaseServiceProxy::connectUploadDocumentResultSignals() {
    auto *connection = KnowledgeBaseServer::getInstance().getConnection();
    const std::string interfaceName = std::string(interface);
    uploadSubscriptionId_ = g_dbus_connection_signal_subscribe(
        connection, nullptr, interfaceName.c_str(), uploadDocumentResultSignal,
        objectPath, nullptr, G_DBUS_SIGNAL_FLAGS_NONE, onUploadDocumentCallback,
        this, nullptr);
}

void KnowledgeBaseServiceProxy::connectChatResultSignals(int sessionId) {
    auto *connection = KnowledgeBaseServer::getInstance().getConnection();
    std::string interfaceName =
        std::string(interface) + "_" + std::to_string(sessionId);
    chatSubscriptionId_ = g_dbus_connection_signal_subscribe(
        connection, nullptr, interfaceName.c_str(), chatResultSignal,
        objectPath, nullptr, G_DBUS_SIGNAL_FLAGS_NONE, onChatResultCallback,
        this, nullptr);
}

std::string KnowledgeBaseServiceProxy::buildChatHistoryJsonString(
    const std::vector<ChatTurn> &chatTurns) {
    std::vector<std::map<std::string, std::string>> historyVector;
    historyVector.reserve(chatTurns.size() * 2);  // 预分配空间提高性能
    for (const auto &chatTurn : chatTurns) {
        // 添加用户消息
        if (!chatTurn.question.empty()) {
            std::map<std::string, std::string> userEntry;
            userEntry["role"] = "user";
            userEntry["content"] = chatTurn.question;
            historyVector.push_back(std::move(userEntry));
        }

        // 添加助手消息
        if (!chatTurn.response.empty()) {
            std::map<std::string, std::string> assistantEntry;
            assistantEntry["role"] = "assistant";
            assistantEntry["content"] = chatTurn.response;
            historyVector.push_back(std::move(assistantEntry));
        }
    }
    return util::buildJsonArray(historyVector);
}

bool KnowledgeBaseServiceProxy::isConnectionActive() {
    return KnowledgeBaseServer::getInstance().isConnectionActive();
}

}  // namespace kyai::advancedai::knowledgebase
