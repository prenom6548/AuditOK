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

#include "textEmbeddingTask.h"

#include <kyai-business-framework/log.h>
#include <kyai-business-framework/utils.h>

#include "aiIndex.h"
#include "dataManagementVectorDatabase.h"
#include "embedding.h"
#include "fileDatabase.h"

namespace DataManagement {
TextEmbeddingTask::TextEmbeddingTask() : BaseTask("text-embedding-task") {}

/**
 * 每次处理一个分块，向量化暂时不支持打断
 * 唤不唤醒目前都可以，因为当前线程未休眠
 */
void TextEmbeddingTask::handlerTask() {
    PRINT_INFO("[%s] handlerTask start...\n", taskName.c_str());
    bool success;

    // 先处理分块队列中的任务
    if (!chunkLists.empty()) {
        // 文件可能还未处理完，又有更新，检查当前任务状态是否为处理中
        if (taskDO.state != TASK_STATE_PROCESSING) {
            PRINT_DEBUG("[%s] task not in processing, state: %d.\n", taskName.c_str(), taskDO.state);
            // 清空队列
            std::queue<std::string>().swap(chunkLists);
            isRunning = false;
            return;
        }

        // 检查下文件id是否存在，有可能任务还没处理完，文件已经被删了
        std::string filePath = FileDatabase::getFilePathById(db, taskDO.fileId);
        if (filePath.empty()) {
            PRINT_DEBUG("[%s] file id: %d not exist, clear all queue.\n", taskName.c_str(), taskDO.fileId);
            // 清空队列
            std::queue<std::string>().swap(chunkLists);
            isRunning = false;
            return;
        }

        success = handlerProcessingTask();
    } else {
        if (!queryTask(taskType)) {
            PRINT_ERROR("[%s] cannot get task.\n", taskName.c_str());
            isRunning = false;
            return;
        }
        PRINT_DEBUG("[%s] get task: %s.\n", taskName.c_str(), taskDO.toString().c_str());

        success = handlerNewTask();
    }

    if (!success) {
        modifyTaskState(TASK_STATE_FAILED);
    }

    // 文档分段为空，认为索引建立成功
    if (success && chunkLists.empty()) {
        // 使能文件索引
        success = successTaskAndEnableFileIndex();
        if (!success) {
            PRINT_ERROR("[%s] enable file index failed, file_id:%d.\n", taskName.c_str(), taskDO.fileId);
            modifyTaskState(TASK_STATE_FAILED);
        }
    }

    isRunning = false;
    PRINT_INFO("[%s] handlerTask end...\n", taskName.c_str());
}

void TextEmbeddingTask::stopTask() {
    // 文档分段做向量化，暂时不支持暂停，走不到这里
    PRINT_INFO("[%s] stopTask stop...\n", taskName.c_str());
}

bool TextEmbeddingTask::haveTask() { return !chunkLists.empty() || findTask(taskType); }

bool TextEmbeddingTask::isReadyToStart() { return AIIndex::getInstance().isEnable() && !isRunning && haveTask(); }

bool TextEmbeddingTask::isReadyToStop() { return !AIIndex::getInstance().isEnable() && isRunning; }

bool TextEmbeddingTask::handlerProcessingTask() {
    std::string chunk = chunkLists.front();
    chunkLists.pop();

    // 文档内容向量化
    Embedding textEmbedding(DataManagementEmbeddingDataType::Text, scene::TaskPriority::MEDIUM);
    std::vector<float> embedding = textEmbedding.getEmbedding(chunk);
    if (embedding.empty()) {
        // 向量化结果为空，创建索引流程继续，比如生僻字模型不识别
        PRINT_WARN("[%s] text embedding file_id:%d, result is empty.\n", taskName.c_str(), taskDO.fileId);
        return true;
    }

    // 存入向量数据库
    std::string uuid = Utils::generateUUID();
    nlohmann::json metadata;
    metadata["file_id"] = taskDO.fileId;

    bool success = DataManagementVectorDatabase::getInstance().insertVectorData(
        TEXT_FILE_CONTENT_COLLECTION_NAME, {DataManagementVectorInfo{uuid, embedding, metadata}});
    if (!success) {
        PRINT_ERROR("[%s] %s insert data failed.\n", taskName.c_str(), TEXT_FILE_CONTENT_COLLECTION_NAME);
        return false;
    }
    return true;
}

bool TextEmbeddingTask::handlerNewTask() {
    // 任务状态改为处理中
    bool success = modifyTaskState(TASK_STATE_PROCESSING);
    if (!success) {
        PRINT_ERROR("[%s] modify %s state to processing failed.\n", taskName.c_str(), taskDO.filePath.c_str());
        return false;
    }

    // 文档分段
    std::vector<std::string> chunkResults;
    success = getDocumentChunk(chunkResults);
    if (!success) {
        PRINT_ERROR("[%s] %s getDocumentChunk failed.\n", taskName.c_str(), taskDO.filePath.c_str());
        return false;
    }

    // 存入队列
    for (const auto& chunk : chunkResults) {
        chunkLists.emplace(chunk);
    }

    std::string expression = "file_id == " + std::to_string(taskDO.fileId);
    success =
        DataManagementVectorDatabase::getInstance().deleteVectorData(TEXT_FILE_CONTENT_COLLECTION_NAME, expression);
    if (!success) {
        PRINT_ERROR("[%s] %s delete data failed.\n", taskName.c_str(), TEXT_FILE_CONTENT_COLLECTION_NAME);
        return false;
    }
    return true;
}
}  // namespace DataManagement
