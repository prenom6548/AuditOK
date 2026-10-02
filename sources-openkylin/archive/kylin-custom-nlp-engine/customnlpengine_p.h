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

#ifndef CUSTOMNLPENGINE_P_H
#define CUSTOMNLPENGINE_P_H

#include <jsoncpp/json/json.h>
#include <kylin-ai/ai-engine/aiengineerror.h>
#include <kylin-ai/ai-engine/textgenerationengine.h>

#include <functional>

#include "cpr/cpr.h"

class CustomNlpEnginePrivate {
public:
    CustomNlpEnginePrivate();
    ~CustomNlpEnginePrivate();
    std::string engineName() const;
    bool isCloud() const { return true; }
    bool isBuiltInEngine() const { return false; }
    bool isCustomModelEngine() const { return true; }

    void setConfig(const std::string &config);
    bool setCurrentModel(const std::string &model);
    std::string currentModel() const { return currentModel_; }
    std::string currentModelName() const { return currentModelName_; }
    std::string modelInfo() const;

    void setTextGenerationResultCallback(ai_engine::gen_ai::nlp::TextGenerationResultCallback callback);

    ai_engine::result::AiEngineBaseResult generateContent(const std::string &input);
    ai_engine::result::AiEngineBaseResult chat(const std::string &message);
    void stopGenerateContent();
    void stopChat();

    ai_engine::result::AiEngineBaseResult init();
    ai_engine::result::AiEngineBaseResult destroy();

private:
    bool isValidResponse(const std::string &response) const;
    Json::Value formatResultJsonFromInputString(const std::string &inputString);
    bool preProcessResultJson(const Json::Value &inputResultJson);
    bool isOpenAiApiResult(const Json::Value &result) const;
    bool isOllamaApiResult(const Json::Value &result) const;
    bool isResultEnd(const Json::Value &result) const;
    bool isThinking() const;
    std::string getThinkingResult(const Json::Value &result) const;
    std::string getResult(const Json::Value &result) const;
    void buildChatAndContentGenerationUrl(const std::string &apiUrl);
    void decreaseChatCounter();
    void increaseChatCounter();
    bool isChatRunning() const;

private:
    void clearContext();
    void addContextMessage(const std::string &role, const Json::Value &message);
    void addContextMember(const Json::Value &context);
    friend bool writeChatData(std::string data, intptr_t userData);
    ai_engine::result::AiEngineBaseResult processChatResponse(cpr::Response &response);
    bool waitForStopped();

    bool hasToolCalls(Json::Value &result);
    ai_engine::result::AiEngineBaseResult buildChatMessage(const std::string &message);
    Json::Value dissectToolFunction(const Json::Value &tool);
    void accumulateToolCall(int index, const Json::Value &toolCall);
    ai_engine::gen_ai::nlp::TextGenerationResult handleToolCallsCompletion();

private:
    std::string apiKey_;
    std::string currentModel_;
    std::string currentModelName_;
    std::string currentDisplayName_;
    std::string chatUrl_;
    std::string generateContentUrl_;

    ai_engine::gen_ai::nlp::TextGenerationResultCallback textGenerationResultCallback_{nullptr};

    Json::Value context_;
    int sentenceId_{0};

    std::string endStr_ = "\n";

    std::vector<Json::Value> streamChatData_;
    std::string tempStreamChatData_;

    ai_engine::EngineError currentError_;
    std::string currentErrorInfo_;

    std::atomic<bool> isStopped_{false};
    std::atomic<int> chatCounter_{0};
    bool isGenerateContentCalled_{false};

    static std::string systemRole_;

    bool isThinking_{false};
    bool hasReasoningContent_{false};

    std::string nonStreamData_{};
    std::atomic_bool stream_{true};
    std::vector<std::pair<int, Json::Value>> toolCallsAccumulator_;
 cpr::Session session_;
};

#endif  // CUSTOMNLPENGINE_P_H
