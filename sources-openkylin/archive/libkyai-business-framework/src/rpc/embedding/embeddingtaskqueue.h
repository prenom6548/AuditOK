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

#ifndef EMBEDDINGTASKQUEUE_H
#define EMBEDDINGTASKQUEUE_H

#include <kyai-business-framework/log.h>
#include <kyai-business-framework/rpc/embedding/embeddingtask.h>

#include <condition_variable>
#include <mutex>
#include <queue>

namespace scene {
class EmbeddingTaskQueue {
public:
    EmbeddingTaskQueue() = default;

    void addTask(const EmbeddingTask& task) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(task);
        cv_.notify_one();
    }

    EmbeddingTask blockPopTask() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]() { return !queue_.empty() || stop_; });
        {
            std::unique_lock<std::mutex> lock(stopMutex_);
            if (stop_) {
                stop_ = false;
                return EmbeddingTask();
            }
        }

        EmbeddingTask task = queue_.top();
        queue_.pop();
        return task;
    }

    void unbolck() {
        std::unique_lock<std::mutex> lock(stopMutex_);
        if (!stop_) {
            stop_ = true;
            cv_.notify_one();
        }
    }

private:
    bool stop_ = false;
    std::mutex stopMutex_;

    std::mutex mutex_;
    std::condition_variable cv_;
    std::priority_queue<EmbeddingTask> queue_;
};
}  // namespace scene

#endif  // EMBEDDINGTASKQUEUE_H
