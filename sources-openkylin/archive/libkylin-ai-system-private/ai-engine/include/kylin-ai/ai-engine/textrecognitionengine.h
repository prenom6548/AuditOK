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

#ifndef TEXTRECOGNITIONENGINE_H
#define TEXTRECOGNITIONENGINE_H

#include <functional>
#include <vector>

#include "aiengine.h"
#include "aiengineerror.h"
#include "aiengineresult.h"

namespace ai_engine {
namespace core_ai {
namespace vision {

typedef struct {
    int x;
    int y;
} PixelPoint;

struct TextLine {
    std::string text;
    std::vector<PixelPoint> points;
    float conf;
};

struct TextRecognitionResult : result::AiEngineModelResult {
    std::string allText;
    std::vector<TextLine> lines;
};

using TextRecognitionResultCallback =
    std::function<void(const TextRecognitionResult &result)>;

class AbstractTextRecognitionEngine : public ai_engine::AbstractAiEngine {
public:
    virtual ~AbstractTextRecognitionEngine() {}

    virtual void setTextRecognitionResultCallback(
        TextRecognitionResultCallback callback) = 0;

    virtual result::AiEngineBaseResult init() = 0;
    virtual result::AiEngineBaseResult destroy() = 0;

    // 从图片文件中识别文本
    virtual result::AiEngineBaseResult recognizeTextFromFile(
        const std::string &imageFilePath) = 0;

    // 从图片数据中识别文本
    virtual result::AiEngineBaseResult recognizeTextFromBase64ImageData(
        const std::string &imageBase64Data) = 0;
};

}  // namespace vision
}  // namespace core_ai
}  // namespace ai_engine
#endif  // TEXTRECOGNITIONENGINE_H