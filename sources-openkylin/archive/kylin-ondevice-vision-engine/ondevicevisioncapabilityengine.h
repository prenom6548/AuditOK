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

#pragma once

#include <kylin-ai/ai-engine/imageprocessengine.h>
#include <kylin-ai/ai-engine/textrecognitionengine.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace ai_engine::core_ai::vision {
class PortraitMattingTritonClient;
class UniversalMattingTritonClient;
class BinaryImage;
class SegTritonClient;
class OcrTritonClient;

class OndeviceVisionImageProcessEngine : public AbstractImageProcessEngine {
public:
    OndeviceVisionImageProcessEngine();
    virtual ~OndeviceVisionImageProcessEngine();
    bool isCloud() const override { return false; };
    bool isBuiltInEngine() const override { return true; };

    // 抠图模块
    result::AiEngineBaseResult initMattingsModule() override;
    result::AiEngineBaseResult destroyMattingsModule() override;
    void setMattingResultCallback(ImageResultCallback callback) override;
    result::AiEngineBaseResult mattingFromBase64ImageData(const std::string& imageBase64Data,
                                                          MattingMode mode) override;
    result::AiEngineBaseResult mattingFromImageFile(const std::string& imageFilePath, MattingMode mode) override;
    void stopMattingProcess() override;
    std::string currentPortraitSegmentationModel() const;
    std::string currentGeneralSegmentationModel() const;
    void setCurrentPortraitSegmentationModel(const std::string& model);
    void setCurrentGeneralSegmentationModel(const std::string& model);

public:
    // 图像二值化模块
    result::AiEngineBaseResult initImageBinaryModule() override;
    result::AiEngineBaseResult destroyImageBinaryModule() override;
    void setImageBinaryResultCallback(ImageResultCallback callback) override;
    result::AiEngineBaseResult binarizeImageFromBase64ImageData(const std::string& imageBase64Data) override;
    result::AiEngineBaseResult binarizeImageFromImageFile(const std::string& imageFilePath) override;
    void stopImageBinarization() override;

public:
    // 通用分割模块
    result::AiEngineBaseResult initSegmentationModule() override;
    result::AiEngineBaseResult destroySegmentationModule() override;
    void setSegmentationResultCallback(const SegmentationResultCallback& callback) override;
    result::AiEngineBaseResult encodeSegmentationFromBase64ImageData(const std::string& imageBase64Data) override;
    result::AiEngineBaseResult encodeSegmentationFromImageFile(const std::string& imageFilePath) override;
    result::AiEngineBaseResult decodeSegmentation(const std::vector<SegmentationPoint>& points,
                                                  uint32_t minArea) override;
    void clearSegmentationCache() override;
    result::AiEngineBaseResult stopSegmentation() override;
    std::string currentSegmentationModel() const override;
    void setCurrentSegmentationModel(const std::string &model) override;

public:
    std::string engineName() const override { return "ondevice"; };
    std::string currentModel() const override { return modelName_; };
    std::string currentModelName() const override { return modelName_; };

    std::string modelInfo() const override;

private:
    std::string modelName_ = "端侧视觉模型";
    PortraitMattingTritonClient* portraitMattingClient_ = nullptr;
    UniversalMattingTritonClient* universalMattingClient_ = nullptr;
    SegTritonClient* segClient_ = nullptr;
    BinaryImage* binImage_ = nullptr;
};

class OndeviceVisionTextRecognitionEngine : public AbstractTextRecognitionEngine {
public:
    OndeviceVisionTextRecognitionEngine();
    virtual ~OndeviceVisionTextRecognitionEngine();
    bool isCloud() const override { return false; };
    bool isBuiltInEngine() const override { return true; };

    void setTextRecognitionResultCallback(TextRecognitionResultCallback callback) override;

    result::AiEngineBaseResult init() override;
    result::AiEngineBaseResult destroy() override;

    // 从图片文件中识别文本
    result::AiEngineBaseResult recognizeTextFromFile(const std::string& imageFilePath) override;

    // 从图片数据中识别文本
    result::AiEngineBaseResult recognizeTextFromBase64ImageData(const std::string& imageBase64Data) override;

    std::string currentTextRecognitionModel() const override;
    void setCurrentTextRecognitionModel(const std::string& model) override;

    void stopTextRecognition() override;

    std::string engineName() const override { return "ondevice"; };
    std::string currentModel() const override { return modelName_; };
    std::string currentModelName() const override { return modelName_; };

    std::string modelInfo() const override;

private:
    std::string modelName_ = "端侧视觉模型";
    OcrTritonClient* ocrClient_ = nullptr;
};

}  // namespace ai_engine::core_ai::vision
