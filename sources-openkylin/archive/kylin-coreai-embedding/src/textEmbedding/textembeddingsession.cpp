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

#include "textembeddingsession.h"

#include <memory>

#include "error.h"

_TextEmbeddingSession::_TextEmbeddingSession()
    : textEmbeddingProcessorProxy_(
          std::make_shared<
              kyai::coreai::embedding::TextEmbeddingProcessorProxy>()) {}

_TextEmbeddingSession::~_TextEmbeddingSession() { stopEventLoop(); }

int _TextEmbeddingSession::connect() {
    return textEmbeddingProcessorProxy_->connect();
}

int _TextEmbeddingSession::init() {
    return textEmbeddingProcessorProxy_->initEngine();
}

int _TextEmbeddingSession::initModel(const std::string &model) {
    return textEmbeddingProcessorProxy_->initModel(model);
}

EmbeddingResult *_TextEmbeddingSession::embeddingText(const std::string &text) {
    return textEmbeddingProcessorProxy_->embeddingText(text);
}

void _TextEmbeddingSession::embeddingTextAsync(
    const std::string &text, TextEmbeddingResultCallback callback,
    void *userData) {
    textEmbeddingProcessorProxy_->embeddingTextAsync(text, callback, userData);
}

_EmbeddingModelList *_TextEmbeddingSession::getModelList(int *errorCode) {
    return textEmbeddingProcessorProxy_->getModelList(errorCode);
}

std::string _TextEmbeddingSession::getModelInfo() {
    return textEmbeddingProcessorProxy_->getModelInfo();
}

void _TextEmbeddingSession::enableInternalEventLoop(bool enable) {
    if (enable) {
        startEventLoop();
        return;
    }

    stopEventLoop();
}

void _TextEmbeddingSession::startEventLoop() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (loop_) {
        // 已开启内部事件循环
        return;
    }

    thread_ = std::thread([this]() {
        loop_ = g_main_loop_new(nullptr, false);
        g_main_loop_run(loop_);
    });
}

void _TextEmbeddingSession::stopEventLoop() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loop_) {
        // loop 为空，说明没有开启内部事件循环，不需要销毁
        return;
    }

    g_main_loop_quit(loop_);
    if (thread_.joinable()) {
        thread_.join();
    }

    g_main_loop_unref(loop_);
    loop_ = nullptr;
}