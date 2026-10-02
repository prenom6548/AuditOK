/*
 * libkysdk-system's Library
 *
 * Copyright (C) 2023-2025, KylinSoft Co., Ltd.
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
 * Authors: ZhiMin Shao <shaozhimin@kylinos.cn>
 *
 */

#include "libkypowermanager.h"
#include <stdio.h>
#include <kysdk/kysdk-base/libkygsetting.h>
#include <string.h>
#include <kysdk/kysdk-base/cstring-extension.h>
#include <systemd/sd-bus.h>

#define SERVICE_NAME "org.freedesktop.login1"
#define OBJECT_PATH "/org/freedesktop/login1"
#define INTERFACE_NAME "org.freedesktop.login1.Manager"

#ifndef STRNLEN_MAX
#define STRNLEN_MAX 1024 * 1024 * 1024
#endif

int kdk_power_get_screenidle_timeout()
{
    int value = kdk_settings_get_int("org.ukui.power-manager", "sleep-display-ac");
    return value;
}

int kdk_power_is_active()
{
    char line[512] = {0};
    int active = -1;
    FILE *fp = popen("xset q", "r");
    if (!fp)
    {
        return -1;
    }

    while (fgets(line, 512, fp))
    {
        if (strstr(line, "timeout"))
        {
            sscanf(line, "%*s %d", &active);
            break;
        }
    }
    return active;
}

int kdk_power_get_mode()
{
    int value = kdk_settings_get_int("org.ukui.power-manager", "power-policy-ac");
    return value;
}

char *kdk_power_is_hibernate()
{
    char line[128] = {0};
    char *hibernate = NULL;
    FILE *fp = fopen("/sys/power/state", "r");
    if (!fp)
    {
        return NULL;
    }

    fgets(line, 128, fp);
    if(strnlen(line, STRNLEN_MAX) != 0)
    {
        hibernate = (char *)malloc(128 * sizeof(char));
        if(!hibernate)
            return NULL;
        strstripspace(line);
        strncpy(hibernate, line, 127); 
    }
    return hibernate;
}

char* kdk_power_get_control_disk_status()
{
    char line[128] = {0};
    char *status = NULL;
    FILE *fp = fopen("/sys/power/disk", "r");
    if (!fp)
    {
        return NULL;
    }

    fgets(line, 128, fp);
    if(strnlen(line, STRNLEN_MAX) != 0)
    {
        status = (char *)malloc(128 * sizeof(char));
        if(!status)
            return NULL;
        strstripspace(line);
        strncpy(status, line, 127);
    }
    return status;
}

bool kdk_power_is_support_hibernate() {
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *reply = NULL;
    sd_bus *bus = NULL;
    const char *can_hibernate = NULL;
    int r;

    // Connect to the system bus
    r = sd_bus_open_system(&bus);
    if (r < 0) {
        // fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
        return false;
    }

    // Call CanHibernate method
    r = sd_bus_call_method(bus,
                          SERVICE_NAME,
                          OBJECT_PATH,
                          INTERFACE_NAME,
                          "CanHibernate",
                          &error,
                          &reply,
                          NULL);  // No input parameters needed

    if (r < 0) {
        // fprintf(stderr, "Failed to check hibernate support: %s\n", error.message);
        sd_bus_error_free(&error);
        sd_bus_unref(bus);
        return false;
    }

    // Read the reply string
    r = sd_bus_message_read(reply, "s", &can_hibernate);
    if (r < 0) {
        // fprintf(stderr, "Failed to parse reply: %s\n", strerror(-r));
        sd_bus_message_unref(reply);
        sd_bus_unref(bus);
        return false;
    }

    bool is_supported = false;
    if (can_hibernate != NULL && (strcmp(can_hibernate, "yes") == 0 || 
                                 strcmp(can_hibernate, "challenge") == 0)) {
        is_supported = true;
    }

    // Cleanup
    sd_bus_message_unref(reply);
    sd_bus_unref(bus);

    return is_supported;
}

bool kdk_power_is_support_suspend() {
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *reply = NULL;
    sd_bus *bus = NULL;
    const char *can_suspend = NULL;
    int r;

    // Connect to the system bus
    r = sd_bus_open_system(&bus);
    if (r < 0) {
        // fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
        return false;
    }

    // Call CanSuspend method
    r = sd_bus_call_method(bus,
                          SERVICE_NAME,
                          OBJECT_PATH,
                          INTERFACE_NAME,
                          "CanSuspend",
                          &error,
                          &reply,
                          NULL);  // No input parameters needed

    if (r < 0) {
        // fprintf(stderr, "Failed to check suspend support: %s\n", error.message);
        sd_bus_error_free(&error);
        sd_bus_unref(bus);
        return false;
    }

    // Read the reply string
    r = sd_bus_message_read(reply, "s", &can_suspend);
    if (r < 0) {
        // fprintf(stderr, "Failed to parse reply: %s\n", strerror(-r));
        sd_bus_message_unref(reply);
        sd_bus_unref(bus);
        return false;
    }

    bool is_supported = false;
    if (can_suspend != NULL && (strcmp(can_suspend, "yes") == 0 || 
                               strcmp(can_suspend, "challenge") == 0)) {
        is_supported = true;
    }

    // Cleanup
    sd_bus_message_unref(reply);
    sd_bus_unref(bus);

    return is_supported;
}

int kdk_power_set_hibernate(void) {
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *m = NULL;
    sd_bus *bus = NULL;
    int r;

    // 连接到系统总线
    r = sd_bus_open_system(&bus);
    if (r < 0) {
        // fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
        return -1;
    }

    // 调用Hibernate方法
    r = sd_bus_call_method(bus,
                          SERVICE_NAME,
                          OBJECT_PATH,
                          INTERFACE_NAME,
                          "Hibernate",
                          &error,
                          &m,
                          "b",
                          false);  // interactive参数设为false

    if (r < 0) {
        // fprintf(stderr, "Failed to issue method call: %s\n", error.message);
        sd_bus_error_free(&error);
        sd_bus_unref(bus);
        return -1;
    }

    sd_bus_message_unref(m);
    sd_bus_unref(bus);
    
    return 0;
}

int kdk_power_set_suspend(void) {
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *m = NULL;
    sd_bus *bus = NULL;
    int r;

    // 连接到系统总线
    r = sd_bus_open_system(&bus);
    if (r < 0) {
        // fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
        return -1;
    }

    // 调用Hibernate方法
    r = sd_bus_call_method(bus,
                          SERVICE_NAME,
                          OBJECT_PATH,
                          INTERFACE_NAME,
                          "Suspend",
                          &error,
                          &m,
                          "b",
                          false);  // interactive参数设为false

    if (r < 0) {
        // fprintf(stderr, "Failed to issue method call: %s\n", error.message);
        sd_bus_error_free(&error);
        sd_bus_unref(bus);
        return -1;
    }

    sd_bus_message_unref(m);
    sd_bus_unref(bus);
    
    return 0;
}
