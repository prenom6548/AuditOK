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

#include "genainlpservice.h"

#include <kylin-ai/aiengine.h>
#include <kylin-ai/common/error.h>
#include <sodium.h>

#include "chatbot.h"
#include "error.h"
#include "utils/logger.h"
#include "utils/util.h"

namespace {
constexpr const char* kSdkVersionAnnotation = "SdkVersion";
constexpr const char* kDefaultSdkVersion = "1.0.0";

constexpr const char* interface = "com.kylin.AiRuntime.GenAiNlp";
constexpr const char* objectPath = "/com/kylin/AiRuntime/GenAiNlp";
constexpr const char* chatNlpResultSignal = "ChatNlpResult";
constexpr const char* toolHandlerResultSignal = "ToolHandlerResult";
constexpr const char* serviceName = "GenAiNlpService";

GVariant* buildGVariantFromChatResult(const Chatbot::ChatResult& result) {
    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE_TUPLE);
    g_variant_builder_add(&builder, "s", result.reason_.c_str());
    g_variant_builder_add(&builder, "s", result.finishReason_.c_str());
    g_variant_builder_add(&builder, "i",
                          result.needConvertErrorCode_ ? gen_ai::nlp::error::getNlpErrorCode(result.error_.getCode())
                                                       : result.error_.getCode());
    g_variant_builder_add(&builder, "s", result.error_.getMessage().c_str());
    g_variant_builder_add(&builder, "b", result.isEnd_);
    g_variant_builder_add(&builder, "s", result.reasoningContent_.c_str());
    g_variant_builder_add(&builder, "s", result.modelName_.c_str());
    g_variant_builder_add(&builder, "s", result.searchInfo_.c_str());
    if (!result.requestId_.empty()) {
        g_variant_builder_add(&builder, "s", result.requestId_.c_str());
    }
    GVariant* resultVariant = g_variant_builder_end(&builder);
    return resultVariant;
}

GVariant* buildGVariantFromToolHandlerResult(const Chatbot::ToolHandlerResult& result) {
    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE_TUPLE);
    g_variant_builder_add(&builder, "s", result.toolsJson.c_str());
    GVariant* resultVariant = g_variant_builder_end(&builder);
    return resultVariant;
}
}  // namespace

GenAiNlpService::GenAiNlpService(GDBusConnection& connection, EngineSelector& engineSelector)
    : connection_(connection), engineSelector_(engineSelector) {
    exportSkeleton();
    const std::string protocolVersion = getProtocolVersion();
    if (protocolVersion.empty()) {
        LOGW("Protocol version is empty, use default protocol version: {}", kDefaultSdkVersion);
        setRuntimeProtocolVersion(kDefaultSdkVersion);
        LOGI("GenAiNlpService protocol version: {}", kDefaultSdkVersion);
    } else {
        setRuntimeProtocolVersion(protocolVersion);
        LOGI("GenAiNlpService protocol version: {}", protocolVersion);
    }
}

GenAiNlpService::~GenAiNlpService() {
    unexportSkeleton();
    LOGD("GenAiNlpService destructor execution completed.");
}

void GenAiNlpService::stop() {
    for (const auto& handlerId : handlerIds_) {
        g_signal_handler_disconnect(delegate_, handlerId);
    }
    handlerIds_.clear();
    for (const auto& [sessionId, chatbot] : chatbots_) {
        chatbot->destroy();
    }
}

void GenAiNlpService::exportSkeleton() {
    delegate_ = ai_runtime_gen_ai_nlp_service_skeleton_new();

    connectSignal();

    GError* error{nullptr};

    isExported_ =
        g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(delegate_), &connection_, objectPath, &error);
    if (!isExported_) {
        LOGE("Error creating server at address {} {}.", objectPath, error->message);
        g_printerr("Error creating server at address %s: %s\n", objectPath, error->message);
        g_error_free(error);
    }
}

void GenAiNlpService::unexportSkeleton() {
    if (delegate_ == nullptr) {
        return;
    }

    if (isExported_) {
        g_dbus_interface_skeleton_unexport_from_connection(G_DBUS_INTERFACE_SKELETON(delegate_), &connection_);
    }
    g_object_unref(delegate_);
}

