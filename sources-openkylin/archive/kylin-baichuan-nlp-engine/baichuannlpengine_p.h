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

#ifndef BAICHUANNLPENGINE_P_H
#define BAICHUANNLPENGINE_P_H

#include <jsoncpp/json/json.h>
#include <kylin-ai/ai-engine/aiengineerror.h>
#include <kylin-ai/ai-engine/textgenerationengine.h>

#include <functional>

#include "cpr/cpr.h"

class BaichuanNlpEnginePrivate
    : public ai_engine::gen_ai::nlp::AbstractTextGenerationEngine {
public:
    BaichuanNlpEnginePrivate();
    ~BaichuanNlpEnginePrivate();
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

    ai_engine::result::AiEngineBaseResult generateContent(
        const std::string &input) override;
    ai_engine::result::AiEngineBaseResult chat(
        const std::string &message) override;
    void stopGenerateContent() override;
    void stopChat() override;

    ai_engine::result::AiEngineBaseResult init() override;
    ai_engine::result::AiEngineBaseResult destroy() override;

private:
    std::string apiKey_;
    std::string currentModel_{"baichuan3-turbo"};
    std::string currentModelName_{"百川-Turbo-3"};
    int maxTokens_;

    ai_engine::gen_ai::nlp::TextGenerationResultCallback
        textGenerationResultCallback_{nullptr};

    Json::Value context_;
    int sentenceId_{0};
    void clearContext();
    void addContext_(const std::string &role, const std::string &message);
    void removeLastContext_();
    std::vector<Json::Value> streamChatData_;
    std::string tempStreamChatData_;

    friend bool writeChatData(std::string data, intptr_t userData);
    int getMaxTokens(const std::string &model);

    ai_engine::result::AiEngineBaseResult processChatResponse_(
        cpr::Response &response);

    ai_engine::EngineError currentError_;
    std::string currentErrorInfo_;

    bool waitForStopped();
    std::atomic<bool> isStopped_{false};
    bool isChatCall_{false};
};

#endif  // BAICHUANNLPENGINE_P_H
