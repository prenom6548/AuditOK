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

#include "assistantservice.h"

#include <gio/gio.h>
#include <jsoncpp/json/json.h>
#include <kylin-ai/ai-engine/imagegenerationengine.h>
#include <kylin-ai/ai-engine/textgenerationengine.h>
#include <sodium.h>

#include <cstdio>
#include <memory>
#include <utility>

#include "assistantserviceglue.h"
#include "config/prompttemplate.h"
#include "utils/logger.h"
#include "utils/platform.h"
#include "utils/util.h"

namespace {

constexpr const char *objectPath = "/com/kylin/AiRuntime/Assistant";
constexpr const char *serviceName = "AssistantService";

const std::set<int> aiassistantPromptIdSet = {
    int(config::PromptTemplate::PromptId::TextExpansion),                 // 1
    int(config::PromptTemplate::PromptId::TextPolishing),                 // 2
    int(config::PromptTemplate::PromptId::TranslateChineseToEnglish),     // 3
    int(config::PromptTemplate::PromptId::FileQuestionAnswering),         // 4
    int(config::PromptTemplate::PromptId::TodoListGeneration),            // 5
    int(config::PromptTemplate::PromptId::WeeklyReportGeneration),        // 6
    int(config::PromptTemplate::PromptId::MeetingInformationExtraction),  // 7
    int(config::PromptTemplate::PromptId::ErrorCorrection),               // 8
    int(config::PromptTemplate::PromptId::TextSummary),                   // 9
};

}  // namespace

bool AssistantService::onHandleInit(AiRuntimeAssistant *object, GDBusMethodInvocation *invocation, gpointer userData) {
    auto *assistant = static_cast<AssistantService *>(userData);
    if (assistant == nullptr) {
        LOGE("Assistant::onHandleInit: userData is null.");
        return true;
    }
    std::shared_ptr sharedAssistantService = assistant->shared_from_this();

    LOGI("Assistant::onHandleInit...");

    int sessionId = sharedAssistantService->generateRandomSessionId();
    sharedAssistantService->dialogueManager_->addSession(sessionId);

    sharedAssistantService->initNlpEngines(sessionId);

    LOGI("Assistant::onHandleInit: set NLP engine prompt ID.");
    using PromptId = config::PromptTemplate::PromptId;
    sharedAssistantService->setNlpEnginePromptId(sessionId, int(PromptId::Default));

    LOGI("Assistant::onHandleInit: init vision engine.");
    assistant::Error visionError = sharedAssistantService->initVisionEngine(sessionId);
    if (visionError) {
        LOGE("Init vision engine error, error info: {} {}.", int(visionError.code), visionError.message);
    }

    visionError = sharedAssistantService->initImageProcessingEngine(sessionId, "");
    if (visionError) {
        LOGE("Init image processing engine error, error info: {} {}.", int(visionError.code), visionError.message);
    }

    LOGI("Assistant::onHandleInit: init intention recognition.");
    sharedAssistantService->dialogueManager_->initIntentionRecognition(sessionId);

    ai_runtime_assistant_complete_init(object, invocation, sessionId, 0, "");
    LOGI("Assistant::onHandleInit: sessionId={}", sessionId);
    return true;
}

