#ifndef EMBEDDINGTASKMANAGER_H
#define EMBEDDINGTASKMANAGER_H

#include "embeddingtaskmanager/embeddingtask.h"
#include "embeddingtaskmanager/baseembeddingservice.h"

#include <memory>

class EmbeddingTaskManager
{
public:
    static EmbeddingTaskManager* getInstance() {
        static EmbeddingTaskManager manager;
        return &manager;
    }

    ~EmbeddingTaskManager();
    void addImageSideTask(const EmbeddingTask& task);
    void addTextSideTask(const EmbeddingTask& task);

    std::vector<float> getImageSideResult(TaskID id);
    std::vector<float> getTextSideResult(TaskID id);

private:
    EmbeddingTaskManager();

private:
    std::unique_ptr<BaseEmbeddingService> textEmbeddingService_;
    std::unique_ptr<BaseEmbeddingService> imageEmbeddingService_;
};

#endif // EMBEDDINGTASKMANAGER_H
