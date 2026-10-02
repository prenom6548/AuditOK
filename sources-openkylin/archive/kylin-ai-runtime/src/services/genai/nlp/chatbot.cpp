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

#include "chatbot.h"

#include <jsoncpp/json/value.h>
#include <modelconfig/modelconfigenums.h>

#include <chrono>

#include "config/engineconfiguration.h"
#include "error.h"
#include "modelconfig/modelstatustracker.h"
#include "taskmanager/taskrunner.h"
#include "utils/logger.h"
#include "utils/util.h"

namespace {
const std::vector<std::string> modelContextMember{"max_completion_tokens", "temperature", "top_p", "presence_penalty",
                                                  "n"};
constexpr int64_t kDefaultChatTimeoutMs = 60LL * 60 * 1000;
constexpr size_t kChatRequestQueueMaxSize = 10;

int64_t loadChatTimeoutMs() {
    const auto taskTimeout = task::loadTaskTimeoutMs(task::TaskType::TextChat);
    if (taskTimeout.has_value()) {
        return *taskTimeout;
    }
    LOGW("Chat task timeout not found in task config, use default.");
    return kDefaultChatTimeoutMs;
}

size_t loadChatRequestQueueMaxSize() {
    const auto maxSize = task::loadTaskQueueMaxSize(task::TaskType::TextChat);
    if (maxSize.has_value()) {
        return static_cast<size_t>(*maxSize);
    }
    LOGW("Chat request queue max size not found in task config, use default.");
    return kChatRequestQueueMaxSize;
}
}  // namespace

Chatbot::Chatbot(EngineSelector& engineSelector) : engineSelector_{engineSelector} {}

Chatbot::~Chatbot() {
    destroy();
    LOGD("~Chatbot");
}

void Chatbot::init(int sessionId) {
    std::shared_ptr chatbot = getSharedObject();
    std::string uuid = "GenAiNlpService" + std::to_string(sessionId);
    ModelConfigManager::getInstance().updateAllDeployTypeCallback(
        AiCapability::Nlp, uuid, [chatbot](ModelChangeType) { chatbot->defaultModelChanged_ = true; });
}

void Chatbot::chat(const std::string& request, const ChatResultCallback& resultCallback,
                   const ChatFinishedCallback& finishedCallback, const ToolHandlerResultCallback& toolHandlerCallback) {
    chat(request, std::string{}, resultCallback, finishedCallback, toolHandlerCallback);
}

void Chatbot::chat(const std::string& request, const std::string& requestId, const ChatResultCallback& resultCallback,
                   const ChatFinishedCallback& finishedCallback, const ToolHandlerResultCallback& toolHandlerCallback) {
    Json::Value requestBody = util::stringToJson(request);
    ChatRequest chatRequest{};
    chatRequest.body = requestBody;
    chatRequest.withHistoryMessage = false;
    chatRequest.resultCallback = resultCallback;
    chatRequest.finishedCallback = finishedCallback;
    chatRequest.requestType = ChatRequest::RequestType::Chat;
    chatRequest.toolHandlerCallback = toolHandlerCallback;
    chatRequest.requestId_ = requestId;
    if (!tryEnqueueChatRequest(std::move(chatRequest))) {
        return;
    }
    chatAsync();
}

void Chatbot::generateContent(const std::string& prompt, const ChatResultCallback& resultCallback,
                              const ChatFinishedCallback& finishedCallback,
                              const ToolHandlerResultCallback& toolHandlerCallback) {
    generateContent(prompt, std::string{}, resultCallback, finishedCallback, toolHandlerCallback);
}

void Chatbot::generateContent(const std::string& prompt, const std::string& requestId,
                              const ChatResultCallback& resultCallback, const ChatFinishedCallback& finishedCallback,
                              const ToolHandlerResultCallback& toolHandlerCallback) {
    Json::Value requestBody = util::stringToJson(prompt);
    ChatRequest chatRequest;
    chatRequest.body = requestBody;
    chatRequest.withHistoryMessage = false;
    chatRequest.resultCallback = resultCallback;
    chatRequest.finishedCallback = finishedCallback;
    chatRequest.requestType = ChatRequest::RequestType::GenerateContent;
    chatRequest.toolHandlerCallback = toolHandlerCallback;
    chatRequest.requestId_ = requestId;
    if (!tryEnqueueChatRequest(std::move(chatRequest))) {
        return;
    }
    chatAsync();
}

