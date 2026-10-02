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

#include "modelinference.h"

#include "assistant/handlers/requesthandler.h"
#include "assistant/message.h"
#include "modelconfig/modelstatustracker.h"
#include "threadpool/async.h"
#include "utils/logger.h"
#include "utils/util.h"

ModelInference::~ModelInference() {
    // 等待所有异步初始化任务完成
    if (chatEngineInitFuture_.valid()) {
        try {
            if (chatEngineInitFuture_.wait_for(std::chrono::seconds(5)) == std::future_status::ready) {
                chatEngineInitFuture_.get();
            }
        } catch (const std::exception& e) {
            LOGE("Exception during chat engine cleanup: {}", e.what());
        }
    }

    if (intentEngineInitFuture_.valid()) {
        try {
            if (intentEngineInitFuture_.wait_for(std::chrono::seconds(5)) == std::future_status::ready) {
                intentEngineInitFuture_.get();
            }
        } catch (const std::exception& e) {
            LOGE("Exception during intent engine cleanup: {}", e.what());
        }
    }
    LOGI("ModelInference::~ModelInference");
}

int ModelInference::getCurrentModelMaxTokens() {
    if (chatEngine_ == nullptr) {
        return 0;
    }
    std::string currentModelName = chatEngine_->currentModel();

    Json::Value modelInfoJson = util::stringToJson(chatEngine_->modelInfo());
    int maxContextTokens{};
    for (const auto& item : modelInfoJson["models"]) {
        if (item["model"].asString() == currentModelName) {
            maxContextTokens = item["maxTokens"].asInt();
            break;
        }
    }
    return maxContextTokens;
}

bool ModelInference::isCloudModel() {
    if (chatEngine_ != nullptr) {
        return chatEngine_->isCloud();
    }
    return false;
}

bool ModelInference::isChatModelConfigured() { return chatEngine_ != nullptr; }

bool ModelInference::isIntentModelNotLoaded() {
    if (intentEngine_ == nullptr) {
        return false;
    }
    waitForIntentEngineInited();
    return intentModelCurrentErrorCode_ == static_cast<int>(assistant::ErrorCode::LocalNlpModelNotLoaded);
}

void ModelInference::stopChat() {
    waitForChatEngineInited();
    if (chatEngine_ != nullptr) {
        chatEngine_->stopChat();
    }
    if (intentEngine_ != nullptr) {
        intentEngine_->stopChat();
    }
    if (fallbackChatEngine_ != nullptr) {
        fallbackChatEngine_->stopChat();
    }
}