void GenAiNlpService::connectSignal() {
    gulong handlerId{};
    handlerId = g_signal_connect(delegate_, "handle-init", G_CALLBACK(onHandleInit), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-destroy", G_CALLBACK(onHandleDestroy), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-chat", G_CALLBACK(onHandleChat), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-generate-content", G_CALLBACK(onHandleGenerateContent), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-clear-chat-message", G_CALLBACK(onHandleClearChatMessage), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-stop-chat", G_CALLBACK(onHandleStopChat), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-stop-chat-with-request-id",
                                 G_CALLBACK(onHandleStopChatWithRequestId), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-add-system-message", G_CALLBACK(onHandleAddSystemMessage), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-set-chat-system-prompt-id",
                                 G_CALLBACK(onHandleSetChatSystemPromptId), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-chat-with-history-messages",
                                 G_CALLBACK(onHandleChatWithHistoryMessages), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-get-nlp-model-list", G_CALLBACK(onHandleGetNlpModelList), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-register-tool", G_CALLBACK(onHandleRegisterTool), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-clear-tools", G_CALLBACK(onHandleClearTools), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-submit-tool-result", G_CALLBACK(onHandleSubmitToolResult), this);
    handlerIds_.push_back(handlerId);
}

std::string GenAiNlpService::getProtocolVersion() {
    if (delegate_ == nullptr) {
        LOGE("Delegate is null, get protocol version failed.");
        return kDefaultSdkVersion;
    }

    GDBusInterfaceInfo* interfaceInfo = ai_runtime_gen_ai_nlp_service_interface_info();
    if (interfaceInfo == nullptr) {
        LOGW("Interface info is null, use default protocol version.");
        return kDefaultSdkVersion;
    }
    if (interfaceInfo->annotations == nullptr) {
        LOGW("Protocol version annotation not found, use default protocol version.");
        return kDefaultSdkVersion;
    }

    for (GDBusAnnotationInfo** annotation = interfaceInfo->annotations; annotation && *annotation; ++annotation) {
        if ((*annotation)->key == nullptr || (*annotation)->value == nullptr) {
            continue;
        }
        if (g_strcmp0((*annotation)->key, kSdkVersionAnnotation) == 0) {
            return (*annotation)->value;
        }
    }
    return kDefaultSdkVersion;
}

void GenAiNlpService::setRuntimeProtocolVersion(const std::string& version) {
    if (delegate_ == nullptr) {
        LOGE("Delegate is null, set runtime version failed.");
        return;
    }
    ai_runtime_gen_ai_nlp_service_set_runtime_version(delegate_, version.c_str());
}

int GenAiNlpService::generateRandomSessionId() {
    uint32_t sessionId;
    randombytes_buf(&sessionId, sizeof(sessionId));
    sessionId = 1 + (sessionId & INT_MAX);
    while (sessionIdSet_.find(sessionId) != sessionIdSet_.end()) {
        randombytes_buf(&sessionId, sizeof(sessionId));
        sessionId = 1 + (sessionId & INT_MAX);
    }
    sessionIdSet_.insert(sessionId);
    return sessionId;
}

void GenAiNlpService::releaseRandomSessionId(int sessionId) { sessionIdSet_.erase(sessionId); }

void GenAiNlpService::onChatResult(const Chatbot::ChatResult& chatResult, int sessionId) {
    std::string resultSignalInterfaceName = std::string(interface) + std::to_string(sessionId);
    GVariant* resultVariant = buildGVariantFromChatResult(chatResult);
    g_dbus_connection_emit_signal(&connection_, interface, objectPath, resultSignalInterfaceName.c_str(),
                                  chatNlpResultSignal, resultVariant, nullptr);
}

void GenAiNlpService::onToolHandlerResult(const Chatbot::ToolHandlerResult& toolHandlerResult, int sessionId) {
    std::string resultSignalInterfaceName = std::string(interface) + std::to_string(sessionId);
    GVariant* resultVariant = buildGVariantFromToolHandlerResult(toolHandlerResult);
    g_dbus_connection_emit_signal(&connection_, interface, objectPath, resultSignalInterfaceName.c_str(),
                                  toolHandlerResultSignal, resultVariant, nullptr);
}

std::shared_ptr<GenAiNlpService> GenAiNlpService::getSharedObject() { return shared_from_this(); }

bool GenAiNlpService::isSessionIdValid(int sessionId) {
    if (chatbots_.find(sessionId) == chatbots_.end()) {
        return false;
    }
    return true;
}

