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

#include <genai/text/chat.h>

#include <atomic>

#include "baseTask.h"

namespace DataManagement {
class NlpBaseTask : public BaseTask {
public:
    friend void nlpCallback(ChatResult* result, void* user_data);

    NlpBaseTask(const std::string& taskName, TaskType type);

    void handlerTask() override;

    void stopTask() override;

    bool haveTask() override;

    bool isReadyToStart() override;

    bool isReadyToStop() override;

    TaskType getTaskType() override { return taskType; }

    void resetTaskState() override;

    virtual void setPrompt() = 0;

    virtual bool worker() = 0;

    ~NlpBaseTask() override;

private:
    bool connectNLP();

    bool nlpChat();

protected:
    virtual ChatModelConfig* getChatModelConfig() = 0;

    void stripThoughtContent();

protected:
    GenAiTextSession* session;
    std::string nlpResult;

private:
    TaskType taskType;
    std::atomic<bool> canExecute;  // 用于总体控制nlp任务 执行/停止
};
}  // namespace DataManagement