std::future<bool> ModelInference::callChatModelInference(const std::string& modelInput) {
    // TODO 函数过长，后续优化
    LOGI("Call chat model inference:\n{}", modelInput);

    return cpr::async([this, modelInput] {
        std::lock_guard locker(chatEngineChatMutex_);
        if (chatEngine_ == nullptr) {
            LOGE("Base chat nlp engine is not set.");
            DialogueCallback::ChatData chatData;
            chatData.type_ = DialogueCallback::ChatErrorOccurred;
            chatData.text_ = "";
            chatData.errorCode_ = int(assistant::ErrorCode::NlpModelNotFound);
            chatData.errorMessage_ = "模型加载失败";
            chatData.modelName_ = "";
            callChatModelCallback(chatData);
            return false;
        }

        waitForChatEngineInited();
        // 共享端，如果助手当前选中的本地模型和共享的模型不一样, 报错
        if (!chatEngine_->isCloud() && !isDefaultMode_) {
            auto& modelConfig = ModelConfigManager::getInstance();
            std::string modelConfigCurrentModelName =
                modelConfig.getCurrentModelName(AiCapability::Nlp, DeployType::Local);
            if (modelConfig.isEnableSharedComputingPower() &&
                chatEngine_->currentModelName() != modelConfigCurrentModelName) {
                LOGE("The model has been unloaded. current model name: {}, shared model name: {}",
                     chatEngine_->currentModelName(), modelConfigCurrentModelName);
                DialogueCallback::ChatData chatData;
                chatData.type_ = DialogueCallback::ChatErrorOccurred;
                chatData.text_ = "";
                chatData.errorCode_ = int(assistant::ErrorCode::LocalNlpModelNotLoaded);
                chatData.errorMessage_ = "模型已被卸载";
                chatData.modelName_ = "";
                callChatModelCallback(chatData);
                return false;
            }
        }
        if (chatEngine_->isCloud()) {
            bool isConfigured = ModelConfigManager::getInstance().isNlpModelConfigured(chatEngine_->currentModelName());
            if (!isConfigured) {
                LOGE("Base chat nlp engine is not configured.");
                DialogueCallback::ChatData chatData;
                chatData.type_ = DialogueCallback::ChatErrorOccurred;
                chatData.text_ = "";
                chatData.errorCode_ = int(assistant::ErrorCode::NlpModelNotFound);
                chatData.errorMessage_ = "模型加载失败";
                chatData.modelName_ = "";
                callChatModelCallback(chatData);
                return false;
            }
        }

        LOGI("Chat model name: {} {}", chatEngine_->engineName(), chatEngine_->currentModel());
        auto chatGenerateCallback = [this](const ai_engine::gen_ai::nlp::TextGenerationResult& result) {
            DialogueCallback::ChatData chatData;
            if (result.success) {
                chatData.type_ = DialogueCallback::ChatNormal;
            } else {
                chatData.type_ = DialogueCallback::ChatErrorOccurred;
            }
            chatData.text_ = result.text;
            chatData.errorCode_ = int(assistant::Error::getErrorCodeFromNlpEngine(result.error.getCode()));
            chatData.errorMessage_ = result.error.getMessage();
            chatData.modelName_ = result.modelName;
            if (chatEngine_ != nullptr && chatEngine_->isCloud()) {
                Json::Value modelInfoJson = util::stringToJson(chatEngine_->modelInfo());
                for (const auto& item : modelInfoJson["models"]) {
                    if (item["name"].asString() == chatData.modelName_) {
                        chatData.feedbackUrl_ = item["feedbackUrl"].asString();
                        break;
                    }
                }
            }
            callChatModelCallback(chatData);
        };

        if (!isChatEngineInited_) {
            auto result = chatEngine_->init();
            if (!result.success) {
                int errorCode = int(assistant::Error::getErrorCodeFromNlpEngine(result.error.getCode()));
                // 智能模式下，尝试使用下一优先级模型
                if (errorCode && isDefaultMode_) {
                    // 这里不报错，成功直接返回，不成功继续
                    auto type = getDeployTypeByEngine(chatEngine_.get());
                    bool success = switchToNextPriorityModel(modelInput, type, chatGenerateCallback);
                    if (success) {
                        return true;
                    }
                }
                DialogueCallback::ChatData chatData;
                chatData.type_ = DialogueCallback::ChatErrorOccurred;
                chatData.text_ = "";
                chatData.errorCode_ = errorCode;
                chatData.errorMessage_ = result.error.getMessage();
                chatData.modelName_ = "";
                callChatModelCallback(chatData);
                return false;
            }
            isChatEngineInited_ = true;
            LOGI("Chat engine init success.");
        }

        chatEngine_->setTextGenerationResultCallback(chatGenerateCallback);
        ModelStatusTracker statusTracker(chatEngine_->currentModelName());
        auto result = chatEngine_->chat(modelInput);
        if (!result.success) {
            int errorCode = int(assistant::Error::getErrorCodeFromNlpEngine(result.error.getCode()));
            // 智能模式下，尝试使用下一优先级模型
            if (errorCode && isDefaultMode_) {
                // 这里不报错，成功直接返回，不成功继续
                auto type = getDeployTypeByEngine(chatEngine_.get());
                bool success = switchToNextPriorityModel(modelInput, type, chatGenerateCallback);
                if (success) {
                    return true;
                }
            }
            DialogueCallback::ChatData chatData;
            chatData.type_ = DialogueCallback::ChatErrorOccurred;
            chatData.text_ = "";
            chatData.errorCode_ = errorCode;
            chatData.errorMessage_ = result.error.getMessage();
            chatData.modelName_ = "";

            callChatModelCallback(chatData);
            LOGD("nlp engine error {} {}", errorCode, result.error.getMessage());
            return false;
        }
        return true;
    });
}