void Chatbot::chatWithHistoryMessage(const std::string& messages, const ChatResultCallback& resultCallback,
                                     const ChatFinishedCallback& finishedCallback,
                                     const ToolHandlerResultCallback& toolHandlerCallback) {
    chatWithHistoryMessage(messages, std::string{}, resultCallback, finishedCallback, toolHandlerCallback);
}

void Chatbot::chatWithHistoryMessage(const std::string& messages, const std::string& requestId,
                                     const ChatResultCallback& resultCallback,
                                     const ChatFinishedCallback& finishedCallback,
                                     const ToolHandlerResultCallback& toolHandlerCallback) {
    Json::Value requestBody = util::stringToJson(messages);
    ChatRequest chatRequest{};
    chatRequest.body = requestBody;
    chatRequest.withHistoryMessage = true;
    chatRequest.resultCallback = resultCallback;
    chatRequest.finishedCallback = finishedCallback;
    chatRequest.requestType = ChatRequest::RequestType::Chat;
    chatRequest.toolHandlerCallback = toolHandlerCallback;
    chatRequest.requestId_ = requestId;
    if (!tryEnqueueChatRequest(std::move(chatRequest))) {
        return;
    }
    chatAsync();
}

void Chatbot::stopChat() {
    clearChatContext();
    if (nlpEngine_) {
        nlpEngine_->stopChat();
    }
    if (fallbackNlpEngine_) {
        fallbackNlpEngine_->stopChat();
    }
    if (currentChatRequest_.finishedCallback != nullptr) {
        currentChatRequest_.finishedCallback();
    }
    ChatRequest dropped{};
    while (chatRequestQueue_.try_dequeue(dropped)) {
        decrementChatQueueSize();
        notifyChatRequestStopped(dropped);
    }
}

void Chatbot::stopChat(const std::string& requestId) {
    if (requestId.empty()) {
        stopChat();
        return;
    }

    if (currentChatRequest_.requestId_ == requestId) {
        if (nlpEngine_) {
            nlpEngine_->stopChat();
        }
        if (fallbackNlpEngine_) {
            fallbackNlpEngine_->stopChat();
        }
        notifyChatRequestStopped(currentChatRequest_);
        return;
    }

    ChatRequest pending{};
    std::vector<ChatRequest> keptRequests;
    while (chatRequestQueue_.try_dequeue(pending)) {
        decrementChatQueueSize();
        if (pending.requestId_ == requestId) {
            notifyChatRequestStopped(pending);
            continue;
        }
        keptRequests.push_back(std::move(pending));
    }
    for (auto& request : keptRequests) {
        chatRequestQueue_.enqueue(std::move(request));
        chatQueueSize_.fetch_add(1, std::memory_order_relaxed);
    }
}

void Chatbot::clearChatContext() { chatMessages_.clear(); }

void Chatbot::setTopK(int topK) {
    chatMessages_.setTopK(topK);
    topK_ = topK;
}

void Chatbot::setStream(bool stream) {
    chatMessages_.setStream(stream);
    stream_ = stream;
}

void Chatbot::addSystemMessage(const std::string& message) { chatMessages_.setSystemMessage(message, -1); }

void Chatbot::addSystemMessage(const std::string& message, int taskId) {
    chatMessages_.setSystemMessage(message, taskId);
}

void Chatbot::addSystemMessageByPromptId(int promptId, DeployType deployType) {
    if (!promptTemplate_) {
        LOGE("Prompt template is not initialized, skip setting system message by promptId {}.", promptId);
        return;
    }

    const auto prompt = promptTemplate_->prompt(promptId);
    if (prompt.content.empty()) {
        LOGE("Prompt content is empty for promptId {}.", promptId);
        return;
    }

    using PromptId = config::PromptTemplate::PromptId;
    if (promptId == static_cast<int>(PromptId::DocTagExtraction) && deployType == DeployType::Local) {
        LOGI("Current promptId is {}, deployType is {}", promptId, int(deployType));
        LOGI("Current prompt content: {}", prompt.content);
        addSystemMessage(prompt.content, static_cast<int>(TaskId::TagExtraction));
        return;
    }
    addSystemMessage(prompt.content);
}

