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

#ifndef SRC_SERVICES_CORE_AI_VISION_SEGMENTATION_SESSION_H
#define SRC_SERVICES_CORE_AI_VISION_SEGMENTATION_SESSION_H

#include <kylin-ai/aiengine.h>

#include "concurrentqueue/blockingconcurrentqueue.h"

class SegmentationSession : public std::enable_shared_from_this<SegmentationSession> {
public:
    using SegmentationResult = ai_engine::core_ai::vision::ImageSegmentationResult;

    using SegmentationResultCallback = ai_engine::core_ai::vision::SegmentationResultCallback;
    using SegmentationPoints = std::vector<ai_engine::core_ai::vision::SegmentationPoint>;

    struct SegmentationRequest {
        enum class Type {
            EncodeFromImagePath,
            EncodeFromImageData,
            RunSegmentation,
        };
        Type type{Type::EncodeFromImagePath};
        std::string imagePath;
        std::string imageData;
        SegmentationPoints points;
        uint32_t minArea;
        std::function<void(int errorCode)> onFinished{};

        void callOnFinished(int errorCode) const {
            if (onFinished) {
                onFinished(errorCode);
            }
        }
    };

private:
    using IImageProcessEngine = ai_engine::core_ai::vision::AbstractImageProcessEngine;

    using ModelResult = ai_engine::result::AiEngineBaseResult;

public:
    explicit SegmentationSession(std::unique_ptr<IImageProcessEngine> engine);

    ~SegmentationSession();

    ModelResult init();

    ModelResult destroy();

    void setSegmentationResultCallback(const SegmentationResultCallback& callback);

    void submitSegmentationRequest(const SegmentationRequest& request);

    int clearCache() const;

private:
    std::shared_ptr<SegmentationSession> getSharedObject() { return shared_from_this(); }

    void segmentationAsync();

    void segmentationTask(const SegmentationRequest& request);

    void notifyRequestFailed(int errorCode, const std::string& errorMessage) const;

    void rejectPendingRequests(int errorCode, const std::string& errorMessage);

    void executeCallback(const SegmentationResult& result) const;

private:
    std::unique_ptr<IImageProcessEngine> engine_{nullptr};
    moodycamel::BlockingConcurrentQueue<SegmentationRequest> requestsQueue_{};
    SegmentationResultCallback callback_{};

    mutable std::mutex mutex_;

    std::atomic_bool taskRunning_{false};
    std::atomic_bool timedOut_{false};
};

#endif  // SRC_SERVICES_CORE_AI_VISION_SEGMENTATION_SESSION_H
