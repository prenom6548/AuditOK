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

#ifndef BAIDUNLPENGINE_P_H
#define BAIDUNLPENGINE_P_H

#include <functional>
#include <jsoncpp/json/json.h>
#include <kylin-ai/ai-engine/aiengineerror.h>
#include <kylin-ai/ai-engine/textgenerationengine.h>
#include "cpr/cpr.h"

class BaiduNlpEnginePrivate {
public:
    BaiduNlpEnginePrivate();
    ~BaiduNlpEnginePrivate();
    std::string engineName() const;
    bool isCloud() const { return true; }
    bool isBuiltInEngine() const { return false; }
    bool isCustomModelEngine() const { return false; }

    void setConfig(const std::string &config);
    bool setCurrentModel(const std::string &model);
    std::string currentModel() const { return currentModel_; }
    std::string currentModelName() const { return currentModelName_; }
    std::string modelInfo() const;

    void setTextGenerationResultCallback(const ai_engine::gen_ai::nlp::TextGenerationResultCallback &callback);

    ai_engine::result::AiEngineBaseResult generateContent(const std::string &input);
    ai_engine::result::AiEngineBaseResult chat(const std::string &message);
    void stopGenerateContent();
    void stopChat();

    ai_engine::result::AiEngineBaseResult init();
    ai_engine::result::AiEngineBaseResult destroy();

private:
    std::string &getAccessToken();
    bool generateAccessToken();
    void clearContext();
    void addContextMessage(const std::string &role, const std::string &message);
    void convertContextMember(Json::Value &context);
    void addContextMember(const Json::Value &context);
    void addSystemContext();
    void removeLastContext();
    void decreaseChatCounter();
    void increaseChatCounter();
    bool isChatRunning() const;
    bool handleStreamMessage(const std::string &data);
    bool handleNonStreamMessage(const std::string &data);
    static bool onMessage(const std::string &data, intptr_t userData);
    int getMaxTokens(const std::string &model);
    ai_engine::result::AiEngineBaseResult processChatResponse(const cpr::Response &response);
    bool waitForStopped();

private:
    std::string apiKey_;
    std::string secretKey_;
    std::string accessToken_;
    std::string currentModel_{"ernie-4.0-8k"};
    std::string currentModelName_{"baidu-ernie-4.0-8k"};
    std::string currentDisplayName_{"百度-ERNIE-Bot-4"};
    bool refreshAccessTokenRequired_{false};
    bool refreshAccessTokenCompleted_{false};
    int maxTokens_{};
    std::mutex mutex_;
    ai_engine::gen_ai::nlp::TextGenerationResultCallback textGenerationResultCallback_{nullptr};
    Json::Value context_;
    std::vector<Json::Value> streamChatData_;
    std::string tempStreamChatData_;
    ai_engine::EngineError currentError_;
    std::atomic<bool> isStopped_{false};
    std::atomic<int> chatCounter_{0};
    std::string systemRole_;
    std::atomic_bool stream_{true};
};

#endif // BAIDUNLPENGINE_P_H
