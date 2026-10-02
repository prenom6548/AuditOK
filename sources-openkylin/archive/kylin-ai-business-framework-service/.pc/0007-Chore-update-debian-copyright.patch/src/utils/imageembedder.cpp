#include "imageembedder.h"
#include "math.h"

#include <opencv4/opencv2/opencv.hpp>

#include <iostream>

static const char* TXT_MODEL_FOLDER = "/usr/share/kylin-datamanagement-models/cn-clip-onnx";
static const char* VISION_MODEL_PATH = "/usr/share/kylin-datamanagement-models/cn-clip-onnx/vit-b-16.img.fp32.onnx";
static const char* TXT_MODEL_PATH = "/usr/share/kylin-datamanagement-models/cn-clip-onnx/vit-b-16.txt.fp32.onnx";
static const int INPUT_CONTEXT_LENGTH = 52;


ImageEmbedder::ImageEmbedder()
    : textSession_(Ort::Env(ORT_LOGGING_LEVEL_WARNING, "Default"),
                   TXT_MODEL_PATH, Ort::SessionOptions(nullptr))
    , imageSession_(Ort::Env(ORT_LOGGING_LEVEL_WARNING, "Default"),
                     VISION_MODEL_PATH, Ort::SessionOptions(nullptr))
    , tokenizer_(TXT_MODEL_FOLDER)

{
}

std::vector<float> ImageEmbedder::embeddingText(const std::string &text)
{
    if (text.empty()) {
        return std::vector<float>();
    }

    const std::vector<const char*> inputNames = {"text"};
    const std::vector<int64_t> inputShape = {1, 52};
    const std::vector<const char*> outputNames = {"unnorm_text_features"};
    const std::vector<int64_t> outputShape = {1, 512};
    const int batch = 1;

    std::vector<int64_t> tokens = tokenizer_.tokenize(text)["input_ids"];
    if (tokens.empty()) {
        std::cerr << "Tokenize error, can't embedding text: "
                  << text << std::endl;
        return std::vector<float>();
    }
    Math::normalizeTokens(tokens, INPUT_CONTEXT_LENGTH);

    tokens.insert(tokens.begin(), batch);


    Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeCPU);

    Ort::Value inputTensorValue =
        Ort::Value::CreateTensor<int64_t>(memoryInfo, tokens.data(), tokens.size(),
                                        inputShape.data(), inputShape.size());

    std::vector<float> result(outputShape[1]);
    Ort::Value outputTensorValue =
        Ort::Value::CreateTensor<float>(memoryInfo, result.data(), result.size(),
                                        outputShape.data(), outputShape.size());

    std::cout << "Chinese-Clip start inference" << std::endl;
    Ort::RunOptions runOpts(nullptr);
    textSession_.Run(runOpts, inputNames.data(), &inputTensorValue, inputNames.size(),
                     outputNames.data(), &outputTensorValue, outputNames.size());
    std::cout << "Chinese-Clip finish inference" << std::endl;

    Math::normalize(result);

    return result;
}

std::vector<float> ImageEmbedder::embeddingImage(const std::string& filePath)
{
    if (filePath.empty()) {
        return std::vector<float>();
    }

    const std::vector<const char*> inputNames = {"image"};
    const std::vector<int64_t> inputShape = {1, 3, 224, 224};
    const std::vector<const char*> outputNames = {"unnorm_image_features"};
    const std::vector<int64_t> outputShape = {1, 512};
    const int batch = 1;

    std::vector<float> vision = preprocessVision(filePath);

    if (vision.empty()) {
        std::cerr << "Preprocess vision filed, can't embedding vision: "
                  << filePath << std::endl;
        return std::vector<float>();
    }

    // 添加批量大小维度
    vision.insert(vision.begin(), batch);

    Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeCPU);
    Ort::Value inputTensorValue =
        Ort::Value::CreateTensor<float>(memoryInfo, vision.data(), vision.size(),
                                        inputShape.data(), inputShape.size());

    std::vector<float> result(outputShape[1]);
    Ort::Value outputTensorValue =
        Ort::Value::CreateTensor<float>(memoryInfo, result.data(), result.size(),
                                        outputShape.data(), outputShape.size());

    std::cout << "Start embedding image " << filePath << std::endl;
    Ort::RunOptions runOpts(nullptr);
    imageSession_.Run(runOpts, inputNames.data(), &inputTensorValue, inputNames.size(),
                       outputNames.data(), &outputTensorValue, outputNames.size());
    std::cout << "Finish embedding image " << filePath << std::endl;

    Math::normalize(result);

    return result;
}

std::vector<float> ImageEmbedder::preprocessVision(const std::string &filePath) const
{
    const int width = 224;
    const int height = 224;
    const cv::Scalar mean = cv::Scalar(0.48145466, 0.4578275, 0.40821073);
    const cv::Scalar std = cv::Scalar(0.26862954, 0.26130258, 0.27577711);

    cv::Mat image = cv::imread(filePath);
    if(image.empty()) {
        std::cerr << "Can't read image: " << filePath << std::endl;
        return std::vector<float>();
    }

    cv::Size inputSize(width, height);
    cv::resize(image, image, inputSize, 0, 0, cv::INTER_CUBIC);
    cv::cvtColor(image, image, cv::COLOR_BGR2RGB);

    // Normalize
    image.convertTo(image, CV_32FC3);
    image /= 255.0;
    image -= mean;
    image /= std;

    // Convert to PyTorch tensor format by transposing the channels
    cv::transpose(image, image);

    return imageToFloatVector(image);
}

std::vector<float> ImageEmbedder::imageToFloatVector(const cv::Mat &image) const
{
    std::vector<float> floatVector;

    // 确保图像不为空且为三通道图像
    if (image.empty() || image.channels() != 3) {
        std::cerr << "Image is empty or not 3 channels, "
                     "can't convert to vector" << std::endl;
        return floatVector;
    }

    // 将每个通道的像素值添加到向量中
    for (int c = 0; c < image.channels(); ++c) {
        for (int i = 0; i < image.rows; ++i) {
            for (int j = 0; j < image.cols; ++j) {
                float pixelValue = image.at<cv::Vec3f>(i, j)[c];
                floatVector.push_back(pixelValue);
            }
        }
    }

    return floatVector;
}
