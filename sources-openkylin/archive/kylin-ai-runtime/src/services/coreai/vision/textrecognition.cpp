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

#include "textrecognition.h"

#include <jsoncpp/json/json.h>
#include <kylin-ai/common/error.h>

#include <chrono>
#include <utility>

#include "taskmanager/taskrunner.h"
#include "utils/logger.h"
#include "vision.h"

namespace {
constexpr int64_t kDefaultTextRecognitionTimeoutMs = 5LL * 60 * 1000;
constexpr size_t kTextRecognitionRequestQueueMaxSize = 10;

int64_t loadTextRecognitionTimeoutMs() {
    const auto taskTimeout = task::loadTaskTimeoutMs(task::TaskType::TextRecognition);
    if (taskTimeout.has_value()) {
        return *taskTimeout;
    }
    LOGW("Text recognition task timeout not found in task config, use default.");
    return kDefaultTextRecognitionTimeoutMs;
}
}  // namespace

TextRecognition::TextRecognition(
    std::unique_ptr<ai_engine::core_ai::vision::AbstractTextRecognitionEngine> visionEngine)
    : visionEngine_(std::move(visionEngine)) {}

TextRecognition::~TextRecognition() {
    recognitionResultCallback_ = nullptr;
    visionEngine_->setTextRecognitionResultCallback(nullptr);
    // TODO 不析构，待查
    LOGD("~TextRecognition");
}

std::vector<TextLine> convertResult2Textlines(const ai_engine::core_ai::vision::TextRecognitionResult& result) {
    std::vector<TextLine> textLines;
    textLines.reserve(result.lines.size());
    for (const auto& line : result.lines) {
        std::vector<PixelPoint> points;
        points.reserve(line.points.size());
        for (const auto& point : line.points) {
            points.push_back({point.x, point.y});
        }
        textLines.push_back({line.text, points, line.conf});
    }
    return textLines;
}

void TextRecognition::recognizeTextFromImageFile(const std::string& imageFilePath,
                                                 RecognitionResultCallback resultCallback) {
    recognizeTextFromImageFile(imageFilePath, std::string{}, std::move(resultCallback));
}

void TextRecognition::recognizeTextFromImageFile(const std::string& imageFilePath, const std::string& requestId,
                                                 RecognitionResultCallback resultCallback) {
    if (resultCallback == nullptr) {
        return;
    }
    const auto& request =
        generateTextRecognitionRequestWithFilePath(imageFilePath, requestId, std::move(resultCallback));
    if (!tryEnqueueRequest(request)) {
        return;
    }
    textRecogniseAsync();
}

void TextRecognition::recognizeTextFromImageData(const std::string& imageData,
                                                 RecognitionResultCallback resultCallback) {
    recognizeTextFromImageData(imageData, std::string{}, std::move(resultCallback));
}

void TextRecognition::recognizeTextFromImageData(const std::string& imageData, const std::string& requestId,
                                                 RecognitionResultCallback resultCallback) {
    if (resultCallback == nullptr) {
        return;
    }
    const auto& request = generateTextRecognitionRequestWithBase64Data(imageData, requestId, std::move(resultCallback));
    if (!tryEnqueueRequest(request)) {
        return;
    }
    textRecogniseAsync();
}

TextRecognition::TextRecognitionRequest TextRecognition::generateTextRecognitionRequestWithFilePath(
    const std::string& imageFilePath, const std::string& requestId, RecognitionResultCallback resultCallback) {
    TextRecognitionRequest request{};
    request.resultCallback = std::move(resultCallback);
    request.type = TextRecognition::TextRecognitionRequest::Type::ImagePath;
    request.imagePath = imageFilePath;
    request.requestId = requestId;
    return request;
}

TextRecognition::TextRecognitionRequest TextRecognition::generateTextRecognitionRequestWithBase64Data(
    const std::string& base64Data, const std::string& requestId, RecognitionResultCallback resultCallback) {
    TextRecognitionRequest request{};
    request.resultCallback = std::move(resultCallback);
    request.type = TextRecognition::TextRecognitionRequest::Type::Base64Data;
    request.base64Data = base64Data;
    request.requestId = requestId;
    return request;
}

void TextRecognition::textRecogniseAsync() {
    bool expected = false;
    if (!taskRunning_.compare_exchange_strong(expected, true)) {
        return;
    }
    TextRecognitionRequest nextRequest{};
    if (!textRecognitionRequests_.try_dequeue(nextRequest)) {
        taskRunning_.store(false);
        return;
    }
    decrementRequestQueueSize();
    std::shared_ptr sharedObject = getSharedObject();
    auto enqueueResult = task::TaskRunner::getInstance().addTask(
        {task::generateTaskId(), task::TaskType::TextRecognition, [sharedObject, nextRequest]() mutable {
             sharedObject->textRecognizeTask(std::move(nextRequest));
             sharedObject->taskRunning_.store(false);
             sharedObject->textRecogniseAsync();
         }});
    if (enqueueResult != task::TaskRunner::EnqueueResult::Accepted) {
        LOGE("Failed to enqueue TextRecognition task.");
        taskRunning_.store(false);
        notifyRequestFailed(nextRequest, AiSdkCommonErrorCode::AI_COMMON_TOO_MANY_REQUESTS,
                            "Too many requests, please try again later.");
        rejectPendingRequests(AiSdkCommonErrorCode::AI_COMMON_TOO_MANY_REQUESTS,
                              "Too many requests, please try again later.");
    }
}