bool AssistantService::onHandleInitWithChatHistory(AiRuntimeAssistant *object, GDBusMethodInvocation *invocation,
                                                   const gchar *messages, gint promptId, gpointer userData) {
    auto *assistant = static_cast<AssistantService *>(userData);
    if (!assistant) {
        LOGE("Assistant::onHandleInit: userData is null.");
        return true;
    }
    std::shared_ptr sharedAssistantService = assistant->shared_from_this();

    int sessionId = sharedAssistantService->generateRandomSessionId();
    sharedAssistantService->dialogueManager_->addSession(sessionId);
    sharedAssistantService->dialogueManager_->setChatContextMessages(sessionId, promptId, messages);

    auto nlpError = sharedAssistantService->initNlpEngines(sessionId);
    if (nlpError) {
        ai_runtime_assistant_complete_init_with_chat_history(object, invocation, sessionId, int(nlpError.code),
                                                             nlpError.message.c_str());
        return true;
    }

    if (promptId == -1) {
        promptId = int(config::PromptTemplate::PromptId::Default);
    }

    sharedAssistantService->setNlpEnginePromptId(sessionId, promptId);

    assistant::Error visionError = sharedAssistantService->initVisionEngine(sessionId);
    if (visionError) {
        LOGE("Init vision engine error, error info {} {}.", int(visionError.code), visionError.message);
    }

    visionError = sharedAssistantService->initImageProcessingEngine(sessionId, "");
    if (visionError) {
        LOGE("Init image processing engine error, error info {} {}.", int(visionError.code), visionError.message);
    }

    sharedAssistantService->dialogueManager_->initIntentionRecognition(sessionId);

    ai_runtime_assistant_complete_init_with_chat_history(object, invocation, sessionId, 0, "");
    LOGI("Assistant::onHandleInitWithChatHistory: sessionId={}", sessionId);
    LOGI("Assistant::onHandleInitWithChatHistory: message={}", messages);
    return true;
}

bool AssistantService::onHandleSupportedFeatures(AiRuntimeAssistant *delegate, GDBusMethodInvocation *invocation,
                                                 gint sessionId, gpointer userData) {
    auto *assistant = static_cast<AssistantService *>(userData);
    if (assistant == nullptr) {
        LOGE("Assistant::onHandleSupportedFeatures: userData is null.");
        return true;
    }
    std::shared_ptr sharedAssistantService = assistant->shared_from_this();
#ifdef SUPPORT_VECTOR_DB
    bool isRagSupported = RagServerProxy::instance().isValid();
#else
    bool isRagSupported = false;
#endif
    bool isImageProcessingSupported = sharedAssistantService->engineSelector_.isLocalCoreAiImageProcessEngineExist();
    int features =
        TextChat | TextToImage | (isImageProcessingSupported ? ImageProcessing : 0) | (isRagSupported ? DocumentQA : 0);
    LOGI("Supported features: {:b}.", features);
    ai_runtime_assistant_complete_supported_features(delegate, invocation, features);
    return true;
}

void AssistantService::tryToChangeNlpModelWithDefaultMode(int sessionId) {
    if (defaultModeNlpModelChangedMap_.find(sessionId) == defaultModeNlpModelChangedMap_.end()) {
        return;
    }
    LOGI("The base nlp model need changed flag: {}.", defaultModeNlpModelChangedMap_[sessionId].load());
    if (!defaultModeNlpModelChangedMap_[sessionId]) {
        return;
    }
    std::shared_ptr baseNlpEngine = initBaseNlpEngine(sessionId);
    if (baseNlpEngine == nullptr) {
        LOGI("Try to change base nlp model, the nlp engine is nullptr.");
    }
    // 即使是空的也设置，后续有保护
    dialogueManager_->setBaseNlpEngine(sessionId, baseNlpEngine, engineSelector_, true);
    dialogueManager_->setIntentNlpEngine(sessionId, baseNlpEngine);
    defaultModeNlpModelChangedMap_[sessionId] = false;
}

void AssistantService::tryToChangeNlpModelWithUserMode(int sessionId, bool isDefaultMode) {
    if (userModeNlpModelChangedMap_.find(sessionId) == userModeNlpModelChangedMap_.end()) {
        return;
    }
    if (!userModeNlpModelChangedMap_[sessionId]) {
        return;
    }
    std::shared_ptr baseNlpEngine = initBaseNlpEngine(sessionId);
    if (baseNlpEngine == nullptr) {
        LOGI("Try to change base nlp model, the nlp engine is nullptr.");
    }
    // 即使是空的也设置，后续有保护
    dialogueManager_->setBaseNlpEngine(sessionId, baseNlpEngine, engineSelector_, isDefaultMode);
    dialogueManager_->setIntentNlpEngine(sessionId, baseNlpEngine);
    userModeNlpModelChangedMap_[sessionId] = false;
}

