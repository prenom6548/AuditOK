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

#include <rpc/embedding/embeddingtask.h>

#include "utils.h"

using namespace scene;

EmbeddingTask::EmbeddingTask(const std::string& data, EmbeddingDataType type, TaskPriority priority)
    : data(data), dataType(type), priority(priority), id(Utils::generateUUID()) {}

bool EmbeddingTask::operator<(const EmbeddingTask& other) const {
    if (priority == other.priority) {
        // 优先级相同则先入队的先出队
        return false;
    }
    // 优先级高的先出队
    return priority < other.priority;
}

bool EmbeddingTask::empty() const { return id.empty(); }
