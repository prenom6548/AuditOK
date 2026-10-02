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
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "textembeddingservice.h"
#include "utils/textembedder.h"

#include <iostream>


FeatureStatus TextEmbeddingService::checkServiceFeatureStatus()
{
    if (!TextEmbedder::isModelInstalled()) {
        return FeatureStatus::NotInstalled;
    } else if (!TextEmbedder::isModelSupport()) {
        return FeatureStatus::NotSupported;
    }

    return FeatureStatus::Available;
}

void TextEmbeddingService::initResourceInThread()
{
    if (embedder_.get() != nullptr)
        return;

    embedder_ = std::make_unique<TextEmbedder>();
    std::cout << "Finish text embedding resource init" << std::endl;
}

std::vector<float> TextEmbeddingService::doEmbeddingInThread(const EmbeddingTask &task)
{
    if (task.type() != EmbeddingTask::EmbeddingDataType::Text) {
        std::cerr << "Text embedder just process text task" << std::endl;
        return std::vector<float>();
    }
    return embedder_->embeddingText(task.textData());
}
