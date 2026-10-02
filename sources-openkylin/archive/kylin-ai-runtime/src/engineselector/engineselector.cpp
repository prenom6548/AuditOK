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

#include "engineselector.h"

#include <jsoncpp/json/json.h>

#include <climits>

#include "utils/logger.h"
#include "utils/util.h"

#ifdef SUPPORT_LIFE_CYCLE
using lifecycle::ModelDescriptor;
#endif

namespace {
Json::Value authensToEngineConfigJson(const std::vector<MultiAuthentication> &multiAuthens) {
    Json::Value root;
    for (const auto &multiAuth : multiAuthens) {
        for (const auto &auth : multiAuth.authentications) {
            if (multiAuth.enabled) {
                root[auth.key] = auth.value;
            } else {
                root[auth.key] = std::string{};
            }
        }
    }
    return root;
}

}  // namespace

EngineSelector::EngineSelector(ai_engine::AiEnginePluginManager &aiEnginePluginManager,
                               ModelConfigManager &modelConfigManager)
    : aiEnginePluginManager_(aiEnginePluginManager), modelConfigManager_(modelConfigManager) {}

EngineSelector::EngineSelector(const EngineSelector &other)
    : aiEnginePluginManager_(other.aiEnginePluginManager_), modelConfigManager_(other.modelConfigManager_) {}

std::unique_ptr<ai_engine::core_ai::speech::AbstractSpeechEngine> EngineSelector::selectCoreAiSpeechEngine() const {
    DeployType deployType = selectModelDeployType(AiCapability::Speech);
    LOGI("Get current deploy type: {}.", int(deployType));
    if (deployType == DeployType::Unknown) {
        LOGE("Select speech engine failed, deploy is unknown.");
        return nullptr;
    }
    std::string modelName = modelConfigManager_.getCurrentModelName(AiCapability::Speech, deployType);
    LOGI("Get current model name: {}.", modelName);
    if (modelName.empty()) {
        LOGE("Select speech engine failed，get current model name is empty.");
        return nullptr;
    }
    return selectCoreAiSpeechEngine(modelName);
}

std::unique_ptr<ai_engine::core_ai::speech::AbstractSpeechEngine> EngineSelector::selectCoreAiSpeechEngine(
    const std::string &targetName) const {
    LOGI("Select core AI speech engine: {}", targetName);

    if (targetName.empty()) {
        LOGE("Select speech engine failed, modelName is empty.");
        return nullptr;
    }

    std::string modelName;
    if (modelConfigManager_.isDisplayName(targetName)) {
        modelName = modelConfigManager_.getModelNameByDisplayName(targetName);
    } else {
        modelName = targetName;
    }

    if (!modelConfigManager_.isModelEnabled(modelName)) {
        return nullptr;
    }

    ModelInfo modelInfo = modelConfigManager_.getModelInfo(modelName);
    if (modelInfo.model.modelName.empty()) {
        LOGE("Select speech engine failed, modelInfo.model.modelName is empty.");
        return nullptr;
    }

    std::vector<MultiAuthentication> multiAuthentications = modelConfigManager_.getModelAuthentications(modelName);
    std::unique_ptr engine = aiEnginePluginManager_.createSpeechEngine(modelInfo.model.engineName);
    if (engine == nullptr) {
        return nullptr;
    }

    Json::Value multiAuthenticationsJson = authensToEngineConfigJson(multiAuthentications);

    if (engine->isCloud()) {
        engine->setConfig(multiAuthenticationsJson.toStyledString());
        return engine;
    }
#ifdef SUPPORT_LIFE_CYCLE
    return wrapWithLifecycle<lifecycle::LifecycleAwareSpeechEngine>(std::move(engine));
#endif
    return engine;
}

std::unique_ptr<ai_engine::core_ai::speech::AbstractSpeechEngine> EngineSelector::selectCoreAiSpeechEngine(
    DeployType deployType) const {
    if (!modelConfigManager_.getModelDeployEnabled(deployType)) {
        return nullptr;
    }
    std::string modelName = modelConfigManager_.getCurrentModelName(AiCapability::Speech, deployType);
    LOGI("Get current model name: {}.", modelName);
    if (modelName.empty()) {
        LOGE("Select speech engine failed，get current model name is empty.");
        return nullptr;
    }
    return selectCoreAiSpeechEngine(modelName);
}

