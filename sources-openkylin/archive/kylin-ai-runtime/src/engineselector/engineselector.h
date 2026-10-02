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

#ifndef ENGINESELECTOR_H
#define ENGINESELECTOR_H

#include <jsoncpp/json/json.h>
#include <kylin-ai/aiengine.h>

#include <memory>

#include "engine/aienginepluginmanager.h"
#ifdef SUPPORT_LIFE_CYCLE
#include "lifecycle/lifecycleawareengine.h"
#endif
#include "services/modelconfig/modelconfigmanager.h"

class EngineSelector {
public:
    EngineSelector(ai_engine::AiEnginePluginManager &aiEnginePluginManager, ModelConfigManager &modelConfigManager);
    EngineSelector(const EngineSelector &other);
    EngineSelector &operator=(const EngineSelector &other) = delete;

    // core_ai speech
    std::unique_ptr<ai_engine::core_ai::speech::AbstractSpeechEngine> selectCoreAiSpeechEngine() const;
    std::unique_ptr<ai_engine::core_ai::speech::AbstractSpeechEngine> selectCoreAiSpeechEngine(
        const std::string &targetName) const;
    std::unique_ptr<ai_engine::core_ai::speech::AbstractSpeechEngine> selectCoreAiSpeechEngine(
        DeployType deployType) const;
    std::unique_ptr<ai_engine::core_ai::speech::AbstractSpeechEngine> selectCoreAiSpeechEngine(
        DeployType deployType, const std::string &targetName) const;

    using CoreAiSpeechEnginePredicate =
        std::function<bool(const std::unique_ptr<ai_engine::core_ai::speech::AbstractSpeechEngine> &engine)>;

    std::unique_ptr<ai_engine::core_ai::speech::AbstractSpeechEngine> selectNextPriorityCoreAiSpeechEngine(
        DeployType currentDeployType, const CoreAiSpeechEnginePredicate &predicate) const;

    // core_ai image-process
    std::unique_ptr<ai_engine::core_ai::vision::AbstractImageProcessEngine> selectCoreAiImageProcessEngine() const;
    bool isLocalCoreAiImageProcessEngineExist() const;

    // core_ai text-recognition
    std::unique_ptr<ai_engine::core_ai::vision::AbstractTextRecognitionEngine> selectTextRecognitionEngine() const;

    // core_ai embedding
    std::unique_ptr<ai_engine::core_ai::embedding::AbstractEmbeddingEngine> selectCoreAiEmbeddingEngine(
        const std::string &targetName) const;

    // gen_ai text-generation
    std::unique_ptr<ai_engine::gen_ai::nlp::AbstractTextGenerationEngine> selectGenAiTextGenerationEngine() const;
    std::unique_ptr<ai_engine::gen_ai::nlp::AbstractTextGenerationEngine> selectGenAiTextGenerationEngine(
        const std::string &targetName) const;
    std::unique_ptr<ai_engine::gen_ai::nlp::AbstractTextGenerationEngine> selectGenAiTextGenerationEngine(
        DeployType deployType) const;
    std::unique_ptr<ai_engine::gen_ai::nlp::AbstractTextGenerationEngine> selectGenAiTextGenerationEngine(
        DeployType deployType, const std::string &targetName) const;
    bool isLocalGenAiTextGenerationEngineExist() const;

    using GenAiTextGenerationEnginePredicate =
        std::function<bool(const std::unique_ptr<ai_engine::gen_ai::nlp::AbstractTextGenerationEngine> &engine)>;

    std::unique_ptr<ai_engine::gen_ai::nlp::AbstractTextGenerationEngine> selectNextPriorityGenAiTextGenerationEngine(
        DeployType currentDeployType, GenAiTextGenerationEnginePredicate predicate) const;

    // gen_ai image-generation
    std::unique_ptr<ai_engine::gen_ai::vision::AbstractImageGenerationEngine> selectGenAiImageGenerationEngine() const;
    std::unique_ptr<ai_engine::gen_ai::vision::AbstractImageGenerationEngine> selectGenAiImageGenerationEngine(
        const std::string &targetName) const;
    std::unique_ptr<ai_engine::gen_ai::vision::AbstractImageGenerationEngine> selectGenAiImageGenerationEngine(
        DeployType deployType) const;

private:
    std::map<DeployType, int> getCurrentModelSchedulingPriority(AiCapability capability) const;

    DeployType selectModelDeployType(AiCapability capability) const;
    DeployType selectNextPriorityDeployType(AiCapability capability, DeployType currentDeployType) const;

    template <typename ProxyType, typename EngineInterface>
    std::unique_ptr<EngineInterface> wrapWithLifecycle(std::unique_ptr<EngineInterface> engine) const;

private:
    ai_engine::AiEnginePluginManager &aiEnginePluginManager_;
    ModelConfigManager &modelConfigManager_;
};

template <typename ProxyType, typename EngineInterface>
std::unique_ptr<EngineInterface> EngineSelector::wrapWithLifecycle(std::unique_ptr<EngineInterface> engine) const {
    if (!engine) {
        return nullptr;
    }
    auto proxy = std::make_unique<ProxyType>();
    if (!proxy->attach(std::move(engine))) {
        return nullptr;
    }
    return proxy;
}

#endif
