/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "knowledgebaseproxy.h"

#include <map>
#include <utility>

#include "assistant/error.h"
#include "utils/logger.h"
#include "utils/util.h"
namespace advanced::knowledgebase {
const char* objectPath = "/com/kylin/AiBusiness/KnowledgeBaseService";
const char* interface = "com.kylinos.aisdk.business.KnowledgeBaseService";
const std::string serverUnixPath =
    "unix:abstract=/tmp/.kylin-ai-business-unix/" + std::to_string(getuid()) + "/KnowledgeBaseService.sock";
const std::string appId = "kylin-ai-knowledge-base-service";
const std::string password = "k6bf1d_349ecd";
const std::string databasePath = "/usr/share/kylin-ai-runtime/knowledge-base/";
const char* chatResultSignal = "StreamingChatResponse";

KBChatSource dissectChatSource(const Json::Value& dataJson) {
    KBChatSource sourceItem;
    if (dataJson.isMember("metaData")) {
        auto metaDataJson = util::stringToJson(dataJson["metaData"].asString());
        if (metaDataJson.isMember("filePath")) {
            sourceItem.filePath = metaDataJson["filePath"].asString();
        }
        if (metaDataJson.isMember("toolIndex")) {
            sourceItem.toolIndex = metaDataJson["toolIndex"].asInt();
        }
        if (metaDataJson.isMember("toolPath")) {
            sourceItem.toolPath = metaDataJson["toolPath"].asString();
        }
    }
    sourceItem.fileContent = dataJson["content"].asString();
    sourceItem.confidence = static_cast<double>(dataJson["similarity"].asFloat());
    return sourceItem;
}

KBChatResponse dissectChatResult(GVariant* parameters) {
    KBChatResponse result{};
    GVariantIter iter;
    g_variant_iter_init(&iter, parameters);

    GVariant* response = g_variant_iter_next_value(&iter);

    if (response == nullptr) {
        LOGD("[KnowledgeBase] Chat result error: result is nullptr!");
        g_variant_unref(response);
        return result;
    }

    const std::string responseStr = g_variant_get_string(response, nullptr);
    LOGD("[KnowledgeBase] Chat response received {}", responseStr.c_str());
    Json::Value responseJson = util::stringToJson(responseStr);

    if (!responseJson.isMember("data") || !responseJson.isMember("err_no") ||
        !responseJson["data"].isMember("chatId") || !responseJson["data"].isMember("requestId") ||
        !responseJson["data"].isMember("isFinal") || !responseJson["data"].isMember("result") ||
        !responseJson["data"].isMember("chatModel")) {
        LOGD("[KnowledgeBase] No available result {}", responseStr.c_str());
        g_variant_unref(response);
        return result;
    }
    result.errorCode = responseJson["err_no"].asInt();
    result.isFinished = responseJson["data"]["isFinal"].asBool();
    result.reasoningContent = responseJson["data"]["reasoningMsg"].asString();
    result.response = responseJson["data"]["result"].asString();
    result.model = responseJson["data"]["chatModel"].asString();

    if (result.isFinished) {
        if (responseJson["data"].isMember("chunks")) {
            Json::Value chunksJson = responseJson["data"]["chunks"];
            for (const auto& dataJson : chunksJson) {
                KBChatSource sourceItem = dissectChatSource(dataJson);
                result.sources.push_back(sourceItem);
            }
        }
    }
    g_variant_unref(response);
    return result;
}

KnowledgeBaseProxy& KnowledgeBaseProxy::instance() {
    static KnowledgeBaseProxy instance;
    return instance;
}

KnowledgeBaseProxy::KnowledgeBaseProxy() = default;

int KnowledgeBaseProxy::init() {
    if (!initServer(serverUnixPath)) {
        return SERVER_ERROR;
    }
    int errorCode = initSession();
    if (errorCode != 0) {
        return errorCode;
    }
    connectChatResultSignals();
    return SUCCESS;
}

bool KnowledgeBaseProxy::initServer(const std::string& unixPath) {
    GError* error{nullptr};
    if (connection_) {
        g_object_unref(connection_);
        connection_ = nullptr;
    }
    connection_ =
        g_dbus_connection_new_for_address_sync(unixPath.c_str(), G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT,
                                               nullptr,  // GDBusAuthObserver
                                               nullptr,  // GCancellable
                                               &error);
    if (connection_ == nullptr) {
        g_printerr("Error connecting to D-Bus address %s: %s\n", unixPath.c_str(), error->message);
        g_error_free(error);
        return false;
    }

    if (proxy_) {
        g_object_unref(proxy_);
        proxy_ = nullptr;
    }

    proxy_ = ai_business_knowledge_base_service_proxy_new_sync(connection_, G_DBUS_PROXY_FLAGS_NONE, nullptr,
                                                               objectPath, nullptr, &error);
    if (proxy_ == nullptr) {
        if (error) {
            g_printerr("Error creating text processor proxy %s: %s\n", objectPath, error->message);
            g_error_free(error);
        }
        return false;
    }

    g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(proxy_), 2 * 60 * 60 * 1000);
    return true;
}

