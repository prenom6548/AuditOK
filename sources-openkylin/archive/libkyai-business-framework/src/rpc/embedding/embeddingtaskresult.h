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

#ifndef EMBEDDINGTASKRESULT_H
#define EMBEDDINGTASKRESULT_H

#include <kyai-business-framework/rpc/embedding/embeddingtask.h>

#include <condition_variable>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace scene {
class EmbeddingTaskResult {
public:
    EmbeddingTaskResult() = default;

    void addResult(TaskID taskId, std::vector<float> embedding) {
        std::unique_lock<std::mutex> lock(mutex_);
        results_[taskId] = embedding;
        cv_.notify_all();
    }

    std::vector<float> blockPopResult(TaskID taskId) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this, &taskId]() {
            auto it = results_.find(taskId);
            if (it != results_.end()) return true;

            return false;
        });

        auto it = results_.find(taskId);
        std::vector<float> result = std::move(it->second);
        results_.erase(it);
        return result;
    }

private:
    std::unordered_map<TaskID, std::vector<float>> results_;
    std::mutex mutex_;
    std::condition_variable cv_;
};
}  // namespace scene

#endif  // EMBEDDINGTASKRESULT_H
