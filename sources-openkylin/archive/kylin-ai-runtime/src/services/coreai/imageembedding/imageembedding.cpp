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

#include "imageembedding.h"

#include <kylin-ai/common/error.h>

#include "utils/logger.h"

namespace {
constexpr int64_t kDefaultImageEmbeddingTimeoutMs = 5LL * 60 * 1000;

int64_t loadImageEmbeddingTimeoutMs() {
    const auto taskTimeout = task::loadTaskTimeoutMs(task::TaskType::ImageEmbedding);
    if (taskTimeout.has_value()) {
        return *taskTimeout;
    }
    LOGW("ImageEmbedding task timeout not found in task config, use default.");
    return kDefaultImageEmbeddingTimeoutMs;
}
}  // namespace

ImageEmbedding::ImageEmbedding(
    std::unique_ptr<ai_engine::core_ai::embedding::AbstractEmbeddingEngine> imageEmbeddingEngine)
    : imageEmbeddingEngine_(std::move(imageEmbeddingEngine)) {}

ImageEmbedding::~ImageEmbedding() {
    imageEmbeddingResultCallback_ = nullptr;
    imageEmbeddingEngine_->setEmbeddingResultCallback(nullptr);
    LOGD("~ImageEmbedding");
}

void ImageEmbedding::setEmbeddingResultCallback(const EmbeddingResultCallback& callback) {
    if (callback == nullptr) {
        return;
    }
    imageEmbeddingResultCallback_ = callback;
}

std::string ImageEmbedding::getModelInfo() const { return imageEmbeddingEngine_->modelInfo(); }

void ImageEmbedding::embeddingImageFromBase64Data(const std::string& base64Data,
                                                  EmbeddingResultCallback resultCallback) {
    if (resultCallback == nullptr) {
        return;
    }
    const auto& request = generateImageEmbeddingRequestWithBase64Data(base64Data, std::move(resultCallback));
    imageEmbeddingRequests_.enqueue(request);
    imageEmbeddingAsync();
}

void ImageEmbedding::embeddingImageFromFile(const std::string& imageFilePath, EmbeddingResultCallback resultCallback) {
    if (resultCallback == nullptr) {
        return;
    }
    const auto& request = generateImageEmbeddingRequestWithFilePath(imageFilePath, std::move(resultCallback));
    imageEmbeddingRequests_.enqueue(request);
    imageEmbeddingAsync();
}

void ImageEmbedding::embeddingText(const std::string& text, EmbeddingResultCallback resultCallback) {
    if (resultCallback == nullptr) {
        return;
    }
    const auto& request = generateImageEmbeddingRequestWithText(text, std::move(resultCallback));
    imageEmbeddingRequests_.enqueue(request);
    imageEmbeddingAsync();
}

ImageEmbedding::ImageEmbeddingRequest ImageEmbedding::generateImageEmbeddingRequestWithFilePath(
    const std::string& imageFilePath, EmbeddingResultCallback resultCallback) {
    ImageEmbeddingRequest request;
    request.imageFilePath = imageFilePath;
    request.resultCallback = std::move(resultCallback);
    request.type = ImageEmbeddingRequest::Type::ImagePath;
    return request;
}

ImageEmbedding::ImageEmbeddingRequest ImageEmbedding::generateImageEmbeddingRequestWithBase64Data(
    const std::string& base64Data, EmbeddingResultCallback resultCallback) {
    ImageEmbeddingRequest request;
    request.base64ImageData = base64Data;
    request.resultCallback = std::move(resultCallback);
    request.type = ImageEmbeddingRequest::Type::Base64Data;
    return request;
}

ImageEmbedding::ImageEmbeddingRequest ImageEmbedding::generateImageEmbeddingRequestWithText(
    const std::string& text, EmbeddingResultCallback resultCallback) {
    ImageEmbeddingRequest request;
    request.text = text;
    request.resultCallback = std::move(resultCallback);
    request.type = ImageEmbeddingRequest::Type::Text;
    return request;
}

void ImageEmbedding::imageEmbeddingAsync() {
    bool expected = false;
    if (!taskRunning_.compare_exchange_strong(expected, true)) {
        return;
    }
    ImageEmbeddingRequest nextRequest{};
    if (!imageEmbeddingRequests_.try_dequeue(nextRequest)) {
        taskRunning_.store(false);
        return;
    }
    std::shared_ptr shareObject = getSharedObject();
    auto enqueueResult = task::TaskRunner::getInstance().addTask(
        {task::generateTaskId(), task::TaskType::ImageEmbedding, [shareObject, nextRequest]() mutable {
             shareObject->imageEmbeddingTask(std::move(nextRequest));
             shareObject->taskRunning_.store(false);
             shareObject->imageEmbeddingAsync();
         }});
    if (enqueueResult != task::TaskRunner::EnqueueResult::Accepted) {
        LOGE("Failed to enqueue ImageEmbedding task.");
        taskRunning_.store(false);
        if (enqueueResult == task::TaskRunner::EnqueueResult::RejectedQueueFull) {
            notifyRequestFailed(nextRequest, AiSdkCommonErrorCode::AI_COMMON_TOO_MANY_REQUESTS,
                                "Too many requests, please try again later.");
            rejectPendingRequests(AiSdkCommonErrorCode::AI_COMMON_TOO_MANY_REQUESTS,
                                  "Too many requests, please try again later.");
        } else {
            notifyRequestFailed(nextRequest, AiSdkCommonErrorCode::AISDK_SERVICE_ERROR, "Task runner is stopping.");
            rejectPendingRequests(AiSdkCommonErrorCode::AISDK_SERVICE_ERROR, "Task runner is stopping.");
        }
    }
}

