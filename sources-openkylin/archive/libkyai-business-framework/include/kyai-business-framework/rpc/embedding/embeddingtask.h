/*
 * Copyright (C) 2024 KylinSoft Co., Ltd.
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

#include <string>

namespace scene {
typedef std::string TaskID;

enum class EmbeddingDataType { Text = 0, ImageFlie, ImageBase64 };
enum class TaskPriority { LOW = 0, MEDIUM, HIGH, CRITICAL };
enum class EmbeddingModelType { TextModel = 0, ImageModel };

struct EmbeddingTask {
    EmbeddingTask() = default;
    EmbeddingTask(const std::string& data, EmbeddingDataType type, TaskPriority priority = TaskPriority::MEDIUM);

    std::string data;
    EmbeddingDataType dataType;
    TaskID id;
    TaskPriority priority;

    bool operator<(const EmbeddingTask& other) const;
    bool empty() const;
};
}  // namespace scene

#endif  // EMBEDDINGTASK_H
