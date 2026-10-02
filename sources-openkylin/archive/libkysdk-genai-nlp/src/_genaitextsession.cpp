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

#include "_genaitextsession.h"

#include <map>
#include <random>

#include "_chatmessage.h"
#include "chat.h"
#include "error.h"
#include "logger.h"
#include "protocolversion.h"

namespace {
void setErrorCode(int *pErrorCode, int errorCode) {
    if (pErrorCode != nullptr) {
        *pErrorCode = errorCode;
    }
}

std::string generateRequestId() {
    static constexpr char kHexDigits[] = "0123456789abcdef";
    static thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, 255);
    std::string requestId;
    requestId.reserve(32);
    for (int i = 0; i < 16; ++i) {
        const unsigned value = static_cast<unsigned>(dist(rng));
        requestId.push_back(kHexDigits[(value >> 4) & 0x0F]);
        requestId.push_back(kHexDigits[value & 0x0F]);
    }
    return requestId;
}
}  // namespace

GenAiTextSession *genai_text_create_session() { return new _GenAiTextSession{}; }

void genai_text_destroy_session(GenAiTextSession **session) {
    if (session == nullptr) {
        return;
    }
    if (*session == nullptr) {
        return;
    }
    (*session)->destroy();
    delete *session;
    *session = nullptr;
}

int genai_text_init_session(GenAiTextSession *session) {
    if (session == nullptr) {
        return AI_COMMON_INVALID_SESSION;
    }
    return session->init();
}

void genai_text_init_session_async(GenAiTextSession *session, GenAiTextInitCallback callback, void *user_data) {
    if (session == nullptr) {
        if (callback) {
            callback(AI_COMMON_INVALID_SESSION, user_data);
        }
        return;
    }
    session->init(callback, user_data);
}

void genai_text_result_set_callback(GenAiTextSession *session, ChatResultCallback callback, void *user_data) {
    if (session == nullptr) {
        return;
    }
    session->setResultCallback(callback, user_data);
}

void genai_text_enable_internal_event_loop(GenAiTextSession *session, bool enable) {
    if (session == nullptr) {
        return;
    }
    session->enableInternalEventLoop(enable);
}

void genai_text_set_model_config(GenAiTextSession *session, ChatModelConfig *config) {
    if (session == nullptr) {
        return;
    }

    if (config == nullptr) {
        session->setChatModelConfig(ChatModelConfig{std::string{}, ModelDeployType{ModelDeployType(-1)}, int{}});
        return;
    }

    session->setChatModelConfig(*config);
}

// 根据提示进行文本生成补全
void genai_text_generate_content_async(GenAiTextSession *session, const char *prompt) {
    if (session == nullptr) {
        return;
    }
    session->generateContent(prompt);
}

// 进行对话，支持缓存上下文
void genai_text_chat_async(GenAiTextSession *session, const char *question) {
    if (session == nullptr) {
        return;
    }
    session->chat(question);
}

int genai_text_chat_async_with_request_id(GenAiTextSession *session, const char *request_id, const char *question) {
    if (session == nullptr) {
        return AI_COMMON_INVALID_SESSION;
    }
    if (question == nullptr) {
        return AI_COMMON_INVALID_PARAMETER;
    }
    if (question[0] == '\0') {
        return AI_COMMON_INVALID_PARAMETER;
    }
    if (request_id == nullptr) {
        const std::string generated = generateRequestId();
        return session->chatWithRequestId(question, generated);
    }
    if (request_id[0] == '\0') {
        session->chat(question);
        return AI_COMMON_SUCCESS;
    }
    return session->chatWithRequestId(question, request_id);
}

// 进行对话，需要传入历史消息
void genai_text_chat_with_history_messages_async(GenAiTextSession *session, ChatMessage *history_messages) {
    if (session == nullptr) {
        return;
    }
    session->chatWithHistoryMessages(*history_messages);
}

int genai_text_chat_with_history_messages_async_with_request_id(GenAiTextSession *session, const char *request_id,
                                                                ChatMessage *history_messages) {
    if (session == nullptr) {
        return AI_COMMON_INVALID_PARAMETER;
    }
    if (history_messages == nullptr) {
        return AI_COMMON_INVALID_PARAMETER;
    }
    if (request_id == nullptr) {
        const std::string generated = generateRequestId();
        session->chatWithHistoryMessagesWithRequestId(*history_messages, generated);
        return AI_COMMON_SUCCESS;
    }
    if (request_id[0] == '\0') {
        session->chatWithHistoryMessages(*history_messages);
        return AI_COMMON_SUCCESS;
    }
    session->chatWithHistoryMessagesWithRequestId(*history_messages, request_id);
    return AI_COMMON_SUCCESS;
}

// 设置对话提示词
void genai_text_set_chat_system_prompt(GenAiTextSession *session, const char *prompt) {
    if (session == nullptr or prompt == nullptr) {
        return;
    }
    session->setChatSystemPrompt(prompt);
}

// 设置对话的提示词，使用SDK内置的提示词
void genai_text_set_chat_system_prompt_id(GenAiTextSession *session, PromptId prompt_id) {
    if (session == nullptr) {
        return;
    }
    session->setChatSystemPromptId(prompt_id);
}

// 清除对话上下文
void genai_text_clear_chat_history_messages(GenAiTextSession *session) {
    if (session == nullptr) {
        return;
    }
    session->clearChatMessage();
}

void genai_text_stop_chat(GenAiTextSession *session) {
    if (session == nullptr) {
        return;
    }
    session->stopChat();
}

