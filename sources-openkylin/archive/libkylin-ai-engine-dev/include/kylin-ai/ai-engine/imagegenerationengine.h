#ifndef GENAIIMAGEENGINE_H
#define GENAIIMAGEENGINE_H

#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "aiengine.h"
#include "aiengineerror.h"
#include "aiengineresult.h"

namespace ai_engine {
namespace gen_ai {
namespace vision {

struct ImageGenerationResult : result::AiEngineModelResult {
    int width;
    int height;
    int index;
    int total;
    std::string format;
    std::string imageBase64Data;
};

using ImageGenerationCallback =
    std::function<void(const ImageGenerationResult &imgResult)>;

class AbstractImageGenerationEngine : public ai_engine::AbstractAiEngine {
public:
    virtual ~AbstractImageGenerationEngine() {}

    virtual void setGenerationResultCallback(
        ImageGenerationCallback callback) = 0;

    // 初始化图像生成模块
    virtual result::AiEngineBaseResult init() = 0;

    // 销毁图像生成模块
    virtual result::AiEngineBaseResult destroy() = 0;

    // 获取支持的图像生成参数
    virtual std::vector<std::string> supportedImageStyles() const = 0;

    // 获取支持的图像生成数量
    virtual int supportedImageNumber() const = 0;

    // 获取支持的图像生成尺寸
    virtual std::vector<std::pair<int, int>> supportedImageSizes() const = 0;

    // 设置图像生成风格
    virtual void setImageStyle(const std::string &style) = 0;

    // 设置图像生成数量
    virtual void setImageNumber(int number) = 0;

    // 设置图像生成尺寸
    virtual void setImageSize(int width, int height) = 0;

    // 生成图像
    virtual result::AiEngineBaseResult generateImage(
        const std::string &prompt) = 0;

    virtual void stopImageGeneration() = 0;
};

}  // namespace vision
}  // namespace gen_ai
}  // namespace ai_engine

#endif