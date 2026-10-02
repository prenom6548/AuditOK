#include "imageembeddingservice.h"

#include <iostream>

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
