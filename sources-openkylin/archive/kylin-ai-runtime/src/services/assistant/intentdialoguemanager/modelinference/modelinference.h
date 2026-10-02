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

#ifndef SERVICES_ASSISTANT_MODEL_INFERENCE_H
#define SERVICES_ASSISTANT_MODEL_INFERENCE_H

#include <future>
#include <mutex>

#include "assistant/sessionbase.h"
#include "engineselector/engineselector.h"
#include "modelconfig/modelconfigmanager.h"
#include "modelinferenceinterface.h"

class ModelInference : public ModelInferenceInterface {
public:
    ModelInference() = default;

    ~ModelInference() override;

    std::future<bool> callChatModelInference(const std::string &modelInput) override;

    DialogueData::CallSuccessIdentifier callIntentionModelInference(const std::string &modelInput) override;

    std::string intentModelName() override;

    std::string intentModelDisplayName() override;

    std::string chatModelName() override;

    void initChatEngine(const std::shared_ptr<assistant::TextGenerationEngine> &nlpEngine,
                        EngineSelector &engineSelector_, bool isDefaultMode);

    void initIntentEngine(const std::shared_ptr<assistant::TextGenerationEngine> &nlpEngine);

    int getCurrentModelMaxTokens() override;
    bool isCloudModel() override;
    bool isChatModelConfigured() override;
    bool isIntentModelNotLoaded() override;

    void stopChat() override;

private:
    DeployType getDeployTypeByEngine(const assistant::TextGenerationEngine *engine);
    bool switchToNextPriorityModel(
        const std::string &modelInput, DeployType currentType,
        const std::function<void(const ai_engine::gen_ai::nlp::TextGenerationResult &result)> &callback);

    void waitForChatEngineInited();
    void waitForIntentEngineInited();

    // 用于在初始化方法中等待之前的初始化完成
    void waitForChatEngineInitInternal();
    void waitForIntentEngineInitInternal();

private:
    std::shared_ptr<assistant::TextGenerationEngine> chatEngine_;
    std::shared_ptr<assistant::TextGenerationEngine> intentEngine_;
    std::string intentReplyCache_;
    std::string intentResult_;
    std::string intentModelName_;
    std::atomic_int intentModelCurrentErrorCode_;
    EngineSelector *engineSelector_{nullptr};
    std::atomic_bool isChatEngineInited_{false};
    std::atomic_bool isDefaultMode_{true};
    std::future<void> chatEngineInitFuture_;
    std::future<void> intentEngineInitFuture_;
    std::mutex chatEngineInitMutex_;    // 保护聊天引擎初始化过程
    std::mutex intentEngineInitMutex_;  // 保护意图引擎初始化过程
    std::mutex chatEngineChatMutex_;
    std::shared_ptr<assistant::TextGenerationEngine> fallbackChatEngine_;  // 存储当前切换的备用引擎，用于停止会话
};

#endif  // SERVICES_ASSISTANT_MODEL_INFERENCE_H
