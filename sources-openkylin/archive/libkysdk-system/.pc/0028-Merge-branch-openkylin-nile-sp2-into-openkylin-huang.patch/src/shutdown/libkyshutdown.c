/*
 * libkysdk-system's Library
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#include "libkyshutdown.h"

#define SERVICE_NAME "org.freedesktop.login1"
#define OBJECT_PATH "/org/freedesktop/login1"
#define INTERFACE_NAME "org.freedesktop.login1.Manager"


int kdk_shutdown_power_off(int min)
{
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *m = NULL;
    sd_bus *bus = NULL;
    int r;

    // 连接到系统总线
    r = sd_bus_open_system(&bus);
    if (r < 0)
    {
        // fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
        return -1;
    }

    // 获取当前时间
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
    {
        // fprintf(stderr, "Failed to get current time: %s\n", strerror(errno));
        sd_bus_unref(bus);
        return -1;
    }

    // 计算关机时间（当前时间 + 指定分钟数）
    uint64_t shutdown_time = (uint64_t)ts.tv_sec * 1000000ULL +
                             (uint64_t)ts.tv_nsec / 1000ULL +
                             (uint64_t)min * 60ULL * 1000000ULL;

    // 调用 ScheduleShutdown 方法
    r = sd_bus_call_method(bus,
                           SERVICE_NAME,
                           OBJECT_PATH,
                           INTERFACE_NAME,
                           "ScheduleShutdown",
                           &error,
                           &m,
                           "st",
                           "poweroff",
                           shutdown_time);

    if (r < 0)
    {
        // fprintf(stderr, "Failed to schedule shutdown: %s\n", error.message);
        sd_bus_error_free(&error);
        sd_bus_unref(bus);
        return -1;
    }

    sd_bus_message_unref(m);
    sd_bus_unref(bus);

    return 0;
}

static uint64_t parse_time(const char *time_str)
{
    struct tm tm = {0};
    char *format = "%Y-%m-%d %H:%M:%S";

    if (strptime(time_str, format, &tm) == NULL)
    {
        // If parsing with seconds fails, try without seconds
        format = "%Y-%m-%d %H:%M";
        if (strptime(time_str, format, &tm) == NULL)
        {
            return 0; // Parsing failed
        }
    }

    time_t t = mktime(&tm);
    return (uint64_t)t * 1000000ULL; // Convert to microseconds
}

int kdk_shutdown_schedule_power_off(struct tm shutdown_time)
{
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *m = NULL;
    sd_bus *bus = NULL;
    int r;
    struct timespec ts;
    
    // 转换为时间戳
    time_t shutdown_timestamp = mktime(&shutdown_time);
    if (shutdown_timestamp == -1) {
        //fprintf(stderr, "Invalid time specified\n");
        return -1;
    }

    // 获取当前时间
    if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
        //fprintf(stderr, "Failed to get current time\n");
        return -1;
    }

    // 参数校验
    if (shutdown_timestamp <= ts.tv_sec) {
        //fprintf(stderr, "Invalid shutdown time: must be in the future\n");
        return -1;
    }

    // 转换为微秒（systemd需要的格式）
    uint64_t shutdown_usec = (uint64_t)shutdown_timestamp * 1000000ULL;

    // 连接系统总线
    r = sd_bus_open_system(&bus);
    if (r < 0) {
        fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
        return -1;
    }

    // 调用关机方法
    r = sd_bus_call_method(bus,
                          SERVICE_NAME,
                          OBJECT_PATH,
                          INTERFACE_NAME,
                          "ScheduleShutdown",
                          &error,
                          &m,
                          "st",
                          "poweroff",
                          shutdown_usec);

    if (r < 0) {
        //fprintf(stderr, "Failed to schedule shutdown: %s\n", error.message);
        sd_bus_error_free(&error);
        sd_bus_unref(bus);
        return -1;
    }

    sd_bus_message_unref(m);
    sd_bus_unref(bus);

    return 0;
}

bool kdk_shutdown_cancel_power_off()
{
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *m = NULL;
    sd_bus *bus = NULL;
    int r;
    bool cancelled = false;

    // Connect to the system bus
    r = sd_bus_open_system(&bus);
    if (r < 0)
    {
        // fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
        return false;
    }

    // Call CancelScheduledShutdown method
    r = sd_bus_call_method(bus,
                           SERVICE_NAME,
                           OBJECT_PATH,
                           INTERFACE_NAME,
                           "CancelScheduledShutdown",
                           &error,
                           &m,
                           NULL); // No input parameters

    if (r < 0)
    {
        // fprintf(stderr, "Failed to cancel scheduled shutdown: %s\n", error.message);
        sd_bus_error_free(&error);
        sd_bus_unref(bus);
        return false;
    }

    // Read the boolean return value
    r = sd_bus_message_read(m, "b", &cancelled);
    if (r < 0)
    {
        // fprintf(stderr, "Failed to parse response: %s\n", strerror(-r));
        sd_bus_message_unref(m);
        sd_bus_unref(bus);
        return false;
    }

    sd_bus_message_unref(m);
    //sd_bus_unref(bus);

    return cancelled;
}

bool kdk_shutdown_is_schedule_power_off()
{
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus *bus = NULL;
    sd_bus_message *reply = NULL;
    const char *shutdown_type = NULL;
    uint64_t shutdown_time = 0;
    int r;

    // Connect to the system bus
    r = sd_bus_open_system(&bus);
    if (r < 0)
    {
        // fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
        return false;
    }

    // Read the ScheduledShutdown property
    r = sd_bus_get_property(bus,
                            SERVICE_NAME,
                            OBJECT_PATH,
                            INTERFACE_NAME,
                            "ScheduledShutdown",
                            &error,
                            &reply,
                            "(st)");

    if (r < 0)
    {
        // fprintf(stderr, "Failed to get ScheduledShutdown property: %s\n", error.message);
        sd_bus_error_free(&error);
        sd_bus_unref(bus);
        return false;
    }

    // Parse the reply
    r = sd_bus_message_read(reply, "(st)", &shutdown_type, &shutdown_time);
    if (r < 0)
    {
        // fprintf(stderr, "Failed to parse ScheduledShutdown property: %s\n", strerror(-r));
        sd_bus_message_unref(reply);
        sd_bus_unref(bus);
        return false;
    }

    bool is_scheduled = false;
    if (shutdown_type != NULL && strcmp(shutdown_type, "poweroff") == 0 && shutdown_time > 0)
    {
        is_scheduled = true;
    }

    sd_bus_message_unref(reply);
    sd_bus_unref(bus);

    return is_scheduled;
}

int kdk_shutdown_exit_window(char* user) {
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *m = NULL;
    sd_bus *bus = NULL;
    uid_t uid;
    int r;

    if (user == NULL) {
        // fprintf(stderr, "Invalid user parameter\n");
        return -1;
    }

    // Get user ID from username
    struct passwd *pwd = getpwnam(user);
    if (pwd == NULL) {
        // fprintf(stderr, "User not found: %s\n", user);
        return -1;
    }
    uid = pwd->pw_uid;

    // Connect to the system bus
    r = sd_bus_open_system(&bus);
    if (r < 0) {
        // fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
        return -1;
    }

    // Call TerminateUser method
    r = sd_bus_call_method(bus,
                          SERVICE_NAME,
                          OBJECT_PATH,
                          INTERFACE_NAME,
                          "TerminateUser",
                          &error,
                          &m,
                          "u",      // uint32 parameter for user ID
                          (uint32_t)uid);

    if (r < 0) {
        //fprintf(stderr, "Failed to terminate user: %s\n", error.message);
        sd_bus_error_free(&error);
        goto cleanup;
    }
    
    // 成功时返回 0
    r = 0;
    
    cleanup:
        if (m) sd_bus_message_unref(m);
        sd_bus_unref(bus);
        return r < 0 ? -1 : 0;  // 失败返回 -1，成功返回 0
}

int kdk_shutdown_lock_work_station() {
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *m = NULL;
    sd_bus *bus = NULL;
    char *session = NULL;
    int r;

    // 1. 移除root权限检查（锁定会话通常不需要root权限）
    
    // 2. 改进会话获取方式：先尝试当前会话，再尝试seat0
    r = sd_pid_get_session(getpid(), &session);
    if (r < 0) {
        // 如果获取当前会话失败，尝试获取seat0的会话
        r = sd_session_get_seat("seat0", &session);
        if (r < 0) {
            // 如果还是失败，尝试使用ListSessions获取第一个会话
            r = sd_bus_open_system(&bus);
            if (r >= 0) {
                sd_bus_message *reply = NULL;
                r = sd_bus_call_method(bus,
                                     SERVICE_NAME,
                                     OBJECT_PATH,
                                     INTERFACE_NAME,
                                     "ListSessions",
                                     &error,
                                     &reply,
                                     "");
                if (r >= 0) {
                    char *id, *user, *seat;
                    uint32_t uid;
                    char *type, *service, *object_path;
                    
                    if (sd_bus_message_enter_container(reply, 'a', "(susso)") >= 0) {
                        if (sd_bus_message_enter_container(reply, 'r', "susso") > 0) {
                            sd_bus_message_read(reply, "susso", &id, &uid, &user, 
                                              &seat, &type, &service, &object_path);
                            if (id) {
                                session = strdup(id);
                            }
                            sd_bus_message_exit_container(reply);
                        }
                        sd_bus_message_exit_container(reply);
                    }
                    sd_bus_message_unref(reply);
                }
                sd_bus_error_free(&error);
                sd_bus_unref(bus);
                bus = NULL;
            }
            
            if (!session) {
                return -1;
            }
        }
    }

    // 连接到系统总线
    r = sd_bus_open_system(&bus);
    if (r < 0) {
        free(session);
        return -1;
    }

    // 尝试LockSession方法
    r = sd_bus_call_method(bus,
                          SERVICE_NAME,
                          OBJECT_PATH,
                          INTERFACE_NAME,
                          "LockSession",
                          &error,
                          &m,
                          "s",
                          session);

    free(session);

    // 如果LockSession失败，尝试LockSessions方法
    if (r < 0) {
        sd_bus_error_free(&error);
        if (m) {
            sd_bus_message_unref(m);
            m = NULL;
        }
        
        r = sd_bus_call_method(bus,
                              SERVICE_NAME,
                              OBJECT_PATH,
                              INTERFACE_NAME,
                              "LockSessions",
                              &error,
                              &m,
                              "");
    }

    if (r < 0) {
        sd_bus_error_free(&error);
        sd_bus_unref(bus);
        return -1;
    }

    // Cleanup
    if (m != NULL) {
        sd_bus_message_unref(m);
    }
    sd_bus_unref(bus);

    return 0;
}


