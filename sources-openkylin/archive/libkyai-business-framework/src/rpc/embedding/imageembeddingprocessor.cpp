/*
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <rpc/embedding/imageembeddingprocessor.h>

#include "imageembeddingproxy.h"
#include "log.h"

using namespace scene;

ImageEmbeddingProcessor::~ImageEmbeddingProcessor() {
    if (!proxy_) return;

    delete proxy_;
    proxy_ = nullptr;
}

bool ImageEmbeddingProcessor::initializeResources() {
    if (proxy_) return true;

    proxy_ = new ImageEmbeddingProxy();
    if (!proxy_->init()) {
        delete proxy_;
        proxy_ = nullptr;
        return false;
    }
    return true;
}

std::vector<float> ImageEmbeddingProcessor::embedding(const EmbeddingTask &task) {
    std::vector<float> vector;
    if (task.data.empty()) {
        PRINT_ERROR("ImageEmbeddingProcessor input task data is empty.\n");
        return vector;
    }

    switch (task.dataType) {
        case EmbeddingDataType::ImageBase64:
            vector = proxy_->embeddingImage(task.data);
            break;
        case EmbeddingDataType::ImageFlie:
            vector = proxy_->embeddingImageFile(task.data);
            break;
        case EmbeddingDataType::Text:
            vector = proxy_->embeddingText(task.data);
        default:
            break;
    }

    return vector;
}

std::string ImageEmbeddingProcessor::modelInfo() {
    if (!proxy_) initializeResources();
    return proxy_->modelInfo();
}
