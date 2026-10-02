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

    virtual std::string currentTextRecognitionModel() const { return {}; }
    virtual void setCurrentTextRecognitionModel(const std::string &model) = 0;

    virtual result::AiEngineBaseResult init() = 0;
    virtual result::AiEngineBaseResult destroy() = 0;

    // 从图片文件中识别文本
    virtual result::AiEngineBaseResult recognizeTextFromFile(
        const std::string &imageFilePath) = 0;

    // 从图片数据中识别文本
    virtual result::AiEngineBaseResult recognizeTextFromBase64ImageData(
        const std::string &imageBase64Data) = 0;

    virtual void stopTextRecognition() = 0;
};

}  // namespace vision
}  // namespace core_ai
}  // namespace ai_engine
#endif  // TEXTRECOGNITIONENGINE_H