void Chatbot::addUserMessage(const std::string& message) { chatMessages_.addUserQuestion(message); }

void Chatbot::addAssistantMessage(const std::string& message) { chatMessages_.addAssistantReply(message); }

void Chatbot::addToolMessage(const std::string& message) { chatMessages_.addToolReply(message); }

void Chatbot::setDeployType(DeployType deployType) { deployType_ = deployType; }

DeployType Chatbot::getDeployType() const { return deployType_; }

void Chatbot::destroy() {
    try {
        stopChat();
        for (auto& item : engineCache_) {
            if (item.second) {
                item.second->setTextGenerationResultCallback(nullptr);
            }
        }
    } catch (std::exception& except) {
        LOGE("{}", except.what());
    }
}

void Chatbot::registerTool(const std::string& toolName, int toolType, const std::string& jsonSchema) {
    if (toolName.empty() || jsonSchema.empty()) {
        LOGD("toolName or jsonSchema is empty");
        return;
    }
    // toolType 目前仅支持 GENAI_TEXT_TOOL_TYPE_FUNCTION
    if (toolType != static_cast<int>(GenAiToolType::GENAI_TEXT_TOOL_TYPE_FUNCTION)) {
        LOGD("toolType is not supported");
        return;
    }
    for (const auto& existingTool : tools) {
        if (existingTool.name == toolName) {
            LOGD("Tool {} already exists", toolName);
            return;
        }
    }
    GenAiTool tool;
    tool.name = toolName;
    tool.type = static_cast<GenAiToolType>(toolType);
    tool.jsonSchema = jsonSchema;
    LOGI("Register tool: {} {} {}", toolName, toolType, jsonSchema);
    tools.emplace_back(tool);
}

void Chatbot::clearTools() { tools.clear(); }

void Chatbot::submitToolResult(const std::string& toolCallsJson, const std::string& argsJson) {
    Json::Value toolCallsJsonValue = util::stringToJson(toolCallsJson);
    if (toolCallsJsonValue.isArray()) {
        addAssistantMessage(toolCallsJson);
    } else if (toolCallsJsonValue.isObject()) {
        Json::Value toolCallsArray(Json::arrayValue);
        toolCallsArray.append(toolCallsJsonValue);
        addAssistantMessage(util::jsonToString(toolCallsArray));
    } else {
        addAssistantMessage(toolCallsJson);
    }
    Json::Value argsJsonValue = util::stringToJson(argsJson);
    if (argsJsonValue.isArray()) {
        for (auto& toolCall : argsJsonValue) {
            addToolMessage(util::jsonToString(toolCall));
        }
    }
}

#ifdef TASKRUNNER_TESTING
void Chatbot::setEngineForTesting(const std::shared_ptr<INlpEngine>& engine, const GenNlpModelConfig& config) {
    if (!engine) {
        return;
    }
    std::string key = buildEngineCacheKey(config);
    engineCache_[key] = engine;
    engineInitState_[key] = false;
}
#endif

void Chatbot::handleChatResult(const ai_engine::gen_ai::nlp::TextGenerationResult& result) {
    if (timedOut_.load()) {
        return;
    }
    ChatResult chatResult;

    Json::Value resultText = util::stringToJson(result.text);
    // 处理reasoning_content
    if (resultText.isMember("reasoning_content")) {
        chatResult.reasoningContent_ = resultText["reasoning_content"].asString();
    }
    // 处理reason
    if (resultText.isMember("result")) {
        chatResult.reason_ = resultText["result"].asString();
    }
    // 处理is_end
    if (resultText.isMember("is_end")) {
        chatResult.isEnd_ = resultText["is_end"].asBool();
        if (chatResult.isEnd_ && chatResult.reason_ == "<|stopChat_8167431|>") {
            chatResult.reason_ = "";
        } else {
            replyCache_ += chatResult.reason_;
        }
        if (chatResult.isEnd_ && !currentChatRequest_.withHistoryMessage) {
            chatMessages_.addAssistantReply(replyCache_);
            replyCache_.clear();
        }
    }
    // 处理finishReason
    if (resultText.isMember("finish_reason")) {
        chatResult.finishReason_ = resultText["finish_reason"].asString();
    }
    // 处理modelName
    chatResult.modelName_ = result.modelName;
    chatResult.error_ = result.error;
    if (resultText.isMember("search_info") && resultText["search_info"].isArray()) {
        chatResult.searchInfo_ = util::jsonToString(resultText["search_info"]);
    }
    if (!currentChatRequest_.requestId_.empty()) {
        chatResult.requestId_ = currentChatRequest_.requestId_;
    }

    // 检查是否有工具调用
    if (resultText.isMember("tool_calls")) {
        Json::Value toolCalls = resultText["tool_calls"];
        if (toolCalls.isArray() && !toolCalls.empty()) {
            Json::Value toolsJson;
            for (auto toolCall : toolCalls) {
                if (toolCall.isMember("function")) {
                    const Json::Value& function = toolCall["function"];
                    Json::Value toolJson;
                    toolJson["tool_name"] = function["name"].asString();
                    toolJson["call_id"] = toolCall["id"].asString();
                    toolJson["arguments_json"] = function["arguments"].asString();
                    toolsJson.append(toolJson);
                }
            }
            ToolHandlerResult toolResult;
            toolResult.toolsJson = util::jsonToString(toolsJson);
            if (currentChatRequest_.toolHandlerCallback) {
                currentChatRequest_.toolHandlerCallback(toolResult);
            }
            return;
        }
    }
    callChatResultCallback(chatResult);
}

