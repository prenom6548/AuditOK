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

#include "libkyrestart.h"

#define SERVICE_NAME "org.freedesktop.login1"
#define OBJECT_PATH "/org/freedesktop/login1"
#define INTERFACE_NAME "org.freedesktop.login1.Manager"


int kdk_restart_reboot(int min)
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
    uint64_t restart_time = (uint64_t)ts.tv_sec * 1000000ULL +
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
                           "reboot",
                           restart_time);

    if (r < 0)
    {
        // fprintf(stderr, "Failed to schedule restart: %s\n", error.message);
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

int kdk_restart_schedule_reboot(struct tm restart_time)
{
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *m = NULL;
    sd_bus *bus = NULL;
    int r;
    struct timespec ts;
    
    // 转换为时间戳
    time_t restart_timestamp = mktime(&restart_time);
    if (restart_timestamp == -1) {
        //fprintf(stderr, "Invalid time specified\n");
        return -1;
    }

    // 获取当前时间
    if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
        //fprintf(stderr, "Failed to get current time\n");
        return -1;
    }

    // 参数校验
    if (restart_timestamp <= ts.tv_sec) {
        //fprintf(stderr, "Invalid shutdown time: must be in the future\n");
        return -1;
    }

    // 转换为微秒（systemd需要的格式）
    uint64_t restart_usec = (uint64_t)restart_timestamp * 1000000ULL;

    // Connect to the system bus
    r = sd_bus_open_system(&bus);
    if (r < 0)
    {
        // fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
        return -1;
    }

    // Call ScheduleShutdown method
    r = sd_bus_call_method(bus,
                           SERVICE_NAME,
                           OBJECT_PATH,
                           INTERFACE_NAME,
                           "ScheduleShutdown",
                           &error,
                           &m,
                           "st",
                           "reboot",
                           restart_usec);

    if (r < 0)
    {
        // fprintf(stderr, "Failed to schedule restart: %s\n", error.message);
        sd_bus_error_free(&error);
        sd_bus_unref(bus);
        return -1;
    }

    sd_bus_message_unref(m);
    sd_bus_unref(bus);

    return 0;
}

bool kdk_restart_cancel_reboot()
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
        // fprintf(stderr, "Failed to cancel scheduled restart: %s\n", error.message);
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

bool kdk_restart_is_schedule_reboot()
{
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus *bus = NULL;
    sd_bus_message *reply = NULL;
    const char *restart_type = NULL;
    uint64_t restart_time = 0;
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
    r = sd_bus_message_read(reply, "(st)", &restart_type, &restart_time);
    if (r < 0)
    {
        // fprintf(stderr, "Failed to parse ScheduledShutdown property: %s\n", strerror(-r));
        sd_bus_message_unref(reply);
        sd_bus_unref(bus);
        return false;
    }

    bool is_scheduled = false;
    if (restart_type != NULL && strcmp(restart_type, "reboot") == 0 && restart_time > 0)
    {
        is_scheduled = true;
    }

    sd_bus_message_unref(reply);
    sd_bus_unref(bus);

    return is_scheduled;
}