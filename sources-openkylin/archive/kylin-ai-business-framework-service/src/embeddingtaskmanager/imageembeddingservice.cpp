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

#include "imageembeddingservice.h"

#include <iostream>

FeatureStatus ImageEmbeddingService::checkServiceFeatureStatus()
{
    if (!ImageEmbedder::isModelSupport()) {
        return FeatureStatus::NotSupported;
    } else if (!ImageEmbedder::isModelInstalled()) {
        return FeatureStatus::NotInstalled;
    }

    return FeatureStatus::Available;
}

void ImageEmbeddingService::initResourceInThread()
{
    if (embedder_.get() != nullptr)
        return;

    embedder_ = std::make_unique<ImageEmbedder>();
    std::cout << "Finish image embedding resource init" << std::endl;
}

std::vector<float> ImageEmbeddingService::doEmbeddingInThread(const EmbeddingTask &task)
{
    std::vector<float> result;
    switch (task.type()) {
    case EmbeddingTask::EmbeddingDataType::FilePath:
        result = embedder_->embeddingImage(task.textData());
        break;
    case EmbeddingTask::EmbeddingDataType::Text:
        result = embedder_->embeddingText(task.textData());
        break;
    default:
        std::cerr << "Image embedder can't process task type: " << (int)task.type() << std::endl;
        return std::vector<float>();
    }

    return result;
}
