#include "embeddingtaskmanager.h"
#include "embeddingtaskmanager/textembeddingservice.h"
#include "embeddingtaskmanager/imageembeddingservice.h"

#include <iostream>

EmbeddingTaskManager::EmbeddingTaskManager()
    : textEmbeddingService_(std::make_unique<TextEmbeddingService>())
    , imageEmbeddingService_(std::make_unique<ImageEmbeddingService>())
{
    textEmbeddingService_->start();
    imageEmbeddingService_->start();
}

EmbeddingTaskManager::~EmbeddingTaskManager()
{
    textEmbeddingService_->stop();
    imageEmbeddingService_->stop();
}

void EmbeddingTaskManager::addImageSideTask(const EmbeddingTask &task)
{
    imageEmbeddingService_->addTask(task);
}

void EmbeddingTaskManager::addTextSideTask(const EmbeddingTask &task)
{
    textEmbeddingService_->addTask(task);
}

std::vector<float> EmbeddingTaskManager::getImageSideResult(TaskID id)
{
    return imageEmbeddingService_->getTaskResult(id);
}

std::vector<float> EmbeddingTaskManager::getTextSideResult(TaskID id)
{
    return textEmbeddingService_->getTaskResult(id);
}