DialogueData::CallSuccessIdentifier ModelInference::callIntentionModelInference(const std::string& modelInput) {
    LOGD("意图推理功能输入：{}", modelInput);

    // 新请求开始时清空上一轮状态，避免未收到 is_end 的残留影响本次结果
    intentReplyCache_.clear();
    intentResult_.clear();
    intentModelName_.clear();

    DialogueData::CallSuccessIdentifier ret{};

    if (intentEngine_ == nullptr) {
        LOGE("意图识别引擎未设置");
        ret.successRes = false;
        ret.intentSupported = false;
        ret.errorCode = int(assistant::ErrorCode::IntentNlpModelNotFound);
        ret.errorMessage = "意图识别引擎未设置";
        return ret;
    }

    waitForIntentEngineInited();

    ret.intentSupported = true;
    auto intentGenerateCallback = [this](const ai_engine::gen_ai::nlp::TextGenerationResult& result) {
        Json::Value resultText = util::stringToJson(result.text);
        if (resultText.isMember("result")) {
            intentReplyCache_ += resultText["result"].asString();
        }
        if (resultText.isMember("is_end")) {
            if (resultText["is_end"].asBool()) {
                intentResult_ = intentReplyCache_;
                intentReplyCache_.clear();
                intentModelName_ = result.modelName;
            }
        }

        DialogueCallback::ChatData chatData;
        if (result.success) {
            chatData.type_ = DialogueCallback::IntentNormal;
        } else {
            chatData.type_ = DialogueCallback::IntentErrorOccurred;
        }
        chatData.text_ = result.text;
        chatData.errorCode_ = int(assistant::Error::getErrorCodeFromNlpEngine(result.error.getCode()));
        chatData.errorMessage_ = result.error.getMessage();
        chatData.modelName_ = result.modelName;
        callChatModelCallback(chatData);
    };
    intentEngine_->setTextGenerationResultCallback(intentGenerateCallback);

    Json::Value userInputJson = util::stringToJson(modelInput);
    userInputJson["task_id"] = 1;  // 调用端侧意图模型必加
    std::string userInput = util::jsonToString(userInputJson);

    LOGI("Chat engine name: {}", intentEngine_->engineName());

    ModelStatusTracker statusTracker(intentEngine_->currentModelName());

    LOGD("userInput:{} {}", intentEngine_->currentModelName(), userInput);

    auto result = intentEngine_->chat(userInput);
    if (!result.success) {
        int errorCode = int(assistant::Error::getErrorCodeFromNlpEngine(result.error.getCode()));
        // 智能模式下，尝试使用下一优先级模型
        if (errorCode && isDefaultMode_) {
            // 这里不报错，成功直接返回，不成功继续
            auto type = getDeployTypeByEngine(intentEngine_.get());
            bool success = switchToNextPriorityModel(userInput, type, intentGenerateCallback);
            if (!success) {
                LOGE("NLP engine error info: {} {}", errorCode, result.error.getMessage());
                ret.errorCode = errorCode;
                ret.errorMessage = result.error.getMessage();
                return ret;
            }
        } else if (errorCode) {
            LOGE("NLP engine error info: {} {}", errorCode, result.error.getMessage());
            ret.errorCode = errorCode;
            ret.errorMessage = result.error.getMessage();
            return ret;
        }
    }

    // 处理前后 ```json``` 及 JSON 尾随逗号
    if (intentResult_.length() >= 7 && intentResult_.substr(0, 7) == "```json") {
        intentResult_ = intentResult_.substr(7);
    }
    if (intentResult_.length() >= 3 && intentResult_.substr(intentResult_.size() - 3, intentResult_.size()) == "```") {
        intentResult_ = intentResult_.substr(0, intentResult_.size() - 3);
    }
    intentResult_ = util::removeJsonTrailingCommas(intentResult_);

    ret.successRes = true;
    ret.responseContent = intentResult_;
    ret.modelName = intentModelName_;
    return ret;
}

