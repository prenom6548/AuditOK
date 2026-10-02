#ifndef SRC_SERVICES_GENAI_VISION_IMAGE_GENERATION_REQUEST_H
#define SRC_SERVICES_GENAI_VISION_IMAGE_GENERATION_REQUEST_H

#include <functional>
#include <string>

namespace genai {

enum ImageFormat { IMAGE_FORMAT_UNKNOWN = -1, IMAGE_FORMAT_JPEG, IMAGE_FORMAT_PNG, IMAGE_FORMAT_WEBP };

struct ImageGenerationResult {
    int format{IMAGE_FORMAT_UNKNOWN};
    int width{0};
    int height{0};
    int total{0};
    int index{-1};
    std::string imageBase64Data;
    int errorCode{0};
    std::string errorMessage;
    std::string requestId;
};

struct ImageGenerationConfig {
    virtual ~ImageGenerationConfig() = default;

    int width{};
    int height{};
    int number{};
    std::string style;
    std::string modelName;

    virtual bool fromString(const std::string& config);
};

struct ImageGenerationRequest : public ImageGenerationConfig {
    using ResultCallback = std::function<void(const ImageGenerationResult&)>;
    using FinishedCallback = std::function<void()>;
    std::string prompt;
    std::string requestId;
    ResultCallback resultCallback;
    FinishedCallback finishedCallback;

    bool fromString(const std::string& request) override;

    std::string toString() const;
};

}  // namespace genai

#endif  // SRC_SERVICES_GENAI_VISION_IMAGE_GENERATION_REQUEST_H
