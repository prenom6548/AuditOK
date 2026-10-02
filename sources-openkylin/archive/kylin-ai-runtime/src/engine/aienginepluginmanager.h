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

#ifndef AIENGINEPLUGINMANAGER_H
#define AIENGINEPLUGINMANAGER_H

#include <kylin-ai/aiengine.h>

#include <memory>
#include <set>

#include "utils/library.h"
namespace ai_engine {

class AiEnginePluginManager {
public:
    static AiEnginePluginManager &getInstance();

    ~AiEnginePluginManager();

    AiEnginePluginManager(const AiEnginePluginManager &) = delete;

    AiEnginePluginManager &operator=(const AiEnginePluginManager &) = delete;

    // core_ai speech(asr,tts)
    std::unique_ptr<core_ai::speech::AbstractSpeechEngine> createSpeechEngine(const std::string &engineName);

    // core_ai image-process(binarization,image-matting)
    std::unique_ptr<core_ai::vision::AbstractImageProcessEngine> createImageProcessEngine(
        const std::string &engineName = "ondevice");

    // core_ai text-recognition(ocr)
    std::unique_ptr<core_ai::vision::AbstractTextRecognitionEngine> createTextRecognitionEngine();

    // core_ai embedding
    std::unique_ptr<core_ai::embedding::AbstractEmbeddingEngine> createEmbeddingEngine(const std::string &engineName);

    // gen_ai text-generation(nlp)
    std::unique_ptr<gen_ai::nlp::AbstractTextGenerationEngine> createTextGenerationEngine(
        const std::string &engineName);

    // gen_ai image-generation
    std::unique_ptr<gen_ai::vision::AbstractImageGenerationEngine> createImageGenerationEngine(
        const std::string &engineName);

    std::set<std::string> allEngineName() { return allEngineName_; };

private:
    AiEnginePluginManager();

    void loadAllEngines();

    void unloadAllEngines();

    template <typename EngineFactoryType>
    EngineFactoryType *aiEngineFactory(Library &library, const char *engineFactoryInstance);

    template <typename EngineFactoryType>
    void loadEngines(std::unordered_map<std::string, EngineFactoryType *> &engineFactoryMap,
                     const std::string &pluginPath, const char *engineFactoryInstance);

private:
    std::unordered_map<std::string, AbstractAiEnginePluginFactory *> aiEngineFactories_;

    std::vector<Library> engineLibraries_;
    std::set<std::string> allEngineName_;
};

}  // namespace ai_engine

#endif  // AIENGINEPLUGINMANAGER_H
