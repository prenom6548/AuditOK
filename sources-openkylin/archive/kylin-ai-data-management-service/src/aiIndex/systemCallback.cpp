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

#include "systemCallback.h"

#include <kyai-business-framework/conf.h>
#include <kyai-business-framework/log.h>

#include "aiIndex.h"
#include "cpptime.h"
#include "embedding.h"

CppTime::timer_id timerId;

/**
 * system-signal    idle-status
 *      0               0       忙碌
 *      3               1       空闲
 * @param status
 * @return
 */
DataManagement::IdleState systemStatusToIdleState(guint32 status) {
    DataManagement::IdleState idleState = DataManagement::IDLE_STATE_MAX;
    switch (status) {
        case 0:
            idleState = DataManagement::IDLE_STATE_BUSY;
            break;
        case 3:
            idleState = DataManagement::IDLE_STATE_SYSTEM_IDLE;
            break;
        default:
            idleState = DataManagement::IDLE_STATE_MAX;
            break;
    }
    PRINT_INFO("SystemStatusToIdleState receive signal is %d, idle status is %d.\n", status, idleState);
    return idleState;
}

void judgeIsIdle(GDBusConnection* conn, const gchar* sender_name, const gchar* object_path, const gchar* interface_name,
                 const gchar* signal_name, GVariant* parameters, gpointer user_data) {
    PRINT_DEBUG("JudgeIsIdle start...\n");

    if (!g_str_equal(signal_name, "StatusChanged")) {
        PRINT_ERROR("JudgeIsIdle receive unexpected signal name, %s\n", signal_name);
        return;
    }

    guint32 status = UINT32_MAX;
    g_variant_get(parameters, "(u)", &status);

    DataManagement::IdleState IdleState = systemStatusToIdleState(status);
    if (IdleState == DataManagement::IDLE_STATE_MAX) {
        PRINT_WARN("JudgeIsIdle receive unexpected signal %d.\n", status);
        return;
    }

    DataManagement::AIIndex::getInstance().setIdleState(IdleState);

    if (IdleState == DataManagement::IDLE_STATE_SYSTEM_IDLE) {
        // 启动定时器
        int idleSecond = Configuration::getInt("idle_second");
        auto t = CppTime::getGlobalTimer();
        timerId = t->add(std::chrono::seconds(idleSecond), [t](CppTime::timer_id id) {
            // 注：一定要先改变 IdleState 值，再重置任务状态，最后唤醒所有任务
            DataManagement::AIIndex::getInstance().setIdleState(DataManagement::IDLE_STATE_TRUE_IDLE);
            DataManagement::AIIndex::getInstance().resetTaskState();
            DataManagement::AIIndex::getInstance().notifyTaskByType(
                {DataManagement::TaskType::TASK_TYPE_TAG, DataManagement::TaskType::TASK_TYPE_SUMMARY});
            t->discard(id);
            PRINT_INFO("JudgeIsIdle ready change to true idle.\n");
        });
    } else {
        // 系统状态变为繁忙
        auto t = CppTime::getGlobalTimer();
        t->discard(timerId);
        DataManagement::AIIndex::getInstance().notifyTaskByType(
            {DataManagement::TaskType::TASK_TYPE_TAG, DataManagement::TaskType::TASK_TYPE_SUMMARY});
        PRINT_INFO("JudgeIsIdle change to busy.\n");
    }

    PRINT_DEBUG("JudgeIsIdle end...\n");
}

void judgeIsAIIndexEnable(GSettings* settings, const gchar* key, gpointer user_data) {
    PRINT_DEBUG("JudgeIsAIIndexEnable start...\n");
    gboolean value = g_settings_get_boolean(settings, key);
    DataManagement::AIIndex::getInstance().setAIIndexEnable(value);
    if (value) {
        // 打开索引开关
        Embedding textEmbedding(DataManagementEmbeddingDataType::Text, scene::TaskPriority::MEDIUM);
        Embedding imageTextEmbedding(DataManagementEmbeddingDataType::ImageText, scene::TaskPriority::HIGH);
        PRINT_DEBUG("Current AIIndexEnable is: %s,load embedding model.\n", value ? "true" : "false");
    }

    // 索引开关状态变化时 唤醒所有任务
    DataManagement::AIIndex::getInstance().notifyTaskByType({DataManagement::TaskType::TASK_TYPE_TEXT_EMBEDDING,
                                                             DataManagement::TaskType::TASK_TYPE_TAG,
                                                             DataManagement::TaskType::TASK_TYPE_SUMMARY});
    PRINT_INFO("JudgeIsAIIndexEnable current AIIndexEnable is: %s.\n", value ? "true" : "false");
    PRINT_DEBUG("JudgeIsAIIndexEnable end...\n");
}