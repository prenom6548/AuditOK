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

#ifndef VISION_TEXTRECOGNITION_H
#define VISION_TEXTRECOGNITION_H

#include <kylin-ai/aiengine.h>

#include <atomic>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>

#include "concurrentqueue/blockingconcurrentqueue.h"
#include "coreaivisionmodelconfig.h"
#include "corevisionserviceglue.h"
#include "taskmanager/taskrunner.h"
#include "utils/watchdog.h"

struct PixelPoint {
    int x;
    int y;
};

struct TextLine {
    std::string text;
    std::vector<PixelPoint> points;
    float conf;
};

struct TextRecognitionResult {
    std::string allText;
    std::vector<TextLine> lines;
    int errorCode;
    std::string errorMessage;
    std::string requestId;
};

class TextRecognition : public std::enable_shared_from_this<TextRecognition> {
public:
    using RecognitionResultCallback = std::function<void(const TextRecognitionResult&)>;

    explicit TextRecognition(std::unique_ptr<ai_engine::core_ai::vision::AbstractTextRecognitionEngine> visionEngine);

    ~TextRecognition();

    bool init();

    void recognizeTextFromImageFile(const std::string& imageFilePath, RecognitionResultCallback resultCallback);

    void recognizeTextFromImageFile(const std::string& imageFilePath, const std::string& requestId,
                                    RecognitionResultCallback resultCallback);

    void recognizeTextFromImageData(const std::string& imageData, RecognitionResultCallback resultCallback);

    void recognizeTextFromImageData(const std::string& imageData, const std::string& requestId,
                                    RecognitionResultCallback resultCallback);

private:
    struct TextRecognitionRequest {
        enum Type { Unknown, ImagePath, Base64Data };
        Type type{Unknown};
        std::filesystem::path imagePath{};
        std::string base64Data{};
        std::string requestId{};
        RecognitionResultCallback resultCallback{nullptr};
    };

    TextRecognitionRequest generateTextRecognitionRequestWithFilePath(const std::string& imageFilePath,
                                                                      const std::string& requestId,
                                                                      RecognitionResultCallback resultCallback);

    TextRecognitionRequest generateTextRecognitionRequestWithBase64Data(const std::string& base64Data,
                                                                        const std::string& requestId,
                                                                        RecognitionResultCallback resultCallback);

private:
    std::shared_ptr<TextRecognition> getSharedObject() { return shared_from_this(); }

    TextRecognitionResult makeResultFromEngineError(const ai_engine::EngineError& error,
                                                    const std::string& requestId) const;

    void callTextRecognitionResultCallback(const TextRecognitionResult& result) const;

    void textRecogniseAsync();

    void textRecognizeTask(TextRecognitionRequest request);

    bool tryEnqueueRequest(TextRecognitionRequest request);
    void decrementRequestQueueSize();

    void notifyRequestFailed(const TextRecognitionRequest& request, int errorCode, const std::string& message);
    void rejectPendingRequests(int errorCode, const std::string& message);

private:
    std::unique_ptr<ai_engine::core_ai::vision::AbstractTextRecognitionEngine> visionEngine_{nullptr};
    RecognitionResultCallback recognitionResultCallback_{nullptr};
    moodycamel::BlockingConcurrentQueue<TextRecognitionRequest> textRecognitionRequests_{};
    TextRecognitionRequest currentRequest_{};
    std::atomic_bool taskRunning_{false};
    std::atomic_bool timedOut_{false};
    std::atomic<size_t> requestQueueSize_{0};
};

#endif
