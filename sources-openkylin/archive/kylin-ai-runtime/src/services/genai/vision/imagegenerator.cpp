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

#include "imagegenerator.h"

#include <jsoncpp/json/json.h>
#include <kylin-ai/common/error.h>

#include "engineselector/engineselector.h"
#include "modelconfig/modelconfigmanager.h"
#include "modelconfig/modelstatustracker.h"
#include "taskmanager/taskrunner.h"
#include "utils/logger.h"
#include "utils/util.h"

namespace {

constexpr const char *serviceName = "GenAiVisionService";

constexpr int64_t kDefaultImageGenerationTimeoutMs = 3LL * 60 * 1000;
constexpr size_t kDefaultImageGenerationRequestQueueMaxSize = 10;

int64_t loadImageGenerationTimeoutMs() {
    const auto taskTimeout = task::loadTaskTimeoutMs(task::TaskType::TextToImage);
    if (taskTimeout.has_value()) {
        return *taskTimeout;
    }
    LOGW("Image generation task timeout not found in task config, use default.");
    return kDefaultImageGenerationTimeoutMs;
}

size_t loadImageGenerationQueueMaxSize() {
    const auto queueMaxSize = task::loadTaskQueueMaxSize(task::TaskType::TextToImage);
    if (queueMaxSize.has_value()) {
        return static_cast<size_t>(*queueMaxSize);
    }
    LOGW("Image generation request queue max size not found in task config, use default.");
    return kDefaultImageGenerationRequestQueueMaxSize;
}
}  // namespace

ImageGenerator::ImageGenerator() {}

ImageGenerator::~ImageGenerator() {
    if (visionEngine_ != nullptr) {
        visionEngine_->setGenerationResultCallback(nullptr);
    }
    LOGD("~ImageGenerator");
}

void ImageGenerator::init(int sessionId) {
    std::string uuid = serviceName + std::to_string(sessionId);
    ModelConfigManager::getInstance().updateAllDeployTypeCallback(
        AiCapability::Vision, uuid,
        [sharedObject = getSharedObject()](ModelChangeType) { sharedObject->defaultModelChanged_ = true; });
}

void ImageGenerator::generateImage(const genai::ImageGenerationRequest &request) {
    if (!tryEnqueueRequest(std::move(request))) {
        return;
    }
    generateImageAsync();
}

void ImageGenerator::generateImageWithoutConfig(genai::ImageGenerationRequest request) {
    request.width = currentConfig_.width;
    request.height = currentConfig_.height;
    request.number = currentConfig_.number;
    request.style = currentConfig_.style;
    request.modelName = currentConfig_.modelName;
    generateImage(request);
}

void ImageGenerator::stopImageGeneration() {
    if (visionEngine_) {
        visionEngine_->stopImageGeneration();
    }
    engineInUse_ = false;

    if (currentRequest_.finishedCallback != nullptr) {
        currentRequest_.finishedCallback();
    }
    genai::ImageGenerationRequest dropped{};
    while (requestQueue_.try_dequeue(dropped)) {
        decrementRequestQueueSize();
        notifyRequestStopped(dropped);
    }
}

void ImageGenerator::stopImageGeneration(const std::string &requestId) {
    std::fprintf(stderr, "ImageGenerator::stopImageGeneration(%s)\n", requestId.c_str());
    if (requestId.empty()) {
        stopImageGeneration();
        return;
    }

    if (currentRequest_.requestId == requestId) {
        if (visionEngine_) {
            visionEngine_->stopImageGeneration();
        }
        engineInUse_ = false;
        return;
    }

    genai::ImageGenerationRequest pending{};
    std::vector<genai::ImageGenerationRequest> keptRequests;
    while (requestQueue_.try_dequeue(pending)) {
        decrementRequestQueueSize();
        if (pending.requestId == requestId) {
            notifyRequestStopped(pending);
            continue;
        }
        keptRequests.push_back(std::move(pending));
    }
    for (auto &request : keptRequests) {
        requestQueue_.enqueue(std::move(request));
        requestQueueSize_.fetch_add(1, std::memory_order_relaxed);
    }
}

int ImageGenerator::getSupportedImageNumber() {
    if (visionEngine_ == nullptr) {
        return -1;
    }
    return visionEngine_->supportedImageNumber();
}

std::string ImageGenerator::getSupportedImageSize() {
    std::vector<std::pair<int, int>> sizes;
    if (visionEngine_ != nullptr) {
        sizes = visionEngine_->supportedImageSizes();
    }
    Json::Value value;
    for (const auto &[width, height] : sizes) {
        value.append(Json::Value(std::to_string(width) + "x" + std::to_string(height)));
    }
    return util::jsonToString(value);
}