void AssistantService::tryToChangeVisionModel(int sessionId) {
    if (visionModelChangedMap_.find(sessionId) == visionModelChangedMap_.end()) {
        return;
    }
    LOGI("The text-to-image vision model need changed flag: {}.", visionModelChangedMap_[sessionId].load());
    if (!visionModelChangedMap_[sessionId]) {
        return;
    }
    std::unique_ptr visionEngine = engineSelector_.selectGenAiImageGenerationEngine();
    if (visionEngine == nullptr) {
        LOGI("Try to change vision model, the vision engine is nullptr.");
    }
    // 即使是空的也设置，后续有保护
    dialogueManager_->setVisionEngine(sessionId, std::move(visionEngine));
    visionModelChangedMap_[sessionId] = false;
}

bool AssistantService::onHandleChat(AiRuntimeAssistant *delegate, GDBusMethodInvocation *invocation, gchar *message,
                                    gint sessionId, gpointer userData) {
    auto *assistant = static_cast<AssistantService *>(userData);
    if (!assistant) {
        LOGE("Assistant::onHandleInit: userData is null.");
        return true;
    }
    ModelConfigManager::getInstance().updateCloudNlpCurrentModelName();
    std::shared_ptr sharedAssistantService = assistant->shared_from_this();
    if (sharedAssistantService->isDefaultMode(sessionId)) {
        sharedAssistantService->tryToChangeNlpModelWithDefaultMode(sessionId);
    } else {
        sharedAssistantService->tryToChangeNlpModelWithUserMode(sessionId, false);
    }
    sharedAssistantService->tryToChangeVisionModel(sessionId);

    LOGI("Assistant::onHandleChat: sessionId={}", sessionId);
    ai_runtime_assistant_complete_chat(delegate, invocation);
    sharedAssistantService->doHandleChat(sessionId, message);

    return true;
}

bool AssistantService::onHandleStopChat(AiRuntimeAssistant *delegate, GDBusMethodInvocation *invocation, gint sessionId,
                                        gpointer userData) {
    auto *assistant = static_cast<AssistantService *>(userData);
    if (!assistant) {
        LOGE("Assistant::onHandleStopChat: userData is null.");
        return true;
    }
    std::shared_ptr sharedAssistantService = assistant->shared_from_this();

    sharedAssistantService->stopNlpEngine(sessionId);
    sharedAssistantService->stopVisionEngine(sessionId);

    return true;
}

bool AssistantService::onHandleSetPromptId(AiRuntimeAssistant *delegate, GDBusMethodInvocation *invocation,
                                           gint sessionId, gint promptId, gpointer userData) {
    auto *assistant = static_cast<AssistantService *>(userData);
    if (!assistant) {
        LOGE("Assistant::onHandleSetActorId: userData is null.");
        return true;
    }
    std::shared_ptr sharedAssistantService = assistant->shared_from_this();
    sharedAssistantService->setNlpEnginePromptId(sessionId, promptId);
    return true;
}

bool AssistantService::onHandleClearContext(AiRuntimeAssistant *delegate, GDBusMethodInvocation *invocation,
                                            gint sessionId, gpointer userData) {
    auto *assistant = static_cast<AssistantService *>(userData);
    if (!assistant) {
        LOGE("Assistant::onHandleClearContext: userData is null.");
        return true;
    }
    std::shared_ptr sharedAssistantService = assistant->shared_from_this();
    using PromptId = config::PromptTemplate::PromptId;
    sharedAssistantService->setNlpEnginePromptId(sessionId, int(PromptId::Default));
    sharedAssistantService->clearContext(sessionId);
    return true;
}

bool AssistantService::onHandlePrompts(AiRuntimeAssistant *delegate, GDBusMethodInvocation *invocation, gint sessionId,
                                       gpointer userData) {
    auto *assistant = static_cast<AssistantService *>(userData);
    if (!assistant) {
        LOGE("Assistant::onHandlePrompts: userData is null.");
        ai_runtime_assistant_complete_prompts(delegate, invocation, "");
        return true;
    }
    std::shared_ptr sharedAssistantService = assistant->shared_from_this();
    ai_runtime_assistant_complete_prompts(delegate, invocation,
                                          sharedAssistantService->getAllPrompts(sessionId).c_str());
    return true;
}

