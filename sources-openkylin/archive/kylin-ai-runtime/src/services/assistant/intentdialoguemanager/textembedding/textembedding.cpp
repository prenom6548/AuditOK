/*
 * Copyright 2026 KylinSoft Co., Ltd.
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

#include <condition_variable>

#include "assistant/error.h"
#include "utils/logger.h"

TextVectorizationInference::~TextVectorizationInference() {
    // 等待异步初始化任务完成
    if (embeddingEngineInitFuture_.valid()) {
        try {
            if (embeddingEngineInitFuture_.wait_for(std::chrono::seconds(5)) == std::future_status::ready) {
                embeddingEngineInitFuture_.get();
            }
        } catch (const std::exception& e) {
            LOGE("Exception during embedding engine cleanup: {}", e.what());
        }
    }
    LOGI("TextVectorizationInference::~TextVectorizationInference");
}

void TextVectorizationInference::initEmbeddingEngine(
    const std::shared_ptr<ai_engine::core_ai::embedding::AbstractEmbeddingEngine>& textEmbeddingEngine) {
    LOGI("Initializing embedding engine: start.");
    std::lock_guard<std::mutex> lock(embeddingEngineInitMutex_);

    // 检查是否已经在初始化中
    if (embeddingEngineInitFuture_.valid()) {
        LOGW("Embedding engine is already initializing, waiting for completion...");
        if (embeddingEngineInitFuture_.wait_for(std::chrono::seconds(60)) == std::future_status::ready) {
            try {
                embeddingEngineInitFuture_.get();
            } catch (const std::exception& e) {
                LOGE("Exception during previous embedding engine initialization: {}", e.what());
            }
        } else {
            LOGE("Previous embedding engine init timeout, proceeding with new initialization.");
        }
    }

    // 检查是否已经初始化完成且引擎相同
    if (isEmbeddingEngineInited() && textEmbeddingEngine_ == textEmbeddingEngine) {
        LOGI("Embedding engine is already initialized with the same engine.");
        return;
    }

    textEmbeddingEngine_ = textEmbeddingEngine;
    initState_ = InitState::Initializing;

    if (textEmbeddingEngine_ == nullptr) {
        LOGE("Embedding engine is nullptr, cannot initialize.");
        initState_ = InitState::Failed;
        return;
    }

    embeddingEngineInitFuture_ = std::async(std::launch::async, [this, textEmbeddingEngine]() {
        try {
            auto result = textEmbeddingEngine->init();
            textEmbeddingEngine->setCurrentTextEmbeddingModel("ensemble-embd_gte-base_uint8-text");

            if (result.success) {
                LOGI("Embedding engine init success.");
                initState_ = InitState::Initialized;
            } else {
                LOGE("Embedding engine init failed, error info: {} {}.", result.error.getCode(),
                     result.error.getMessage());
                initState_ = InitState::Failed;
            }
        } catch (const std::exception& e) {
            LOGE("Embedding engine init exception: {}", e.what());
            initState_ = InitState::Failed;
        } catch (...) {
            LOGE("Embedding engine init unknown exception");
            initState_ = InitState::Failed;
        }
    });
    LOGI("Initializing embedding engine: stop.");
}

void TextVectorizationInference::waitForEmbeddingEngineInited() {
    if (!embeddingEngineInitFuture_.valid()) {
        return;
    }

    if (embeddingEngineInitFuture_.wait_for(std::chrono::seconds(60)) == std::future_status::ready) {
        try {
            embeddingEngineInitFuture_.get();
        } catch (const std::exception& e) {
            LOGE("Exception during embedding engine initialization: {}", e.what());
            std::lock_guard<std::mutex> lock(embeddingEngineInitMutex_);
            initState_ = InitState::Failed;
        }
    } else {
        LOGE("Embedding engine init timeout.");
        std::lock_guard<std::mutex> lock(embeddingEngineInitMutex_);
        initState_ = InitState::Failed;
    }
}

DialogueCallback::TextEmbeddingResult TextVectorizationInference::textVectorization(const std::string& textStr) {
    LOGD("text {}", textStr);

    DialogueCallback::TextEmbeddingResult ret{};

    if (textStr.empty()) {
        ret.errorCode = int(assistant::ErrorCode::InputTextLengthInvalid);
        ret.errorMessage = "输入文本为空";
        ret.type = DialogueCallback::Type::EmbeddingErrorOccurred;
        return ret;
    }

    waitForEmbeddingEngineInited();

    if (initState_ == InitState::Failed) {
        ret.errorCode = int(assistant::ErrorCode::InitNotCompleted);
        ret.errorMessage = "文本向量化引擎初始化失败";
        ret.type = DialogueCallback::Type::EmbeddingErrorOccurred;
        return ret;
    }

    if (!isEmbeddingEngineInited()) {
        ret.errorCode = int(assistant::ErrorCode::InitNotCompleted);
        ret.errorMessage = "文本向量化引擎未初始化";
        ret.type = DialogueCallback::Type::EmbeddingErrorOccurred;
        return ret;
    }

    // 调用文本向量化接口
    bool done = false;
    std::vector<float> embeddingResult;
    std::mutex mtx;
    std::condition_variable cv;
    auto embeddingCallback = [&](const ai_engine::core_ai::embedding::EmbeddingResult& result) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (result.success) {
                embeddingResult = result.embeddings;
            }
            done = true;
        }

        cv.notify_one();  // 通知主线程结果已准备好
    };

    textEmbeddingEngine_->setEmbeddingResultCallback(embeddingCallback);
    textEmbeddingEngine_->embeddingText(textStr);

    // 等待结果
    {
        std::unique_lock<std::mutex> lock(mtx);
        using namespace std::chrono_literals;
        cv.wait_for(lock, 60s, [&done]() { return done; });
    }

    LOGI("Embedding vector size is: {}", embeddingResult.size());

    ret.embedding = embeddingResult;
    ret.type = DialogueCallback::Type::EmbeddingNormal;

    // 调用回调函数，传递结果
    callTextEmbeddingCallback(ret);
    return ret;
}