std::string ImageGenerator::getSupportedImageStyle() {
    std::vector<std::string> styles;
    if (visionEngine_ != nullptr) {
        styles = visionEngine_->supportedImageStyles();
    }
    Json::Value value;
    for (const std::string &style : styles) {
        value.append(style);
    }
    return util::jsonToString(value);
}

void ImageGenerator::setImageConfig(const genai::ImageGenerationConfig &config) { currentConfig_ = config; }

void ImageGenerator::onVisionEngineResultCallback(const ai_engine::gen_ai::vision::ImageGenerationResult &imageData) {
    if (timedOut_.load()) {
        return;
    }
    genai::ImageGenerationResult result{};
    result.format = genai::getVisionImageFormat(imageData.format);
    result.height = imageData.height;
    result.width = imageData.width;
    result.total = imageData.total;
    result.index = imageData.index;
    result.imageBase64Data = imageData.imageBase64Data;
    result.errorCode = imageData.error.getCode();
    result.errorMessage = imageData.error.getMessage();
    result.requestId = currentRequest_.requestId;
    if (currentRequest_.resultCallback != nullptr) {
        currentRequest_.resultCallback(result);
    }
}

void ImageGenerator::onErrorOccurred(const ai_engine::EngineError &error) {
    if (timedOut_.load()) {
        return;
    }
    genai::ImageGenerationResult result{};
    result.format = int(genai::ImageFormat::IMAGE_FORMAT_UNKNOWN);
    result.width = 0;
    result.height = 0;
    result.total = 0;
    result.index = -1;
    result.errorCode = error.getCode();
    result.errorMessage = error.getMessage();
    result.requestId = currentRequest_.requestId;
    if (currentRequest_.resultCallback) {
        currentRequest_.resultCallback(result);
    }
}

int ImageGenerator::getImageNumberFromJsonString(const std::string &params) {
    Json::Value root;
    Json::Reader reader;

    bool parsingSuccessful = reader.parse(params, root);
    if (!parsingSuccessful) {
        LOGE("Failed to parse JSON: {}", reader.getFormattedErrorMessages());
        return 0;
    }

    if (!root.isMember("image-number")) {
        LOGE("Key 'image-number' not found in JSON");
        return 0;
    }
    int imageNumber = root["image-number"].asInt();
    return imageNumber;
}

