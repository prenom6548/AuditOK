#include "embeddingplugin.h"

#include <kylin-ai/ai-engine/embeddingengine.h>

#include "embeddingengineprivate.h"

std::string EmbeddingEnginePlugin::name() const { return "Embedding"; }

std::unique_ptr<ai_engine::core_ai::embedding::AbstractEmbeddingEngine>
EmbeddingEnginePlugin::createEmbeddingEngine() {
    return std::make_unique<EmbeddingEngineP>();
}

AI_ENGINE_PLUGIN_FACTORY(EmbeddingEnginePlugin);