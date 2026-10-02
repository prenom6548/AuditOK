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

#include "nlpBaseTask.h"

#include <genai/text/error.h>
#include <kyai-business-framework/conf.h>
#include <kyai-business-framework/log.h>

#include "aiIndex.h"

namespace DataManagement {

NlpBaseTask::NlpBaseTask(const std::string& taskName, TaskType type)
    : session(nullptr), taskType(type), BaseTask(taskName), canExecute(true) {}

void NlpBaseTask::handlerTask() {
    PRINT_INFO("[%s] handlerTask start...\n", taskName.c_str());

    if (!queryTask(taskType)) {
        PRINT_ERROR("[%s] cannot get task.\n", taskName.c_str());
        isRunning = false;
        return;
    }

    PRINT_DEBUG("[%s] get task: %s.\n", taskName.c_str(), taskDO.toString().c_str());

    // 任务状态改为处理中
    bool success = modifyTaskState(TASK_STATE_PROCESSING);
    if (!success) {
        PRINT_ERROR("[%s] modify %s state to processing failed.\n", taskName.c_str(), taskDO.filePath.c_str());
        modifyTaskState(TASK_STATE_FAILED);
        isRunning = false;
        return;
    }
    // 清空nlp回调的结果
    nlpResult.clear();

    success = connectNLP();
    if (!success) {
        canExecute.store(false);
        PRINT_ERROR("[%s] connect nlp failed, stop task until next idle.\n", taskName.c_str());
        modifyTaskState(TASK_STATE_STOP);
        isRunning = false;
        return;
    }

    success = nlpChat();
    if (!success) {
        PRINT_ERROR("[%s] nlpChat failed.\n", taskName.c_str());
        modifyTaskState(TASK_STATE_FAILED);
        isRunning = false;
        return;
    }

    PRINT_INFO("[%s] handlerTask end...\n", taskName.c_str());
}

void NlpBaseTask::stopTask() {
    PRINT_INFO("[%s] stopTask stop...\n", taskName.c_str());
    genai_text_stop_chat(session);
    PRINT_INFO("[%s] stopTask end...\n", taskName.c_str());
}

bool NlpBaseTask::haveTask() { return findTask(taskType); }

bool NlpBaseTask::isReadyToStart() {
    if (canExecute.load() != true) {
        PRINT_ERROR("[%s] isReadyToStart, nlp task stop, can't start.\n", taskName.c_str());
        return false;
    }

    return AIIndex::getInstance().isIdle() && AIIndex::getInstance().isEnable() && !isRunning && haveTask();
}

bool NlpBaseTask::isReadyToStop() {
    // 正常来说 闲时处理任务的时候，索引开关状态从打开到关闭，会首先触发监听空闲信号的回调
    return (!AIIndex::getInstance().isIdle() || !AIIndex::getInstance().isEnable()) && isRunning;
}

void NlpBaseTask::resetTaskState() {
    /* nlp任务在遇到连接失败或者callback错误时，停止nlp任务，直到再次空闲尝试处理任务，
    空闲的回调函数中，会调用resetTaskState来重置canExecute的状态
    */
    PRINT_INFO("[%s] resetTaskState, set nlp task state is can execute.\n", taskName.c_str());
    canExecute.store(true);
}

bool NlpBaseTask::connectNLP() {
    if (session != nullptr) {
        return true;
    }

    session = genai_text_create_session();
    if (session == nullptr) {
        PRINT_ERROR("[%s] Create nlp session error.\n", taskName.c_str());
        return false;
    }

    ChatModelConfig* config = getChatModelConfig();
    if (config == nullptr) {
        PRINT_ERROR("[%s] Create chat model config error.\n", taskName.c_str());
        genai_text_destroy_session(&session);
        session = nullptr;
        return false;
    }

    genai_text_set_model_config(session, config);
    if (genai_text_init_session(session) != 0) {
        PRINT_ERROR("[%s] Init nlp session error.\n", taskName.c_str());
        genai_text_destroy_session(&session);
        session = nullptr;
        chat_model_config_destroy(&config);
        return false;
    }

    chat_model_config_destroy(&config);
    return true;
}

void nlpCallback(ChatResult* result, void* user_data) {
    auto nlpTask = static_cast<NlpBaseTask*>(user_data);
    nlpTask->nlpResult += chat_result_get_assistant_message(result);

    int code = chat_result_get_error_code(result);
    if (code != AiSdkCommonErrorCode::AISDK_NO_ERROR) {
        nlpTask->canExecute.store(false);
        PRINT_ERROR("[%s] Nlp chat end, code: %d not success, stop task until next idle.\n", nlpTask->taskName.c_str(),
                    code);
        nlpTask->modifyTaskState(TASK_STATE_STOP);
        nlpTask->isRunning = false;
        nlpTask->awakeSelf();
        return;
    }

    if (!chat_result_get_is_end(result)) {
        PRINT_DEBUG("[%s] Nlp callback not end.\n", nlpTask->taskName.c_str());
        return;
    }

    const char* finishReasonCStr = chat_result_get_finish_reason_message(result);
    std::string finishReason = finishReasonCStr == nullptr ? "" : finishReasonCStr;

    // 判断是否为 stop
    std::string stopFlag = "<|stopChat_8167431|>";
    bool isStopByFlag = nlpTask->nlpResult.size() >= stopFlag.size() &&
                        nlpTask->nlpResult.substr(nlpTask->nlpResult.length() - stopFlag.length()) == stopFlag;
    bool isStopByFinishReason = (finishReason == "stop");

    if (isStopByFlag || isStopByFinishReason) {
        PRINT_INFO("[%s] Nlp chat stop, file: %s, finish_reason: %s.\n", nlpTask->taskName.c_str(),
                   nlpTask->taskDO.filePath.c_str(), finishReason.c_str());
        nlpTask->modifyTaskState(TASK_STATE_STOP);
        nlpTask->isRunning = false;
        nlpTask->awakeSelf();
        return;
    }

    PRINT_INFO("[%s] Nlp chat end success.\n", nlpTask->taskName.c_str());
    bool success = nlpTask->worker();
    if (!success) {
        PRINT_ERROR("[%s] worker failed, %s.\n", nlpTask->taskName.c_str(), nlpTask->taskDO.filePath.c_str());
        nlpTask->modifyTaskState(TASK_STATE_FAILED);
        nlpTask->isRunning = false;
        nlpTask->awakeSelf();
        return;
    }
    success = nlpTask->successTaskAndEnableFileIndex();
    if (!success) {
        PRINT_ERROR("[%s] enable file index failed, %s.\n", nlpTask->taskName.c_str(),
                    nlpTask->taskDO.filePath.c_str());
        nlpTask->modifyTaskState(TASK_STATE_FAILED);
    }

    // 任务结束
    nlpTask->isRunning = false;
    nlpTask->awakeSelf();
}

bool NlpBaseTask::nlpChat() {
    genai_text_clear_chat_history_messages(session);
    setPrompt();
    genai_text_result_set_callback(session, nlpCallback, this);

    std::vector<std::string> chunkResults;
    bool success = getDocumentChunk(chunkResults);
    if (!success) {
        PRINT_ERROR("[%s] nlpChat get document chunk failed.\n", taskName.c_str());
        return false;
    }

    // 文档分块为空，认为成功
    if (chunkResults.empty() || chunkResults[0].empty()) {
        PRINT_DEBUG("[%s] nlpChat get document chunk is empty.\n", taskName.c_str());
        nlpResult = "";
        success = worker();
        if (!success) {
            PRINT_ERROR("[%s] nlp task worker failed, %s.\n", taskName.c_str(), taskDO.filePath.c_str());
            return false;
        }

        success = successTaskAndEnableFileIndex();
        if (!success) {
            PRINT_ERROR("[%s] enable file index failed, %s.\n", taskName.c_str(), taskDO.filePath.c_str());
            return false;
        }
        isRunning = false;
        return true;
    }
    PRINT_DEBUG("[%s] nlpChat chat text is %s.\n", taskName.c_str(), chunkResults[0].c_str());
    genai_text_chat_async(session, chunkResults[0].c_str());
    return true;
}

void NlpBaseTask::stripThoughtContent() {
    // 部分模型会有思考过程，有的话就过滤掉
    const std::string delimiter = "\n</think>\n\n";
    size_t pos = nlpResult.find(delimiter);

    if (pos == std::string::npos) {
        // 没有思考过程，直接返回
        PRINT_DEBUG("[%s] chat result not have thought process.\n", taskName.c_str());
        return;
    }

    // 输出有思考过程，进行过滤，返回分隔符之后的所有内容
    PRINT_DEBUG("[%s] chat result have thought process, strip thought content.\n", taskName.c_str());
    nlpResult = nlpResult.substr(pos + delimiter.length());
}

NlpBaseTask::~NlpBaseTask() {
    genai_text_destroy_session(&session);
    session = nullptr;
}
}  // namespace DataManagement