bool AssistantService::onHandleCreatePrompt(AiRuntimeAssistant *delegate, GDBusMethodInvocation *invocation,
                                            gchar *promptName, gchar *promptContent, gpointer userData) {
    auto *assistant = static_cast<AssistantService *>(userData);
    if (!assistant) {
        LOGE("Assistant::onHandleCreatePrompt: userData is null.");
        ai_runtime_assistant_complete_create_prompt(delegate, invocation, -1);
        return true;
    }
    std::shared_ptr sharedAssistantService = assistant->shared_from_this();
    int promptId{-1};
    if (sharedAssistantService->promptManager_ != nullptr) {
        promptId = sharedAssistantService->promptManager_->createPrompt(
            sharedAssistantService->aiEnginePluginManager_.allEngineName(), promptName, promptContent);
    }
    ai_runtime_assistant_complete_create_prompt(delegate, invocation, promptId);
    return true;
}

bool AssistantService::onHandleDeletePrompt(AiRuntimeAssistant *delegate, GDBusMethodInvocation *invocation,
                                            gint promptId, gpointer userData) {
    auto *assistant = static_cast<AssistantService *>(userData);
    if (!assistant) {
        LOGE("Assistant::onHandleDeletePrompt: userData is null.");
        ai_runtime_assistant_complete_delete_prompt(delegate, invocation, false);
        return true;
    }
    std::shared_ptr sharedAssistantService = assistant->shared_from_this();
    bool success{false};
    if (sharedAssistantService->promptManager_ != nullptr) {
        success = sharedAssistantService->promptManager_->deletePrompt(
            sharedAssistantService->aiEnginePluginManager_.allEngineName(), promptId);
    }
    ai_runtime_assistant_complete_delete_prompt(delegate, invocation, success);
    return true;
}

bool AssistantService::onHandleUpdatePrompt(AiRuntimeAssistant *delegate, GDBusMethodInvocation *invocation,
                                            gint promptId, gchar *promptName, gchar *promptContent, gpointer userData) {
    auto *assistant = static_cast<AssistantService *>(userData);
    if (!assistant) {
        LOGE("Assistant::onHandleUpdatePrompt: userData is null.");
        ai_runtime_assistant_complete_update_prompt(delegate, invocation, false);
        return true;
    }
    std::shared_ptr sharedAssistantService = assistant->shared_from_this();
    bool success{false};
    if (sharedAssistantService->promptManager_ != nullptr) {
        success = sharedAssistantService->promptManager_->updatePrompt(
            sharedAssistantService->aiEnginePluginManager_.allEngineName(), promptId, promptName, promptContent);
    }
    ai_runtime_assistant_complete_update_prompt(delegate, invocation, success);
    return true;
}

bool AssistantService::onHandleGetConfiguredNlpModelList(AiRuntimeAssistant *delegate,
                                                         GDBusMethodInvocation *invocation, gint sessionId,
                                                         gpointer userData) {
    auto *assistant = static_cast<AssistantService *>(userData);
    if (!assistant) {
        LOGE("Assistant::onHandleGetConfiguredModelList: userData is null.");
        ai_runtime_assistant_complete_get_configured_nlp_model_list(delegate, invocation, "");
        return true;
    }
    std::shared_ptr sharedAssistantService = assistant->shared_from_this();
    std::string modelName = sharedAssistantService->currentModelName(sessionId);
    Json::Value value;
    value["currentModel"] = modelName;
    value["version"] = "1.3.0";  // 和子系统版本保持一致，每次协议更新后需要更新版本号
    std::string list = ModelConfigManager::getInstance().getConfiguredNlpModelList(false);
    value["modelInfos"] = util::stringToJson(list);
    std::string jsonString = util::jsonToString(value);
    LOGI("Configured NLP model list: {}", jsonString);
    ai_runtime_assistant_complete_get_configured_nlp_model_list(delegate, invocation, jsonString.c_str());
    return true;
}

