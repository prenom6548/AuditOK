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

#pragma once

#include <kylin-ai/ai-engine/embeddingengine.h>

namespace ai_engine {
namespace core_ai {
namespace embedding {
class EmbeddingClient;

class OndeviceEmbeddingEngine : public AbstractEmbeddingEngine {
public:
    OndeviceEmbeddingEngine();
    ~OndeviceEmbeddingEngine();
    void setEmbeddingResultCallback(EmbeddingResultCallback callback) override;

    std::string engineName() const override {
        return "ondevice-embedding-engine";
    }

    bool isCloud() const override { return false; }

    bool isBuiltInEngine() const override { return true; }

    std::string modelInfo() const override;

    result::AiEngineBaseResult init() override;

    // 文本向量化
    // 获取当前引擎使用的文本向量化模型
    std::string currentTextEmbeddingModel() override;
    // 设置当前引擎使用的文本向量化模型
    result::AiEngineBaseResult setCurrentTextEmbeddingModel(
        const std::string& model) override;
    result::AiEngineBaseResult embeddingText(const std::string& text) override;

    // 图像向量化
    // 获取当前引擎使用的图像向量化模型
    std::string currentImageEmbeddingModel() override;
    // 设置当前引擎使用的图像向量化模型
    result::AiEngineBaseResult setCurrentImageEmbeddingModel(
        const std::string& model) override;
    result::AiEngineBaseResult embeddingImageFromImageFile(
        const std::string& filePath) override;
    result::AiEngineBaseResult embeddingImageFromBase64ImageData(
        const std::string& fileData) override;
    result::AiEngineBaseResult embeddingTextWithImageModel(
        const std::string& text) override;
    void stopImageEmbedding() override;
    void stopTextEmbedding() override;

private:
    EmbeddingClient* embeddingClient_ = nullptr;
};
}  // namespace embedding
}  // namespace core_ai
}  // namespace ai_engine