void Chatbot::callChatResultCallback(const ChatResult& result) const {
    if (timedOut_.load()) {
        return;
    }
    if (currentChatRequest_.resultCallback != nullptr) {
        currentChatRequest_.resultCallback(result);
    }
}

void Chatbot::callChatFinishedCallback() const {
    if (timedOut_.load()) {
        return;
    }
    if (currentChatRequest_.finishedCallback != nullptr) {
        currentChatRequest_.finishedCallback();
    }
}

void Chatbot::notifyChatRequestFailed(const ChatRequest& request, const ai_engine::EngineError& error) const {
    if (request.resultCallback != nullptr) {
        ChatResult chatResult{};
        chatResult.error_ = error;
        chatResult.requestId_ = request.requestId_;
        chatResult.needConvertErrorCode_ = false;
        request.resultCallback(chatResult);
    }
    if (request.finishedCallback != nullptr) {
        request.finishedCallback();
    }
}

void Chatbot::notifyChatRequestStopped(const ChatRequest& request) const {
    if (request.resultCallback != nullptr) {
        ChatResult chatResult{};
        chatResult.finishReason_ = "stop";
        chatResult.isEnd_ = true;
        chatResult.error_ = ai_engine::EngineError(AiSdkCommonErrorCode::AI_COMMON_SUCCESS, "");
        chatResult.requestId_ = request.requestId_;
        chatResult.needConvertErrorCode_ = false;
        request.resultCallback(chatResult);
    }
    if (request.finishedCallback != nullptr) {
        request.finishedCallback();
    }
}

void Chatbot::rejectPendingRequests(const ai_engine::EngineError& error) {
    ChatRequest dropped{};
    while (chatRequestQueue_.try_dequeue(dropped)) {
        decrementChatQueueSize();
        notifyChatRequestFailed(dropped, error);
    }
}

GenNlpModelConfig Chatbot::getRequestModelConfig() const {
    return GenNlpModelConfig::parseModelConfig(util::jsonToString(currentChatRequest_.body));
}

std::string Chatbot::buildEngineCacheKey(const GenNlpModelConfig& config) const {
    if (config.isDefaultMode()) {
        return "default";
    }
    return fmt::format("{}#{}", config.name, int(config.type));
}