bool GenAiNlpService::onHandleInit(AiRuntimeGenAiNlpService* service, GDBusMethodInvocation* invocation,
                                   const gchar* config, gpointer userData) {
    auto* nlpService = static_cast<GenAiNlpService*>(userData);
    if (nlpService == nullptr) {
        ai_runtime_gen_ai_nlp_service_complete_init(service, invocation, -1, AISDK_RUNTIME_ERROR);
        return true;
    }

    std::shared_ptr sharedNlpService = nlpService->getSharedObject();

    const GenNlpModelConfig genNlpModelConfigInfo = GenNlpModelConfig::parseModelConfig(config);

    bool isConfigured = genNlpModelConfigInfo.name.empty()
                            ? true
                            : ModelConfigManager::getInstance().isNlpModelConfigured(genNlpModelConfigInfo.name);

    std::unique_ptr uniqueNlpEngine = sharedNlpService->engineSelector_.selectGenAiTextGenerationEngine(
        genNlpModelConfigInfo.type, genNlpModelConfigInfo.name);
    LOGI("Config info name: {}, Config info name: {}", genNlpModelConfigInfo.name, int(genNlpModelConfigInfo.type));
    // 设置参数
    std::shared_ptr nlpEngine = std::move(uniqueNlpEngine);

    if (nlpEngine == nullptr || !isConfigured) {
        ai_runtime_gen_ai_nlp_service_complete_init(service, invocation, -1, AISDK_MODEL_NOT_FOUND);
        LOGE("No nlp engine found!");
        return true;
    }
    int sessionId = sharedNlpService->generateRandomSessionId();
    auto chatbot = std::make_shared<Chatbot>(sharedNlpService->engineSelector_);
    chatbot->init(sessionId);
    sharedNlpService->chatbots_[sessionId] = chatbot;
    sharedNlpService->chatbots_[sessionId]->setTopK(genNlpModelConfigInfo.topK);
    sharedNlpService->chatbots_[sessionId]->setStream(genNlpModelConfigInfo.stream);
    if (nlpEngine->isCustomModelEngine()) {
        sharedNlpService->chatbots_[sessionId]->setDeployType(DeployType::Custom);
    } else if (nlpEngine->isCloud()) {
        sharedNlpService->chatbots_[sessionId]->setDeployType(DeployType::PublicCloud);
    } else {
        sharedNlpService->chatbots_[sessionId]->setDeployType(DeployType::Local);
    }
    ai_runtime_gen_ai_nlp_service_complete_init(service, invocation, sessionId, 0);
    LOGI("Init success, nlp engine name: {}.", nlpEngine->engineName());

    std::string uuid = serviceName + std::to_string(sessionId);
    return true;
}

bool GenAiNlpService::onHandleDestroy(AiRuntimeGenAiNlpService* service, GDBusMethodInvocation* invocation,
                                      int sessionId, gpointer userData) {
    auto* nlpService = static_cast<GenAiNlpService*>(userData);
    if (nlpService == nullptr) {
        LOGE("Gen ai nlp service is null.");
        ai_runtime_gen_ai_nlp_service_complete_destroy(service, invocation);
        return true;
    }
    std::shared_ptr sharedNlpService = nlpService->getSharedObject();

    if (sharedNlpService->isSessionIdValid(sessionId)) {
        sharedNlpService->chatbots_[sessionId]->destroy();
        sharedNlpService->chatbots_.erase(sessionId);
    }
    ai_runtime_gen_ai_nlp_service_complete_destroy(service, invocation);
    return true;
}

