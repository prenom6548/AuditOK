#include "embeddingtask.h"
#include "utils/utils.h"

#include <chrono>
#include <random>

EmbeddingTask::EmbeddingTask(const std::string &textData, EmbeddingDataType type, TaskPriority priority)
    : taskData_(encodeText(textData))
    , taskType_(type)
    , taskId_(utils::generateUUID())
    , priority_(priority)
{ }

EmbeddingTask::EmbeddingTask(const std::vector<unsigned char> &data, EmbeddingDataType type, TaskPriority priority)
    : taskData_(data)
    , taskType_(type)
    , taskId_(utils::generateUUID())
    , priority_(priority)
{}

bool EmbeddingTask::operator <(const EmbeddingTask& other) const
{
    if (priority() == other.priority()) {
        // 优先级相同则先入队的先出队
        return false;
    }
    // 优先级高的先出队
    return priority() < other.priority();
}

std::vector<unsigned char> EmbeddingTask::encodeText(const std::string &data) const
{
    std::vector<unsigned char> encodedData;

    for(char c : data) {
        // 将每个字符转换为 unsigned char 类型并添加到 encodedData 中
        encodedData.push_back(static_cast<unsigned char>(c));
    }

    return encodedData;
}

std::string EmbeddingTask::decodeText(const std::vector<unsigned char> &data) const
{
    std::string decodedText;

    for(unsigned char c : data) {
        decodedText.push_back(static_cast<char>(c));
    }

    return decodedText;
}
