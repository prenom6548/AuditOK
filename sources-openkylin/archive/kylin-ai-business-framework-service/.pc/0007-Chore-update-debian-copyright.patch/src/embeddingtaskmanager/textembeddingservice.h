#ifndef TEXTEMBEDDINGSERVICE_H
#define TEXTEMBEDDINGSERVICE_H

#include "embeddingtaskmanager/baseembeddingservice.h"
#include "utils/textembedder.h"

#include <memory>

class TextEmbeddingService : public BaseEmbeddingService
{
protected:
    void initResourceInThread() override;
    std::vector<float> doEmbeddingInThread(const EmbeddingTask& task) override;

private:
    std::unique_ptr<TextEmbedder> embedder_;
};

#endif // TEXTEMBEDDINGSERVICE_H