// TODO chat chatWithHistory GenerateContent三个接口归一，可以减少维护成本
bool GenAiNlpService::onHandleChat(AiRuntimeGenAiNlpService* service, GDBusMethodInvocation* invocation, gint sessionId,
                                   gchar* question, gpointer userData) {
    auto* nlpService = static_cast<GenAiNlpService*>(userData);
    if (nlpService == nullptr) {
        ai_runtime_gen_ai_nlp_service_complete_chat(service, invocation);
        return true;
    }
    ModelConfigManager::getInstance().updateCloudNlpCurrentModelName();
    std::shared_ptr sharedNlpService = nlpService->getSharedObject();
    if (!sharedNlpService->isSessionIdValid(sessionId)) {
        std::string errorMessage = fmt::format("SessionId {} not valid", sessionId);
        // 此时sessionId无效，不能通过信号的方式进行通知
        g_dbus_method_invocation_return_error(invocation, G_IO_ERROR, G_IO_ERROR_FAILED, errorMessage.c_str(),
                                              sessionId);
        return true;
    }
    std::string requestId;
    Json::Value requestBody = util::stringToJson(question);
    requestId = requestBody["request_id"].asString();
    auto requestState = std::make_shared<std::atomic_bool>(false);
    sharedNlpService->chatbots_[sessionId]->chat(
        question, requestId,
        [sharedNlpService, sessionId](const Chatbot::ChatResult& result) {
            sharedNlpService->onChatResult(result, sessionId);
        },
        [service, invocation, requestState]() {
            if (requestState->exchange(true)) {
                return;
            }
            ai_runtime_gen_ai_nlp_service_complete_chat(service, invocation);
        },
        [sharedNlpService, sessionId](const Chatbot::ToolHandlerResult& result) {
            sharedNlpService->onToolHandlerResult(result, sessionId);
        });

    return true;
}

bool GenAiNlpService::onHandleGenerateContent(AiRuntimeGenAiNlpService* service, GDBusMethodInvocation* invocation,
                                              gint sessionId, gchar* prompt, gpointer userData) {
    auto* nlpService = static_cast<GenAiNlpService*>(userData);
    if (nlpService == nullptr) {
        LOGE("Gen ai nlp service is null.");
        ai_runtime_gen_ai_nlp_service_complete_generate_content(service, invocation);
        return true;
    }
    std::shared_ptr sharedNlpService = nlpService->getSharedObject();
    if (!sharedNlpService->isSessionIdValid(sessionId)) {
        std::string errorMessage = fmt::format("SessionId {} not valid", sessionId);
        // 此时sessionId无效，不能通过信号的方式进行通知
        g_dbus_method_invocation_return_error(invocation, G_IO_ERROR, G_IO_ERROR_FAILED, errorMessage.c_str(),
                                              sessionId);
        return true;
    }
    LOGD("prompt {}", prompt);
    std::string requestId;
    Json::Value requestBody = util::stringToJson(prompt);
    requestId = requestBody["request_id"].asString();
    auto requestState = std::make_shared<std::atomic_bool>(false);
    sharedNlpService->chatbots_[sessionId]->generateContent(
        prompt, requestId,
        [sharedNlpService, sessionId](const Chatbot::ChatResult& result) {
            sharedNlpService->onChatResult(result, sessionId);
        },
        [service, invocation, requestState]() {
            if (requestState->exchange(true)) {
                return;
            }
            ai_runtime_gen_ai_nlp_service_complete_generate_content(service, invocation);
        });

    return true;
}

bool GenAiNlpService::onHandleClearChatMessage(AiRuntimeGenAiNlpService* service, GDBusMethodInvocation* invocation,
                                               gint sessionId, gpointer userData) {
    auto* nlpService = static_cast<GenAiNlpService*>(userData);
    if (nlpService == nullptr) {
        LOGE("Gen ai nlp service is null.");
        ai_runtime_gen_ai_nlp_service_complete_clear_chat_message(service, invocation);
        return true;
    }
    std::shared_ptr sharedNlpService = nlpService->getSharedObject();
    if (sharedNlpService->isSessionIdValid(sessionId)) {
        sharedNlpService->chatbots_[sessionId]->clearChatContext();
    }
    ai_runtime_gen_ai_nlp_service_complete_clear_chat_message(service, invocation);
    return true;
}

bool GenAiNlpService::onHandleStopChat(AiRuntimeGenAiNlpService* service, GDBusMethodInvocation* invocation,
                                       gint sessionId, gpointer userData) {
    auto* nlpService = static_cast<GenAiNlpService*>(userData);
    if (nlpService == nullptr) {
        LOGE("Gen ai nlp service is null.");
        ai_runtime_gen_ai_nlp_service_complete_stop_chat(service, invocation);
        return true;
    }
    std::shared_ptr sharedNlpService = nlpService->getSharedObject();
    if (sharedNlpService->isSessionIdValid(sessionId)) {
        sharedNlpService->chatbots_[sessionId]->stopChat();
    }
    ai_runtime_gen_ai_nlp_service_complete_stop_chat(service, invocation);
    return true;
}