bool AssistantService::onHandleSetCurrentNlpModel(AiRuntimeAssistant *delegate, GDBusMethodInvocation *invocation,
                                                  gint sessionId, gchar *modelName, gpointer userData) {
    auto *assistant = static_cast<AssistantService *>(userData);
    if (!assistant) {
        LOGE("Assistant::onHandleSetCurrentNlpModel: userData is null.");
        ai_runtime_assistant_complete_set_current_nlp_model(delegate, invocation);
        return true;
    }
    std::shared_ptr sharedAssistantService = assistant->shared_from_this();
    sharedAssistantService->currentModels_[sessionId] = modelName;
    if (std::string{modelName}.empty()) {
        // 设置为空时为智能模式，需要将智能模式下的modelChanged置位，下次对话时，触发模型切换逻辑
        sharedAssistantService->defaultModeNlpModelChangedMap_[sessionId] = true;
    } else {
        sharedAssistantService->userModeNlpModelChangedMap_[sessionId] = true;
    }
    LOGI("Set current NLP model: {}", modelName);
    // TODO 开始：临时处理提示词更新问题
    std::unique_ptr<AssistantService::INlpEngine> nlpEngine{nullptr};
    if (sharedAssistantService->isDefaultMode(sessionId)) {
        nlpEngine = sharedAssistantService->engineSelector_.selectGenAiTextGenerationEngine();
    } else {
        nlpEngine = sharedAssistantService->engineSelector_.selectGenAiTextGenerationEngine(
            sharedAssistantService->currentModels_[sessionId]);
    }
    if (nlpEngine != nullptr) {
        LOGD("Select NLP engine: {}", nlpEngine->currentModel());
        const std::string currentModel = nlpEngine->engineName() == "custom" ? "custom" : nlpEngine->currentModel();
        sharedAssistantService->promptManager_ = std::make_unique<PromptManager>(nlpEngine->engineName(), currentModel);
    }
    // TODO 结束：临时处理提示词更新问题
    ai_runtime_assistant_complete_set_current_nlp_model(delegate, invocation);
    return true;
}

assistant::ErrorCode AssistantService::doHandleChat(int sessionId, const std::string &message) {
    return dialogueManager_->handleRequest(sessionId, message);
}

void AssistantService::clearContext(int sessionId) { dialogueManager_->clearContext(sessionId); }

std::string AssistantService::getAllPrompts(int sessionId) const {
    if (promptManager_ == nullptr) {
        return "";
    }
    auto prompts = promptManager_->prompts();
#ifdef SUPPORT_VECTOR_DB
    bool ragIsValid = RagServerProxy::instance().isValid();
#else
    bool ragIsValid = false;
#endif
    if (!ragIsValid) {
        prompts.erase(std::remove_if(prompts.begin(), prompts.end(),
                                     [](const config::PromptTemplate::Prompt &prompt) {
                                         return prompt.id ==
                                                int(config::PromptTemplate::PromptId::FileQuestionAnswering);
                                     }),
                      prompts.end());
    }
    return aiassistantPrompts(prompts, sessionId);
}

AssistantService::AssistantService(GDBusConnection &connection, ai_engine::AiEnginePluginManager &aiEnginePluginManager,
                                   EngineSelector &engineSelector)
    : connection_(connection),
      dialogueManager_(std::make_shared<DialogueManager>(connection)),
      aiEnginePluginManager_(aiEnginePluginManager),
      engineSelector_(engineSelector) {
    exportSkeleton();
    connectHandlers();
    // 需求：默认加载百度提示词
    promptManager_ = std::make_unique<PromptManager>("baidu", "ernie-4.0-8k");
}

AssistantService::~AssistantService() {
    for (const auto &sessionId : sessionIdSet_) {
        std::string uuid = serviceName + std::to_string(sessionId);
        ModelConfigManager::getInstance().updateAllDeployTypeCallback(AiCapability::Nlp, uuid, nullptr);
        ModelConfigManager::getInstance().updateAllDeployTypeCallback(AiCapability::Vision, uuid, nullptr);
    }

    unexportSkeleton();

    LOGD("AssistantService::~AssistantService completed.");
}

void AssistantService::stop() {
    for (const auto &handlerId : handlerIds_) {
        g_signal_handler_disconnect(delegate_, handlerId);
    }
    handlerIds_.clear();
    for (const auto &sessionId : sessionIdSet_) {
        stopNlpEngine(sessionId);
        stopVisionEngine(sessionId);
    }
}