bool TextRecognition::tryEnqueueRequest(TextRecognitionRequest request) {
    size_t pending = requestQueueSize_.fetch_add(1, std::memory_order_relaxed);
    if (pending >= kTextRecognitionRequestQueueMaxSize) {
        requestQueueSize_.fetch_sub(1, std::memory_order_relaxed);
        LOGW("Text recognition request queue is full, reject request.");
        notifyRequestFailed(request, AiSdkCommonErrorCode::AI_COMMON_TOO_MANY_REQUESTS,
                            "Too many requests, please try again later.");
        return false;
    }
    textRecognitionRequests_.enqueue(std::move(request));
    return true;
}

void TextRecognition::decrementRequestQueueSize() {
    size_t current = requestQueueSize_.fetch_sub(1, std::memory_order_relaxed);
    if (current == 0) {
        requestQueueSize_.store(0, std::memory_order_relaxed);
    }
}

void TextRecognition::textRecognizeTask(TextRecognitionRequest request) {
    currentRequest_ = std::move(request);
    timedOut_.store(false);
    util::Watchdog watchdog;
    watchdog.Start(loadTextRecognitionTimeoutMs(), [this]() {
        timedOut_.store(true);
        notifyRequestFailed(currentRequest_, AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT,
                            "Text recognition timed out.");
    });
    ai_engine::result::AiEngineBaseResult result{};
    switch (currentRequest_.type) {
        case TextRecognitionRequest::Type::Base64Data: {
            result = visionEngine_->recognizeTextFromBase64ImageData(currentRequest_.base64Data);
            break;
        }
        case TextRecognitionRequest::Type::ImagePath: {
            result = visionEngine_->recognizeTextFromFile(currentRequest_.imagePath);
            break;
        }
        default: {
            LOGE("Unknown input type.");
            if (watchdog.TryFinish()) {
                notifyRequestFailed(currentRequest_, AiSdkCommonErrorCode::AISDK_BAD_REQUEST,
                                    "Unknown text recognition input type.");
            }
            watchdog.Finish();
            return;
        }
    }
    if (!result.success) {
        if (watchdog.TryFinish()) {
            TextRecognitionResult err = makeResultFromEngineError(result.error, currentRequest_.requestId);
            callTextRecognitionResultCallback(err);
        }
        watchdog.Finish();
        return;
    }
    watchdog.TryFinish();
    watchdog.Finish();
}

bool TextRecognition::init() {
    auto result = visionEngine_->init();

    if (!result.success) {
        LOGE("Failed to init vision engine: {} {}", result.error.getCode(), result.error.getMessage());
        callTextRecognitionResultCallback(makeResultFromEngineError(result.error, std::string{}));
        return false;
    }

    std::shared_ptr sharedObject = getSharedObject();
    if (sharedObject == nullptr) {
        return false;
    }
    visionEngine_->setTextRecognitionResultCallback(
        [sharedObject](const ai_engine::core_ai::vision::TextRecognitionResult& result) {
            if (sharedObject->timedOut_.load()) {
                return;
            }
            TextRecognitionResult recognitionResult{result.allText, convertResult2Textlines(result),
                                                    visionEngineError::getCoreVisionErrorCode(result.error.getCode()),
                                                    result.error.getMessage(), sharedObject->currentRequest_.requestId};
            sharedObject->callTextRecognitionResultCallback(recognitionResult);
        });

    return true;
}

void TextRecognition::callTextRecognitionResultCallback(const TextRecognitionResult& result) const {
    if (timedOut_.load()) {
        return;
    }
    if (currentRequest_.resultCallback != nullptr) {
        currentRequest_.resultCallback(result);
    }
}

void TextRecognition::notifyRequestFailed(const TextRecognitionRequest& request, int errorCode,
                                          const std::string& message) {
    TextRecognitionResult result{"", {}, errorCode, message, request.requestId};
    if (request.resultCallback) {
        request.resultCallback(result);
    }
}

void TextRecognition::rejectPendingRequests(int errorCode, const std::string& message) {
    TextRecognitionRequest dropped{};
    while (textRecognitionRequests_.try_dequeue(dropped)) {
        decrementRequestQueueSize();
        notifyRequestFailed(dropped, errorCode, message);
    }
}

TextRecognitionResult TextRecognition::makeResultFromEngineError(const ai_engine::EngineError& error,
                                                                 const std::string& requestId) const {
    return TextRecognitionResult{
        "", {}, visionEngineError::getCoreVisionErrorCode(error.getCode()), error.getMessage(), requestId};
}
