#pragma once

// #include <kylin-ai/ai-engine/embeddingengine.h>
#include <kylin-ai/plugins/ai-engines/ondeviceembeddingengine.h>

class EmbeddingEngineP
    : public ai_engine::core_ai::embedding::AbstractEmbeddingEngine {
public:
    EmbeddingEngineP();
    ~EmbeddingEngineP();

    std::string engineName() const;
    bool isCloud() const;
    bool isBuiltInEngine() const;
    std::string modelInfo() const;

    void setEmbeddingResultCallback(
        ai_engine::core_ai::embedding::EmbeddingResultCallback callback);
    ai_engine::result::AiEngineBaseResult init();

    ///[0] 文本向量化
    std::string currentTextEmbeddingModel();
    ai_engine::result::AiEngineBaseResult setCurrentTextEmbeddingModel(
        const std::string& model);
    ai_engine::result::AiEngineBaseResult embeddingText(
        const std::string& text);
    ///[0]

    ///[1] 图像向量化
    std::string currentImageEmbeddingModel();
    ai_engine::result::AiEngineBaseResult setCurrentImageEmbeddingModel(
        const std::string& model);
    ai_engine::result::AiEngineBaseResult embeddingImageFromImageFile(
        const std::string& filePath);
    ai_engine::result::AiEngineBaseResult embeddingImageFromBase64ImageData(
        const std::string& fileData);
    ai_engine::result::AiEngineBaseResult embeddingTextWithImageModel(
        const std::string& text);
    ///[1]
    void stopImageEmbedding();
    void stopTextEmbedding();

private:
    ai_engine::core_ai::embedding::OndeviceEmbeddingEngine* p;
};
