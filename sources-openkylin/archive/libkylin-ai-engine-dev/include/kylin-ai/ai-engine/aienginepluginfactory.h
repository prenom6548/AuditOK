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

#ifndef AIENGINEPLUGINFACTORY_H
#define AIENGINEPLUGINFACTORY_H

#include <memory>

#include "embeddingengine.h"
#include "imagegenerationengine.h"
#include "imageprocessengine.h"
#include "speechengine.h"
#include "textgenerationengine.h"
#include "textrecognitionengine.h"

namespace ai_engine {

class AbstractAiEnginePluginFactory {
public:
    virtual ~AbstractAiEnginePluginFactory() {}
    virtual std::string name() const = 0;

    virtual std::unique_ptr<gen_ai::nlp::AbstractTextGenerationEngine>
    createTextGenerationEngine() {
        return nullptr;
    }

    virtual std::unique_ptr<gen_ai::vision::AbstractImageGenerationEngine>
    createImageGenerationEngine() {
        return nullptr;
    }
    virtual std::unique_ptr<core_ai::speech::AbstractSpeechEngine>
    createSpeechEngine() {
        return nullptr;
    }

    virtual std::unique_ptr<core_ai::vision::AbstractTextRecognitionEngine>
    createTextRecognitionEngine() {
        return nullptr;
    }

    virtual std::unique_ptr<core_ai::vision::AbstractImageProcessEngine>
    createImageProcessEngine() {
        return nullptr;
    }
    virtual std::unique_ptr<core_ai::embedding::AbstractEmbeddingEngine>
    createEmbeddingEngine() {
        return nullptr;
    }
};

}  // namespace ai_engine

#define AI_ENGINE_PLUGIN_FACTORY(ClassName)     \
    extern "C" {                                \
    ::ai_engine::AbstractAiEnginePluginFactory* \
    ai_engine_plugin_factory_instance() {       \
        static ClassName factory;               \
        return &factory;                        \
    }                                           \
    }

#endif  // AIENGINEPLUGINFACTORY_H