std::string ModelInference::intentModelName() {
    waitForIntentEngineInited();
    if (intentEngine_ == nullptr) {
        return "";
    }
    return intentEngine_->currentModelName();
}

std::string ModelInference::intentModelDisplayName() {
    waitForIntentEngineInited();
    if (intentEngine_ == nullptr) {
        return "";
    }
    return ModelConfigManager::getInstance().getDisplayName(intentEngine_->currentModelName());
}

std::string ModelInference::chatModelName() {
    if (chatEngine_ == nullptr) {
        return "";
    }
    return chatEngine_->currentModelName();
}

void ModelInference::initChatEngine(const std::shared_ptr<assistant::TextGenerationEngine>& nlpEngine,
                                    EngineSelector& engineSelector, bool isDefaultMode) {
    LOGI("Initializing chat engine: start.");
    std::lock_guard<std::mutex> lock(chatEngineInitMutex_);

    // 检查是否已经在初始化中，如果是则等待完成
    if (chatEngineInitFuture_.valid()) {
        LOGW("Chat engine is already initializing, waiting for completion...");
        waitForChatEngineInitInternal();
    }

    // 检查是否已经初始化完成且引擎相同
    if (isChatEngineInited_ && chatEngine_ == nlpEngine) {
        LOGI("Chat engine is already initialized with the same engine.");
        return;
    }

    chatEngine_ = nlpEngine;
    engineSelector_ = &engineSelector;  // TODO 暂时放这里赋值，后续需要优化
    isChatEngineInited_ = false;
    isDefaultMode_ = isDefaultMode;

    if (nlpEngine == nullptr) {
        LOGE("Chat engine is nullptr, cannot initialize.");
        return;
    }

    chatEngineInitFuture_ = std::async(std::launch::async, [this, nlpEngine]() {
        auto result = nlpEngine->init();
        if (result.success) {
            isChatEngineInited_ = true;
            LOGI("Chat engine init success.");
        } else {
            LOGI("Chat engine init failed, error info: {} {}.", result.error.getCode(), result.error.getMessage());
        }
    });
    LOGI("Initializing chat engine: stop.");
}

void ModelInference::initIntentEngine(const std::shared_ptr<assistant::TextGenerationEngine>& nlpEngine) {
    LOGI("Initializing intent engine: start.");
    std::lock_guard<std::mutex> lock(intentEngineInitMutex_);

    // 检查是否已经在初始化中
    if (intentEngineInitFuture_.valid()) {
        LOGW("Intent engine is already initializing, waiting for completion...");
        waitForIntentEngineInitInternal();
    }

    // 检查是否已经初始化完成且引擎相同
    if (intentEngine_ == nlpEngine) {
        LOGI("Intent engine is already initialized with the same engine.");
        return;
    }

    intentEngine_ = nlpEngine;
    if (intentEngine_ == nullptr) {
        // 意图引擎初始化时出错，不报错处理
        return;
    }

    intentEngineInitFuture_ = std::async(std::launch::async, [this, nlpEngine]() {
        auto result = nlpEngine->init();
        if (result.success) {
            intentModelCurrentErrorCode_ = static_cast<int>(assistant::ErrorCode::Ok);
            LOGI("Intent engine init success.");
        } else {
            intentModelCurrentErrorCode_ =
                static_cast<int>(assistant::Error::getErrorCodeFromNlpEngine(result.error.getCode()));
            LOGI("Intent engine init failed, error info: {} {}.", result.error.getCode(), result.error.getMessage());
        }
    });
    LOGI("Initializing intent engine: stop.");
}

