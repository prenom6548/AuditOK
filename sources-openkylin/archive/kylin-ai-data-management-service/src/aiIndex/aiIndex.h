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

#include <gio/gio.h>

#include <condition_variable>
#include <memory>
#include <set>
#include <shared_mutex>
#include <vector>

#include "baseTask.h"

namespace DataManagement {
enum IdleState : int {
    IDLE_STATE_BUSY = 0,     // 忙碌
    IDLE_STATE_SYSTEM_IDLE,  // 收到系统空闲信号
    IDLE_STATE_TRUE_IDLE,    // 等待时间过后可以开始建索引
    IDLE_STATE_MAX
};

class AIIndex {
public:
    static AIIndex& getInstance();

    int exec();

    void setIdleState(IdleState newStatus);

    void setAIIndexEnable(bool enable);

    void notifyTaskByType(std::set<TaskType> taskType);

    void resetTaskState();

    bool isIdle();

    bool isEnable();

private:
    AIIndex();

    void syncData();

    void migrateVectorDatabase();

    void resetFailedTasksOnRestart();

    int checkAndRecoverDatabase();

    int listenSignal();

    int listenAIIndexToggle();

    bool isAiIndexKeyValid(GSettingsSchema* schema, GSettings* settings, const gchar* key, const gchar* expectedType);

    bool checkIdleState(IdleState newStatus) const;

    void createWorkTask();

    void startWorkTask();

    void loadResourcesAfterDelay();

    ~AIIndex();

private:
    GDBusConnection* idleSignalConnection;
    guint idleSignalId;

    GSettings* gSettings;
    GSettingsSchema* schema;
    bool isAIIndexEnable;

    std::shared_mutex idleLock;
    std::shared_mutex aiIndexLock;
    IdleState idleState;

    std::vector<std::shared_ptr<BaseTask>> taskLists;
};
}  // namespace DataManagement
