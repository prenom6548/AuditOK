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