bool Chatbot::ensureEngineReady(const GenNlpModelConfig& config, const std::string& input, util::Watchdog& watchdog) {
    std::string key = buildEngineCacheKey(config);
    std::shared_ptr<INlpEngine> engine;
    // 未切换过，使用缓存，其他条件都要重选模型
    if (!defaultModelChanged_) {
        if (auto cached = engineCache_.find(key); cached != engineCache_.end()) {
            engine = cached->second;
        }
    }
    if (!engine) {
        if (!config.isDefaultMode() && !config.name.empty() &&
            !ModelConfigManager::getInstance().isNlpModelConfigured(config.name)) {
            LOGE("Nlp model {} not configured.", config.name);
            if (watchdog.TryFinish()) {
                ai_engine::EngineError error(AiSdkCommonErrorCode::AISDK_MODEL_NOT_FOUND, "Nlp model not configured.");
                notifyChatRequestFailed(currentChatRequest_, error);
            }
            return false;
        }

        std::unique_ptr uniqueEngine = config.isDefaultMode()
                                           ? engineSelector_.selectGenAiTextGenerationEngine()
                                           : engineSelector_.selectGenAiTextGenerationEngine(config.type, config.name);
        engine = std::move(uniqueEngine);
        if (!engine) {
            LOGE("No nlp engine found!");
            if (watchdog.TryFinish()) {
                ai_engine::EngineError error(AiSdkCommonErrorCode::AISDK_MODEL_NOT_FOUND, "No nlp engine found.");
                notifyChatRequestFailed(currentChatRequest_, error);
            }
            return false;
        }
        engineCache_[key] = engine;
        engineInitState_[key] = false;
        if (key == "default") {
            defaultModelChanged_ = false;
        }
    }
    nlpEngine_ = engine;
    currentEngineKey_ = key;
    promptTemplate_ = std::make_shared<config::PromptTemplate>(
        config::defaultPromptTemplateConfigFile(nlpEngine_->engineName()),
        config::defaultPromptFilePath(nlpEngine_->engineName(), nlpEngine_->currentModel()));
    if (!engineInitState_[key]) {
        auto result = nlpEngine_->init();
        if (!result.success) {
            LOGE("Failed to init nlp engine: {}", result.error.getMessage());
            int errorCode = gen_ai::nlp::error::getNlpErrorCode(result.error.getCode());
            if (errorCode && config.isDefaultMode()) {
                bool success = switchToNextPriorityModel(config, input);
                if (success) {
                    if (watchdog.TryFinish()) {
                        callChatFinishedCallback();
                    }
                    return false;
                }
            }
            if (watchdog.TryFinish()) {
                notifyChatRequestFailed(currentChatRequest_, result.error);
            }
            return false;
        }
        engineInitState_[key] = true;
    }
    std::shared_ptr sharedObject = getSharedObject();
    nlpEngine_->setTextGenerationResultCallback(
        [sharedObject](const ai_engine::gen_ai::nlp::TextGenerationResult& result) {
            sharedObject->handleChatResult(result);
        });
    deployType_ = currentEngineDeployType();
    return true;
}

void Chatbot::runChatRequest(const std::string& input, util::Watchdog& watchdog, const GenNlpModelConfig& config) {
    ModelStatusTracker statusTracker(nlpEngine_->currentModelName());
    ai_engine::result::AiEngineBaseResult result{};
    if (currentChatRequest_.requestType == ChatRequest::RequestType::Chat) {
        result = nlpEngine_->chat(input);

    } else {
        result = nlpEngine_->generateContent(input);
    }
    if (watchdog.IsFinished()) {
        return;
    }
    if (!result.success) {
        int errorCode = gen_ai::nlp::error::getNlpErrorCode(result.error.getCode());
        if (errorCode && config.isDefaultMode()) {
            bool success = switchToNextPriorityModel(config, input);
            if (success) {
                if (watchdog.TryFinish()) {
                    callChatFinishedCallback();
                }
                return;
            }
        }
        if (watchdog.TryFinish()) {
            ChatResult chatResult{};
            chatResult.error_ = result.error;
            if (!currentChatRequest_.requestId_.empty()) {
                chatResult.requestId_ = currentChatRequest_.requestId_;
            }
            callChatResultCallback(chatResult);
            callChatFinishedCallback();
        }
        return;
    }
    if (watchdog.TryFinish()) {
        callChatFinishedCallback();
    }
}

void Chatbot::chatAsync() {
    bool expected = false;
    if (!taskRunning_.compare_exchange_strong(expected, true)) {
        return;
    }
    ChatRequest nextRequest{};
    if (!chatRequestQueue_.try_dequeue(nextRequest)) {
        taskRunning_.store(false);
        return;
    }
    decrementChatQueueSize();
    task::TaskId taskId = task::generateTaskId();
    std::shared_ptr sharedObject = getSharedObject();
    auto enqueueResult = task::TaskRunner::getInstance().addTask(
        {taskId, task::TaskType::TextChat, [sharedObject, nextRequest]() mutable {
             sharedObject->chatTask(std::move(nextRequest));
             sharedObject->taskRunning_.store(false);
             sharedObject->chatAsync();
         }});
    if (enqueueResult != task::TaskRunner::EnqueueResult::Accepted) {
        LOGE("Failed to enqueue TextChat task.");
        taskRunning_.store(false);
        ai_engine::EngineError error(AI_COMMON_TOO_MANY_REQUESTS, "Too many requests, please try again later.");
        if (enqueueResult == task::TaskRunner::EnqueueResult::RejectedStopping) {
            error = ai_engine::EngineError(AISDK_SERVICE_ERROR, "Task runner is stopping.");
        }
        notifyChatRequestFailed(nextRequest, error);
        rejectPendingRequests(error);
    }
}

