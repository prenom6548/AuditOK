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

#include "aienginepluginmanager.h"

#include <filesystem>

#include "utils/logger.h"

namespace ai_engine {

const char *aiEnginePluginFactoryInstance = "ai_engine_plugin_factory_instance";
const char *vectorDatabaseEngineFactoryInstance = "ai_vector_database_engine_factory_instance";

AiEnginePluginManager &AiEnginePluginManager::getInstance() {
    static AiEnginePluginManager instance;
    return instance;
}

AiEnginePluginManager::AiEnginePluginManager() { loadAllEngines(); }

AiEnginePluginManager::~AiEnginePluginManager() { unloadAllEngines(); }

std::unique_ptr<core_ai::speech::AbstractSpeechEngine> AiEnginePluginManager::createSpeechEngine(
    const std::string &engineName) {
    auto iter = aiEngineFactories_.find(engineName);
    if (iter != aiEngineFactories_.end()) {
        return iter->second->createSpeechEngine();
    }
    return nullptr;
}

std::unique_ptr<core_ai::vision::AbstractImageProcessEngine> AiEnginePluginManager::createImageProcessEngine(
    const std::string &engineName) {
    auto iter = aiEngineFactories_.find(engineName);
    if (iter != aiEngineFactories_.end()) {
        return iter->second->createImageProcessEngine();
    }
    return nullptr;
}

std::unique_ptr<core_ai::vision::AbstractTextRecognitionEngine> AiEnginePluginManager::createTextRecognitionEngine() {
    auto iter = aiEngineFactories_.find("ondevice");
    if (iter != aiEngineFactories_.end()) {
        return iter->second->createTextRecognitionEngine();
    }
    return nullptr;
}

std::unique_ptr<core_ai::embedding::AbstractEmbeddingEngine> AiEnginePluginManager::createEmbeddingEngine(
    const std::string &engineName) {
    auto iter = aiEngineFactories_.find(engineName);
    if (iter != aiEngineFactories_.end()) {
        return iter->second->createEmbeddingEngine();
    }
    return nullptr;
}

std::unique_ptr<gen_ai::nlp::AbstractTextGenerationEngine> AiEnginePluginManager::createTextGenerationEngine(
    const std::string &engineName) {
    auto iter = aiEngineFactories_.find(engineName);
    if (iter != aiEngineFactories_.end()) {
        return iter->second->createTextGenerationEngine();
    }
    return nullptr;
}

std::unique_ptr<gen_ai::vision::AbstractImageGenerationEngine> AiEnginePluginManager::createImageGenerationEngine(
    const std::string &engineName) {
    auto iter = aiEngineFactories_.find(engineName);
    if (iter != aiEngineFactories_.end()) {
        return iter->second->createImageGenerationEngine();
    }
    return nullptr;
}

void AiEnginePluginManager::loadAllEngines() {
    loadEngines<AbstractAiEnginePluginFactory>(aiEngineFactories_, AI_ENGINE_PLUGIN_PATH,
                                               aiEnginePluginFactoryInstance);
}

void AiEnginePluginManager::unloadAllEngines() {
    for (auto &library : engineLibraries_) {
        library.unload();
    }
}

template <typename EngineFactoryType>
EngineFactoryType *AiEnginePluginManager::aiEngineFactory(Library &library, const char *engineFactoryInstance) {
    auto *funcPtr = library.resolve(engineFactoryInstance);
    if (!funcPtr) {
        LOGE("Resolve factory instance failed: {}", library.error());
    }
    auto func = Library::toFunction<EngineFactoryType *()>(funcPtr);
    return func();
}

template <typename EngineFactoryType>
void AiEnginePluginManager::loadEngines(std::unordered_map<std::string, EngineFactoryType *> &engineFactoryMap,
                                        const std::string &pluginPath, const char *engineFactoryInstance) {
    std::filesystem::path path(pluginPath);
    if (!std::filesystem::exists(path)) {
        LOGE("Plugin path {} not exists.", pluginPath);
        return;
    }
    for (auto &entry : std::filesystem::directory_iterator(path)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        auto &filePath = entry.path();
        if (filePath.extension() != ".so") {
            continue;
        }
        Library library(filePath.string());
        if (!library.load()) {
            LOGE("Load {} failed: {}", filePath.string(), library.error());
            continue;
        }
        EngineFactoryType *factory = aiEngineFactory<EngineFactoryType>(library, engineFactoryInstance);
        LOGI("Engine name: {}", factory->name());

        allEngineName_.insert(factory->name());
        engineFactoryMap[factory->name()] = factory;
        engineLibraries_.emplace_back(std::move(library));
    }
}

}  // namespace ai_engine