std::unique_ptr<ai_engine::core_ai::speech::AbstractSpeechEngine> EngineSelector::selectCoreAiSpeechEngine(
    DeployType deployType, const std::string &targetName) const {
    if (targetName.empty()) {
        if (deployType == DeployType::Unknown) {
            // 未设置模型名称，未设置部署类型
            return selectCoreAiSpeechEngine();
        } else {
            // 未设置模型名称，设置部署类型
            return selectCoreAiSpeechEngine(deployType);
        }
    } else {
        if (deployType == DeployType::Unknown) {
            // 设置模型名称，未设置部署类型
            return selectCoreAiSpeechEngine(targetName);
        } else {
            if (!modelConfigManager_.getModelDeployEnabled(deployType)) {
                return nullptr;
            }
            std::string modelName;
            if (modelConfigManager_.isDisplayName(targetName)) {
                modelName = modelConfigManager_.getModelNameByDisplayName(targetName);
            } else {
                modelName = targetName;
            }
            // 设置模型名称，设置部署类型
            ModelInfo modelInfo = modelConfigManager_.getModelInfo(modelName);
            // 找到的模型部署类型与设置的部署类型不一致，返回nullptr
            if (modelInfo.model.deployType != deployType) {
                return nullptr;
            }
            std::vector<MultiAuthentication> multiAuthentications =
                modelConfigManager_.getModelAuthentications(modelName);
            std::unique_ptr engine = aiEnginePluginManager_.createSpeechEngine(modelInfo.model.engineName);
            if (engine == nullptr) {
                LOGE("Select speech engine failed.");
                return nullptr;
            }
            const Json::Value jsonMultiAuthentications = authensToEngineConfigJson(multiAuthentications);
            if (engine->isCloud()) {
                engine->setConfig(util::jsonToString(jsonMultiAuthentications));
                return engine;
            }
#ifdef SUPPORT_LIFE_CYCLE
            return wrapWithLifecycle<lifecycle::LifecycleAwareSpeechEngine>(std::move(engine));
#endif
            return engine;
        }
    }
}

std::unique_ptr<ai_engine::core_ai::speech::AbstractSpeechEngine> EngineSelector::selectNextPriorityCoreAiSpeechEngine(
    DeployType currentDeployType, const CoreAiSpeechEnginePredicate &predicate) const {
    DeployType nextDeployType = selectNextPriorityDeployType(AiCapability::Speech, currentDeployType);
    if (nextDeployType == DeployType::Unknown) {
        LOGE("Select next priority speech engine failed, no suitable next deploy type found.");
        return nullptr;
    }
    LOGI("Selecting next priority speech engine with deploy type: {}", int(nextDeployType));
    std::unique_ptr engine = selectCoreAiSpeechEngine(nextDeployType);
    if (engine != nullptr && predicate(engine)) {
        return engine;
    }
    return selectNextPriorityCoreAiSpeechEngine(nextDeployType, predicate);
}

std::unique_ptr<ai_engine::core_ai::vision::AbstractImageProcessEngine> EngineSelector::selectCoreAiImageProcessEngine()
    const {
    auto engine = aiEnginePluginManager_.createImageProcessEngine();
    if (!engine) {
        return nullptr;
    }
    if (engine->isCloud()) {
        return engine;
    }
    const std::string modelName = "CoreImageProcess";
    engine->setCurrentModel(modelName);
#ifdef SUPPORT_LIFE_CYCLE
    return wrapWithLifecycle<lifecycle::LifecycleAwareImageProcessEngine>(std::move(engine));
#endif
    return engine;
}

bool EngineSelector::isLocalCoreAiImageProcessEngineExist() const {
    return aiEnginePluginManager_.createImageProcessEngine() != nullptr;
}

std::unique_ptr<ai_engine::core_ai::vision::AbstractTextRecognitionEngine> EngineSelector::selectTextRecognitionEngine()
    const {
    auto engine = aiEnginePluginManager_.createTextRecognitionEngine();
    if (!engine) {
        return nullptr;
    }
    if (engine->isCloud()) {
        return engine;
    }
#ifdef SUPPORT_LIFE_CYCLE
    return wrapWithLifecycle<lifecycle::LifecycleAwareTextRecognitionEngine>(std::move(engine));
#endif
    return engine;
}

