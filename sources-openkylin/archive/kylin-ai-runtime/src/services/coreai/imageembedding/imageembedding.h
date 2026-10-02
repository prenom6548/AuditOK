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

#ifndef IMAGEEMBEDDING_H
#define IMAGEEMBEDDING_H

#include <kylin-ai/aiengine.h>

#include "concurrentqueue/blockingconcurrentqueue.h"
#include "taskmanager/taskrunner.h"
#include "utils/watchdog.h"

struct ImageEmbeddingResult {
    std::vector<float> embeddings;
    int errorCode;
    std::string errorMessage;
};

class ImageEmbedding : public std::enable_shared_from_this<ImageEmbedding> {
public:
    using EmbeddingResultCallback = std::function<void(const ImageEmbeddingResult&)>;

    explicit ImageEmbedding(
        std::unique_ptr<ai_engine::core_ai::embedding::AbstractEmbeddingEngine> imageEmbeddingEngine);

    ~ImageEmbedding();

    bool init();

    bool setCurrentImageEmbeddingModel(std::string modelName);

    std::string getModelInfo() const;

    void setEmbeddingResultCallback(const EmbeddingResultCallback& callback);

    void embeddingText(const std::string& text, EmbeddingResultCallback resultCallback);

    void embeddingImageFromFile(const std::string& imageFilePath, EmbeddingResultCallback resultCallback);

    void embeddingImageFromBase64Data(const std::string& base64Data, EmbeddingResultCallback resultCallback);

private:
    struct ImageEmbeddingRequest {
        enum class Type { Unknown, Text, ImagePath, Base64Data };
        Type type{Type::Unknown};
        std::string imageFilePath;
        std::string base64ImageData;
        std::string text;
        EmbeddingResultCallback resultCallback{nullptr};
    };
    ImageEmbeddingRequest generateImageEmbeddingRequestWithFilePath(const std::string& imageFilePath,
                                                                    EmbeddingResultCallback resultCallback);
    ImageEmbeddingRequest generateImageEmbeddingRequestWithBase64Data(const std::string& base64Data,
                                                                      EmbeddingResultCallback resultCallback);
    ImageEmbeddingRequest generateImageEmbeddingRequestWithText(const std::string& text,
                                                                EmbeddingResultCallback resultCallback);

private:
    std::shared_ptr<ImageEmbedding> getSharedObject() { return shared_from_this(); }

    void callEmbeddingResultCallback(const ImageEmbeddingResult& result) const;

    void imageEmbeddingAsync();

    void imageEmbeddingTask(ImageEmbeddingRequest request);

    void notifyRequestFailed(const ImageEmbeddingRequest& request, int errorCode, const std::string& message);
    void rejectPendingRequests(int errorCode, const std::string& message);

private:
    std::unique_ptr<ai_engine::core_ai::embedding::AbstractEmbeddingEngine> imageEmbeddingEngine_{nullptr};
    EmbeddingResultCallback imageEmbeddingResultCallback_{nullptr};
    moodycamel::BlockingConcurrentQueue<ImageEmbeddingRequest> imageEmbeddingRequests_{};
    ImageEmbeddingRequest currentRequest_{};
    std::atomic_bool taskRunning_{false};
    std::atomic_bool timedOut_{false};
};

#endif
