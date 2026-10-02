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

#include <rpc/embedding/textembeddingprocessor.h>

#include "log.h"
#include "textembeddingproxy.h"

using namespace scene;

TextEmbeddingProcessor::~TextEmbeddingProcessor() {
    if (!proxy_) return;

    delete proxy_;
    proxy_ = nullptr;
}

bool TextEmbeddingProcessor::initializeResources() {
    if (proxy_) return true;

    proxy_ = new TextEmbeddingProxy();
    if (!proxy_->init()) {
        delete proxy_;
        proxy_ = nullptr;
        return false;
    }
    return true;
}

std::vector<float> TextEmbeddingProcessor::embedding(const EmbeddingTask &task) {
    if (task.data.empty()) {
        PRINT_ERROR("TextEmbeddingProcessor input task data is empty.\n");
        return std::vector<float>();
    }
    return proxy_->embeddingText(task.data);
}

std::string TextEmbeddingProcessor::modelInfo() {
    if (!proxy_) initializeResources();
    return proxy_->modelInfo();
}