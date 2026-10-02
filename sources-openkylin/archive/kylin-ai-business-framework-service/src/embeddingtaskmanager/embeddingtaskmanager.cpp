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

#include "embeddingtaskmanager.h"
#include "embeddingtaskmanager/textembeddingservice.h"
#include "embeddingtaskmanager/imageembeddingservice.h"

#include <iostream>

EmbeddingTaskManager::EmbeddingTaskManager()
    : textEmbeddingService_(std::make_unique<TextEmbeddingService>())
    , imageEmbeddingService_(std::make_unique<ImageEmbeddingService>())
{

}

EmbeddingTaskManager::~EmbeddingTaskManager()
{
    textEmbeddingService_->stop();
    imageEmbeddingService_->stop();
}

void EmbeddingTaskManager::initEmbeddingService()
{
    std::unique_lock<std::mutex> lock(embeddingServiceInitMutex_);
    if (hasInitEmbeddingService_)
        return;

    if (checkEmbeddingFeatureStatus() == FeatureStatus::Available) {
        textEmbeddingService_->start();
        imageEmbeddingService_->start();
        hasInitEmbeddingService_ = true;
    }

}

FeatureStatus EmbeddingTaskManager::checkEmbeddingFeatureStatus()
{
    FeatureStatus textStatus = textEmbeddingService_->checkServiceFeatureStatus();
    FeatureStatus imageStatus = imageEmbeddingService_->checkServiceFeatureStatus();
    if (textStatus == FeatureStatus::NotSupported || imageStatus == FeatureStatus::NotSupported) {
        return FeatureStatus::NotSupported;
    } else if (textStatus == FeatureStatus::NotInstalled || imageStatus == FeatureStatus::NotInstalled) {
        return FeatureStatus::NotInstalled;
    }

    return FeatureStatus::Available;
}
void EmbeddingTaskManager::addImageSideTask(const EmbeddingTask &task)
{
    if (!hasInitEmbeddingService_)
        initEmbeddingService();

    imageEmbeddingService_->addTask(task);
}

void EmbeddingTaskManager::addTextSideTask(const EmbeddingTask &task)
{
    if (!hasInitEmbeddingService_)
        initEmbeddingService();

    textEmbeddingService_->addTask(task);
}

std::vector<float> EmbeddingTaskManager::getImageSideResult(TaskID id)
{
    if (!hasInitEmbeddingService_)
        initEmbeddingService();

    return imageEmbeddingService_->getTaskResult(id);
}

std::vector<float> EmbeddingTaskManager::getTextSideResult(TaskID id)
{
    if (!hasInitEmbeddingService_)
        initEmbeddingService();

    return textEmbeddingService_->getTaskResult(id);
}
