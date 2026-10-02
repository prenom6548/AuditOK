/*
 * Copyright 2025 KylinSoft Co., Ltd.
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

#include "imagehandler.h"

#include "assistant/handlers/requesthandler.h"
#include "assistant/message.h"
#include "modelconfig/modelstatustracker.h"
#include "utils/logger.h"
#include "utils/util.h"

ImageHandler::~ImageHandler() {
    if (imageGenerationEngine_) {
        imageGenerationEngine_->setGenerationResultCallback(nullptr);
    }
    if (imageProcessEngine_) {
        imageProcessEngine_->setMattingResultCallback(nullptr);
        imageProcessEngine_->setImageBinaryResultCallback(nullptr);
    }

    destroyImageGenerationEngine();
    destroyImageProcessEngine();
    LOGI("ImageHandler::~ImageHandler");
}

std::string ImageHandler::getFeedbackUrl(const std::string& imageModelName) {
    if (imageGenerationEngine_ != nullptr) {
        std::string feedbackUrl;
        Json::Value modelInfoJson = util::stringToJson(imageGenerationEngine_->modelInfo());
        for (const auto& item : modelInfoJson["models"]) {
            if (item["name"].asString() == imageModelName && item.isMember("feedbackUrl")) {
                feedbackUrl = item["feedbackUrl"].asString();
                return feedbackUrl;
            }
        }
    }
    return "";
}
DialogueData::CallSuccessIdentifier ImageHandler::textToImage(const std::string& prompt) {
    DialogueData::CallSuccessIdentifier ret{};

    if (imageGenerationEngine_ == nullptr) {
        int errorCode = int(assistant::ErrorCode::VisionModelNotFound);
        DialogueCallback::TextToImageResult imageData{};
        imageData.success = false;
        imageData.errorCode_ = errorCode;
        imageData.errorMessage_ = "vision引擎错误";
        imageData.type_ = DialogueCallback::ImageErrorOccurred;
        callTextToImageCallback(imageData);
        ret.successRes = false;
        ret.errorCode = errorCode;
        ret.errorMessage = "vision引擎错误";
        return ret;
    }
    LOGI("text to image generating");
    DialogueCallback::TextToImageResult imageData{};
    imageData.success = true;
    imageData.errorCode_ = 0;
    imageData.errorMessage_ = "";
    imageData.type_ = DialogueCallback::ImageGenerating;
    imageData.modelName_ = imageGenerationEngine_->currentModel();
    callTextToImageCallback(imageData);
    ModelStatusTracker statusTracker(imageGenerationEngine_->currentModelName());
    imageGenerationEngine_->setImageSize(512, 512);
    auto result = imageGenerationEngine_->generateImage(prompt);
    if (!result.success) {
        assistant::ErrorCode errorCode = assistant::Error::getErrorCodeFromVisionEngine(result.error.getCode());
        DialogueCallback::TextToImageResult errorImageData{};
        errorImageData.success = false;
        errorImageData.errorCode_ = int(errorCode);
        errorImageData.errorMessage_ = result.error.getMessage();
        errorImageData.type_ = DialogueCallback::ImageErrorOccurred;
        errorImageData.modelName_ = "";
        callTextToImageCallback(errorImageData);
        LOGI("text to image error: {} {}", result.error.getCode(), result.error.getMessage().c_str());
        ret.successRes = false;
        ret.errorCode = int(errorCode);
        ret.errorMessage = result.error.getMessage();
        return ret;
    }
    ret.successRes = true;
    return ret;
}

DialogueData::CallSuccessIdentifier ImageHandler::imageMatting(const std::string& imagePath) {
    DialogueData::CallSuccessIdentifier ret{};
    if (imageProcessEngine_ == nullptr) {
        int errorCode = int(assistant::ErrorCode::VisionServerError);
        DialogueCallback::ImageMattingResult result{};
        result.errorCode = errorCode;
        result.errorMessage = "vision引擎错误";
        result.type = DialogueCallback::ImageErrorOccurred;
        callImageMattingCallback(result);
        ret.errorCode = errorCode;
        ret.errorMessage = "vision引擎错误";
        return ret;
    }

    waitForImageMattingModuleInited();

    DialogueCallback::ImageMattingResult generatingResult{};
    generatingResult.success = true;
    generatingResult.type = DialogueCallback::ImageGenerating;
    generatingResult.modelName = imageProcessEngine_->currentModel();
    callImageMattingCallback(generatingResult);

    auto result =
        imageProcessEngine_->mattingFromImageFile(imagePath, ai_engine::core_ai::vision::MATTING_MODE_PORTRAIT);
    if (!result.success) {
        assistant::ErrorCode errorCode = assistant::Error::getErrorCodeFromVisionEngine(result.error.getCode());
        DialogueCallback::ImageMattingResult generatedResult{};
        generatedResult.type = DialogueCallback::ImageErrorOccurred;
        generatedResult.errorCode = int(errorCode);
        generatedResult.errorMessage = result.error.getMessage();
        LOGI("image matting error: {}{}", result.error.getCode(), result.error.getMessage().c_str());
        callImageMattingCallback(generatedResult);
    }
    return ret;
}

DialogueData::CallSuccessIdentifier ImageHandler::imageBinarization(const std::string& imagePath) {
    DialogueData::CallSuccessIdentifier ret{};
    if (imageProcessEngine_ == nullptr) {
        int errorCode = int(assistant::ErrorCode::VisionServerError);
        DialogueCallback::ImageBinarizationResult result{};
        result.errorCode = errorCode;
        result.errorMessage = "vision引擎错误";
        result.type = DialogueCallback::ImageErrorOccurred;
        callImageBinarizationCallback(result);
        ret.errorCode = errorCode;
        ret.errorMessage = "vision引擎错误";
        return ret;
    }

    waitForImageBinaryModuleInited();

    DialogueCallback::ImageBinarizationResult generatingResult{};
    generatingResult.success = true;
    generatingResult.type = DialogueCallback::ImageGenerating;
    generatingResult.modelName = imageProcessEngine_->currentModel();
    callImageBinarizationCallback(generatingResult);

    auto result = imageProcessEngine_->binarizeImageFromImageFile(imagePath);
    if (!result.success) {
        assistant::ErrorCode errorCode = assistant::Error::getErrorCodeFromVisionEngine(result.error.getCode());
        DialogueCallback::ImageBinarizationResult generatedResult{};
        generatedResult.type = DialogueCallback::ImageErrorOccurred;
        generatedResult.errorCode = int(errorCode);
        generatedResult.errorMessage = result.error.getMessage();
        LOGI("image binarization error: {}{}", result.error.getCode(), result.error.getMessage().c_str());
        callImageBinarizationCallback(generatedResult);
    }
    return ret;
}

void ImageHandler::initImageGenerationEngine(const std::shared_ptr<assistant::ImageGenerationEngine>& visionEngine) {
    imageGenerationEngine_ = visionEngine;

    if (imageGenerationEngine_ == nullptr) {
        return;
    }

    auto result = imageGenerationEngine_->init();
    if (result.success) {
        imageGenerationEngineInited_ = true;
    } else {
        LOGE("图片生成引擎初始化失败");
    }

    imageGenerationEngine_->setGenerationResultCallback(
        [this](const ai_engine::gen_ai::vision::ImageGenerationResult& result) {
            int errorCode = int(assistant::Error::getErrorCodeFromVisionEngine(result.error.getCode()));
            DialogueCallback::TextToImageResult imageData{};
            imageData.success = result.success;
            imageData.imageFormat = result.format;
            imageData.height_ = result.height;
            imageData.width_ = result.width;
            imageData.total_ = result.total;
            imageData.index_ = result.index;
            imageData.imageBase64Data_ = result.imageBase64Data;
            imageData.errorCode_ = errorCode;
            imageData.errorMessage_ = result.error.getMessage();
            imageData.type_ = result.success ? DialogueCallback::ImageGenerated : DialogueCallback::ImageErrorOccurred;
            imageData.modelName_ = result.modelName;
            callTextToImageCallback(imageData);
        });
}

void ImageHandler::destroyImageGenerationEngine() {
    if (imageGenerationEngineInited_) {
        if (imageGenerationEngine_ != nullptr) {
            imageGenerationEngine_->destroy();
        }
        imageGenerationEngineInited_ = false;
    }
}

void ImageHandler::initImageProcessEngine(const std::shared_ptr<assistant::ImageProcessEngine>& imageMattingEngine) {
    imageProcessEngine_ = imageMattingEngine;
    if (imageProcessEngine_ == nullptr) {
        return;
    }

    imageMattingInitFuture_ = std::async(std::launch::async, [this]() { initImageMattingModule(); });

    imageBinaryInitFuture_ = std::async(std::launch::async, [this]() { initImageBinaryModule(); });
}

void ImageHandler::destroyImageProcessEngine() {
    destroyImageMattingModule();
    destroyImageBinaryModule();
}

void ImageHandler::stopImageBinarization() {
    if (imageMattingEngineInited_) {
        imageProcessEngine_->stopImageBinarization();
    }
}

void ImageHandler::stopImageMatting() {
    if (imageBinaryEngineInited_) {
        imageProcessEngine_->stopMattingProcess();
    }
}

void ImageHandler::initImageMattingModule() {
    auto result = imageProcessEngine_->initMattingsModule();
    if (result.success) {
        imageMattingEngineInited_ = true;
    } else {
        LOGE("图片生成引擎初始化失败");
    }
    imageProcessEngine_->setMattingResultCallback([this](const ai_engine::core_ai::vision::ImageResult& result) {
        int errorCode = int(assistant::Error::getErrorCodeFromVisionEngine(result.error.getCode()));
        DialogueCallback::ImageMattingResult imageMattingResult{};
        imageMattingResult.success = result.success;
        imageMattingResult.imageFormat = result.imageFormat;
        imageMattingResult.imageBase64Data = result.imageBase64Data;
        imageMattingResult.errorCode = int(errorCode);
        imageMattingResult.errorMessage = result.error.getMessage();
        imageMattingResult.modelName = result.modelName;
        imageMattingResult.type =
            result.success ? DialogueCallback::ImageGenerated : DialogueCallback::ImageErrorOccurred;
        callImageMattingCallback(imageMattingResult);
    });
}

void ImageHandler::destroyImageMattingModule() {
    if (imageMattingEngineInited_) {
        if (imageProcessEngine_ != nullptr) {
            imageProcessEngine_->destroyMattingsModule();
        }
        imageMattingEngineInited_ = false;
    }
}

void ImageHandler::initImageBinaryModule() {
    auto result = imageProcessEngine_->initImageBinaryModule();
    if (result.success) {
        imageBinaryEngineInited_ = true;
    } else {
        LOGE("图片生成引擎初始化失败");
    }
    imageProcessEngine_->setImageBinaryResultCallback([this](const ai_engine::core_ai::vision::ImageResult& result) {
        int errorCode = int(assistant::Error::getErrorCodeFromVisionEngine(result.error.getCode()));
        DialogueCallback::ImageBinarizationResult imageBinarizationResult{};
        imageBinarizationResult.success = result.success;
        imageBinarizationResult.imageFormat = result.imageFormat;
        imageBinarizationResult.imageBase64Data = result.imageBase64Data;
        imageBinarizationResult.errorCode = errorCode;
        imageBinarizationResult.errorMessage = result.error.getMessage();
        imageBinarizationResult.modelName = result.modelName;
        imageBinarizationResult.type =
            result.success ? DialogueCallback::ImageGenerated : DialogueCallback::ImageErrorOccurred;
        callImageBinarizationCallback(imageBinarizationResult);
    });
}

void ImageHandler::destroyImageBinaryModule() {
    if (imageBinaryEngineInited_) {
        if (imageProcessEngine_ != nullptr) {
            imageProcessEngine_->destroyImageBinaryModule();
        }
        imageBinaryEngineInited_ = false;
    }
}

void ImageHandler::waitForImageMattingModuleInited() {
    if (imageMattingInitFuture_.valid()) {
        if (imageMattingInitFuture_.wait_for(std::chrono::seconds(25)) == std::future_status::ready) {
            try {
                imageMattingInitFuture_.get();
            } catch (const std::exception& e) {
                LOGE("Exception during image matting module initialization: {}", e.what());
            }
        } else {
            LOGE("Image matting module init timeout.");
        }
    }
}

void ImageHandler::waitForImageBinaryModuleInited() {
    if (imageBinaryInitFuture_.valid()) {
        if (imageBinaryInitFuture_.wait_for(std::chrono::seconds(25)) == std::future_status::ready) {
            try {
                imageBinaryInitFuture_.get();
            } catch (const std::exception& e) {
                LOGE("Exception during image binary module initialization: {}", e.what());
            }
        } else {
            LOGE("Image binary module init timeout.");
        }
    }
}