void ImageEmbedding::imageEmbeddingTask(ImageEmbedding::ImageEmbeddingRequest request) {
    currentRequest_ = request;
    timedOut_.store(false);
    util::Watchdog watchdog;
    watchdog.Start(loadImageEmbeddingTimeoutMs(), [this]() {
        timedOut_.store(true);
        notifyRequestFailed(currentRequest_, AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT,
                            "Image Embedding timed out.");
        imageEmbeddingEngine_->stopImageEmbedding();
    });
    ai_engine::result::AiEngineBaseResult result{};
    switch (currentRequest_.type) {
        case ImageEmbeddingRequest::Type::Base64Data: {
            result = imageEmbeddingEngine_->embeddingImageFromBase64ImageData(currentRequest_.base64ImageData);
            break;
        }
        case ImageEmbeddingRequest::Type::ImagePath: {
            result = imageEmbeddingEngine_->embeddingImageFromImageFile(currentRequest_.imageFilePath);
            break;
        }
        case ImageEmbeddingRequest::Type::Text: {
            result = imageEmbeddingEngine_->embeddingTextWithImageModel(currentRequest_.text);
            break;
        }
        default:
            LOGE("Invalid ImageEmbeddingRequest type.");
            if (watchdog.TryFinish()) {
                notifyRequestFailed(currentRequest_, AiSdkCommonErrorCode::AISDK_BAD_REQUEST,
                                    "Unknown image Embedding input type.");
            }
            watchdog.Finish();
            break;
    }
    if (!result.success) {
        LOGE("Failed to image Embedding: {} {}", result.error.getCode(), result.error.getMessage());
        if (watchdog.TryFinish()) {
            callEmbeddingResultCallback(ImageEmbeddingResult{{}, result.error.getCode(), result.error.getMessage()});
        }
        watchdog.Finish();
        return;
    }
    watchdog.TryFinish();
    watchdog.Finish();
}

bool ImageEmbedding::init() {
    auto result = imageEmbeddingEngine_->init();

    if (!result.success) {
        LOGE("Failed to init embedding engine: {} {}", result.error.getCode(), result.error.getMessage());
        ImageEmbeddingResult embeddingResult{{}, result.error.getCode(), result.error.getMessage()};
        callEmbeddingResultCallback(embeddingResult);
        return false;
    }
    std::shared_ptr sharedObject = getSharedObject();
    if (sharedObject == nullptr) {
        return false;
    }

    imageEmbeddingEngine_->setEmbeddingResultCallback(
        [sharedObject](const ai_engine::core_ai::embedding::EmbeddingResult& result) {
            if (sharedObject->timedOut_.load()) {
                return;
            }
            ImageEmbeddingResult imageEmbeddingResult{result.embeddings, result.error.getCode(),
                                                      result.error.getMessage()};
            sharedObject->callEmbeddingResultCallback(imageEmbeddingResult);
        });

    return true;
}

bool ImageEmbedding::setCurrentImageEmbeddingModel(std::string modelName) {
    auto result = imageEmbeddingEngine_->setCurrentImageEmbeddingModel(modelName);
    if (!result.success) {
        LOGE("Failed to set embedding model: {} {}", result.error.getCode(), result.error.getMessage());
    }
    return result.success;
}

void ImageEmbedding::callEmbeddingResultCallback(const ImageEmbeddingResult& result) const {
    if (timedOut_.load()) {
        return;
    }
    if (currentRequest_.resultCallback != nullptr) {
        currentRequest_.resultCallback(result);
    }
}

void ImageEmbedding::notifyRequestFailed(const ImageEmbeddingRequest& request, int errorCode,
                                         const std::string& message) {
    ImageEmbeddingResult result{};
    result.errorCode = errorCode;
    result.errorMessage = message;
    if (request.resultCallback) {
        request.resultCallback(result);
    }
}

void ImageEmbedding::rejectPendingRequests(int errorCode, const std::string& message) {
    ImageEmbeddingRequest dropped{};
    while (imageEmbeddingRequests_.try_dequeue(dropped)) {
        notifyRequestFailed(dropped, errorCode, message);
    }
}