bool Chatbot::tryEnqueueChatRequest(ChatRequest request) {
    const size_t maxSize = loadChatRequestQueueMaxSize();
    size_t pending = chatQueueSize_.fetch_add(1, std::memory_order_relaxed);
    if (pending >= maxSize) {
        chatQueueSize_.fetch_sub(1, std::memory_order_relaxed);
        LOGW("Chat request queue is full, reject request.");
        ai_engine::EngineError error(AI_COMMON_TOO_MANY_REQUESTS, "Too many requests, please try again later.");
        notifyChatRequestFailed(request, error);
        return false;
    }
    chatRequestQueue_.enqueue(std::move(request));
    return true;
}

void Chatbot::decrementChatQueueSize() {
    size_t current = chatQueueSize_.fetch_sub(1, std::memory_order_relaxed);
    if (current == 0) {
        chatQueueSize_.store(0, std::memory_order_relaxed);
    }
}

std::string Chatbot::createChatInput() {
    std::string input;
    if (currentChatRequest_.requestType == ChatRequest::RequestType::Chat) {
        if (currentChatRequest_.withHistoryMessage) {
            Json::Value processedBody = currentChatRequest_.body;
            if (!processedBody.isMember("tools") && processedBody.isMember("tool_choice")) {
                processedBody.removeMember("tool_choice");
            }
            input = util::jsonToString(processedBody);
        } else {
            std::string question = currentChatRequest_.body["question"].asString();
            chatMessages_.addUserQuestion(question);
            chatMessages_.setTopK(currentChatRequest_.body["top_k"].asInt());
            chatMessages_.setStream(currentChatRequest_.body["stream"].asBool());
            chatMessages_.setWebSearch(currentChatRequest_.body["web_search"].asBool());
            chatMessages_.setThinkingEnabled(currentChatRequest_.body["enable_thinking"].asBool());
            if (currentChatRequest_.body.isMember("tool_choice")) {
                chatMessages_.setTools(tools, currentChatRequest_.body["tool_choice"]);
            } else {
                // 不使用工具调用时应重置该字段，否则会干扰模型输出
                chatMessages_.resetToolChoice();
            }
            Json::Value modelRequest = util::stringToJson(chatMessages_.generateJsonString());
            for (const auto& item : modelContextMember) {
                if (!currentChatRequest_.body.isMember(item)) {
                    continue;
                }
                modelRequest[item] = currentChatRequest_.body[item];
            }
            input = util::jsonToString(modelRequest);
        }
    } else {
        Json::Value json;
        json["prompt"] = currentChatRequest_.body["prompt"];
        if (currentChatRequest_.body.isMember("top_k") && currentChatRequest_.body["top_k"].isInt()) {
            if (currentChatRequest_.body["top_k"].asInt() != 0) {
                json["top_k"] = currentChatRequest_.body["top_k"].asInt();
            }
        }
        if (currentChatRequest_.body.isMember("stream") && currentChatRequest_.body["stream"].isBool()) {
            json["stream"] = currentChatRequest_.body["stream"].asBool();
        }
        if (currentChatRequest_.body.isMember("web_search") && currentChatRequest_.body["web_search"].isBool()) {
            json["web_search"] = currentChatRequest_.body["web_search"].asBool();
        }
        if (currentChatRequest_.body.isMember("enable_thinking") &&
            currentChatRequest_.body["enable_thinking"].isBool()) {
            json["enable_thinking"] = currentChatRequest_.body["enable_thinking"].asBool();
        }
        for (const auto& item : modelContextMember) {
            if (!currentChatRequest_.body.isMember(item)) {
                continue;
            }
            json[item] = currentChatRequest_.body[item];
        }
        input = util::jsonToString(json);
    }
    return input;
}

