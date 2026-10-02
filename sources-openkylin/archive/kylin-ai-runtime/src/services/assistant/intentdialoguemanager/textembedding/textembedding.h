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

#pragma once

#include <kylin-ai/ai-engine/embeddingengine.h>

#include "textembeddinginterface.h"

#include <future>
#include <mutex>

class TextVectorizationInference : public TextEmbeddingInterface {
public:
    TextVectorizationInference() = default;
    ~TextVectorizationInference() override;

    void initEmbeddingEngine(
        const std::shared_ptr<ai_engine::core_ai::embedding::AbstractEmbeddingEngine>& textEmbeddingEngine);
    // 实现文本向量化调用
    DialogueCallback::TextEmbeddingResult textVectorization(const std::string& textStr);

private:
    void waitForEmbeddingEngineInited();

    bool isEmbeddingEngineInited() const {
        return initState_ == InitState::Initialized;
    }

    // 初始化状态
    enum class InitState {
        Uninitialized,  // 未初始化
        Initializing,   // 初始化中
        Initialized,    // 初始化完成
        Failed          // 初始化失败
    };

protected:
    std::shared_ptr<ai_engine::core_ai::embedding::AbstractEmbeddingEngine> textEmbeddingEngine_{nullptr};
    std::atomic<InitState> initState_{InitState::Uninitialized};
    std::future<void> embeddingEngineInitFuture_;
    std::mutex embeddingEngineInitMutex_;
};