void ImageGenerator::generateImageAsync() {
    bool expected = false;
    if (!taskRunning_.compare_exchange_strong(expected, true)) {
        return;
    }
    genai::ImageGenerationRequest nextRequest{};
    if (!requestQueue_.try_dequeue(nextRequest)) {
        taskRunning_.store(false);
        return;
    }
    decrementRequestQueueSize();
    task::TaskId taskId = task::generateTaskId();
    std::shared_ptr sharedObject = getSharedObject();
    auto enqueueResult = task::TaskRunner::getInstance().addTask(
        {taskId, task::TaskType::TextToImage, [sharedObject, nextRequest]() mutable {
             sharedObject->generateImageTask(std::move(nextRequest));
             sharedObject->taskRunning_.store(false);
             sharedObject->generateImageAsync();
         }});
    if (enqueueResult != task::TaskRunner::EnqueueResult::Accepted) {
        LOGE("Failed to enqueue TextToImage task.");
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

bool ImageGenerator::tryEnqueueRequest(genai::ImageGenerationRequest request) {
    size_t pending = requestQueueSize_.fetch_add(1, std::memory_order_relaxed);
    if (pending >= loadImageGenerationQueueMaxSize()) {
        requestQueueSize_.fetch_sub(1, std::memory_order_relaxed);
        LOGW("Image generation request queue is full, reject request.");
        notifyRequestFailed(request, AiSdkCommonErrorCode::AI_COMMON_TOO_MANY_REQUESTS,
                            "Too many requests, please try again later.");
        return false;
    }
    requestQueue_.enqueue(std::move(request));
    return true;
}

void ImageGenerator::decrementRequestQueueSize() {
    size_t current = requestQueueSize_.fetch_sub(1, std::memory_order_relaxed);
    if (current == 0) {
        requestQueueSize_.store(0, std::memory_order_relaxed);
    }
}

void ImageGenerator::generateImageTask(genai::ImageGenerationRequest request) {
    currentRequest_ = std::move(request);
    timedOut_.store(false);
    util::Watchdog watchdog;
    watchdog.Start(loadImageGenerationTimeoutMs(), [this]() {
        timedOut_.store(true);
        if (visionEngine_ != nullptr) {
            visionEngine_->stopImageGeneration();
        }
        notifyRequestFailed(currentRequest_, AISDK_MODEL_RUN_TIME_OUT, "Image generation timed out.");
    });
    bool success = ensureEngineReady(currentRequest_, watchdog);
    if (!success) {
        watchdog.Finish();
        return;
    }
    ModelStatusTracker statusTracker(visionEngine_->currentModelName());
    engineInUse_ = true;
    visionEngine_->setImageNumber(currentRequest_.number);
    visionEngine_->setImageSize(currentRequest_.width, currentRequest_.height);
    visionEngine_->setImageStyle(currentRequest_.style);
    auto result = visionEngine_->generateImage(currentRequest_.prompt);
    LOGI("ImageGenerator::generateImageTask {}", visionEngine_->currentModelName());
    engineInUse_ = false;
    if (!result.success) {
        LOGE("Generate image error info: {}, {}.", result.error.getCode(), result.error.getMessage());
        if (watchdog.TryFinish()) {
            onErrorOccurred(result.error);
            if (currentRequest_.finishedCallback) {
                currentRequest_.finishedCallback();
            }
        }
        watchdog.Finish();
        return;
    }
    if (watchdog.TryFinish()) {
        if (currentRequest_.finishedCallback) {
            currentRequest_.finishedCallback();
        }
    }
    watchdog.Finish();
}

void ImageGenerator::notifyRequestFailed(const genai::ImageGenerationRequest &request, int errorCode,
                                         const std::string &message) {
    genai::ImageGenerationResult result{};
    result.errorCode = errorCode;
    result.errorMessage = message;
    result.requestId = request.requestId;
    if (request.resultCallback) {
        request.resultCallback(result);
    }
    if (request.finishedCallback) {
        request.finishedCallback();
    }
}

void ImageGenerator::notifyRequestStopped(const genai::ImageGenerationRequest &request) {
    if (request.resultCallback) {
        genai::ImageGenerationResult result{};
        result.errorCode = AI_COMMON_TASK_CANCELLED;
        result.errorMessage = "Image generation request was stopped.";
        result.requestId = request.requestId;
        request.resultCallback(result);
    }
    if (request.finishedCallback) {
        request.finishedCallback();
    }
}

void ImageGenerator::rejectPendingRequests(int errorCode, const std::string &message) {
    genai::ImageGenerationRequest dropped{};
    while (requestQueue_.try_dequeue(dropped)) {
        decrementRequestQueueSize();
        notifyRequestFailed(dropped, errorCode, message);
    }
}

bool ImageGenerator::ensureEngineReady(const genai::ImageGenerationRequest &request, util::Watchdog &watchdog) {
    EngineSelector engineSelector(ai_engine::AiEnginePluginManager::getInstance(), ModelConfigManager::getInstance());
    std::shared_ptr<ImageGenerationEngine> engine{nullptr};
    bool defaultMode{true};
    if (request.modelName.empty()) {
        defaultMode = true;
    } else {
        defaultMode = false;
    }
    // 非默认模式或者默认模式时模型未切换过，这两种情况下使用缓存
    if (!defaultMode || !defaultModelChanged_) {
        if (auto cached = engineCache_.find(request.modelName); cached != engineCache_.end()) {
            engine = cached->second;
        }
    }
    if (engine == nullptr) {
        std::unique_ptr<ImageGenerationEngine> uniqueEngine{nullptr};
        if (request.modelName.empty()) {
            uniqueEngine = engineSelector.selectGenAiImageGenerationEngine();
        } else {
            uniqueEngine = engineSelector.selectGenAiImageGenerationEngine(request.modelName);
        }
        engine = std::move(uniqueEngine);
    } else {
        visionEngine_ = std::move(engine);
        return true;
    }
    if (engine == nullptr) {
        LOGE("No vision engine found!");
        if (watchdog.TryFinish()) {
            ai_engine::EngineError error(AI_COMMON_MODEL_NOT_FOUND, "No nlp engine found.");
            notifyRequestFailed(currentRequest_, error.getCode(), error.getMessage());
        }
        return false;
    }
    auto result = engine->init();
    if (!result.success) {
        LOGE("Failed to init nlp engine: {} {}", result.error.getCode(), result.error.getMessage());
        if (watchdog.TryFinish()) {
            notifyRequestFailed(currentRequest_, result.error.getCode(), result.error.getMessage());
        }
        return false;
    }
    std::shared_ptr sharedObject = getSharedObject();
    engine->setGenerationResultCallback(
        [sharedObject](const ai_engine::gen_ai::vision::ImageGenerationResult &imageData) {
            sharedObject->onVisionEngineResultCallback(imageData);
        });
    visionEngine_ = engine;
    engineCache_[request.modelName] = engine;
    if (defaultMode) {
        defaultModelChanged_ = false;
    }
    return true;
}
