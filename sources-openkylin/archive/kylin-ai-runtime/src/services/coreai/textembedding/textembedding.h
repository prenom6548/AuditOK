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

#ifndef TEXTEMBEDDING_H
#define TEXTEMBEDDING_H

#include <kylin-ai/aiengine.h>

#include "concurrentqueue/blockingconcurrentqueue.h"
#include "taskmanager/taskrunner.h"
#include "utils/watchdog.h"

struct TextEmbeddingResult {
    std::vector<float> embeddings;
    int errorCode;
    std::string errorMessage;
};

class TextEmbedding : public std::enable_shared_from_this<TextEmbedding> {
public:
    using EmbeddingResultCallback = std::function<void(const TextEmbeddingResult&)>;

    explicit TextEmbedding(std::unique_ptr<ai_engine::core_ai::embedding::AbstractEmbeddingEngine> textEmbeddingEngine);

    ~TextEmbedding();

    bool init();

    std::string getModelInfo() const;

    bool setCurrentTextEmbeddingModel(std::string modelName);

    void setEmbeddingResultCallback(const EmbeddingResultCallback& callback);

    void embeddingText(const std::string& text, EmbeddingResultCallback resultCallback);

private:
    struct TextEmbeddingRequest {
        enum class Type { Unknown, Text };
        Type type{Type::Unknown};
        std::string text;
        EmbeddingResultCallback resultCallback{nullptr};
    };

    TextEmbeddingRequest generateTextEmbeddingRequestWithText(const std::string& text,
                                                              EmbeddingResultCallback resultCallback);

private:
    std::shared_ptr<TextEmbedding> getSharedObject() { return shared_from_this(); }

    void callEmbeddingResultCallback(const TextEmbeddingResult& result) const;

    void textEmbeddingAsync();

    void textEmbeddingTask(TextEmbeddingRequest request);

    void notifyRequestFailed(const TextEmbeddingRequest& request, int errorCode, const std::string& message);
    void rejectPendingRequests(int errorCode, const std::string& message);

private:
    std::unique_ptr<ai_engine::core_ai::embedding::AbstractEmbeddingEngine> textEmbeddingEngine_{nullptr};
    EmbeddingResultCallback textEmbeddingResultCallback_{nullptr};
    moodycamel::BlockingConcurrentQueue<TextEmbeddingRequest> textEmbeddingRequests_{};
    TextEmbeddingRequest currentRequest_{};
    std::atomic_bool taskRunning_{false};
    std::atomic_bool timedOut_{false};
};

#endif
