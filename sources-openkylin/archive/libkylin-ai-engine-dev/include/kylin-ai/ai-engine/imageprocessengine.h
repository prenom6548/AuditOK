#ifndef COREAIIMAGEPROCESSENGINE_H
#define COREAIIMAGEPROCESSENGINE_H

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "aiengine.h"
#include "aiengineerror.h"
#include "aiengineresult.h"

namespace ai_engine {
namespace core_ai {
namespace vision {

struct ImageResult : public result::AiEngineModelResult {
    std::string imageFormat;
    std::string imageBase64Data;
    std::string maskBase64Data;
    int width;
    int height;
    // json data
    // [
    // [{"x":1,"y":2},{"x":2,"y":3}],      // 第一组
    // [{"x":10,"y":20},{"x":11,"y":21}]   // 第二组
    // ]
    std::string edgePointsData;
};

enum MattingMode {
    MATTING_MODE_PORTRAIT = 0,  // 人像抠图
    MATTING_MODE_GENERAL = 1,   // 通用抠图
};

using ImageResultCallback = std::function<void(const ImageResult &result)>;

// 开始：图像分割

// 输入结构体
struct SegmentationPoint {
    uint32_t x{};
    uint32_t y{};
    bool isForeground{true};  // 默认为前景
};

// 输出结构体
struct RunLengthEncodingRow {
    uint32_t index{};
    uint32_t count{};
};

using RunLengthEncodingMask = std::vector<RunLengthEncodingRow>;

struct ImageSegmentationResult : public result::AiEngineModelResult {
    RunLengthEncodingMask mask{};
    uint32_t width{};   // 宽度
    uint32_t height{};  // 高度
};

using SegmentationResultCallback =
    std::function<void(const ImageSegmentationResult &result)>;

// 结束：图像分割

class AbstractImageProcessEngine : public AbstractAiEngine {
public:
    virtual ~AbstractImageProcessEngine() {}

    // 抠图模块
    virtual result::AiEngineBaseResult initMattingsModule() = 0;
    virtual result::AiEngineBaseResult destroyMattingsModule() = 0;
    virtual void setMattingResultCallback(ImageResultCallback callback) = 0;
    virtual result::AiEngineBaseResult mattingFromBase64ImageData(
        const std::string &imageBase64Data, MattingMode mode) = 0;
    virtual result::AiEngineBaseResult mattingFromImageFile(
        const std::string &imageFilePath, MattingMode mode) = 0;
    virtual void stopMattingProcess() = 0;
    virtual std::string currentPortraitSegmentationModel() const { return {}; }
    virtual std::string currentGeneralSegmentationModel() const { return {}; }
    virtual void setCurrentPortraitSegmentationModel(
        const std::string &model) = 0;
    virtual void setCurrentGeneralSegmentationModel(
        const std::string &model) = 0;

    // 图像二值化模块
    virtual result::AiEngineBaseResult initImageBinaryModule() = 0;
    virtual result::AiEngineBaseResult destroyImageBinaryModule() = 0;
    virtual void setImageBinaryResultCallback(ImageResultCallback callback) = 0;
    virtual result::AiEngineBaseResult binarizeImageFromBase64ImageData(
        const std::string &imageBase64Data) = 0;
    virtual result::AiEngineBaseResult binarizeImageFromImageFile(
        const std::string &imageFilePath) = 0;
    virtual void stopImageBinarization() = 0;

    // 通用分割模块
    virtual result::AiEngineBaseResult initSegmentationModule() = 0;
    virtual result::AiEngineBaseResult destroySegmentationModule() = 0;
    virtual void setSegmentationResultCallback(
        const SegmentationResultCallback &callback) = 0;
    virtual result::AiEngineBaseResult encodeSegmentationFromBase64ImageData(
        const std::string &base64ImageData) = 0;
    virtual result::AiEngineBaseResult encodeSegmentationFromImageFile(
        const std::string &imageFilePath) = 0;
    virtual result::AiEngineBaseResult decodeSegmentation(
        const std::vector<SegmentationPoint> &points, uint32_t minArea) = 0;
    virtual void clearSegmentationCache() = 0;
    virtual result::AiEngineBaseResult stopSegmentation() = 0;
    virtual std::string currentSegmentationModel() const { return {}; }
    virtual void setCurrentSegmentationModel(const std::string &model) = 0;
};

}  // namespace vision
}  // namespace core_ai
}  // namespace ai_engine

#endif