std::unique_ptr<ai_engine::core_ai::embedding::AbstractEmbeddingEngine> EngineSelector::selectCoreAiEmbeddingEngine(
    const std::string &targetName) const {
    std::string modelName;
    if (modelConfigManager_.isDisplayName(targetName)) {
        modelName = modelConfigManager_.getModelNameByDisplayName(targetName);
    } else {
        modelName = targetName;
    }
    auto engine = aiEnginePluginManager_.createEmbeddingEngine(modelName);
    if (!engine) {
        return nullptr;
    }
    ModelInfo modelInfo = modelConfigManager_.getModelInfo(modelName);
    std::string targetModelName = modelInfo.model.modelName.empty() ? modelName : modelInfo.model.modelName;
    if (engine->isCloud()) {
        return engine;
    }
    engine->setCurrentModel(targetModelName);
#ifdef SUPPORT_LIFE_CYCLE
    return wrapWithLifecycle<lifecycle::LifecycleAwareEmbeddingEngine>(std::move(engine));
#endif
    return engine;
}

std::unique_ptr<ai_engine::gen_ai::nlp::AbstractTextGenerationEngine> EngineSelector::selectGenAiTextGenerationEngine()
    const {
    auto deployType = selectModelDeployType(AiCapability::Nlp);
    LOGI("Get current deploy type: {}.", int(deployType));
    if (deployType == DeployType::Unknown) {
        LOGE("Select nlp engine failed, deploy is unknown.");
        return nullptr;
    }
    auto modelName = modelConfigManager_.getCurrentModelName(AiCapability::Nlp, deployType);

    LOGI("Get current model name: {}.", modelName);

    if (modelName.empty()) {
        LOGE("Select nlp engine failed，get current model name is empty.");
        return nullptr;
    }

    return selectGenAiTextGenerationEngine(modelName);
}

std::unique_ptr<ai_engine::gen_ai::nlp::AbstractTextGenerationEngine> EngineSelector::selectGenAiTextGenerationEngine(
    const std::string &targetName) const {
    LOGD("Target name: {}.", targetName);

    if (targetName.empty()) {
        LOGE("Select nlp engine failed, modelName is empty.");
        return nullptr;
    }

    std::string modelName;
    if (modelConfigManager_.isDisplayName(targetName)) {
        modelName = modelConfigManager_.getModelNameByDisplayName(targetName);
    } else {
        modelName = targetName;
    }

    if (!modelConfigManager_.isModelEnabled(modelName)) {
        return nullptr;
    }

    ModelInfo modelInfo = modelConfigManager_.getModelInfo(modelName);
    if (modelInfo.model.modelName.empty()) {
        LOGE("Select nlp engine failed, modelInfo.model.modelName is empty.");
        return nullptr;
    }

    auto authens = modelConfigManager_.getModelAuthentications(modelName);
    auto engine = aiEnginePluginManager_.createTextGenerationEngine(modelInfo.model.engineName);
    if (engine == nullptr) {
        LOGE("Select nlp engine failed.");
        return nullptr;
    }

    auto authensJson = authensToEngineConfigJson(authens);

    if (engine->isCustomModelEngine()) {
        authensJson["apiUrl"] = modelInfo.model.apiUrl;
        authensJson["modelName"] = modelInfo.model.modelName;
        authensJson["displayName"] = modelInfo.model.displayName;
        authensJson["modelVersion"] = modelInfo.model.modelVersion;
    }
    auto &modelConfig = ModelConfigManager::getInstance();
    if (!engine->isCloud()) {
        if (modelConfig.isEnableRequestComputingPower()) {
            engine->setConfig(R"(
                {
                    "use_local": false
                }
            )");
        } else {
            engine->setConfig(R"(
                {
                    "use_local": true
                }
            )");
        }
    } else {
        engine->setConfig(authensJson.toStyledString());
    }
    engine->setCurrentModel(modelInfo.model.modelVersion);

    if (engine->isCloud()) {
        return engine;
    }
#ifdef SUPPORT_LIFE_CYCLE
    return wrapWithLifecycle<lifecycle::LifecycleAwareTextGenerationEngine>(std::move(engine));
#endif
    return engine;
}

std::unique_ptr<ai_engine::gen_ai::nlp::AbstractTextGenerationEngine> EngineSelector::selectGenAiTextGenerationEngine(
    DeployType deployType) const {
    if (!modelConfigManager_.getModelDeployEnabled(deployType)) {
        return nullptr;
    }
    auto modelName = modelConfigManager_.getCurrentModelName(AiCapability::Nlp, deployType);
    LOGD("Model name: {}.", modelName);

    if (modelName.empty()) {
        LOGE("Select nlp engine failed，get current model name is empty.");
        return nullptr;
    }

    return selectGenAiTextGenerationEngine(modelName);
}

