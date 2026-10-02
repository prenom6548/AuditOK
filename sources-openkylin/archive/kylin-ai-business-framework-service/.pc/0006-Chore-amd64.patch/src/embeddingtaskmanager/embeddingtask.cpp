#include "embeddingtask.h"

#include <chrono>
#include <random>

EmbeddingTask::EmbeddingTask(const std::string &textData, EmbeddingDataType type, TaskPriority priority)
    : taskData_(encodeText(textData))
    , taskType_(type)
    , taskId_(generateUniqueId())
    , priority_(priority)
{ }

EmbeddingTask::EmbeddingTask(const std::vector<unsigned char> &data, EmbeddingDataType type, TaskPriority priority)
    : taskData_(data)
    , taskType_(type)
    , taskId_(generateUniqueId())
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

long long EmbeddingTask::generateUniqueId() const
{
    // 获取当前时间戳
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    // 生成随机数
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<long long> distribution(1, 1000);
    long long random = distribution(gen);

    // 将时间戳和随机数结合生成唯一id
    return ms * 1000 + random;
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
