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

#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

#include <string>
#include <kylin-ai/ai-engine/textgenerationengine.h>

namespace ai_engine::gen_ai::nlp {
class LlmTritonClient;

class OndeviceNlpEngine : public AbstractTextGenerationEngine {
public:
    OndeviceNlpEngine();
    ~OndeviceNlpEngine();
    std::string engineName() const override { return "ondevice"; }
    bool isCloud() const override { return false; }
    bool isBuiltInEngine() const override { return true; }
    void setTextGenerationResultCallback(
        TextGenerationResultCallback callback) override;
    virtual result::AiEngineBaseResult
    generateContent(const std::string &input) {
        return result::AiEngineBaseResult(
            {false,
             ai_engine::EngineError((int)NlpEngineErrorCode::InvalidParamType,
                                    "[OndeviceNlpEngine]: Unsupported api, "
                                    "please use chat() api")});
    };
    virtual result::AiEngineBaseResult
    chat(const std::string &message) override;

    virtual void stopGenerateContent() {};
    void stopChat() override;
    // modelInfo函数在init前后都可能被调用
    virtual std::string modelInfo() const override;
    // setCurrentModel函数在init前后都可能被调用
    virtual bool setCurrentModel(const std::string &model) override;
    virtual result::AiEngineBaseResult init() override;
    virtual result::AiEngineBaseResult destroy() override;
    virtual std::string currentModel() const override;
    virtual int maxConcurrentChatTasks() const { return 0; };
    virtual std::string currentModelName() const override;
    virtual void setConfig(const std::string &config) override;

private:
    void releaseTimerCallback();

private:
    LlmTritonClient *llm_triton_client_;
};

} // namespace ai_engine::gen_ai::nlp
