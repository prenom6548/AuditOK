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

#include "baseembeddingservice.h"

#include <iostream>

void BaseEmbeddingService::stop()
{
    if (started()) {
        stop_ = true;
        taskQueueCV_.notify_all();
        if (processTaskThread_.joinable())
            processTaskThread_.join();
        std::cout << "Stop process task thread" << std::endl;
    }
}

void BaseEmbeddingService::start()
{
    std::unique_lock<std::mutex> lock(startMutex_);
    if (started())
        return;
    stop_ = false;
    processTaskThread_ = std::thread(&BaseEmbeddingService::processTaskInThread, this);
}

bool BaseEmbeddingService::started()
{
    return !stop_;
}

void BaseEmbeddingService::addTask(const EmbeddingTask &task)
{
    if (!started()) {
        return;
    }
    std::unique_lock<std::mutex> lock(taskMutex_);

    tasks_.push(task);
    // std::cout << "Add task: " << task.id() << " type: " << (int)task.type()
    //           << " priority:" << (int)task.priority()
    //           << " to task queue, current queue size: " << tasks_.size() << std::endl;
    taskQueueCV_.notify_all();
}

std::vector<float> BaseEmbeddingService::getTaskResult(TaskID id)
{
    if (!started()) {
        return std::vector<float>();
    }
    std::unique_lock<std::mutex> lock(resultMutex_);

    resultQueueCV_.wait(lock, [id, this] {
        for (const auto& res : results_) {
            if (res.first == id) {
                return true;
            }
        }
        return false;
    });

    // std::cout << "Find result of task " << id << " in result list" << std::endl;
    std::vector<float> result;
    for (auto begin = results_.begin(); begin != results_.end(); ++begin) {
        if (begin->first == id) {
            result = begin->second;
            results_.erase(begin);
            break;
        }
    }

    return result;
}

void BaseEmbeddingService::processTaskInThread()
{
    //! \todo 当长时间没有执行过任务时释放资源，在下次执行任务时重新初始化资源
    initResourceInThread();

    while (started()) {
        EmbeddingTask task;
        {
            std::unique_lock<std::mutex> lock(taskMutex_);
            taskQueueCV_.wait(lock, [this]() {
                return !tasks_.empty() || stop_;
            });
            if (stop_) {
                std::cout << "Stop process task in thread" << std::endl;
                break;
            }
            task = tasks_.top();
            tasks_.pop();
        }

        // std::cout << "Start run task: " << task.id()
        //           << ", task queue reaminder task number: " << tasks_.size() << std::endl;
        std::vector<float> result = doEmbeddingInThread(task);
        // std::cout << "Finish task: " << task.id() << std::endl;

        std::unique_lock<std::mutex> lock(resultMutex_);
        results_.push_back(std::make_pair<TaskID, std::vector<float>>(task.id(), std::move(result)));
        resultQueueCV_.notify_all();
    }
}
