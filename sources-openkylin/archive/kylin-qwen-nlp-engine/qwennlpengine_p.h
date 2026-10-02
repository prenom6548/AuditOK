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

#ifndef QWENNLPENGINE_P_H
#define QWENNLPENGINE_P_H

#include <jsoncpp/json/json.h>
#include <kylin-ai/ai-engine/aiengineerror.h>
#include <kylin-ai/ai-engine/textgenerationengine.h>

#include <functional>

#include "cpr/cpr.h"

class QwenNlpEnginePrivate {
public:
    QwenNlpEnginePrivate();
    ~QwenNlpEnginePrivate();
    std::string engineName() const;
    bool isCloud() const { return true; }
    bool isBuiltInEngine() const { return false; }
    bool isCustomModelEngine() const { return false; }

    void setConfig(const std::string &config);
    bool setCurrentModel(const std::string &model);
    std::string currentModel() const { return currentModel_; }
    std::string currentModelName() const { return currentDisplayName_; }
    std::string modelInfo() const;

    void setTextGenerationResultCallback(const ai_engine::gen_ai::nlp::TextGenerationResultCallback &callback);

    ai_engine::result::AiEngineBaseResult generateContent(const std::string &input);
    ai_engine::result::AiEngineBaseResult chat(const std::string &message);
    void stopGenerateContent();
    void stopChat();

    ai_engine::result::AiEngineBaseResult init();
    ai_engine::result::AiEngineBaseResult destroy();

private:
    void addContextMessage(const std::string &role, const Json::Value &content);
    void convertContextMember(Json::Value &context);
    void addContextMember(const Json::Value &context);
    ai_engine::result::AiEngineBaseResult buildChatMessage(const std::string &message);
    void clearContext();
    void removeLastContext();
    Json::Value dissectToolFunction(const Json::Value &tool);
    void accumulateToolCall(int index, const Json::Value &toolCall);
    ai_engine::gen_ai::nlp::TextGenerationResult handleToolCallsCompletion();
    ai_engine::gen_ai::nlp::TextGenerationResult handleNonStreamCompletion(Json::Value &receivedJson);

    friend bool writeChatData(std::string data, intptr_t userData);
    int getMaxTokens(const std::string &model);

    bool isTopKLimitError(const std::string &errorString, ai_engine::result::AiEngineBaseResult &result);
    ai_engine::result::AiEngineBaseResult processChatResponse(cpr::Response &response);

    bool waitForStopped();

    void decreaseChatCounter();
    void increaseChatCounter();
    bool isChatRunning() const;

private:
    std::string apiKey_;
    std::string currentModel_{"qwen-plus"};
    std::string currentModelName_{"ali-qwen-plus"};
    std::string currentDisplayName_{"通义千问-qwen-plus"};
    int maxTokens_;

    ai_engine::gen_ai::nlp::TextGenerationResultCallback textGenerationResultCallback_{nullptr};

    Json::Value context_;
    int sentenceId_{0};

    std::vector<Json::Value> streamChatData_;
    std::vector<std::pair<int, Json::Value>> toolCallsAccumulator_;
    std::string tempStreamChatData_;

    ai_engine::EngineError currentError_;
    std::string currentErrorInfo_;

    std::atomic<bool> isStopped_{false};
    std::atomic<int> chatCounter_{0};

    std::string nonStreamData_{};
    std::atomic_bool stream_{true};
};

#endif  // QWENNLPENGINE_P_H
