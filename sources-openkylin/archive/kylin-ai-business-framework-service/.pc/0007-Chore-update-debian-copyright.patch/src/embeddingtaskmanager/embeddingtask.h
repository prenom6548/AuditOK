#ifndef EMBEDDINGTASK_H
#define EMBEDDINGTASK_H

#include <vector>
#include <string>

typedef std::string TaskID;

class EmbeddingTask {
public:
    enum class EmbeddingDataType {
        Text = 0,
        FilePath
    };

    enum class TaskPriority {
        Default = 0,
        Search
    };

    EmbeddingTask() = default;
    EmbeddingTask(const std::string& data, EmbeddingDataType type, TaskPriority priority = TaskPriority::Default);
    EmbeddingTask(const std::vector<unsigned char>& data, EmbeddingDataType type, TaskPriority priority = TaskPriority::Default);

    TaskID id() const { return taskId_; }
    EmbeddingDataType type() const { return taskType_; }
    TaskPriority priority() const { return priority_; }

    std::vector<unsigned char> mateData() const { return taskData_; }
    std::string textData() const {
        return decodeText(taskData_);
    }

    bool operator<(const EmbeddingTask& other) const;

private:

    std::vector<unsigned char> encodeText(const std::string& data) const;
    std::string decodeText(const std::vector<unsigned char>& data) const;

private:
    std::vector<unsigned char> taskData_;
    EmbeddingDataType taskType_;
    TaskID taskId_;
    TaskPriority priority_;
};

#endif // EMBEDDINGTASK_H
