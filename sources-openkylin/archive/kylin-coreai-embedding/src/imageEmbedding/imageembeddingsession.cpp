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

#include "imageembeddingsession.h"

#include <memory>

_ImageEmbeddingSession::_ImageEmbeddingSession()
    : imageEmbeddingProcessorProxy_(
          std::make_shared<
              kyai::coreai::embedding::ImageEmbeddingProcessorProxy>()) {}

_ImageEmbeddingSession::~_ImageEmbeddingSession() { stopEventLoop(); }

int _ImageEmbeddingSession::connect() {
    return imageEmbeddingProcessorProxy_->connect();
}

int _ImageEmbeddingSession::init() {
    return imageEmbeddingProcessorProxy_->initEngine();
}
int _ImageEmbeddingSession::initModel(const std::string &model) {
    return imageEmbeddingProcessorProxy_->initModel(model);
}

EmbeddingResult *_ImageEmbeddingSession::embeddingText(
    const std::string &text) {
    return imageEmbeddingProcessorProxy_->embeddingText(text);
}

EmbeddingResult *_ImageEmbeddingSession::embeddingImage(
    const std::string &filePath) {
    return imageEmbeddingProcessorProxy_->embeddingImage(filePath);
}

EmbeddingResult *_ImageEmbeddingSession::embeddingBase64Image(
    const std::string &fileData) {
    return imageEmbeddingProcessorProxy_->embeddingBase64Image(fileData);
}
void _ImageEmbeddingSession::embeddingTextAsync(
    const std::string &text, ImageEmbeddingResultCallback callback,
    void *userData) {
    imageEmbeddingProcessorProxy_->embeddingTextAsync(text, callback, userData);
}
void _ImageEmbeddingSession::embeddingImageAsync(
    const std::string &filePath, ImageEmbeddingResultCallback callback,
    void *userData) {
    imageEmbeddingProcessorProxy_->embeddingImageAsync(filePath, callback,
                                                       userData);
}

void _ImageEmbeddingSession::embeddingBase64ImageAsync(
    const std::string &fileData, ImageEmbeddingResultCallback callback,
    void *userData) {
    imageEmbeddingProcessorProxy_->embeddingBase64ImageAsync(fileData, callback,
                                                             userData);
}

_EmbeddingModelList *_ImageEmbeddingSession::getModelList(int *errorCode) {
    return imageEmbeddingProcessorProxy_->getModelList(errorCode);
}

std::string _ImageEmbeddingSession::getModelInfo() {
    return imageEmbeddingProcessorProxy_->getModelInfo();
}

void _ImageEmbeddingSession::enableInternalEventLoop(bool enable) {
    if (enable) {
        startEventLoop();
        return;
    }

    stopEventLoop();
}

void _ImageEmbeddingSession::startEventLoop() {
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

void _ImageEmbeddingSession::stopEventLoop() {
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