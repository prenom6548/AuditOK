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

#ifndef VISION_IMAGE_GENERATOR_H
#define VISION_IMAGE_GENERATOR_H

#include <kylin-ai/aiengine.h>

#include <atomic>
#include <cstddef>
#include <memory>
#include <string>

#include "concurrentqueue/blockingconcurrentqueue.h"
#include "imagegenerationrequest.h"
#include "taskmanager/taskrunner.h"
#include "utils/watchdog.h"
#include "vision.h"

class ImageGenerator : public std::enable_shared_from_this<ImageGenerator> {
private:
    using ImageGenerationEngine = ai_engine::gen_ai::vision::AbstractImageGenerationEngine;

public:
    ImageGenerator();

    ~ImageGenerator();

    void init(int sessionId);

    void generateImage(const genai::ImageGenerationRequest &request);

    void generateImageWithoutConfig(genai::ImageGenerationRequest request);

    void stopImageGeneration();

    void stopImageGeneration(const std::string &requestId);

    int getSupportedImageNumber();
    std::string getSupportedImageSize();
    std::string getSupportedImageStyle();

    void setImageConfig(const genai::ImageGenerationConfig &config);

    bool engineInUse() { return engineInUse_; }

private:
    std::shared_ptr<ImageGenerator> getSharedObject() { return shared_from_this(); }

    void onVisionEngineResultCallback(const ai_engine::gen_ai::vision::ImageGenerationResult &imageData);

    void onErrorOccurred(const ai_engine::EngineError &error);

    int getImageNumberFromJsonString(const std::string &params);

    void generateImageAsync();

    void generateImageTask(genai::ImageGenerationRequest request);

    bool tryEnqueueRequest(genai::ImageGenerationRequest request);
    void decrementRequestQueueSize();

    void notifyRequestFailed(const genai::ImageGenerationRequest &request, int errorCode, const std::string &message);
    void notifyRequestStopped(const genai::ImageGenerationRequest &request);
    void rejectPendingRequests(int errorCode, const std::string &message);

    bool ensureEngineReady(const genai::ImageGenerationRequest &request, util::Watchdog &watchdog);

private:
    std::shared_ptr<ImageGenerationEngine> visionEngine_{nullptr};
    std::map<std::string, std::shared_ptr<ImageGenerationEngine> > engineCache_;

    std::atomic_bool engineInUse_{false};
    std::atomic_bool taskRunning_{false};
    std::atomic_bool timedOut_{false};
    std::atomic<size_t> requestQueueSize_{0};
    moodycamel::BlockingConcurrentQueue<std::string> promptQueue_{};
    moodycamel::BlockingConcurrentQueue<genai::ImageGenerationRequest> requestQueue_{};
    genai::ImageGenerationRequest currentRequest_;
    std::atomic_bool defaultModelChanged_{false};

    genai::ImageGenerationConfig currentConfig_{};
};

#endif