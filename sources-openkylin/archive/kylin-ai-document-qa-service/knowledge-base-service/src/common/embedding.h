#pragma once

#include <rpc/embedding/embeddingtask.h>

#include <string>
#include <vector>

class Embedding {
public:
    Embedding(scene::TaskPriority priority = scene::TaskPriority::MEDIUM);

    std::vector<float> getTextEmbedding(const std::string& data);
    std::vector<std::vector<float>> getTextEmbeddings(const std::vector<std::string>& texts);
    ~Embedding() = default;

private:
    scene::TaskPriority priority_;
    bool inited_;
};
