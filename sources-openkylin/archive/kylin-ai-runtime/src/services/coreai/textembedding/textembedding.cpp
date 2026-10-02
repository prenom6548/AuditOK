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

#include "textembedding.h"

#include <kylin-ai/common/error.h>

#include "utils/logger.h"

namespace {
constexpr int64_t kDefaultTextEmbeddingTimeoutMs = 5LL * 60 * 1000;

int64_t loadTextEmbeddingTimeoutMs() {
    const auto taskTimeout = task::loadTaskTimeoutMs(task::TaskType::TextEmbedding);
    if (taskTimeout.has_value()) {
        return *taskTimeout;
    }
    LOGW("Text Embedding task timeout not found in task config, use default.");
    return kDefaultTextEmbeddingTimeoutMs;
}
}  // namespace

TextEmbedding::TextEmbedding(
    std::unique_ptr<ai_engine::core_ai::embedding::AbstractEmbeddingEngine> textEmbeddingEngine)
    : textEmbeddingEngine_(std::move(textEmbeddingEngine)) {}

TextEmbedding::~TextEmbedding() {
    textEmbeddingResultCallback_ = nullptr;
    textEmbeddingEngine_->setEmbeddingResultCallback(nullptr);
    LOGD("~TextEmbedding");
}

void TextEmbedding::setEmbeddingResultCallback(const EmbeddingResultCallback& callback) {
    if (callback == nullptr) {
        return;
    }
    textEmbeddingResultCallback_ = callback;
}

std::string TextEmbedding::getModelInfo() const { return textEmbeddingEngine_->modelInfo(); }

bool TextEmbedding::setCurrentTextEmbeddingModel(std::string modelName) {
    auto result = textEmbeddingEngine_->setCurrentTextEmbeddingModel(std::move(modelName));
    if (!result.success) {
        LOGE("Failed to set current embedding model: {} {}", result.error.getCode(), result.error.getMessage());
    }
    return result.success;
}

void TextEmbedding::embeddingText(const std::string& text, EmbeddingResultCallback resultCallback) {
    if (resultCallback == nullptr) {
        return;
    }
    const auto& request = generateTextEmbeddingRequestWithText(text, std::move(resultCallback));
    textEmbeddingRequests_.enqueue(request);
    textEmbeddingAsync();
}

TextEmbedding::TextEmbeddingRequest TextEmbedding::generateTextEmbeddingRequestWithText(
    const std::string& text, EmbeddingResultCallback resultCallback) {
    TextEmbeddingRequest request;
    request.text = text;
    request.resultCallback = std::move(resultCallback);
    request.type = TextEmbeddingRequest::Type::Text;
    return request;
}

void TextEmbedding::textEmbeddingAsync() {
    bool expected = false;
    if (!taskRunning_.compare_exchange_strong(expected, true)) {
        return;
    }
    TextEmbeddingRequest nextRequest{};
    if (!textEmbeddingRequests_.try_dequeue(nextRequest)) {
        taskRunning_ = false;
        return;
    }
    std::shared_ptr shareObject = getSharedObject();
    auto enqueueResult = task::TaskRunner::getInstance().addTask(
        {task::generateTaskId(), task::TaskType::TextEmbedding, [shareObject, nextRequest]() {
             shareObject->textEmbeddingTask(std::move(nextRequest));
             shareObject->taskRunning_.store(false);
             shareObject->textEmbeddingAsync();
         }});
    if (enqueueResult != task::TaskRunner::EnqueueResult::Accepted) {
        LOGE("Failed to enqueue TextEmbedding task.");
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

void TextEmbedding::textEmbeddingTask(TextEmbeddingRequest request) {
    currentRequest_ = request;
    timedOut_.store(false);
    util::Watchdog watchdog;
    watchdog.Start(loadTextEmbeddingTimeoutMs(), [this]() {
        timedOut_.store(true);
        notifyRequestFailed(currentRequest_, AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT,
                            "Text embedding timed out.");
        textEmbeddingEngine_->stopTextEmbedding();
    });
    ai_engine::result::AiEngineBaseResult result{};
    switch (currentRequest_.type) {
        case TextEmbeddingRequest::Type::Text: {
            result = textEmbeddingEngine_->embeddingText(currentRequest_.text);
            break;
        }
        default:
            LOGE("Invalid TextEmbeddingRequest type.");
            if (watchdog.TryFinish()) {
                notifyRequestFailed(currentRequest_, AiSdkCommonErrorCode::AISDK_BAD_REQUEST,
                                    "Unknown text embedding input type.");
            }
            watchdog.Finish();
            return;
    }
    if (!result.success) {
        LOGE("Failed to text embedding: {} {}", result.error.getCode(), result.error.getMessage());
        if (watchdog.TryFinish()) {
            callEmbeddingResultCallback(TextEmbeddingResult{{}, result.error.getCode(), result.error.getMessage()});
        }
        watchdog.Finish();
        return;
    }
    watchdog.TryFinish();
    watchdog.Finish();
}

bool TextEmbedding::init() {
    auto result = textEmbeddingEngine_->init();

    if (!result.success) {
        LOGE("Failed to init embedding engine: {} {}", result.error.getCode(), result.error.getMessage());
        TextEmbeddingResult embeddingResult{{}, result.error.getCode(), result.error.getMessage()};
        callEmbeddingResultCallback(embeddingResult);
        return false;
    }
    std::shared_ptr sharedObject = getSharedObject();
    if (sharedObject == nullptr) {
        return false;
    }

    textEmbeddingEngine_->setEmbeddingResultCallback(
        [sharedObject](const ai_engine::core_ai::embedding::EmbeddingResult& result) {
            if (sharedObject->timedOut_.load()) {
                return;
            }
            TextEmbeddingResult embeddingResult{result.embeddings, result.error.getCode(), result.error.getMessage()};
            sharedObject->callEmbeddingResultCallback(embeddingResult);
        });

    return true;
}

void TextEmbedding::callEmbeddingResultCallback(const TextEmbeddingResult& result) const {
    if (timedOut_.load()) {
        return;
    }
    if (currentRequest_.resultCallback != nullptr) {
        currentRequest_.resultCallback(result);
    }
}

void TextEmbedding::notifyRequestFailed(const TextEmbeddingRequest& request, int errorCode,
                                        const std::string& message) {
    TextEmbeddingResult result{};
    result.errorCode = errorCode;
    result.errorMessage = message;
    if (request.resultCallback) {
        request.resultCallback(result);
    }
}

void TextEmbedding::rejectPendingRequests(int errorCode, const std::string& message) {
    TextEmbeddingRequest dropped{};
    while (textEmbeddingRequests_.try_dequeue(dropped)) {
        notifyRequestFailed(dropped, errorCode, message);
    }
}