int KnowledgeBaseProxy::initSession() {
    Json::Value paramJson;
    paramJson["appId"] = appId;
    paramJson["password"] = password;
    paramJson["dbFilePath"] = databasePath;
    const std::string paramString = util::jsonToString(paramJson);

    GError* error = nullptr;
    gchar* result;
    const gboolean success = ai_business_knowledge_base_service_call_init_knowledge_base_session_sync(
        proxy_, paramString.c_str(), &result, nullptr, &error);
    if (!success) {
        if (error) {
            g_printerr("Error init service: %s\n", error->message);
            g_error_free(error);
        }
        sessionId_ = -1;
        return SERVER_ERROR;
    }
    Json::Value resultJson = util::stringToJson(result);
    g_free(result);
    int code = parseErrorCode(resultJson);
    if (code != 0) {
        sessionId_ = -1;
        return code;
    }
    if (resultJson.isMember("data") && resultJson["data"].isMember("sessionId")) {
        sessionId_ = resultJson["data"]["sessionId"].asInt();
        LOGD("[KnowledgeBase] knowledge base session id: {}", sessionId_);
    }
    return SUCCESS;
}

void KnowledgeBaseProxy::chat(const std::string& question, const std::string& model, KBChatStreamCallback callback) {
    LOGD("[KnowledgeBase] Chat");
    chatResultCallback_ = std::move(callback);
    chatRequestId_ = std::to_string(std::rand());
    const std::string paramString = buildChatJsonString(question, model);
    ai_business_knowledge_base_service_call_chat_with_knowledge_base(proxy_, paramString.c_str(), nullptr, nullptr,
                                                                     this);
}

void KnowledgeBaseProxy::stopChat() {
    if (chatRequestId_.empty() || sessionId_ == -1) {
        return;
    }
    Json::Value paramJson;
    paramJson["sessionId"] = sessionId_;
    paramJson["requestId"] = chatRequestId_;
    const std::string paramString = util::jsonToString(paramJson);

    GError* error = nullptr;
    gboolean success =
        ai_business_knowledge_base_service_call_stop_chat_sync(proxy_, paramString.c_str(), nullptr, nullptr, &error);
    LOGD("[KnowledgeBase] Stop chat");
    if (!success) {
        if (error) {
            g_printerr("Error stopping chat: %s\n", error->message);
            g_error_free(error);
        }
    }
    chatRequestId_ = "";
}

std::string KnowledgeBaseProxy::buildChatJsonString(const std::string& question, const std::string& model) const {
    Json::Value configJson;
    configJson["modelName"] = model;
    configJson["expandContext"] = true;
    configJson["newChat"] = true;
    configJson["topK"] = 4;  // 与之前的用户手册保持一致

    Json::Value paramJson;
    paramJson["knowledgeBaseId"] = "";  // 不指定数据库
    paramJson["sessionId"] = sessionId_;
    paramJson["appId"] = appId;
    paramJson["password"] = password;
    paramJson["requestId"] = chatRequestId_;
    paramJson["query"] = question;
    paramJson["config"] = configJson;

    return util::jsonToString(paramJson);
}

void KnowledgeBaseProxy::connectChatResultSignals() {
    std::string interfaceName = fmt::format("{}_{}", interface, sessionId_);
    chatSubscriptionId_ =
        g_dbus_connection_signal_subscribe(connection_, nullptr, interfaceName.c_str(), chatResultSignal, objectPath,
                                           nullptr, G_DBUS_SIGNAL_FLAGS_NONE, onChatResultCallback, this, nullptr);
}

void KnowledgeBaseProxy::onChatResultCallback(GDBusConnection* connection, const gchar* senderName,
                                              const gchar* objectPath, const gchar* interfaceName,
                                              const gchar* signalName, GVariant* parameters, gpointer userData) {
    const auto* proxy = static_cast<KnowledgeBaseProxy*>(userData);
    LOGD("[KnowledgeBase] Chat result callback");
    if (proxy->chatResultCallback_ != nullptr) {
        auto result = dissectChatResult(parameters);
        proxy->chatResultCallback_(result);
    } else {
        LOGD("[KnowledgeBase] Chat result callback is not set");
    }
}

KnowledgeBaseProxy::~KnowledgeBaseProxy() {
    if (sessionId_ != -1) {
        destroyKBSession();
    }
    if (not connection_) {
        g_printerr(
            "Error creating core advanced ai server proxy: Server proxy "
            "connection is unavailable.\n");
        return;
    }
    if (proxy_) {
        g_object_unref(proxy_);
    }
    if (chatSubscriptionId_ != 0) {
        // 取消订阅
        g_dbus_connection_signal_unsubscribe(connection_, chatSubscriptionId_);
    }
    g_object_unref(connection_);
}

void KnowledgeBaseProxy::destroyKBSession() const { destroy(sessionId_); }

void KnowledgeBaseProxy::destroy(int sessionId) const {
    Json::Value paramJson;
    paramJson["sessionId"] = sessionId;
    const std::string paramString = util::jsonToString(paramJson);

    GError* error = nullptr;
    gchar* result;
    const gboolean success = ai_business_knowledge_base_service_call_destroy_knowledge_base_session_sync(
        proxy_, paramString.c_str(), &result, nullptr, &error);
    if (!success) {
        if (error) {
            g_printerr("Error init service: %s\n", error->message);
            g_error_free(error);
        }
    }
}

int KnowledgeBaseProxy::parseErrorCode(const Json::Value& json) {
    if (!json.isMember("err_no")) {
        LOGD("[KnowledgeBase] Missing 'err_no' in response");
        return -1;
    }
    const int errCode = json["err_no"].asInt();
    return errCode;
}

}  // namespace advanced::knowledgebase
