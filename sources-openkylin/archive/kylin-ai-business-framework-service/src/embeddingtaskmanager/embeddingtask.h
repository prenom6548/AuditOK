/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

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
