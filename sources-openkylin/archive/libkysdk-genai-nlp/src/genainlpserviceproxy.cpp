/*
 * Copyright 2025 KylinSoft Co., Ltd.
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

#include "genainlpserviceproxy.h"

#include <chrono>
#include <string>
#include <thread>

#include "_chatmodelconfig.h"
#include "_chatresult.h"
#include "_toolchoice.h"
#include "logger.h"

namespace kyai::genai::nlp {

const char* objectPath = "/com/kylin/AiRuntime/GenAiNlp";
const char* interface = "com.kylin.AiRuntime.GenAiNlp";

const char* nlpResultSignal = "ChatNlpResult";
const char* toolHandlerResultSignal = "ToolHandlerResult";
const int chatResultCallbackTimeoutMs = 60 * 60 * 1000;  // 1 hour timeout

namespace {

ChatResult dissectChatResult(GVariant* parameters) {
    GVariantIter iter;
    g_variant_iter_init(&iter, parameters);

    GVariant* reason = g_variant_iter_next_value(&iter);
    GVariant* finishReason = g_variant_iter_next_value(&iter);
    GVariant* errorCode = g_variant_iter_next_value(&iter);
    GVariant* errorMessage = g_variant_iter_next_value(&iter);
    GVariant* isEnd = g_variant_iter_next_value(&iter);
    GVariant* reasoningContent = g_variant_iter_next_value(&iter);
    GVariant* modelName = g_variant_iter_next_value(&iter);
    GVariant* searchInfo = g_variant_iter_next_value(&iter);
    GVariant* requestId = g_variant_iter_next_value(&iter);

    if (reason == nullptr or reasoningContent == nullptr or finishReason == nullptr or errorCode == nullptr or
        errorMessage == nullptr or isEnd == nullptr or modelName == nullptr or searchInfo == nullptr) {
        LOGE("chat result error: result is nullptr!");
        if (reason) g_variant_unref(reason);
        if (reasoningContent) g_variant_unref(reasoningContent);
        if (finishReason) g_variant_unref(finishReason);
        if (errorCode) g_variant_unref(errorCode);
        if (errorMessage) g_variant_unref(errorMessage);
        if (isEnd) g_variant_unref(isEnd);
        if (modelName) g_variant_unref(modelName);
        if (searchInfo) g_variant_unref(searchInfo);
        if (requestId) g_variant_unref(requestId);
        return {};
    }

    const char* resultReason = static_cast<const char*>(g_variant_get_data(reason));
    const char* resultReasoningContent = static_cast<const char*>(g_variant_get_data(reasoningContent));
    const char* resultFinishReason = static_cast<const char*>(g_variant_get_data(finishReason));
    const int* resultErrorCode = static_cast<const int*>(g_variant_get_data(errorCode));
    const char* resultErrorMessage = static_cast<const char*>(g_variant_get_data(errorMessage));
    const bool* resultIsEnd = static_cast<const bool*>(g_variant_get_data(isEnd));
    const char* resultModelName = static_cast<const char*>(g_variant_get_data(modelName));
    const char* resultSearchInfo = static_cast<const char*>(g_variant_get_data(searchInfo));
    const char* resultRequestId = "";
    if (requestId != nullptr) {
        const char* data = static_cast<const char*>(g_variant_get_data(requestId));
        if (data != nullptr) {
            resultRequestId = data;
        }
    }

    if (resultReason == nullptr or resultReasoningContent == nullptr or resultFinishReason == nullptr or
        resultErrorCode == nullptr or resultErrorMessage == nullptr or resultIsEnd == nullptr or
        resultModelName == nullptr or resultSearchInfo == nullptr) {
        LOGE("chat result error: result is nullptr!");
        if (reason) g_variant_unref(reason);
        if (reasoningContent) g_variant_unref(reasoningContent);
        if (finishReason) g_variant_unref(finishReason);
        if (errorCode) g_variant_unref(errorCode);
        if (errorMessage) g_variant_unref(errorMessage);
        if (isEnd) g_variant_unref(isEnd);
        if (modelName) g_variant_unref(modelName);
        if (searchInfo) g_variant_unref(searchInfo);
        if (requestId) g_variant_unref(requestId);
        return {};
    }

    ChatResult result{resultReason,     resultReasoningContent, resultFinishReason, resultModelName,
                      *resultErrorCode, resultErrorMessage,     *resultIsEnd};
    result.requestId_ = resultRequestId;

    _ChatResultSearchInfo chatResultSearchInfo(resultSearchInfo);
    result.searchInfo = chatResultSearchInfo;

    g_variant_unref(reason);
    g_variant_unref(reasoningContent);
    g_variant_unref(finishReason);
    g_variant_unref(errorCode);
    g_variant_unref(errorMessage);
    g_variant_unref(isEnd);
    g_variant_unref(modelName);
    g_variant_unref(searchInfo);
    if (requestId) g_variant_unref(requestId);

    return result;
}

void dissectToolHandlerResult(GVariant* parameters, std::string& toolName) {
    GVariantIter iter;
    g_variant_iter_init(&iter, parameters);

    GVariant* tools = g_variant_iter_next_value(&iter);

    if (tools == nullptr) {
        LOGE("tool handler result error: result is nullptr!");
        if (tools) {
            g_variant_unref(tools);
        }
    }

    toolName = static_cast<const char*>(g_variant_get_data(tools));
    g_variant_unref(tools);
}

std::string jsonToString(const Json::Value& value) {
    Json::StreamWriterBuilder builder;
    builder["emitUTF8"] = true;
    return Json::writeString(builder, value);
}

Json::Value stringToJson(const std::string& string) {
    Json::Value value;
    if (Json::Reader reader; reader.parse(string, value)) {
        return value;
    } else {
        return Json::objectValue;
    }
}

void setErrorCode(int* pErrorCode, int errorCode) {
    if (pErrorCode != nullptr) {
        *pErrorCode = errorCode;
    }
}

}  // namespace

GenAiNlpServiceProxy::~GenAiNlpServiceProxy() {
    internalLoopRequested_.store(false, std::memory_order_release);
    stopInternalEventLoop();
    if (not GenAiNlpServer::getInstance().available()) {
        g_printerr(
            "Error creating core speech server proxy: Server proxy "
            "connection is unavailable.\n");
        return;
    }

    auto* connection = GenAiNlpServer::getInstance().getConnection();
    if (subscriptionId_ != 0) {
        // 取消订阅
        g_dbus_connection_signal_unsubscribe(connection, subscriptionId_);
    }

    removeCallbackTimerIfExists();
}

bool GenAiNlpServiceProxy::connectToServer() {
    GenAiNlpServer::getInstance().init();
    if (not GenAiNlpServer::getInstance().available()) {
        g_printerr(
            "Error creating core speech server proxy: Server proxy "
            "connection is unavailable.\n");
        return false;
    }

    if (proxy_) {
        g_object_unref(proxy_);
        proxy_ = nullptr;
    }

    GError* error{nullptr};
    auto* connection = GenAiNlpServer::getInstance().getConnection();
    proxy_ = ai_runtime_gen_ai_nlp_service_proxy_new_sync(connection, G_DBUS_PROXY_FLAGS_NONE, nullptr, objectPath,
                                                          nullptr, &error);
    if (proxy_ == nullptr) {
        if (error) {
            g_printerr("Error creating text processor proxy %s: %s\n", objectPath, error->message);
            g_error_free(error);
        }

        return false;
    }

    g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(proxy_), 2 * 60 * 60 * 1000);
    refreshRuntimeProtocolVersion();

    return true;
}
int GenAiNlpServiceProxy::initEngine(const ChatModelConfig& chatModelConfig, int* sessionId) {
    Json::Value chatModelConfigJson = modelConfigToJson(chatModelConfig);
    std::string config = jsonToString(chatModelConfigJson);

    int errorCode = AISDK_NO_ERROR;
    if (!ensureProtocolSupportedForApi("init", &errorCode)) {
        if (sessionId) {
            *sessionId = -1;
        }
        return errorCode;
    }

    GError* error = nullptr;
    bool success =
        ai_runtime_gen_ai_nlp_service_call_init_sync(proxy_, config.c_str(), sessionId, &errorCode, nullptr, &error);

    if (not success) {
        if (error) {
            LOGE("Error calling init: {}:{}", error->code, error->message);
            g_error_free(error);
            return AISDK_RUNTIME_ERROR;
        }
    }

    if (*sessionId != -1) {
        connectChatResultSignals(*sessionId);
        connectToolHandlerResultSignals(*sessionId);
        reconnectAttempts_ = 0;
    }
    return errorCode;
}

void GenAiNlpServiceProxy::initEngineAsync(const ChatModelConfig& chatModelConfig, const InitCallback& callback,
                                           void* userData) {
    initCallback_ = callback;
    initUserData_ = userData;
    Json::Value chatModelConfigJson = modelConfigToJson(chatModelConfig);
    std::string config = jsonToString(chatModelConfigJson);
    ai_runtime_gen_ai_nlp_service_call_init(proxy_, config.c_str(), nullptr, onInitFinished, this);
}

void GenAiNlpServiceProxy::destroyEngine(int sessionId) {
    if (!ensureProtocolSupportedForApi("destroy", nullptr)) {
        return;
    }

    ai_runtime_gen_ai_nlp_service_call_destroy_sync(proxy_, sessionId, nullptr, nullptr);
    reconnectAttempts_ = 0;
}

void GenAiNlpServiceProxy::setChatResultCallback(const ChatResultCallback& callback, void* userData) {
    chatResultCallback_ = callback;
    userData_ = userData;
}

void GenAiNlpServiceProxy::setToolHandler(const GenAiToolHandler& toolHandler, void* toolHandlerUserData) {
    toolHandlerResultCallback_ = toolHandler;
    toolHandlerUserData_ = toolHandlerUserData;
}

void GenAiNlpServiceProxy::setReconnectedCallback(const ReconnectedCallback& callback) {
    reconnectedCallback_ = callback;
}

void GenAiNlpServiceProxy::chat(int sessionId, const ChatModelConfig& chatModelConfig,
                                const GenAiToolChoiceConfig& toolChoiceConfig, const std::string& question) {
    if (!ensureProtocolSupportedForApi("chat", nullptr)) {
        return;
    }

    if (question.empty()) {
        handleMessagesEmptyError();
        return;
    }
    reconnectAttempts_ = 0;
    currentSessionId_ = sessionId;

    // Start the timeout timer
    startCallbackTimer();
    Json::Value request = modelConfigToJson(chatModelConfig);
    request["question"] = question;
    request["tool_choice"] = getToolChoiceConfig(toolChoiceConfig);
    std::string temp = jsonToString(request);
    ai_runtime_gen_ai_nlp_service_call_chat(proxy_, sessionId, temp.c_str(), nullptr, onChatFinishedCallback, this);
}

int GenAiNlpServiceProxy::chatWithRequestId(int sessionId, const ChatModelConfig& chatModelConfig,
                                            const std::string& question, const std::string& requestId) {
    int errorCode = AI_COMMON_SUCCESS;
    if (!ensureProtocolSupportedForApi("chat_with_request_id", &errorCode)) {
        return errorCode;
    }

    if (question.empty()) {
        handleMessagesEmptyError();
        return AI_COMMON_NLP_PARAM_ERROR;
    }
    reconnectAttempts_ = 0;
    currentSessionId_ = sessionId;

    // Start the timeout timer
    startCallbackTimer();
    Json::Value request = modelConfigToJson(chatModelConfig);
    request["question"] = question;
    if (!requestId.empty()) {
        request["request_id"] = requestId;
    }
    std::string temp = jsonToString(request);
    ai_runtime_gen_ai_nlp_service_call_chat(proxy_, sessionId, temp.c_str(), nullptr, onChatFinishedCallback, this);
    return AI_COMMON_SUCCESS;
}

void GenAiNlpServiceProxy::generateContent(int sessionId, const ChatModelConfig& chatModelConfig,
                                           const std::string& prompt) {
    if (prompt.empty()) {
        handleMessagesEmptyError();
        return;
    }
    reconnectAttempts_ = 0;
    currentSessionId_ = sessionId;

    // Start the timeout timer
    startCallbackTimer();
    Json::Value request = modelConfigToJson(chatModelConfig);
    request["prompt"] = prompt;
    std::string temp = jsonToString(request);
    ai_runtime_gen_ai_nlp_service_call_generate_content(proxy_, sessionId, temp.c_str(), nullptr,
                                                        onGenerateContentFinishedCallback, this);
}

void GenAiNlpServiceProxy::handleErrorOccurred(int errorCode) {
    LOGE("Error occurred with code:", errorCode);

    removeCallbackTimerIfExists();

    switch (errorCode) {
        case G_IO_ERROR_CLOSED:
            handleServerClosed();
            break;
        case G_IO_ERROR_TIMED_OUT:
            handleChatTimeout();
            break;
        default:
            handleGeneralError();
            break;
    }
}

void GenAiNlpServiceProxy::handleServerClosed() {
    LOGE("Server closed unexpectedly.");
    if (chatResultCallback_) {
        ChatResult result{"", "", "", "", AISDK_RUNTIME_ERROR, "Server closed unexpectedly.", true};
        chatResultCallback_(&result, userData_);
    }

    if (reconnectServer()) {
        if (reconnectedCallback_) {
            reconnectedCallback_();
        }
    }
}

void GenAiNlpServiceProxy::handleChatTimeout() {
    if (chatResultCallback_) {
        ChatResult result{"", "", "", "", AISDK_MODEL_RUN_TIME_OUT, "Run model time out.", true};
        chatResultCallback_(&result, userData_);
    }
}

void GenAiNlpServiceProxy::handleGeneralError() {
    if (chatResultCallback_) {
        ChatResult result{"", "", "", "", AISDK_RUNTIME_ERROR, "General error occurred.", true};
        chatResultCallback_(&result, userData_);
    }
}

void GenAiNlpServiceProxy::handleMessagesEmptyError() {
    if (chatResultCallback_) {
        ChatResult result{"", "", "", "", NLP_PARAM_ERROR, "The input message is empty.", true};
        chatResultCallback_(&result, userData_);
    };
}

bool GenAiNlpServiceProxy::reconnectServer() {
    if (reconnectAttempts_ >= maxReconnectAttempts) {
        LOGE("Max reconnect attempts reached");
        return false;
    }

    reconnectAttempts_++;
    LOGD("Reconnect attempt {}.", reconnectAttempts_);

    if (!connectToServer()) {
        LOGE("Failed to reconnect to server");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return reconnectServer();
    }

    LOGI("Successfully reconnected to server.");

    return true;
}

void GenAiNlpServiceProxy::stopChat(int sessionId) {
    if (!ensureProtocolSupportedForApi("stop_chat", nullptr)) {
        return;
    }

    ai_runtime_gen_ai_nlp_service_call_stop_chat_sync(proxy_, sessionId, nullptr, nullptr);
}

int GenAiNlpServiceProxy::stopChatWithRequestId(int sessionId, const std::string& requestId) {
    int errorCode = AI_COMMON_SUCCESS;
    if (!ensureProtocolSupportedForApi("stop_chat_with_request_id", &errorCode)) {
        return errorCode;
    }

    if (requestId.empty()) {
        stopChat(sessionId);
        return AI_COMMON_SUCCESS;
    }

    ai_runtime_gen_ai_nlp_service_call_stop_chat_with_request_id_sync(proxy_, sessionId, requestId.c_str(), nullptr,
                                                                      nullptr);
    return AI_COMMON_SUCCESS;
}

void GenAiNlpServiceProxy::clearChatMessage(int sessionId) {
    if (!ensureProtocolSupportedForApi("clear_chat_message", nullptr)) {
        return;
    }

    ai_runtime_gen_ai_nlp_service_call_clear_chat_message_sync(proxy_, sessionId, nullptr, nullptr);
}

void GenAiNlpServiceProxy::setChatSystemPromptId(int sessionId, PromptId promptId) {
    if (!ensureProtocolSupportedForApi("set_chat_system_prompt_id", nullptr)) {
        return;
    }

    ai_runtime_gen_ai_nlp_service_call_set_chat_system_prompt_id_sync(proxy_, sessionId, promptId, nullptr, nullptr);
}

void GenAiNlpServiceProxy::addSystemMessage(int sessionId, const std::string& message) {
    if (!ensureProtocolSupportedForApi("add_system_message", nullptr)) {
        return;
    }

    ai_runtime_gen_ai_nlp_service_call_add_system_message_sync(proxy_, sessionId, message.c_str(), nullptr, nullptr);
}

void GenAiNlpServiceProxy::chatWithHistoryMessages(int sessionId, const ChatModelConfig& chatModelConfig,
                                                   const GenAiToolChoiceConfig& toolChoiceConfig,
                                                   const std::string& messages) {
    if (!ensureProtocolSupportedForApi("chat_history", nullptr)) {
        return;
    }

    if (messages.empty()) {
        handleMessagesEmptyError();
        return;
    }
    currentSessionId_ = sessionId;
    // Start the timeout timer
    startCallbackTimer();
    Json::Value request = modelConfigToJson(chatModelConfig);
    Json::Value messagesJson = stringToJson(messages);
    request["messages"] = messagesJson["messages"];
    request["tool_choice"] = getToolChoiceConfig(toolChoiceConfig);
    std::string temp = jsonToString(request);
    LOGD(temp);

    ai_runtime_gen_ai_nlp_service_call_chat_with_history_messages(proxy_, sessionId, temp.c_str(), nullptr,
                                                                  onChatFinishedCallback, this);
}

int GenAiNlpServiceProxy::chatWithHistoryMessagesWithRequestId(int sessionId, const ChatModelConfig& chatModelConfig,
                                                               const GenAiToolChoiceConfig& toolChoiceConfig,
                                                               const std::string& messages,
                                                               const std::string& requestId) {
    int errorCode = AI_COMMON_SUCCESS;
    if (!ensureProtocolSupportedForApi("chat_history_with_request_id", &errorCode)) {
        return errorCode;
    }

    if (messages.empty()) {
        handleMessagesEmptyError();
        return AI_COMMON_NLP_PARAM_ERROR;
    }
    currentSessionId_ = sessionId;
    // Start the timeout timer
    startCallbackTimer();
    Json::Value request = modelConfigToJson(chatModelConfig);
    Json::Value messagesJson = stringToJson(messages);
    request["messages"] = messagesJson["messages"];
    request["tool_choice"] = getToolChoiceConfig(toolChoiceConfig);
    if (!requestId.empty()) {
        request["request_id"] = requestId;
    }
    std::string temp = jsonToString(request);
    LOGD(temp);

    ai_runtime_gen_ai_nlp_service_call_chat_with_history_messages(proxy_, sessionId, temp.c_str(), nullptr,
                                                                  onChatFinishedCallback, this);
    return AI_COMMON_SUCCESS;
}

TextModelList GenAiNlpServiceProxy::getModelList(int sessionId, int* errorCode) {
    gchar* modelInfos{nullptr};
    GError* error = nullptr;
    ai_runtime_gen_ai_nlp_service_call_get_nlp_model_list_sync(proxy_, sessionId, &modelInfos, nullptr, &error);
    if (error != nullptr) {
        bool dbusError = error->domain == g_dbus_error_quark();
        bool unknownMethod = error->code == G_DBUS_ERROR_UNKNOWN_METHOD;
        if (dbusError && unknownMethod) {
            setErrorCode(errorCode, AI_COMMON_RUNTIME_OUTDATED);
        } else {
            setErrorCode(errorCode, AISDK_RUNTIME_ERROR);
        }
        return TextModelList{};
    }
    Json::Value jsonModelInfos = stringToJson(modelInfos);
    TextModelList textModelList(jsonModelInfos);
    return textModelList;
}

void GenAiNlpServiceProxy::enableInternalEventLoop(bool enable) {
    if (enable) {
        bool expected = false;
        if (!internalLoopRequested_.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
            return;
        }
        startInternalEventLoop();
        if (eventLoopContext_ == nullptr) {
            internalLoopRequested_.store(false, std::memory_order_release);
        }
    } else {
        bool expected = true;
        if (!internalLoopRequested_.compare_exchange_strong(expected, false, std::memory_order_acq_rel)) {
            return;
        }
        stopInternalEventLoop();
    }
}

void GenAiNlpServiceProxy::startInternalEventLoop() {
    if (eventLoopThread_.joinable()) {
        return;
    }

    eventLoopContext_ = g_main_context_default();
    if (eventLoopContext_ == nullptr) {
        return;
    }

    g_main_context_ref(eventLoopContext_);
    try {
        eventLoopThread_ = std::thread([this]() {
            while (internalLoopRequested_.load(std::memory_order_acquire)) {
                g_main_context_iteration(eventLoopContext_, TRUE);
            }
        });
    } catch (const std::system_error& error) {
        LOGE("Failed to start internal event loop thread: {}", error.what());
        g_main_context_unref(eventLoopContext_);
        eventLoopContext_ = nullptr;
        internalLoopRequested_.store(false, std::memory_order_release);
    }
}

void GenAiNlpServiceProxy::stopInternalEventLoop() {
    if (eventLoopContext_ != nullptr) {
        g_main_context_wakeup(eventLoopContext_);
    }

    if (eventLoopThread_.joinable()) {
        eventLoopThread_.join();
    }

    if (eventLoopContext_ != nullptr) {
        g_main_context_unref(eventLoopContext_);
        eventLoopContext_ = nullptr;
    }
}

int GenAiNlpServiceProxy::registerTool(int sessionId, const std::string& toolName, GenAiToolType toolType,
                                       const std::string& jsonSchema) {
    int errorCode = AI_COMMON_SUCCESS;
    if (!ensureProtocolSupportedForApi("register_tool", &errorCode)) {
        return errorCode;
    }
    if (toolName.empty()) {
        LOGE("toolName is empty");
        return AI_COMMON_NLP_PARAM_ERROR;
    }
    if (toolType != GENAI_TEXT_TOOL_TYPE_FUNCTION) {
        LOGE("toolType is invalid,{}", toolType);
        return AI_COMMON_NLP_PARAM_ERROR;
    }
    Json::Value toolJsonSchema;
    try {
        toolJsonSchema = stringToJson(jsonSchema);
    } catch (...) {
        LOGE("Failed to parse jsonSchema: %s", jsonSchema.c_str());
        return AI_COMMON_NLP_PARAM_ERROR;
    }
    if (!toolJsonSchema.isMember("name") || !toolJsonSchema["name"].isString() ||
        toolName != toolJsonSchema["name"].asString()) {
        LOGE("toolName mismatch, toolName='%s', jsonSchema=%s", toolName.c_str(), jsonSchema.c_str());
        return AI_COMMON_NLP_PARAM_ERROR;
    }
    GError* error = nullptr;
    bool success = ai_runtime_gen_ai_nlp_service_call_register_tool_sync(
        proxy_, sessionId, toolName.c_str(), toolType, jsonSchema.c_str(), &errorCode, nullptr, &error);
    if (not success) {
        if (error) {
            LOGE("Error calling register tool: {}:{}", error->code, error->message);
            g_error_free(error);
            return AI_COMMON_RUNTIME_ERROR;
        }
    }
    return errorCode;
}

int GenAiNlpServiceProxy::clearTools(int sessionId) {
    int errorCode = AI_COMMON_SUCCESS;
    if (!ensureProtocolSupportedForApi("clear_tools", &errorCode)) {
        return errorCode;
    }
    GError* error = nullptr;
    bool success = ai_runtime_gen_ai_nlp_service_call_clear_tools_sync(proxy_, sessionId, &errorCode, nullptr, &error);
    if (not success) {
        if (error) {
            LOGE("Error calling clear tools: {}:{}", error->code, error->message);
            g_error_free(error);
            return AI_COMMON_RUNTIME_ERROR;
        }
    }
    return errorCode;
}

int GenAiNlpServiceProxy::submitToolResult(int sessionId, const ChatModelConfig& chatModelConfig,
                                           const std::string& toolCallsJson, const std::string& resultJson) {
    int errorCode = AI_COMMON_SUCCESS;
    if (!ensureProtocolSupportedForApi("submit_tool_result", &errorCode)) {
        return errorCode;
    }
    Json::Value toolCalls = stringToJson(toolCallsJson);
    if (toolCalls.isNull()) {
        LOGE("toolCallsJson is invalid");
        return AI_COMMON_NLP_PARAM_ERROR;
    }
    Json::Value result = stringToJson(resultJson);
    if (result.isNull()) {
        LOGE("resultJson is invalid");
        return AI_COMMON_NLP_PARAM_ERROR;
    }
    Json::Value config = modelConfigToJson(chatModelConfig);
    config["tool_calls"] = toolCalls;
    std::string request = jsonToString(config);
    GError* error = nullptr;
    bool success = ai_runtime_gen_ai_nlp_service_call_submit_tool_result_sync(
        proxy_, sessionId, request.c_str(), resultJson.c_str(), &errorCode, nullptr, &error);
    if (not success) {
        if (error) {
            LOGE("Error calling submit tool result: {}:{}", error->code, error->message);
            g_error_free(error);
            return AI_COMMON_RUNTIME_ERROR;
        }
    }
    return errorCode;
}

void GenAiNlpServiceProxy::connectChatResultSignals(int sessionId) {
    auto* connection = GenAiNlpServer::getInstance().getConnection();

    std::string interfaceName = interface + std::to_string(sessionId);
    subscriptionId_ =
        g_dbus_connection_signal_subscribe(connection, nullptr, interfaceName.c_str(), nlpResultSignal, objectPath,
                                           nullptr, G_DBUS_SIGNAL_FLAGS_NONE, onChatResultCallback, this, nullptr);
}

void GenAiNlpServiceProxy::connectToolHandlerResultSignals(int sessionId) {
    auto* connection = GenAiNlpServer::getInstance().getConnection();
    std::string interfaceName = interface + std::to_string(sessionId);
    subscriptionId_ = g_dbus_connection_signal_subscribe(
        connection, nullptr, interfaceName.c_str(), toolHandlerResultSignal, objectPath, nullptr,
        G_DBUS_SIGNAL_FLAGS_NONE, onToolHandlerResultCallback, this, nullptr);
}

void GenAiNlpServiceProxy::onChatResultCallback(GDBusConnection* connection, const gchar* senderName,
                                                const gchar* objectPath, const gchar* interfaceName,
                                                const gchar* signalName, GVariant* parameters, gpointer userData) {
    auto* proxy = static_cast<GenAiNlpServiceProxy*>(userData);

    // Reset the timeout timer since we received a callback
    proxy->resetCallbackTimer();
    if (proxy->chatResultCallback_ != nullptr) {
        auto result = dissectChatResult(parameters);
        proxy->chatResultCallback_(&result, proxy->userData_);
    } else {
        LOGE("chat result callback is not set!");
    }
}

void GenAiNlpServiceProxy::onToolHandlerResultCallback(GDBusConnection* connection, const gchar* senderName,
                                                       const gchar* objectPath, const gchar* interfaceName,
                                                       const gchar* signalName, GVariant* parameters,
                                                       gpointer userData) {
    auto* proxy = static_cast<GenAiNlpServiceProxy*>(userData);
    if (proxy->toolHandlerResultCallback_ != nullptr) {
        std::string toolsJsonString;
        dissectToolHandlerResult(parameters, toolsJsonString);
        proxy->toolHandlerResultCallback_(toolsJsonString.c_str(), proxy->toolHandlerUserData_);
    }
}

void GenAiNlpServiceProxy::onChatFinishedCallback(GObject* sourceObject, GAsyncResult* res, gpointer userData) {
    GError* error{nullptr};
    auto* proxy = static_cast<GenAiNlpServiceProxy*>(userData);
    proxy->removeCallbackTimerIfExists();
    bool result = ai_runtime_gen_ai_nlp_service_call_chat_finish((AiRuntimeGenAiNlpService*)sourceObject, res, &error);
    if (not result) {
        if (error == nullptr) {
            g_printerr("Error calling chat: Unknown error\n");
        } else {
            g_printerr("Error calling chat: %s\n", error->message);
            proxy->handleErrorOccurred(error->code);
            g_error_free(error);
        }
    }
}

void GenAiNlpServiceProxy::onGenerateContentFinishedCallback(GObject* sourceObject, GAsyncResult* res,
                                                             gpointer userData) {
    GError* error{nullptr};
    auto* proxy = static_cast<GenAiNlpServiceProxy*>(userData);
    proxy->removeCallbackTimerIfExists();
    bool result = ai_runtime_gen_ai_nlp_service_call_generate_content_finish((AiRuntimeGenAiNlpService*)sourceObject,
                                                                             res, &error);
    if (not result) {
        if (error == nullptr) {
            g_printerr("Error calling chat: Unknown error\n");
        } else {
            g_printerr("Error calling chat: %s\n", error->message);
            proxy->handleErrorOccurred(error->code);
            g_error_free(error);
        }
    }
}

void GenAiNlpServiceProxy::onInitFinished(GObject* sourceObject, GAsyncResult* res, gpointer userData) {
    gint sessionId;
    gint errorCode;
    GError* error = nullptr;
    auto* proxy = static_cast<GenAiNlpServiceProxy*>(userData);
    bool ret = ai_runtime_gen_ai_nlp_service_call_init_finish((AiRuntimeGenAiNlpService*)sourceObject, &sessionId,
                                                              &errorCode, res, &error);
    if (!ret) {
        LOGE("Error calling initSubjectSegmentation: ", error->code, error->message);
        sessionId = -1;
        errorCode = AISDK_RUNTIME_ERROR;
    }

    proxy->connectChatResultSignals(sessionId);
    proxy->reconnectAttempts_ = 0;

    if (proxy->initCallback_) {
        proxy->initCallback_(sessionId, errorCode, proxy->initUserData_);
    }
}

void GenAiNlpServiceProxy::startCallbackTimer() {
    removeCallbackTimerIfExists();

    // Start a new timer
    timeoutId_ = g_timeout_add(
        chatResultCallbackTimeoutMs,
        [](gpointer data) -> gboolean {
            auto* proxy = static_cast<GenAiNlpServiceProxy*>(data);
            proxy->handleCallbackTimeout();
            proxy->timeoutId_ = 0;   // Reset the ID since the source is removed
            return G_SOURCE_REMOVE;  // Don't repeat the timeout
        },
        this);
}

void GenAiNlpServiceProxy::removeCallbackTimerIfExists() {
    if (timeoutId_ > 0) {
        g_source_remove(timeoutId_);
        timeoutId_ = 0;
    }
}

void GenAiNlpServiceProxy::resetCallbackTimer() {
    // Reset the timer by starting a new one
    startCallbackTimer();
}

void GenAiNlpServiceProxy::handleCallbackTimeout() {
    LOGE("NLP result callback timeout after 2 minutes");

    stopChat(currentSessionId_);

    if (chatResultCallback_) {
        ChatResult result{"",  "", "", "", AISDK_MODEL_RUN_TIME_OUT, "NLP result callback timeout after 2 minutes",
                          true};
        chatResultCallback_(&result, userData_);
    }
}

void GenAiNlpServiceProxy::refreshRuntimeProtocolVersion() {
    if (proxy_ == nullptr) {
        return;
    }

    const gchar* versionRaw = ai_runtime_gen_ai_nlp_service_get_runtime_version(proxy_);
    const ProtocolVersion parsedVersion = parseProtocolVersion(versionRaw);
    if (parsedVersion.major == 0 && parsedVersion.minor == 0 && parsedVersion.patch == 0) {
        const ProtocolVersion fallbackVersion{1, 0, 0};
        LOGW("Runtime protocol version is invalid:", versionRaw ? versionRaw : "null", ", fallback to ",
             versionToString(fallbackVersion));
        setRuntimeProtocolVersion(fallbackVersion);
        return;
    }

    setRuntimeProtocolVersion(parsedVersion);
    if (isSdkAheadOfRuntime()) {
        LOGW("SDK protocol", versionToString(getSdkProtocolVersion()), "is newer than ", versionToString(parsedVersion),
             "some APIs may be disabled based on requirements.");
    } else {
        LOGD("Runtime protocol", versionToString(parsedVersion), "is compatible with SDK",
             versionToString(getSdkProtocolVersion()));
    }
}

bool GenAiNlpServiceProxy::ensureProtocolSupportedForApi(const char* apiName, int* errorCode) {
    if (ensureApiSupported(apiName, errorCode)) {
        return true;
    }

    const ProtocolRequirement* requirement = findApiRequirement(apiName);
    ProtocolRequirement defaultRequirement{ProtocolVersion{1, 0, 0}, kProtocolVersionMax};
    const ProtocolRequirement& activeRequirement = requirement ? *requirement : defaultRequirement;

    if (apiName) {
        LOGW("Protocol mismatch for API:", apiName, "runtime:", versionToString(getRuntimeProtocolVersion()),
             "sdk:", versionToString(getSdkProtocolVersion()), "min:", versionToString(activeRequirement.minVersion),
             "max:", versionToString(activeRequirement.maxVersion));
    }

    return false;
}

}  // namespace kyai::genai::nlp