bool GenAiNlpService::onHandleStopChatWithRequestId(AiRuntimeGenAiNlpService* service,
                                                    GDBusMethodInvocation* invocation, gint sessionId,
                                                    const gchar* requestId, gpointer userData) {
    auto* nlpService = static_cast<GenAiNlpService*>(userData);
    if (nlpService == nullptr) {
        LOGE("Gen ai nlp service is null.");
        ai_runtime_gen_ai_nlp_service_complete_stop_chat_with_request_id(service, invocation);
        return true;
    }
    std::shared_ptr sharedNlpService = nlpService->getSharedObject();
    if (sharedNlpService->isSessionIdValid(sessionId)) {
        std::string requestIdString = requestId == nullptr ? "" : requestId;
        sharedNlpService->chatbots_[sessionId]->stopChat(requestIdString);
    }
    ai_runtime_gen_ai_nlp_service_complete_stop_chat_with_request_id(service, invocation);
    return true;
}

bool GenAiNlpService::onHandleAddSystemMessage(AiRuntimeGenAiNlpService* service, GDBusMethodInvocation* invocation,
                                               gint sessionId, const gchar* message, gpointer userData) {
    auto* nlpService = static_cast<GenAiNlpService*>(userData);
    if (nlpService == nullptr) {
        LOGE("Gen ai nlp service is null.");
        ai_runtime_gen_ai_nlp_service_complete_add_system_message(service, invocation);
        return true;
    }
    std::shared_ptr sharedNlpService = nlpService->getSharedObject();
    if (sharedNlpService->isSessionIdValid(sessionId)) {
        sharedNlpService->chatbots_[sessionId]->addSystemMessage(message);
    }
    ai_runtime_gen_ai_nlp_service_complete_add_system_message(service, invocation);
    return true;
}

bool GenAiNlpService::onHandleSetChatSystemPromptId(AiRuntimeGenAiNlpService* service,
                                                    GDBusMethodInvocation* invocation, gint sessionId, gint promptId,
                                                    gpointer userData) {
    auto* nlpService = static_cast<GenAiNlpService*>(userData);
    if (nlpService == nullptr) {
        LOGE("Gen ai nlp service is null.");
        ai_runtime_gen_ai_nlp_service_complete_set_chat_system_prompt_id(service, invocation);
        return true;
    }
    std::shared_ptr sharedNlpService = nlpService->getSharedObject();
    if (sharedNlpService->isSessionIdValid(sessionId)) {
        sharedNlpService->chatbots_[sessionId]->addSystemMessageByPromptId(
            promptId, sharedNlpService->chatbots_[sessionId]->getDeployType());
    }
    ai_runtime_gen_ai_nlp_service_complete_set_chat_system_prompt_id(service, invocation);
    return true;
}

bool GenAiNlpService::onHandleChatWithHistoryMessages(AiRuntimeGenAiNlpService* service,
                                                      GDBusMethodInvocation* invocation, gint sessionId,
                                                      const gchar* messages, gpointer userData) {
    auto* nlpService = static_cast<GenAiNlpService*>(userData);
    if (nlpService == nullptr) {
        LOGE("Gen ai nlp service is null.");
        ai_runtime_gen_ai_nlp_service_complete_chat_with_history_messages(service, invocation);
        return true;
    }
    ModelConfigManager::getInstance().updateCloudNlpCurrentModelName();
    std::shared_ptr sharedNlpService = nlpService->getSharedObject();

    if (sharedNlpService->isSessionIdValid(sessionId)) {
        std::string requestId;
        Json::Value requestBody = util::stringToJson(messages);
        requestId = requestBody["request_id"].asString();
        auto requestState = std::make_shared<std::atomic_bool>(false);
        sharedNlpService->chatbots_[sessionId]->chatWithHistoryMessage(
            messages, requestId,
            [sharedNlpService, sessionId](const Chatbot::ChatResult& result) {
                sharedNlpService->onChatResult(result, sessionId);
            },
            [service, invocation, requestState]() {
                if (requestState->exchange(true)) {
                    return;
                }
                ai_runtime_gen_ai_nlp_service_complete_chat_with_history_messages(service, invocation);
            },
            [sharedNlpService, sessionId](const Chatbot::ToolHandlerResult& result) {
                sharedNlpService->onToolHandlerResult(result, sessionId);
            });
    }
    return true;
}