int genai_text_stop_chat_with_request_id(GenAiTextSession *session, const char *request_id) {
    if (session == nullptr) {
        return AI_COMMON_INVALID_SESSION;
    }
    if (request_id == nullptr || request_id[0] == '\0') {
        session->stopChat();
        return AI_COMMON_SUCCESS;
    }
    return session->stopChatWithRequestId(request_id);
}

const TextModelList *genai_text_get_model_list(GenAiTextSession *session, int *error_code) {
    if (session == nullptr) {
        setErrorCode(error_code, AI_COMMON_INVALID_SESSION);
        return nullptr;
    }
    return session->getTextModelList(error_code);
}

// 类：_GenaiTextSession成员函数定义

_GenAiTextSession::_GenAiTextSession() {
    serviceProxy_.setReconnectedCallback([this]() { initSession(); });
}

int _GenAiTextSession::init() {
    if (!initServer()) {
        return AiSdkCommonErrorCode::AISDK_RUNTIME_ERROR;
    }

    return initSession();
}

void _GenAiTextSession::init(GenAiTextInitCallback callback, void *userData) {
    if (!initServer()) {
        if (callback) {
            callback(AI_COMMON_RUNTIME_ERROR, userData);
        }
    }
    initCallback_ = callback;
    initUserData_ = userData;
    initSessionAsync();
}

void _GenAiTextSession::destroy() { serviceProxy_.destroyEngine(sessionId_); }

void _GenAiTextSession::setResultCallback(const ChatResultCallback &callback, void *userData) {
    serviceProxy_.setChatResultCallback(callback, userData);
}

void _GenAiTextSession::setChatModelConfig(const ChatModelConfig &chatModelConfig) {
    chatModelConfig_ = chatModelConfig;
}

void _GenAiTextSession::chat(const std::string &question) {
    serviceProxy_.chat(sessionId_, chatModelConfig_, toolChoiceConfig_, question);
}

int _GenAiTextSession::chatWithRequestId(const std::string &question,
                                         const std::string &requestId) {
    return serviceProxy_.chatWithRequestId(sessionId_, chatModelConfig_,
                                           question, requestId);
}

void _GenAiTextSession::generateContent(const std::string &prompt) {
    serviceProxy_.generateContent(sessionId_, chatModelConfig_, prompt);
}

void _GenAiTextSession::chatWithHistoryMessages(const ChatMessage &message) {
    serviceProxy_.chatWithHistoryMessages(sessionId_, chatModelConfig_, toolChoiceConfig_,
                                          message.generateJsonString());
}

void _GenAiTextSession::chatWithHistoryMessagesWithRequestId(const ChatMessage &message, const std::string &requestId) {
    serviceProxy_.chatWithHistoryMessagesWithRequestId(sessionId_, chatModelConfig_, toolChoiceConfig_,
                                                       message.generateJsonString(), requestId);
}

void _GenAiTextSession::setChatSystemPrompt(const std::string &message) {
    serviceProxy_.addSystemMessage(sessionId_, message);
}

void _GenAiTextSession::setChatSystemPromptId(PromptId promptId) {
    serviceProxy_.setChatSystemPromptId(sessionId_, promptId);
}

void _GenAiTextSession::stopChat() { serviceProxy_.stopChat(sessionId_); }

int _GenAiTextSession::stopChatWithRequestId(const std::string &requestId) {
    if (requestId.empty()) {
        stopChat();
        return AI_COMMON_SUCCESS;
    }
    return serviceProxy_.stopChatWithRequestId(sessionId_, requestId);
}

void _GenAiTextSession::clearChatMessage() { serviceProxy_.clearChatMessage(sessionId_); }

TextModelList *_GenAiTextSession::getTextModelList(int *errorCode) {
    textModelList_ = serviceProxy_.getModelList(sessionId_, errorCode);
    return &textModelList_;
}

void _GenAiTextSession::enableInternalEventLoop(bool enable) { serviceProxy_.enableInternalEventLoop(enable); }

bool _GenAiTextSession::initServer() {
    if (!serviceProxy_.connectToServer()) {
        return serviceProxy_.reconnectServer();
    }
    return true;
}

int _GenAiTextSession::initSession() {
    int errorCode = serviceProxy_.initEngine(chatModelConfig_, &sessionId_);
    if (errorCode != AI_COMMON_SUCCESS) {
        LOGE("Init session error: ", errorCode);
    }
    return errorCode;
}

void _GenAiTextSession::initSessionAsync() { serviceProxy_.initEngineAsync(chatModelConfig_, onSessionInited, this); }

void _GenAiTextSession::onSessionInited(int sessionId, int errorCode, void *userData) {
    auto *session = static_cast<_GenAiTextSession *>(userData);
    session->sessionId_ = sessionId;
    if (session->initCallback_) {
        session->initCallback_(errorCode, session->initUserData_);
    }
}

int _GenAiTextSession::registerTool(const std::string &toolName, GenAiToolType toolType,
                                    const std::string &jsonSchema) {
    return serviceProxy_.registerTool(sessionId_, toolName, toolType, jsonSchema);
}

int _GenAiTextSession::clearTools() { return serviceProxy_.clearTools(sessionId_); }

void _GenAiTextSession::setToolHandler(GenAiToolHandler handler, void *userData) {
    serviceProxy_.setToolHandler(handler, userData);
}

int _GenAiTextSession::submitToolResult(const std::string &toolCallsJson, const std::string &resultJson) {
    return serviceProxy_.submitToolResult(sessionId_, chatModelConfig_, toolCallsJson, resultJson);
}

void _GenAiTextSession::setToolChoiceConfig(const GenAiToolChoiceConfig &config) { toolChoiceConfig_ = config; }
