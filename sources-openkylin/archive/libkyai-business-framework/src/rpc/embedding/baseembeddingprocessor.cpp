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

#include <rpc/embedding/baseembeddingprocessor.h>

#include <thread>

#include "embeddingtaskqueue.h"
#include "embeddingtaskresult.h"
#include "log.h"
#include "textembeddingproxy.h"

using namespace scene;

class BaseEmbeddingProcessorPrivate {
public:
    ~BaseEmbeddingProcessorPrivate() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!inited_) return;

        inited_ = false;
        queue_.unbolck();

        if (consumer_.joinable()) consumer_.join();
    }

    bool inited_ = false;

    std::mutex mutex_;
    std::thread consumer_;

    EmbeddingTaskQueue queue_;
    EmbeddingTaskResult results_;
};

BaseEmbeddingProcessor::BaseEmbeddingProcessor() : d_(new BaseEmbeddingProcessorPrivate) {}

BaseEmbeddingProcessor::~BaseEmbeddingProcessor() { delete d_; }

bool BaseEmbeddingProcessor::init() {
    PRINT_INFO("Start init embedding processor\n");
    std::lock_guard<std::mutex> lock(d_->mutex_);

    if (d_->inited_) return true;

    PRINT_INFO("Start init embedding resources\n");
    if (!initializeResources()) {
        return false;
    }

    d_->inited_ = true;
    d_->consumer_ = std::thread([this]() {
        while (d_->inited_) {
            EmbeddingTask task = d_->queue_.blockPopTask();
            if (task.empty()) continue;

            std::vector<float> vector = embedding(task);
            d_->results_.addResult(task.id, vector);
        }
    });

    return true;
}

bool BaseEmbeddingProcessor::inited() {
    std::lock_guard<std::mutex> lock(d_->mutex_);
    return d_->inited_;
}

TaskID BaseEmbeddingProcessor::addTask(const std::string& data, EmbeddingDataType dataType, TaskPriority priority) {
    if (!inited()) {
        PRINT_ERROR("Processor don't init, can't add task\n");
        return TaskID();
    }

    if (data.empty()) {
        PRINT_ERROR("Input data is empty, can't add task\n");
        return TaskID();
    }

    EmbeddingTask task{data, dataType, priority};
    d_->queue_.addTask(task);
    return task.id;
}

std::vector<float> BaseEmbeddingProcessor::getResult(const TaskID& taskId) {
    if (!inited()) {
        PRINT_ERROR("Processor don't init, can't get result\n");
        return std::vector<float>();
    }

    if (taskId.empty()) {
        PRINT_ERROR("Input task id is empty, can't get result\n");
        return std::vector<float>();
    }

    return d_->results_.blockPopResult(taskId);
}

std::string BaseEmbeddingProcessor::getModelInfo() {
    if (!inited()) {
        PRINT_ERROR("Processor don't init, can't get model info\n");
        return "";
    }
    return modelInfo();
}