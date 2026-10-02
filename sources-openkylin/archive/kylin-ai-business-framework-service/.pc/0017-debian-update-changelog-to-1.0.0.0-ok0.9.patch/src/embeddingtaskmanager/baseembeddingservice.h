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

#ifndef BASEEMBEDDINGSERVICE_H
#define BASEEMBEDDINGSERVICE_H

#include "embeddingtaskmanager/embeddingtask.h"

#include <queue>
#include <list>
#include <mutex>
#include <condition_variable>
#include <thread>

class BaseEmbeddingService
{
public:
    BaseEmbeddingService() = default;
    virtual ~BaseEmbeddingService() = default;

    void stop();
    void start();

    void addTask(const EmbeddingTask& task);
    std::vector<float> getTaskResult(TaskID id);

protected:
    void processTaskInThread();
    virtual void initResourceInThread() = 0;
    virtual std::vector<float> doEmbeddingInThread(const EmbeddingTask& task) = 0;

private:
    std::priority_queue<EmbeddingTask> tasks_;
    std::list<std::pair<TaskID, std::vector<float>>> results_;

    std::thread processTaskThread_;
    std::mutex taskMutex_;
    std::mutex resultMutex_;

    std::condition_variable taskQueueCV_;
    std::condition_variable resultQueueCV_;

    bool stop_ = false;
};

#endif // BASEEMBEDDINGSERVICE_H