void AssistantService::exportSkeleton() {
    delegate_ = ai_runtime_assistant_skeleton_new();

    GError *error = nullptr;
    isExported_ =
        g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(delegate_), &connection_, objectPath, &error);
    if (!isExported_) {
        g_printerr("Error creating server at address %s: %s\n", objectPath, error->message);
        g_error_free(error);
    }
}

void AssistantService::unexportSkeleton() {
    if (isExported_) {
        g_dbus_interface_skeleton_unexport(G_DBUS_INTERFACE_SKELETON(delegate_));
        isExported_ = false;
    }
}

void AssistantService::connectHandlers() {
    gulong handlerId{};
    handlerId = g_signal_connect(delegate_, "handle-init", G_CALLBACK(onHandleInit), this);
    handlerIds_.push_back(handlerId);
    handlerId =
        g_signal_connect(delegate_, "handle_init_with_chat_history", G_CALLBACK(onHandleInitWithChatHistory), this);
    handlerIds_.push_back(handlerId);
    handlerId = g_signal_connect(delegate_, "handle-supported-features", G_CALLBACK(onHandleSupportedFeatures), this);
    handlerIds_.push_back(handlerId);
    handlerId = g_signal_connect(delegate_, "handle-chat", G_CALLBACK(onHandleChat), this);
    handlerIds_.push_back(handlerId);
    handlerId = g_signal_connect(delegate_, "handle-stop-chat", G_CALLBACK(onHandleStopChat), this);
    handlerIds_.push_back(handlerId);
    handlerId = g_signal_connect(delegate_, "handle-set-prompt-id", G_CALLBACK(onHandleSetPromptId), this);
    handlerIds_.push_back(handlerId);
    handlerId = g_signal_connect(delegate_, "handle-clear-context", G_CALLBACK(onHandleClearContext), this);
    handlerIds_.push_back(handlerId);
    handlerId = g_signal_connect(delegate_, "handle-prompts", G_CALLBACK(onHandlePrompts), this);
    handlerIds_.push_back(handlerId);
    handlerId = g_signal_connect(delegate_, "handle-create-prompt", G_CALLBACK(onHandleCreatePrompt), this);
    handlerIds_.push_back(handlerId);
    handlerId = g_signal_connect(delegate_, "handle-delete-prompt", G_CALLBACK(onHandleDeletePrompt), this);
    handlerIds_.push_back(handlerId);
    handlerId = g_signal_connect(delegate_, "handle-update-prompt", G_CALLBACK(onHandleUpdatePrompt), this);
    handlerIds_.push_back(handlerId);
    handlerId = g_signal_connect(delegate_, "handle-get-configured-nlp-model-list",
                                 G_CALLBACK(onHandleGetConfiguredNlpModelList), this);
    handlerIds_.push_back(handlerId);
    handlerId =
        g_signal_connect(delegate_, "handle-set-current-nlp-model", G_CALLBACK(onHandleSetCurrentNlpModel), this);
    handlerIds_.push_back(handlerId);
}

int AssistantService::generateRandomSessionId() {
    uint32_t sessionId;
    randombytes_buf(&sessionId, sizeof(sessionId));
    sessionId = 1 + (sessionId & INT_MAX);
    while (sessionIdSet_.find(sessionId) != sessionIdSet_.end()) {
        randombytes_buf(&sessionId, sizeof(sessionId));
        sessionId = 1 + (sessionId & INT_MAX);
    }
    sessionIdSet_.insert(sessionId);
    return sessionId;
}

std::shared_ptr<AssistantService::INlpEngine> AssistantService::initBaseNlpEngine(int sessionId) {
    LOGI("Init base nlp engines for sessionId={}, start.", sessionId);
    std::shared_ptr<AssistantService::INlpEngine> nlpEngine{nullptr};
    if (isDefaultMode(sessionId)) {
        nlpEngine = engineSelector_.selectGenAiTextGenerationEngine();
    } else {
        nlpEngine = engineSelector_.selectGenAiTextGenerationEngine(currentModels_[sessionId]);
    }
    if (nlpEngine == nullptr) {
        return nullptr;
    }
    const std::string currentModel = nlpEngine->engineName() == "custom" ? "custom" : nlpEngine->currentModel();
    promptManager_ = std::make_unique<PromptManager>(nlpEngine->engineName(), currentModel);
    LOGI("Init base nlp engines for sessionId={}, end.", sessionId);
    return nlpEngine;
}

