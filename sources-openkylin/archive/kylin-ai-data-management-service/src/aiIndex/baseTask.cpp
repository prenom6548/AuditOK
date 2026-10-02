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

#include "baseTask.h"

#include <kyai-business-framework/SQLiteCpp/Transaction.h>
#include <kyai-business-framework/application.h>
#include <kyai-business-framework/conf.h>
#include <kyai-business-framework/log.h>
#include <kyai-business-framework/utils.h>

#include "constant.h"
#include "documentParse.h"

namespace DataManagement {
void BaseTask::start() {
    workThread = std::thread([this] {
        while (true) {
            /**
             * 触发条件：
             * 1、空闲、索引开关enable、有任务
             * 2、忙碌并且有任务在执行
             * 3、退出线程
             */
            std::unique_lock<std::mutex> lock(taskLock);
            taskCV.wait(lock, [this] { return quit || (isReadyToStart()) || (isReadyToStop()); });

            // 退出线程
            if (quit) {
                PRINT_INFO("[%s] BaseTask quit.\n", taskName.c_str());
                break;
            }

            if (isReadyToStart()) {
                // 处理任务
                PRINT_INFO("[%s] BaseTask start to work.\n", taskName.c_str());
                isRunning = true;
                handlerTask();
            } else {
                // 暂停任务
                PRINT_INFO("[%s] BaseTask stop to work.\n", taskName.c_str());
                isRunning = false;
                stopTask();
            }
        }
    });
    PRINT_INFO("[%s] BaseTask start.\n", taskName.c_str());
}

void BaseTask::awakeSelf() { taskCV.notify_all(); }

void BaseTask::notifyOthers(const std::string &filePath) {
    // 通知当前已有的连接
    scene::Application::getInstance().notifyAllConnections("FileIndexFinished", filePath);
    PRINT_INFO("[%s] notifyOthers file: %s.\n", taskName.c_str(), filePath.c_str());
}

bool BaseTask::findTask(TaskType type) {
    /**
     * 0,1,2,3 状态的任务可重新进行处理
     * 5 失败的任务重试次数小于 3 可重新进行
     */
    std::string sql =
        "SELECT id FROM t_ai_index_task WHERE type = ? "
        "and (state in (0,1,2,3) or (state = 5 and retry_num < 3)) limit 1";
    SQLite::Statement query(db, sql);
    query.bind(1, type);

    if (!query.executeStep()) {
        PRINT_DEBUG("[%s] SQL: %s, no record.\n", taskName.c_str(), query.getExpandedSQL().c_str());
        return false;
    }

    PRINT_DEBUG("[%s] SQL: %s, has record.\n", taskName.c_str(), query.getExpandedSQL().c_str());
    return true;
}

bool BaseTask::queryTask(TaskType type) {
    std::string sql =
        "SELECT id,file_id,file_path,state,type,create_time,start_time,end_time,retry_num FROM t_ai_index_task "
        "WHERE type = ? and (state in (0,1,2,3) or (state = 5 and retry_num < 3)) "
        "order by state asc,create_time asc limit 1";
    SQLite::Statement query(db, sql);
    query.bind(1, type);

    if (!query.executeStep()) {
        PRINT_DEBUG("[%s] SQL: %s, no record.\n", taskName.c_str(), query.getExpandedSQL().c_str());
        return false;
    }

    PRINT_DEBUG("[%s] SQL: %s, has record.\n", taskName.c_str(), query.getExpandedSQL().c_str());
    taskDO.id = query.getColumn(0);
    taskDO.fileId = query.getColumn(1);
    taskDO.filePath = query.getColumn(2).getString();
    taskDO.state = query.getColumn(3);
    taskDO.type = query.getColumn(4);
    taskDO.createTime = query.getColumn(5).getString();
    taskDO.startTime = query.getColumn(6).getString();
    taskDO.endTime = query.getColumn(7).getString();
    taskDO.retryNum = query.getColumn(8);

    return true;
}

bool BaseTask::modifyTaskState(TaskState state) {
    std::string sql;
    switch (state) {
        case TASK_STATE_PROCESSING:
            if (taskDO.state == TASK_STATE_SUCCESS) {
                sql =
                    R"(UPDATE t_ai_index_task SET state = ?, start_time = datetime('now', 'localtime'), end_time = '0000-00-00 00:00:00', retry_num = 0 WHERE id = ?)";
            } else {
                sql =
                    R"(UPDATE t_ai_index_task SET state = ?, start_time = datetime('now', 'localtime'), end_time = '0000-00-00 00:00:00' WHERE id = ?)";
            }
            break;
        case TASK_STATE_STOP:
        case TASK_STATE_SUCCESS:
            sql = R"(UPDATE t_ai_index_task SET state = ?, end_time = datetime('now', 'localtime') WHERE id = ?)";
            break;
        case TASK_STATE_FAILED:
            sql =
                R"(UPDATE t_ai_index_task SET state = ?, end_time = datetime('now', 'localtime'), retry_num = retry_num+1 WHERE id = ?)";
            break;
        default:
            PRINT_ERROR("[%s] BaseTask::modifyTaskState wrong state %d.\n", taskName.c_str(), state);
            return false;
    }

    SQLite::Statement statement(db, sql);
    statement.bind(1, state);
    statement.bind(2, taskDO.id);
    int rows = statement.exec();
    if (rows != 1) {
        PRINT_ERROR("[%s] SQL: %s, state modify failed, rows: %d.\n", taskName.c_str(),
                    statement.getExpandedSQL().c_str(), rows);
        return false;
    }

    PRINT_DEBUG("[%s] SQL: %s, state modify success.\n", taskName.c_str(), statement.getExpandedSQL().c_str());
    return true;
}

bool BaseTask::successTaskAndEnableFileIndex() {
    SQLite::Transaction transaction(db);

    bool success = modifyTaskState(TASK_STATE_SUCCESS);
    if (!success) {
        PRINT_ERROR("[%s] enableTextFileIndex modifyTaskState to TASK_STATE_SUCCESS failed.\n", taskName.c_str());
        return false;
    }

    // 检查这个文件的任务是否都已经做完
    std::vector<int> taskType = Configuration::getIntArray("ai_index_task_type", {1, 2, 3});
    std::string sql = "SELECT count(*) FROM t_ai_index_task WHERE file_id = ? AND state != 4 AND type in (" +
                      Utils::joinIds(std::unordered_set<int64_t>(taskType.begin(), taskType.end())) + ")";
    SQLite::Statement query(db, sql);
    query.bind(1, taskDO.fileId);

    if (!query.executeStep()) {
        PRINT_WARN("[%s] SQL: %s, no record.\n", taskName.c_str(), query.getExpandedSQL().c_str());
        return false;
    }

    int count = query.getColumn(0);
    if (count != 0) {
        PRINT_DEBUG("[%s] SQL: %s, has %d unfinished task.\n", taskName.c_str(), query.getExpandedSQL().c_str(), count);
        transaction.commit();
        return true;
    }
    PRINT_DEBUG("[%s] SQL: %s, all task finished.\n", taskName.c_str(), query.getExpandedSQL().c_str());

    sql = R"(UPDATE t_file_info SET enable = 1, last_modify_time = datetime('now', 'localtime') WHERE file_id = ?)";
    SQLite::Statement statement(db, sql);
    statement.bind(1, taskDO.fileId);
    int rows = statement.exec();
    if (rows != 1) {
        PRINT_ERROR("[%s] SQL: %s, enable failed, rows: %d.\n", taskName.c_str(), statement.getExpandedSQL().c_str(),
                    rows);
        return false;
    }

    transaction.commit();
    PRINT_DEBUG("[%s] SQL: %s, enable index success.\n", taskName.c_str(), statement.getExpandedSQL().c_str());
    // 索引创建成功通知其他服务
    notifyOthers(taskDO.filePath);
    return true;
}

bool BaseTask::getDocumentChunk(std::vector<std::string> &chunkResults) {
    chunkResults.clear();

    PRINT_INFO("[%s] BaseTask getDocumentChunk start, file path: %s.\n", taskName.c_str(), taskDO.filePath.c_str());

    // 文档分段
    DocumentParse documentParse;
    nlohmann::json ret = documentParse.syncChunkDocument({taskDO.filePath});
    if (ret.empty()) {
        PRINT_ERROR("[%s] %s chunk document server something wrong.\n", taskName.c_str(), taskDO.filePath.c_str());
        return false;
    }

    if (ret.size() != 1 || ret[0][FILEPATH].get<std::string>() != taskDO.filePath) {
        PRINT_ERROR("[%s] chunk document return file path wrong, %s.\n", taskName.c_str(),
                    ret[0][FILEPATH].get<std::string>().c_str());
        return false;
    }

    // 文件不能分块，返回的 ret[0][CHUNKS]是空列表，文件能分块，也有可能内容是空字符串
    for (const auto &chunk : ret[0][CHUNKS]) {
        const std::string chunkStr = chunk.get<std::string>();
        if (chunkStr.empty()) {
            // chunk 本身是空字符串
            PRINT_INFO("[%s] %s chunk document parse result is empty.\n", taskName.c_str(), taskDO.filePath.c_str());
            continue;
        }

        // chunk 非空
        const std::string item = parseDocumentContent(chunkStr);
        if (item.empty()) {
            // 解析的内容为空
            PRINT_INFO("[%s] %s chunk document parse content is empty.\n", taskName.c_str(), taskDO.filePath.c_str());
            continue;
        }
        chunkResults.push_back(item);
    }
    return true;
}

const std::string BaseTask::parseDocumentContent(const std::string &chunk) {
    // chunk是一个json，解析的文本内容在conten字段的value值中
    nlohmann::json item = nlohmann::json::parse(chunk);
    if (item.empty() || !item.contains(CONTENT) || !item[CONTENT].is_string() ||
        item[CONTENT].get<std::string>().empty()) {
        return std::string();
    }

    return item[CONTENT].get<std::string>();
}

BaseTask::~BaseTask() {
    quit = true;
    awakeSelf();

    if (workThread.joinable()) {
        workThread.join();
    }
}
}  // namespace DataManagement