std::unique_ptr<ai_engine::gen_ai::nlp::AbstractTextGenerationEngine> EngineSelector::selectGenAiTextGenerationEngine(
    DeployType deployType, const std::string &targetName) const {
    if (targetName.empty()) {
        if (deployType == DeployType::Unknown) {
            // 未设置模型名称，未设置部署类型
            return selectGenAiTextGenerationEngine();
        } else {
            // 未设置模型名称，设置部署类型
            return selectGenAiTextGenerationEngine(deployType);
        }
    } else {
        if (deployType == DeployType::Unknown) {
            // 设置模型名称，未设置部署类型
            return selectGenAiTextGenerationEngine(targetName);
        } else {
            // 设置模型名称，设置部署类型
            if (!modelConfigManager_.getModelDeployEnabled(deployType)) {
                return nullptr;
            }
            std::string modelName;
            if (modelConfigManager_.isDisplayName(targetName)) {
                modelName = modelConfigManager_.getModelNameByDisplayName(targetName);
            } else {
                modelName = targetName;
            }
            ModelInfo modelInfo = modelConfigManager_.getModelInfo(modelName);
            // 找到的模型部署类型与设置的部署类型不一致，返回nullptr
            if (modelInfo.model.deployType != deployType) {
                return nullptr;
            }
            std::vector<MultiAuthentication> multiAuthentications =
                modelConfigManager_.getModelAuthentications(modelName);
            std::unique_ptr engine = aiEnginePluginManager_.createTextGenerationEngine(modelInfo.model.engineName);
            if (engine == nullptr) {
                LOGE("Select nlp engine failed.");
                return nullptr;
            }
            Json::Value jsonMultiAuthentications = authensToEngineConfigJson(multiAuthentications);
            if (engine->isCustomModelEngine()) {
                jsonMultiAuthentications["apiUrl"] = modelInfo.model.apiUrl;
                jsonMultiAuthentications["modelName"] = modelInfo.model.modelName;
                jsonMultiAuthentications["displayName"] = modelInfo.model.displayName;
                jsonMultiAuthentications["modelVersion"] = modelInfo.model.modelVersion;
            }
            engine->setConfig(util::jsonToString(jsonMultiAuthentications));
            engine->setCurrentModel(modelInfo.model.modelVersion);
            if (engine->isCloud()) {
                return engine;
            }
#ifdef SUPPORT_LIFE_CYCLE
            return wrapWithLifecycle<lifecycle::LifecycleAwareTextGenerationEngine>(std::move(engine));
#endif
            return engine;
        }
    }
}

bool EngineSelector::isLocalGenAiTextGenerationEngineExist() const {
    auto modelName = modelConfigManager_.getCurrentModelName(AiCapability::Nlp, DeployType::Local);
    if (modelName.empty()) {
        return false;
    }
    return selectGenAiTextGenerationEngine(modelName) != nullptr;
}

std::unique_ptr<ai_engine::gen_ai::vision::AbstractImageGenerationEngine>
EngineSelector::selectGenAiImageGenerationEngine() const {
    DeployType deployType = selectModelDeployType(AiCapability::Vision);
    if (deployType == DeployType::Unknown) {
        return nullptr;
    }
    return selectGenAiImageGenerationEngine(deployType);
}

std::unique_ptr<ai_engine::gen_ai::vision::AbstractImageGenerationEngine>
EngineSelector::selectGenAiImageGenerationEngine(const std::string &targetName) const {
    std::string modelName;
    if (modelConfigManager_.isDisplayName(targetName)) {
        modelName = modelConfigManager_.getModelNameByDisplayName(targetName);
    } else {
        modelName = targetName;
    }
    ModelInfo modelInfo = modelConfigManager_.getModelInfo(modelName);
    std::unique_ptr engine = aiEnginePluginManager_.createImageGenerationEngine(modelInfo.model.engineName);
    if (engine == nullptr) {
        return nullptr;
    }
    std::vector<MultiAuthentication> multiAuthentications = modelConfigManager_.getModelAuthentications(modelName);
    Json::Value config = authensToEngineConfigJson(multiAuthentications);
    if (engine->isCloud()) {
        engine->setConfig(util::jsonToString(config));
        return engine;
    }
    engine->setCurrentModel(modelInfo.model.modelName);
    return engine;
}