void Chatbot::chatTask(ChatRequest request) {
    currentChatRequest_ = std::move(request);
    timedOut_.store(false);
    GenNlpModelConfig requestConfig = getRequestModelConfig();
    std::string input = createChatInput();
    util::Watchdog watchdog;
    watchdog.Start(loadChatTimeoutMs(), [this]() {
        timedOut_.store(true);
        if (nlpEngine_) {
            nlpEngine_->stopChat();
        }
        if (fallbackNlpEngine_) {
            fallbackNlpEngine_->stopChat();
        }
        ai_engine::EngineError error(AiSdkCommonErrorCode::AI_COMMON_MODEL_RUN_TIME_OUT, "Chat request timed out.");
        notifyChatRequestFailed(currentChatRequest_, error);
    });
    if (ensureEngineReady(requestConfig, input, watchdog)) {
        runChatRequest(input, watchdog, requestConfig);
    }
    watchdog.Finish();
}

bool Chatbot::switchToNextPriorityModel(const GenNlpModelConfig& config, const std::string& input) {
    if (!config.isDefaultMode()) {
        // Non-default config does not support fallback.
        return false;
    }
    DeployType type = currentEngineDeployType();

    EngineSelector::GenAiTextGenerationEnginePredicate predicate =
        [](const std::unique_ptr<ai_engine::gen_ai::nlp::AbstractTextGenerationEngine>& engine) {
            if (engine == nullptr) {
                return false;
            }
            auto result = engine->init();
            if (!result.success) {
                return false;
            }
            return true;
        };
    std::unique_ptr engine = engineSelector_.selectNextPriorityGenAiTextGenerationEngine(type, predicate);
    if (engine == nullptr) {
        return false;
    }
    fallbackNlpEngine_ = std::move(engine);
    std::shared_ptr sharedObject = getSharedObject();
    fallbackNlpEngine_->setTextGenerationResultCallback(
        [sharedObject](const ai_engine::gen_ai::nlp::TextGenerationResult& result) {
            sharedObject->handleChatResult(result);
        });

    ai_engine::result::AiEngineBaseResult result{};
    if (currentChatRequest_.requestType == ChatRequest::RequestType::Chat) {
        result = fallbackNlpEngine_->chat(input);
    } else {
        result = fallbackNlpEngine_->generateContent(input);
    }

    if (result.success) {
        fallbackNlpEngine_ = nullptr;
        return true;
    }
    DeployType nextType = DeployType::Unknown;
    if (fallbackNlpEngine_->isCustomModelEngine()) {
        nextType = DeployType::Custom;
    } else if (fallbackNlpEngine_->isCloud()) {
        nextType = DeployType::PublicCloud;
    } else {
        nextType = DeployType::Local;
    }

    LOGI("Switched model chat failed, trying next priority again. DeployType: {}, Error: {} {}", static_cast<int>(type),
         result.error.getCode(), result.error.getMessage());

    std::unique_ptr secondEngine = engineSelector_.selectNextPriorityGenAiTextGenerationEngine(nextType, predicate);
    if (secondEngine == nullptr) {
        LOGE("Switched model chat failed, no more priority model available.");
        fallbackNlpEngine_ = nullptr;
        return false;
    }

    fallbackNlpEngine_ = std::move(secondEngine);
    fallbackNlpEngine_->setTextGenerationResultCallback(
        [sharedObject](const ai_engine::gen_ai::nlp::TextGenerationResult& result) {
            sharedObject->handleChatResult(result);
        });

    ai_engine::result::AiEngineBaseResult secondResult{};
    if (currentChatRequest_.requestType == ChatRequest::RequestType::Chat) {
        secondResult = fallbackNlpEngine_->chat(input);
    } else {
        secondResult = fallbackNlpEngine_->generateContent(input);
    }
    fallbackNlpEngine_ = nullptr;

    return secondResult.success;
}

DeployType Chatbot::currentEngineDeployType() const {
    DeployType type{DeployType::Unknown};
    if (!nlpEngine_) {
        return type;
    }
    if (nlpEngine_->isCustomModelEngine()) {
        type = DeployType::Custom;
    } else if (nlpEngine_->isCloud()) {
        type = DeployType::PublicCloud;
    } else {
        type = DeployType::Local;
    }
    return type;
}
