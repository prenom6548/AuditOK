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

#ifndef SERVICES_CHATBOT_H
#define SERVICES_CHATBOT_H

#include <kylin-ai/aiengine.h>
#include <modelconfig/modelconfigenums.h>

#include <atomic>
#include <cstddef>
#include <map>
#include <memory>

#include "chatmessages.h"
#include "concurrentqueue/blockingconcurrentqueue.h"
#include "config/prompttemplate.h"
#include "engineselector/engineselector.h"
#include "genainlpmodelconfig.h"
#include "taskmanager/taskrunner.h"
#include "utils/watchdog.h"

enum class TaskId {
    NormalChat = -1,
    TagExtraction,
    IntentRecognition,
};

class Chatbot : public std::enable_shared_from_this<Chatbot> {
private:
    using INlpEngine = ai_engine::gen_ai::nlp::AbstractTextGenerationEngine;

public:
    struct ChatResult {
        std::string reason_;
        std::string reasoningContent_;
        std::string modelName_;
        std::string finishReason_;
        ai_engine::EngineError error_;
        bool isEnd_{true};
        std::string searchInfo_;
        std::string requestId_;
        bool needConvertErrorCode_{true};
    };

    struct ToolHandlerResult {
        std::string toolsJson;
    };

    using ChatResultCallback = std::function<void(const ChatResult &)>;
    using ToolHandlerResultCallback = std::function<void(const ToolHandlerResult &)>;
    using ChatFinishedCallback = std::function<void()>;

public:
    Chatbot(EngineSelector &engineSelector);

    ~Chatbot();

    void init(int sessionId);

    void chat(const std::string &question, const ChatResultCallback &resultCallback,
              const ChatFinishedCallback &finishedCallback,
              const ToolHandlerResultCallback &toolHandlerCallback = nullptr);
    void chat(const std::string &question, const std::string &requestId, const ChatResultCallback &resultCallback,
              const ChatFinishedCallback &finishedCallback,
              const ToolHandlerResultCallback &toolHandlerCallback = nullptr);

    void generateContent(const std::string &prompt, const ChatResultCallback &resultCallback,
                         const ChatFinishedCallback &finishedCallback,
                         const ToolHandlerResultCallback &toolHandlerCallback = nullptr);
    void generateContent(const std::string &prompt, const std::string &requestId,
                         const ChatResultCallback &resultCallback, const ChatFinishedCallback &finishedCallback,
                         const ToolHandlerResultCallback &toolHandlerCallback = nullptr);

    void chatWithHistoryMessage(const std::string &messages, const ChatResultCallback &resultCallback,
                                const ChatFinishedCallback &finishedCallback,
                                const ToolHandlerResultCallback &toolHandlerCallback = nullptr);
    void chatWithHistoryMessage(const std::string &messages, const std::string &requestId,
                                const ChatResultCallback &resultCallback, const ChatFinishedCallback &finishedCallback,
                                const ToolHandlerResultCallback &toolHandlerCallback = nullptr);

    void stopChat();
    void stopChat(const std::string &requestId);

    void clearChatContext();

    void setTopK(int topK);

    void setStream(bool stream);

    void addSystemMessage(const std::string &message);

    void addSystemMessage(const std::string &message, int taskId);

    void addSystemMessageByPromptId(int promptId, DeployType deployType);

    void addUserMessage(const std::string &message);

    void addAssistantMessage(const std::string &message);

    void addToolMessage(const std::string &message);

    void setDeployType(DeployType deployType);

    DeployType getDeployType() const;

    void destroy();

    void registerTool(const std::string &toolName, int toolType, const std::string &jsonSchema);

    void clearTools();

    void submitToolResult(const std::string &toolCallsJson, const std::string &argsJson);

#ifdef TASKRUNNER_TESTING
    void setEngineForTesting(const std::shared_ptr<INlpEngine> &engine, const GenNlpModelConfig &config = {});
#endif

private:
    struct ChatRequest {
        enum class RequestType { Chat, GenerateContent };
        Json::Value body;
        bool withHistoryMessage{false};
        ChatResultCallback resultCallback{nullptr};
        ChatFinishedCallback finishedCallback{nullptr};
        RequestType requestType{RequestType::Chat};
        ToolHandlerResultCallback toolHandlerCallback{nullptr};
        std::string requestId_;
    };

private:
    std::shared_ptr<Chatbot> getSharedObject() { return shared_from_this(); }
    void handleChatResult(const ai_engine::gen_ai::nlp::TextGenerationResult &result);
    void callChatResultCallback(const ChatResult &result) const;
    void callChatFinishedCallback() const;

    std::string createChatInput();

    void chatAsync();

    void chatTask(ChatRequest request);

    bool tryEnqueueChatRequest(ChatRequest request);
    void decrementChatQueueSize();

    void notifyChatRequestFailed(const ChatRequest &request, const ai_engine::EngineError &error) const;
    void notifyChatRequestStopped(const ChatRequest &request) const;
    void rejectPendingRequests(const ai_engine::EngineError &error);
    bool ensureEngineReady(const GenNlpModelConfig &config, const std::string &input, util::Watchdog &watchdog);
    void runChatRequest(const std::string &input, util::Watchdog &watchdog, const GenNlpModelConfig &config);
    GenNlpModelConfig getRequestModelConfig() const;
    std::string buildEngineCacheKey(const GenNlpModelConfig &config) const;

    bool switchToNextPriorityModel(const GenNlpModelConfig &config, const std::string &input);

    DeployType currentEngineDeployType() const;

private:
    EngineSelector &engineSelector_;
    std::shared_ptr<INlpEngine> nlpEngine_{nullptr};
    std::shared_ptr<config::PromptTemplate> promptTemplate_{nullptr};
    ChatMessages chatMessages_;
    std::string replyCache_;
    DeployType deployType_{DeployType::Unknown};
    ChatRequest currentChatRequest_;
    moodycamel::BlockingConcurrentQueue<ChatRequest> chatRequestQueue_;
    std::atomic<size_t> chatQueueSize_{0};
    std::atomic_bool taskRunning_{false};
    std::atomic_bool timedOut_{false};
    std::map<std::string, std::shared_ptr<INlpEngine> > engineCache_;
    std::map<std::string, bool> engineInitState_;
    std::string currentEngineKey_;
    std::vector<GenAiTool> tools;

    int topK_{};
    bool stream_{false};
    std::atomic_bool defaultModelChanged_{false};
    std::shared_ptr<INlpEngine> fallbackNlpEngine_;  // 存储当前切换的备用引擎，用于停止会话
};

#endif  // SERVICES_CHATBOT_H
