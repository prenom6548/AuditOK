#ifndef IMAGEEMBEDDINGSERVICE_H
#define IMAGEEMBEDDINGSERVICE_H

#include "embeddingtaskmanager/baseembeddingservice.h"
#include "utils/imageembedder.h"

#include <memory>

class ImageEmbeddingService : public BaseEmbeddingService
{
public:
    ImageEmbeddingService() = default;

protected:
    void initResourceInThread() override;
    std::vector<float> doEmbeddingInThread(const EmbeddingTask& task) override;

private:
    std::unique_ptr<ImageEmbedder> embedder_;
};

#endif // IMAGEEMBEDDINGSERVICE_H
