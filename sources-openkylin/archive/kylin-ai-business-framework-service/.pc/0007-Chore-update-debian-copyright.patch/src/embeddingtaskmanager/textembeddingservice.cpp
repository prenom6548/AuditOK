#include "textembeddingservice.h"
#include "utils/textembedder.h"

#include <iostream>

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