bool GenAiNlpService::onHandleRegisterTool(AiRuntimeGenAiNlpService* service, GDBusMethodInvocation* invocation,
                                           gint sessionId, const gchar* toolName, gint toolType,
                                           const gchar* jsonSchema, gpointer userData) {
    auto* nlpService = static_cast<GenAiNlpService*>(userData);
    if (nlpService == nullptr) {
        LOGE("Gen ai nlp service is null.");
        ai_runtime_gen_ai_nlp_service_complete_register_tool(service, invocation, AISDK_RUNTIME_ERROR);
        return true;
    }
    std::shared_ptr sharedNlpService = nlpService->getSharedObject();
    if (sharedNlpService->isSessionIdValid(sessionId)) {
        sharedNlpService->chatbots_[sessionId]->registerTool(toolName, toolType, jsonSchema);
    }
    ai_runtime_gen_ai_nlp_service_complete_register_tool(service, invocation, AI_COMMON_SUCCESS);
    return true;
}

bool GenAiNlpService::onHandleClearTools(AiRuntimeGenAiNlpService* service, GDBusMethodInvocation* invocation,
                                         gint sessionId, gpointer userData) {
    auto* nlpService = static_cast<GenAiNlpService*>(userData);
    if (nlpService == nullptr) {
        LOGE("Gen ai nlp service is null.");
        ai_runtime_gen_ai_nlp_service_complete_clear_tools(service, invocation, AISDK_RUNTIME_ERROR);
        return true;
    }
    std::shared_ptr sharedNlpService = nlpService->getSharedObject();
    if (sharedNlpService->isSessionIdValid(sessionId)) {
        sharedNlpService->chatbots_[sessionId]->clearTools();
    }
    ai_runtime_gen_ai_nlp_service_complete_clear_tools(service, invocation, AI_COMMON_SUCCESS);
    return true;
}

bool GenAiNlpService::onHandleSubmitToolResult(AiRuntimeGenAiNlpService* service, GDBusMethodInvocation* invocation,
                                               gint sessionId, const gchar* request, const gchar* resultJson,
                                               gpointer userData) {
    auto* nlpService = static_cast<GenAiNlpService*>(userData);
    if (nlpService == nullptr) {
        LOGE("Gen ai nlp service is null.");
        ai_runtime_gen_ai_nlp_service_complete_submit_tool_result(service, invocation, AISDK_RUNTIME_ERROR);
        return true;
    }
    std::shared_ptr sharedNlpService = nlpService->getSharedObject();
    if (sharedNlpService->isSessionIdValid(sessionId)) {
        Json::Value toolCallsJson = util::stringToJson(request);
        if (!toolCallsJson.isMember("tool_calls")) {
            LOGE("tool_calls json is invalid.");
            ai_runtime_gen_ai_nlp_service_complete_submit_tool_result(service, invocation, AI_COMMON_NLP_INPUT_INVALID);
        }
        std::string toolCalls = util::jsonToString(toolCallsJson["tool_calls"]);
        sharedNlpService->chatbots_[sessionId]->submitToolResult(toolCalls, resultJson);
        sharedNlpService->chatbots_[sessionId]->chat(
            request,
            [sharedNlpService, sessionId](const Chatbot::ChatResult& result) {
                sharedNlpService->onChatResult(result, sessionId);
            },
            nullptr,
            [sharedNlpService, sessionId](const Chatbot::ToolHandlerResult& result) {
                sharedNlpService->onToolHandlerResult(result, sessionId);
            });
    }
    ai_runtime_gen_ai_nlp_service_complete_submit_tool_result(service, invocation, AI_COMMON_SUCCESS);
    return true;
}

bool GenAiNlpService::onHandleGetNlpModelList(AiRuntimeGenAiNlpService* delegate, GDBusMethodInvocation* invocation,
                                              gint sessionId, gpointer userData) {
    std::string list = ModelConfigManager::getInstance().getConfiguredNlpModelList(true);
    Json::Value value;
    value["modelInfos"] = util::stringToJson(list);
    std::string jsonString = util::jsonToString(value);
    LOGI("NLP model list: {}", jsonString);
    ai_runtime_gen_ai_nlp_service_complete_get_nlp_model_list(delegate, invocation, jsonString.c_str());
    return true;
}
