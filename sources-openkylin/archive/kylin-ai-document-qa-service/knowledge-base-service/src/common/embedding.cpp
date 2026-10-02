#include "embedding.h"

#include <log.h>
#include <rpc/embedding/imageembeddingprocessor.h>
#include <rpc/embedding/textembeddingprocessor.h>

Embedding::Embedding(scene::TaskPriority priority) : priority_(priority) {
    if (!scene::TextEmbeddingProcessor::getInstance().init()) {
        PRINT_ERROR("TextEmbeddingProcessor init failed\n");
        inited_ = false;
        return;
    }

    inited_ = true;
}

std::vector<float> Embedding::getTextEmbedding(const std::string& data) {
    if (!inited_) {
        PRINT_ERROR("EmbeddingProcessor not inited.\n");
        return std::vector<float>();
    }

    if (data.empty()) {
        PRINT_ERROR("EmbeddingProcessor input data is empty.\n");
        return std::vector<float>();
    }

    scene::TaskID taskID;
    std::vector<float> ret;

    taskID = scene::TextEmbeddingProcessor::getInstance().addTask(data, scene::EmbeddingDataType::Text, priority_);
    ret = scene::TextEmbeddingProcessor::getInstance().getResult(taskID);

    if (ret.empty()) {
        PRINT_ERROR("Embedding result is empty, data: %s.\n", data.c_str());
    }
    return ret;
}

std::vector<std::vector<float>> Embedding::getTextEmbeddings(const std::vector<std::string>& texts) {
    std::vector<std::vector<float>> output;
    for (const auto& text : texts) {
        std::vector<float> res = getTextEmbedding(text);
        output.push_back(res);
    }
    return output;
}