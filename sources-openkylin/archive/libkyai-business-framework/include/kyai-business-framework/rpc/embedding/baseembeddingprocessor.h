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

#ifndef BASEEMBEDDINGPROCESSOR_H
#define BASEEMBEDDINGPROCESSOR_H

#include <kyai-business-framework/rpc/embedding/embeddingtask.h>

#include <vector>

class BaseEmbeddingProcessorPrivate;

namespace scene {
class BaseEmbeddingProcessor {
public:
    BaseEmbeddingProcessor();
    virtual ~BaseEmbeddingProcessor();

    bool init();
    bool inited();

    TaskID addTask(const std::string& data, EmbeddingDataType dataType, TaskPriority priority = TaskPriority::MEDIUM);

    std::vector<float> getResult(const TaskID& taskId);

    std::string getModelInfo();

protected:
    virtual bool initializeResources() = 0;
    virtual std::vector<float> embedding(const EmbeddingTask& task) = 0;
    virtual std::string modelInfo() = 0;

private:
    BaseEmbeddingProcessorPrivate* d_;
};
}  // namespace scene

#endif  // BASEEMBEDDINGPROCESSOR_H