assistant::Error AssistantService::initNlpEngines(int sessionId) {
    LOGI("Init nlp engines for sessionId={}, start.", sessionId);
    std::shared_ptr baseNlpEngine = initBaseNlpEngine(sessionId);
    dialogueManager_->setBaseNlpEngine(sessionId, baseNlpEngine, engineSelector_, true);
    dialogueManager_->setIntentNlpEngine(sessionId, baseNlpEngine);

#ifdef SUPPORT_VECTOR_DB
    // TODO 因为向量化引擎和意图NLP引擎绑定，暂时先放这里初始化，后续如有必要，可以独立出来初始化
    // 配置文本向量化表示engine
    auto embeddingEngine = engineSelector_.selectCoreAiEmbeddingEngine("Embedding");
    if (embeddingEngine != nullptr) {
        dialogueManager_->setEmbeddingEngine(sessionId, std::move(embeddingEngine));
    } else {
        LOGE("Init embedding engine failed.");
    }
#endif
    // 初始化模型切换回调函数
    std::string uuid = serviceName + std::to_string(sessionId);
    ModelConfigManager::getInstance().updateCallback(AiCapability::Nlp, DeployType::Custom, uuid,
                                                     [this, sessionId](ModelChangeType) {
                                                         if (isDefaultMode(sessionId)) {
                                                             defaultModeNlpModelChangedMap_[sessionId] = true;
                                                         } else {
                                                             userModeNlpModelChangedMap_[sessionId] = true;
                                                         }
                                                     });
    ModelConfigManager::getInstance().updateCallback(AiCapability::Nlp, DeployType::PublicCloud, uuid,
                                                     [this, sessionId](ModelChangeType) {
                                                         if (isDefaultMode(sessionId)) {
                                                             defaultModeNlpModelChangedMap_[sessionId] = true;
                                                         } else {
                                                             userModeNlpModelChangedMap_[sessionId] = true;
                                                         }
                                                     });
    ModelConfigManager::getInstance().updateCallback(AiCapability::Nlp, DeployType::Local, uuid,
                                                     [this, sessionId](ModelChangeType) {
                                                         if (isDefaultMode(sessionId)) {
                                                             defaultModeNlpModelChangedMap_[sessionId] = true;
                                                         } else {
                                                             userModeNlpModelChangedMap_[sessionId] = true;
                                                         }
                                                     });
    LOGI("Init nlp engines for sessionId={}, end.", sessionId);

    return {assistant::ErrorCode::Ok};
}

assistant::Error AssistantService::initVisionEngine(int sessionId) {
    std::unique_ptr visionEngine = engineSelector_.selectGenAiImageGenerationEngine();
    if (visionEngine == nullptr) {
        LOGI("Init vision engine, the vision engine is nullptr.");
    }
    // 即使是空的也设置，后续有保护
    dialogueManager_->setVisionEngine(sessionId, std::move(visionEngine));

    std::string uuid = serviceName + std::to_string(sessionId);
    ModelConfigManager::getInstance().updateAllDeployTypeCallback(
        AiCapability::Vision, uuid, [this, sessionId](ModelChangeType) { visionModelChangedMap_[sessionId] = true; });

    return {assistant::ErrorCode::Ok};
}

assistant::Error AssistantService::initImageProcessingEngine(int sessionId, const std::string &config) {
    std::unique_ptr imageProcessingEngine = engineSelector_.selectCoreAiImageProcessEngine();
    if (imageProcessingEngine == nullptr) {
        LOGE("Create ImageProcessing engine failed.");
        return {assistant::ErrorCode::VisionModelNotFound, "Create ImageProcessing engine failed."};
    }
    dialogueManager_->setImageProcessingEngine(sessionId, std::move(imageProcessingEngine));
    return {assistant::ErrorCode::Ok};
}

