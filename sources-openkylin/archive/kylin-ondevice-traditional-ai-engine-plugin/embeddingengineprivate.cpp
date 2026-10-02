#include "embeddingengineprivate.h"

#include <string>
EmbeddingEngineP::EmbeddingEngineP()
    : p(new ai_engine::core_ai::embedding::OndeviceEmbeddingEngine()) {}

EmbeddingEngineP::~EmbeddingEngineP() { delete p; }

std::string EmbeddingEngineP::engineName() const { return p->engineName(); }
bool EmbeddingEngineP::isCloud() const { return p->isCloud(); };
bool EmbeddingEngineP::isBuiltInEngine() const { return p->isBuiltInEngine(); }
std::string EmbeddingEngineP::modelInfo() const { return p->modelInfo(); }

void EmbeddingEngineP::setEmbeddingResultCallback(
    ai_engine::core_ai::embedding::EmbeddingResultCallback callback) {
    p->setEmbeddingResultCallback(callback);
}

ai_engine::result::AiEngineBaseResult EmbeddingEngineP::init() {
    return p->init();
}

///[0] 文本向量化
std::string EmbeddingEngineP::currentTextEmbeddingModel() {
    return p->currentTextEmbeddingModel();
}
ai_engine::result::AiEngineBaseResult
EmbeddingEngineP::setCurrentTextEmbeddingModel(const std::string& model) {
    return p->setCurrentTextEmbeddingModel(model);
}
ai_engine::result::AiEngineBaseResult EmbeddingEngineP::embeddingText(
    const std::string& text) {
    return p->embeddingText(text);
};
void EmbeddingEngineP::stopTextEmbedding() { p->stopTextEmbedding(); }
///[0]

///[1] 图像向量化

std::string EmbeddingEngineP::currentImageEmbeddingModel() {
    return p->currentImageEmbeddingModel();
}
ai_engine::result::AiEngineBaseResult
EmbeddingEngineP::setCurrentImageEmbeddingModel(const std::string& model) {
    return p->setCurrentImageEmbeddingModel(model);
}
ai_engine::result::AiEngineBaseResult
EmbeddingEngineP::embeddingImageFromImageFile(const std::string& filePath) {
    return p->embeddingImageFromImageFile(filePath);
}
ai_engine::result::AiEngineBaseResult
EmbeddingEngineP::embeddingImageFromBase64ImageData(
    const std::string& fileData) {
    return p->embeddingImageFromBase64ImageData(fileData);
}
ai_engine::result::AiEngineBaseResult
EmbeddingEngineP::embeddingTextWithImageModel(const std::string& text) {
    return p->embeddingTextWithImageModel(text);
}
void EmbeddingEngineP::stopImageEmbedding() { p->stopImageEmbedding(); }
///[1]