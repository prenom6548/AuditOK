#pragma once


#include <kylin-ai/ai-engine/aienginepluginfactory.h>

class EmbeddingEnginePlugin : public ai_engine::AbstractAiEnginePluginFactory {
public:
    ~EmbeddingEnginePlugin() = default;
    std::string name() const override;

    std::unique_ptr<ai_engine::core_ai::embedding::AbstractEmbeddingEngine>
    createEmbeddingEngine();
};