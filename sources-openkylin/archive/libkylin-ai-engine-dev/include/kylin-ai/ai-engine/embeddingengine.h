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

#ifndef EMBEDDINGENGINE_H
#define EMBEDDINGENGINE_H

#include <functional>
#include <string>
#include <vector>

#include "aiengine.h"
#include "aiengineerror.h"
#include "aiengineresult.h"

namespace ai_engine {
namespace core_ai {
namespace embedding {

struct EmbeddingResult : result::AiEngineModelResult {
    std::vector<float> embeddings;
};

using EmbeddingResultCallback =
    std::function<void(const EmbeddingResult &result)>;
class AbstractEmbeddingEngine : public ai_engine::AbstractAiEngine {
public:
    virtual ~AbstractEmbeddingEngine() {}

    virtual void setEmbeddingResultCallback(
        EmbeddingResultCallback callback) = 0;

    // 用于获取与triton的连接
    virtual result::AiEngineBaseResult init() = 0;

    // 获取当前引擎使用的文本向量化模型
    virtual std::string currentTextEmbeddingModel() = 0;

    // 设置当前引擎使用的文本向量化模型
    virtual result::AiEngineBaseResult setCurrentTextEmbeddingModel(
        const std::string &model) = 0;

    virtual result::AiEngineBaseResult embeddingText(
        const std::string &text) = 0;

    // 获取当前引擎使用的图像向量化模型
    virtual std::string currentImageEmbeddingModel() = 0;

    // 设置当前引擎使用的图像向量化模型
    virtual result::AiEngineBaseResult setCurrentImageEmbeddingModel(
        const std::string &model) = 0;

    virtual result::AiEngineBaseResult embeddingImageFromImageFile(
        const std::string &filePath) = 0;
    virtual result::AiEngineBaseResult embeddingImageFromBase64ImageData(
        const std::string &fileData) = 0;
    virtual void stopImageEmbedding() = 0;
    virtual result::AiEngineBaseResult embeddingTextWithImageModel(
        const std::string &text) = 0;
    virtual void stopTextEmbedding() = 0;
    ///[1]
};
}  // namespace embedding
}  // namespace core_ai
}  // namespace ai_engine

#endif  // EMBEDDINGENGINE_H
