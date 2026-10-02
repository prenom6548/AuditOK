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

#pragma once

#include <condition_variable>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "fileDatabase.h"

namespace DataManagement {
enum TaskType : int {
    TASK_TYPE_TEXT_EMBEDDING = 1,  // 文档向量化
    TASK_TYPE_TAG,                 // 标签提取
    TASK_TYPE_SUMMARY,             // 摘要提取
    TASK_TYPE_MAX
};

enum TaskState : int {
    TASK_STATE_INIT = 0,    // 任务初始化
    TASK_STATE_PROCESSING,  // 任务处理中
    TASK_STATE_STOP,        // 任务终止
    TASK_STATE_PAUSE,       // 任务暂停
    TASK_STATE_SUCCESS,     // 任务成功
    TASK_STATE_FAILED,      // 任务失败
    TASK_STATE_DISCARD,     // 任务废弃
    TASK_STATE_MAX
};

typedef struct taskDO {
    int id{};
    int fileId{};
    std::string filePath;
    int state{};
    int type{};
    std::string createTime;
    std::string startTime;
    std::string endTime;
    int retryNum{};

public:
    std::string toString() const {
        return "id:" + std::to_string(id) + ",file_id:" + std::to_string(fileId) + ",file_path:" + filePath +
               ",state:" + std::to_string(state) + ",type:" + std::to_string(type) + ",create_time:" + createTime +
               ",start_time:" + startTime + ",end_time:" + endTime + ",retry_num:" + std::to_string(retryNum);
    }
} TaskDO;

class BaseTask {
public:
    explicit BaseTask(std::string name)
        : taskName(std::move(name)), isRunning(false), quit(false), db(FileDatabase::createDatabase()) {}

    void start();

    virtual void handlerTask() = 0;

    virtual void stopTask() = 0;

    virtual TaskType getTaskType() = 0;

    virtual bool haveTask() { return false; };

    virtual bool isReadyToStart() { return false; };

    virtual bool isReadyToStop() { return false; };

    virtual void resetTaskState() {}

    void awakeSelf();

    void notifyOthers(const std::string& filePath);

    virtual ~BaseTask();

protected:
    bool findTask(TaskType type);

    bool queryTask(TaskType type);

    bool modifyTaskState(TaskState state);

    bool successTaskAndEnableFileIndex();

    bool getDocumentChunk(std::vector<std::string>& chunkResults);

    const std::string parseDocumentContent(const std::string& chunk);

protected:
    std::string taskName;
    bool isRunning;
    TaskDO taskDO;
    SQLite::Database db;

private:
    std::thread workThread;
    std::mutex taskLock;
    std::condition_variable taskCV;
    bool quit;
};
}  // namespace DataManagement