std::unique_ptr<ai_engine::gen_ai::vision::AbstractImageGenerationEngine>
EngineSelector::selectGenAiImageGenerationEngine(DeployType deployType) const {
    std::string modelName = modelConfigManager_.getCurrentModelName(AiCapability::Vision, deployType);
    return selectGenAiImageGenerationEngine(modelName);
}

std::map<DeployType, int> EngineSelector::getCurrentModelSchedulingPriority(AiCapability capability) const {
    int cloudPriority = modelConfigManager_.getModelDeployEnabled(DeployType::PublicCloud) &&
                                !modelConfigManager_.getCurrentModelName(capability, DeployType::PublicCloud).empty()
                            ? modelConfigManager_.getModelSchedulingPriority(DeployType::PublicCloud)
                            : -1;
    int localPriority = modelConfigManager_.getModelDeployEnabled(DeployType::Local) &&
                                !modelConfigManager_.getCurrentModelName(capability, DeployType::Local).empty()
                            ? modelConfigManager_.getModelSchedulingPriority(DeployType::Local)
                            : -1;
    int privatePriority = modelConfigManager_.getModelDeployEnabled(DeployType::Custom) &&
                                  !modelConfigManager_.getCurrentModelName(capability, DeployType::Custom).empty()
                              ? modelConfigManager_.getModelSchedulingPriority(DeployType::Custom)
                              : -1;
    int sharedPriority = modelConfigManager_.getModelDeployEnabled(DeployType::Shared) &&
                                 !modelConfigManager_.getCurrentModelName(capability, DeployType::Shared).empty()
                             ? modelConfigManager_.getModelSchedulingPriority(DeployType::Shared)
                             : -1;

    std::map<DeployType, int> priorityMap = {
        {DeployType::Local, localPriority},
        {DeployType::PublicCloud, cloudPriority},
        {DeployType::Custom, privatePriority},
        {DeployType::Shared, sharedPriority},
    };
    return priorityMap;
}
DeployType EngineSelector::selectModelDeployType(AiCapability capability) const {
    auto priorityMap = getCurrentModelSchedulingPriority(capability);

    int32_t currentPriority{INT32_MAX};
    DeployType currentDeployType{DeployType::Unknown};
    for (const auto &[deployType, priority] : priorityMap) {
        if (priority == -1) {
            continue;
        }
        if (currentPriority > priority) {
            // 当前优先级大于当前[key, value]的优先级，
            // 则更新当前优先级和当前部署类型
            currentPriority = priority;
            currentDeployType = deployType;
        }
    }
    return currentDeployType;
}

DeployType EngineSelector::selectNextPriorityDeployType(AiCapability capability, DeployType currentDeployType) const {
    auto priorityMap = getCurrentModelSchedulingPriority(capability);

    // 获取当前部署类型的优先级
    int currentPriorityValue = priorityMap[currentDeployType];
    int32_t nextPriority{INT32_MAX};
    DeployType nextDeployType{DeployType::Unknown};

    // 找到比当前优先级高（数值更大）的最近优先级
    for (const auto &[deployType, priority] : priorityMap) {
        if (priority == -1 || deployType == currentDeployType) {
            continue;
        }
        if (priority > currentPriorityValue && priority < nextPriority) {
            nextPriority = priority;
            nextDeployType = deployType;
        }
    }

    LOGI("Found next deploy type: {} with priority: {}", static_cast<int>(nextDeployType), nextPriority);
    return nextDeployType;
}

std::unique_ptr<ai_engine::gen_ai::nlp::AbstractTextGenerationEngine>
EngineSelector::selectNextPriorityGenAiTextGenerationEngine(DeployType currentDeployType,
                                                            GenAiTextGenerationEnginePredicate predicate) const {
    auto nextDeployType = selectNextPriorityDeployType(AiCapability::Nlp, currentDeployType);
    if (nextDeployType == DeployType::Unknown) {
        LOGE("Select next priority nlp engine failed, no suitable next deploy type found.");
        return nullptr;
    }

    LOGI("Selecting next priority NLP engine with deploy type: {}", static_cast<int>(nextDeployType));
    auto engine = selectGenAiTextGenerationEngine(nextDeployType);
    if (predicate(engine)) {
        return engine;
    }

    return selectNextPriorityGenAiTextGenerationEngine(nextDeployType, predicate);
}
