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

#ifndef SENSETIMENLPENGINE_P_H
#define SENSETIMENLPENGINE_P_H

#include <cpr/cpr.h>
#include <jsoncpp/json/json.h>
#include <kylin-ai/ai-engine/aiengineerror.h>
#include <kylin-ai/ai-engine/textgenerationengine.h>
class SensetimeNlpEnginePrivate

    : public ai_engine::gen_ai::nlp::AbstractTextGenerationEngine {
public:
    SensetimeNlpEnginePrivate();
    ~SensetimeNlpEnginePrivate();
    std::string engineName() const override;
    bool isCloud() const override { return true; }
    bool isBuiltInEngine() const override { return false; }
    bool isCustomModelEngine() const override { return false; }

    void setConfig(const std::string &config) override;
    bool setCurrentModel(const std::string &model) override;
    std::string currentModel() const override { return currentModel_; }
    std::string modelInfo() const override;

    void setTextGenerationResultCallback(
        ai_engine::gen_ai::nlp::TextGenerationResultCallback callback) override;

    ai_engine::result::AiEngineBaseResult
    generateContent(const std::string &input) override;
    ai_engine::result::AiEngineBaseResult
    chat(const std::string &message) override;
    void stopGenerateContent() override;
    void stopChat() override;

    ai_engine::result::AiEngineBaseResult init() override;
    ai_engine::result::AiEngineBaseResult destroy() override;

private:
    std::string apiKey_;
    std::string secretKey_;
    std::string accessToken_;
    std::string currentModel_{"SenseChat-5"};
    std::string currentModelName_{"商汤-SenseChat"};
    int maxTokens_;
    bool refreshAccessTokenRequired_{false};
    bool refreshAccessTokenCompleted_{false};
    std::string &getAccessToken_();
    bool generateAccessToken_();

    std::string sessionId_;

    int contextSize_{-1};
    Json::Value context_{};
    void clearContext();
    void addContext_(const std::string &role, const std::string &message);

    int sentenceId_{-1};
    int getSentenceId_() { return ++sentenceId_; }
    void resetSentenceId_() { sentenceId_ = -1; }

    ai_engine::gen_ai::nlp::TextGenerationResultCallback
        textGenerationResultCallback_{nullptr};

    friend bool writeChatData(std::string data, intptr_t userData);
    int getMaxTokens(const std::string &model);
    ai_engine::result::AiEngineBaseResult
    processChatResponse_(cpr::Response &response);

    ai_engine::EngineError currentError_;

    bool waitForStopped();
    std::atomic<bool> isStopped_{false};
    bool isChatCall_{false};
};

#endif // SENSETIMENLPENGINE_P_H