DeployType ModelInference::getDeployTypeByEngine(const assistant::TextGenerationEngine* engine) {
    if (engine == nullptr) {
        return DeployType::Unknown;
    }

    if (engine->isCustomModelEngine()) {
        return DeployType::Custom;
    }
    if (engine->isCloud()) {
        return DeployType::PublicCloud;
    }
    return DeployType::Local;
}

bool ModelInference::switchToNextPriorityModel(
    const std::string& modelInput, DeployType currentType,
    const std::function<void(const ai_engine::gen_ai::nlp::TextGenerationResult& result)>& callback) {
    if (engineSelector_ == nullptr) {
        return false;
    }

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

    std::unique_ptr engine = engineSelector_->selectNextPriorityGenAiTextGenerationEngine(currentType, predicate);

    if (engine == nullptr) {
        return false;
    }
    fallbackChatEngine_ = std::move(engine);
    fallbackChatEngine_->setTextGenerationResultCallback(callback);
    auto result = fallbackChatEngine_->chat(modelInput);
    if (result.success) {
        fallbackChatEngine_ = nullptr;
        return true;
    }

    LOGW("Switched model chat failed, trying next priority again. Error: {} {}", result.error.getCode(),
         result.error.getMessage());

    DeployType nextType = getDeployTypeByEngine(fallbackChatEngine_.get());

    std::unique_ptr secondEngine = engineSelector_->selectNextPriorityGenAiTextGenerationEngine(nextType, predicate);
    if (secondEngine == nullptr) {
        LOGE("Failed to find engine for next priority type: {}", static_cast<int>(nextType));
        return false;
    }

    fallbackChatEngine_ = std::move(secondEngine);
    fallbackChatEngine_->setTextGenerationResultCallback(callback);
    auto secondResult = fallbackChatEngine_->chat(modelInput);
    fallbackChatEngine_ = nullptr;
    return secondResult.success;
}

void ModelInference::waitForChatEngineInited() {
    std::lock_guard<std::mutex> lock(chatEngineInitMutex_);
    if (chatEngineInitFuture_.valid()) {
        if (chatEngineInitFuture_.wait_for(std::chrono::seconds(25)) == std::future_status::ready) {
            try {
                chatEngineInitFuture_.get();
            } catch (const std::exception& e) {
                LOGE("Exception during chat engine initialization: {}", e.what());
            }
        } else {
            LOGE("Chat engine init timeout.");
        }
    }
}

void ModelInference::waitForIntentEngineInited() {
    std::lock_guard<std::mutex> lock(intentEngineInitMutex_);
    if (intentEngineInitFuture_.valid()) {
        if (intentEngineInitFuture_.wait_for(std::chrono::seconds(25)) == std::future_status::ready) {
            try {
                intentEngineInitFuture_.get();
            } catch (const std::exception& e) {
                LOGE("Exception during intent engine initialization: {}", e.what());
            }
        } else {
            LOGE("Intent engine init timeout.");
        }
    }
}

void ModelInference::waitForChatEngineInitInternal() {
    if (chatEngineInitFuture_.valid()) {
        if (chatEngineInitFuture_.wait_for(std::chrono::seconds(25)) == std::future_status::ready) {
            try {
                chatEngineInitFuture_.get();
            } catch (const std::exception& e) {
                LOGE("Exception during previous chat engine initialization: {}", e.what());
            }
        } else {
            LOGE("Previous chat engine init timeout, proceeding with new initialization.");
        }
    }
}

void ModelInference::waitForIntentEngineInitInternal() {
    if (intentEngineInitFuture_.valid()) {
        if (intentEngineInitFuture_.wait_for(std::chrono::seconds(25)) == std::future_status::ready) {
            try {
                intentEngineInitFuture_.get();
            } catch (const std::exception& e) {
                LOGE("Exception during previous intent engine initialization: {}", e.what());
            }
        } else {
            LOGE("Previous intent engine init timeout, proceeding with new initialization.");
        }
    }
}
