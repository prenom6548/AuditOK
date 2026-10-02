/*
 * Copyright 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not, see
 * <https://www.gnu.org/licenses/>.
 */

#include "segmentationsession.h"

#include <kylin-ai/common/error.h>

#include "taskmanager/taskrunner.h"
#include "utils/logger.h"
#include "utils/watchdog.h"

namespace {
constexpr int64_t kDefaultSegmentationTimeoutMs = 60LL * 60 * 1000;

int64_t loadSegmentationTaskTimeoutMs() {
    const auto taskTimeout = task::loadTaskTimeoutMs(task::TaskType::Segmentation);
    if (taskTimeout.has_value()) {
        return taskTimeout.value();
    }
    LOGW("Segmentation task timeout not found in task config, use default.");
    return kDefaultSegmentationTimeoutMs;
}
}  // namespace

SegmentationSession::SegmentationSession(std::unique_ptr<IImageProcessEngine> engine) : engine_{std::move(engine)} {}

SegmentationSession::~SegmentationSession() {
    destroy();
    LOGI("SegmentationSession::~SegmentationSession()");
}

SegmentationSession::ModelResult SegmentationSession::init() {
    if (engine_ == nullptr) {
        ModelResult result;
        result.success = false;
        result.error = {AI_COMMON_RUNTIME_ERROR, "SegmentationSession engine is null"};
        LOGE("SegmentationSession::init");
        return result;
    }
    std::shared_ptr sharedObject = getSharedObject();
    engine_->setSegmentationResultCallback(
        [sharedObject](const SegmentationResult& result) { sharedObject->executeCallback(result); });
    LOGI("initSegmentationModule");
    return engine_->initSegmentationModule();
}

SegmentationSession::ModelResult SegmentationSession::destroy() {
    if (engine_ == nullptr) {
        ModelResult result;
        result.success = false;
        result.error = {AI_COMMON_RUNTIME_ERROR, "SegmentationSession engine is null"};
        return result;
    }
    {
        std::lock_guard locker(mutex_);
        callback_ = nullptr;
    }
    engine_->setSegmentationResultCallback(nullptr);
    LOGI("destroySegmentationModule");
    return engine_->destroySegmentationModule();
}

void SegmentationSession::setSegmentationResultCallback(const SegmentationResultCallback& callback) {
    std::lock_guard locker(mutex_);
    callback_ = callback;
}

void SegmentationSession::submitSegmentationRequest(const SegmentationRequest& request) {
    requestsQueue_.enqueue(request);
    segmentationAsync();
}

int SegmentationSession::clearCache() const {
    if (engine_ == nullptr) {
        return AI_COMMON_RUNTIME_ERROR;
    }
    engine_->clearSegmentationCache();
    return AI_COMMON_SUCCESS;
}

void SegmentationSession::segmentationAsync() {
    if (taskRunning_) {
        return;
    }
    taskRunning_ = true;
    SegmentationRequest request{};
    if (!requestsQueue_.try_dequeue(request)) {
        taskRunning_ = false;
        return;
    }
    std::shared_ptr sharedObject = getSharedObject();
    task::TaskRunner::EnqueueResult enqueueResult = task::TaskRunner::getInstance().addTask(
        {task::generateTaskId(), task::TaskType::Segmentation, [sharedObject, request]() mutable {
             sharedObject->segmentationTask(request);
             sharedObject->taskRunning_ = false;
             sharedObject->segmentationAsync();
         }});
    if (enqueueResult != task::TaskRunner::EnqueueResult::Accepted) {
        LOGE("Failed to enqueue segmentation task.");
        taskRunning_ = false;
        if (enqueueResult == task::TaskRunner::EnqueueResult::RejectedQueueFull) {
            notifyRequestFailed(AI_COMMON_TOO_MANY_REQUESTS, "Too many requests, please try again later.");
            rejectPendingRequests(AI_COMMON_TOO_MANY_REQUESTS, "Too many requests, please try again later.");
            request.callOnFinished(AI_COMMON_TOO_MANY_REQUESTS);
        } else {
            notifyRequestFailed(AI_COMMON_SERVICE_ERROR, "Task runner is stopping.");
            rejectPendingRequests(AI_COMMON_SERVICE_ERROR, "Task runner is stopping.");
            request.callOnFinished(AI_COMMON_SERVICE_ERROR);
        }
    }
}

void SegmentationSession::segmentationTask(const SegmentationRequest& request) {
    timedOut_ = false;
    util::Watchdog watchdog;
    watchdog.Start(loadSegmentationTaskTimeoutMs(), [this]() {
        timedOut_ = false;
        notifyRequestFailed(AI_COMMON_MODEL_RUN_TIME_OUT, "Speech recognition timed out.");
    });
    ModelResult result{};
    switch (request.type) {
        case SegmentationRequest::Type::EncodeFromImagePath:
            result = engine_->encodeSegmentationFromImageFile(request.imagePath);
            request.callOnFinished(result.error.getCode());
            break;
        case SegmentationRequest::Type::EncodeFromImageData:
            result = engine_->encodeSegmentationFromBase64ImageData(request.imageData);
            request.callOnFinished(result.error.getCode());
            break;
        case SegmentationRequest::Type::RunSegmentation:
            result = engine_->decodeSegmentation(request.points, request.minArea);
            break;
        default:
            break;
    }
    if (!result.success) {
        if (watchdog.TryFinish()) {
            SegmentationResult segmentationResult{};
            segmentationResult.success = false;
            segmentationResult.error = result.error;
            executeCallback(segmentationResult);
        }
        watchdog.Finish();
        return;
    }
    watchdog.TryFinish();
    watchdog.Finish();
}

void SegmentationSession::notifyRequestFailed(int errorCode, const std::string& errorMessage) const {
    SegmentationResult result{};
    result.success = false;
    result.error = ai_engine::EngineError{errorCode, errorMessage};
    executeCallback(result);
}

void SegmentationSession::rejectPendingRequests(int errorCode, const std::string& errorMessage) {
    SegmentationRequest dropped{};
    while (requestsQueue_.try_dequeue(dropped)) {
        notifyRequestFailed(errorCode, errorMessage);
    }
}

void SegmentationSession::executeCallback(const SegmentationResult& result) const {
    std::lock_guard locker(mutex_);
    if (callback_) {
        callback_(result);
    }
}
