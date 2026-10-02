/*
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "ondeviceembeddingengine.h"

#include "src/embeddingclient.h"

namespace ai_engine {
namespace core_ai {
namespace embedding {

OndeviceEmbeddingEngine::OndeviceEmbeddingEngine()
    : embeddingClient_(new EmbeddingClient) {}
OndeviceEmbeddingEngine::~OndeviceEmbeddingEngine() {
    if (!embeddingClient_) {
        delete embeddingClient_;
        embeddingClient_ = nullptr;
    }
}
void OndeviceEmbeddingEngine::setEmbeddingResultCallback(
    EmbeddingResultCallback callback) {
    if (!callback) {
        std::cerr << "EmbeddingResultCallback is nullptr" << std::endl;
        return;
    }
    embeddingClient_->setEmbeddingResultCallback(callback);
}

std::string OndeviceEmbeddingEngine::modelInfo() const {
    return embeddingClient_->modelInfo();
}

result::AiEngineBaseResult OndeviceEmbeddingEngine::init() {
    return embeddingClient_->init();
}

// 文本向量化
std::string OndeviceEmbeddingEngine::currentTextEmbeddingModel() {
    return embeddingClient_->currentTextEmbeddingModel();
}

result::AiEngineBaseResult
OndeviceEmbeddingEngine::setCurrentTextEmbeddingModel(
    const std::string& model) {
    return embeddingClient_->setCurrentTextEmbeddingModel(model);
}

result::AiEngineBaseResult OndeviceEmbeddingEngine::embeddingText(
    const std::string& text) {
    return embeddingClient_->embeddingText(text);
}

// 图像向量化
std::string OndeviceEmbeddingEngine::currentImageEmbeddingModel() {
    return embeddingClient_->currentImageEmbeddingModel();
}

result::AiEngineBaseResult
OndeviceEmbeddingEngine::setCurrentImageEmbeddingModel(
    const std::string& model) {
    return embeddingClient_->setCurrentImageEmbeddingModel(model);
}

result::AiEngineBaseResult OndeviceEmbeddingEngine::embeddingImageFromImageFile(
    const std::string& filePath) {
    return embeddingClient_->embeddingImageFromImageFile(filePath);
}
result::AiEngineBaseResult
OndeviceEmbeddingEngine::embeddingImageFromBase64ImageData(
    const std::string& fileData) {
    return embeddingClient_->embeddingImageFromBase64ImageData(fileData);
}
result::AiEngineBaseResult OndeviceEmbeddingEngine::embeddingTextWithImageModel(
    const std::string& text) {
    return embeddingClient_->embeddingTextWithImageModel(text);
}

void OndeviceEmbeddingEngine::stopImageEmbedding() {
    return embeddingClient_->stopImageEmbedding();
}
void OndeviceEmbeddingEngine::stopTextEmbedding() {
    return embeddingClient_->stopTextEmbedding();
}

}  // namespace embedding
}  // namespace core_ai
}  // namespace ai_engine