void AssistantService::stopNlpEngine(int sessionId) {
    LOGI("stop chat");
    dialogueManager_->stopChat(sessionId);
}
void AssistantService::stopVisionEngine(int sessionId) { dialogueManager_->stopGenerateImage(sessionId); }

void AssistantService::setNlpEnginePromptId(int sessionId, int promptId) {
    if (promptManager_ == nullptr) {
        LOGE("Prompt manager is nullptr, set nlp engine prompt id failed.");
        return;
    }
    const config::PromptTemplate::Prompt &prompt = promptManager_->prompt(promptId);
    dialogueManager_->setChatSystemRole(sessionId, promptId, prompt.name, prompt.content);
    LOGD("Set nlp engine prompt id: promptId: {} prompt.name: {} prompt.content: {}.", promptId, prompt.name,
         prompt.content);
}

std::string AssistantService::currentModelName(int sessionId) const {
    std::string modelName;
    if (currentModels_.find(sessionId) != currentModels_.end()) {
        modelName = currentModels_.at(sessionId);
    }
    return modelName;
}

bool AssistantService::isDefaultMode(int sessionId) const { return currentModelName(sessionId).empty(); }

namespace {
Json::Value promptToJson(const config::PromptTemplate::Prompt &prompt) {
    Json::Value json;
    json["id"] = prompt.id;
    json["name"] = prompt.name;

    if (prompt.type == config::PromptTemplate::PromptContentAccessType::NoAccess) {
        json["content"] = "";
    } else {
        json["content"] = prompt.content;
    }
    json["type"] = int(prompt.type);
    json["active"] = true;
    return json;
}
}  // namespace

std::string AssistantService::aiassistantPrompts(const std::vector<config::PromptTemplate::Prompt> &prompts,
                                                 int sessionId) const {
    Json::Value jsonArray(Json::arrayValue);

    // 1、显示【文本扩写】到【中英互译】的提示词
    for (const config::PromptTemplate::Prompt &prompt : prompts) {
        if (prompt.id >= int(config::PromptTemplate::PromptId::TextExpansion) &&
            prompt.id <= int(config::PromptTemplate::PromptId::TranslateChineseToEnglish)) {
            if (aiassistantPromptIdSet.find(prompt.id) != aiassistantPromptIdSet.end()) {
                jsonArray.append(promptToJson(prompt));
            }
        }
    }

#if defined(ARCH_AMD64) || defined(ARCH_ARM64)
    // 2、显示【系统控制】的提示词
    {
        // 是否高亮与本地模型是否安装无关
        // 智能模式、本地模型下高亮显示；云端、自定义模型置灰显示
        bool intentionRecognitionActive = false;
        bool isDefaultMode = AssistantService::isDefaultMode(sessionId);
        bool isLocalModel = ModelConfigManager::getInstance().isLocalModel(currentModelName(sessionId));

        if (isDefaultMode || isLocalModel) {
            intentionRecognitionActive = true;
        }
        // 添加“系统控制”的提示词
        Json::Value json;
        json["id"] = int(config::PromptTemplate::PromptId::IntentionRecognition);
        json["name"] = "系统控制";
        json["content"] = "";
        json["type"] = config::PromptTemplate::PromptContentAccessType::NoAccess;
        json["active"] = intentionRecognitionActive;
        jsonArray.append(json);
    }
#endif

    // 3、显示【系统问答】到【文档标签提取】的提示词
    for (const config::PromptTemplate::Prompt &prompt : prompts) {
        if (prompt.id >= int(config::PromptTemplate::PromptId::FileQuestionAnswering) &&
            prompt.id <= int(config::PromptTemplate::PromptId::DocTagExtraction)) {
            if (aiassistantPromptIdSet.find(prompt.id) != aiassistantPromptIdSet.end()) {
                jsonArray.append(promptToJson(prompt));
            }
        }
    }

    // 4、显示【自定义】的提示词
    for (const config::PromptTemplate::Prompt &prompt : prompts) {
        if (prompt.id > 100) {
            jsonArray.append(promptToJson(prompt));
        }
    }

    std::string ret = util::jsonToString(jsonArray);
    LOGI("Aiassistant prompts json: {}.", ret);
    return ret;
}
