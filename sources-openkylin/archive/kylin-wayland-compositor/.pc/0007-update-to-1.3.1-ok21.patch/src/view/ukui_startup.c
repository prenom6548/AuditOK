// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ukui-startup-v1-protocol.h"
#include "util/time.h"
#include "view_p.h"

#define UKUI_STARTUP_V1_VERSION 1
#define STARTUP_EXPIRATION_TIME 5000

struct ukui_startup_management {
    struct wl_global *global;
    struct wl_list ukui_startup_infos;

    struct wl_listener display_destroy;
    struct wl_listener server_destroy;

    struct view_manager *view_manager;
};

struct ukui_startup_info {
    struct wl_list link;

    struct kywc_box box;
    pid_t pid;
    uint32_t time_msec;
};

static struct ukui_startup_management *management = NULL;

static pid_t get_parent_pid(pid_t pid)
{
    int ppid;
    char path[256];
    char line[256];
    snprintf(path, sizeof(path), "/proc/%d/status", pid);

    FILE *file = fopen(path, "r");
    if (!file) {
        kywc_log(KYWC_ERROR, "Failed to open /proc/[pid]/status\n");
        return -1;
    }

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "PPid:", 5) == 0) {
            sscanf(line, "PPid: %d", &ppid);
            fclose(file);
            return ppid;
        }
    }

    fclose(file);
    return -1;
}

static void ukui_get_startup_geometry_recursive(pid_t pid, struct view *view)
{
    pid_t current_pid = pid;
    pid_t parent_pid = get_parent_pid(current_pid);

    if (parent_pid <= 0) {
        kywc_log(KYWC_ERROR, "Failed to find parent for PID: %d\n", current_pid);
        return;
    }

    struct ukui_startup_info *info, *tmp;
    wl_list_for_each_safe(info, tmp, &management->ukui_startup_infos, link) {
        if (info->pid == pid) {
            view->startup_geometry.geometry =
                (struct kywc_box){ info->box.x, info->box.y, info->box.width, info->box.height };
            view->startup_geometry.use_startup_geometry = true;
            wl_list_remove(&info->link);
            free(info);
            return;
        }
    }

    /* init/systemd (PID=1) */
    if (parent_pid == 1) {
        kywc_log(KYWC_DEBUG, "Topmost parent process\n");
        return;
    }

    ukui_get_startup_geometry_recursive(parent_pid, view);
}

static void ukui_get_startup_geometry(uint32_t pid, void *data)
{
    struct view *view = data;
    uint32_t time_now = current_time_msec();

    ukui_get_startup_geometry_recursive(pid, view);

    /* remove expired info */
    struct ukui_startup_info *info, *tmp;
    wl_list_for_each_safe(info, tmp, &management->ukui_startup_infos, link) {
        if (info->time_msec < time_now - STARTUP_EXPIRATION_TIME) {
            wl_list_remove(&info->link);
            free(info);
        }
    }
}

static struct ukui_startup_info *ukui_startup_info_from_pid(pid_t pid)
{
    struct ukui_startup_info *info;
    wl_list_for_each(info, &management->ukui_startup_infos, link) {
        if (info->pid == pid) {
            return info;
        }
    }
    return NULL;
}

static void management_handle_set_startup_geometry(struct wl_client *client,
                                                   struct wl_resource *resource, uint32_t pid,
                                                   int32_t x, int32_t y, uint32_t width,
                                                   uint32_t height)
{
    struct ukui_startup_info *info = ukui_startup_info_from_pid(pid);
    if (info) {
        info->box = (struct kywc_box){ x, y, width, height };
        info->time_msec = current_time_msec();
        return;
    }

    info = calloc(1, sizeof(*info));
    if (!info) {
        return;
    }

    info->box = (struct kywc_box){ x, y, width, height };
    info->pid = pid;
    info->time_msec = current_time_msec();

    wl_list_insert(&management->ukui_startup_infos, &info->link);
}

static void management_handle_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct ukui_startup_management_v1_interface ukui_startup_management_v1_ipml = {
    .destroy = management_handle_destroy,
    .set_startup_geometry = management_handle_set_startup_geometry,
};

static const struct startup_geometry_impl ukui_startup_geometry_impl = {
    .get_startup_geometry = ukui_get_startup_geometry,
};

static void ukui_startup_management_bind(struct wl_client *client, void *data, uint32_t version,
                                         uint32_t id)
{
    struct wl_resource *resource =
        wl_resource_create(client, &ukui_startup_management_v1_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }

    wl_resource_set_implementation(resource, &ukui_startup_management_v1_ipml, management, NULL);
    management->view_manager->impl = &ukui_startup_geometry_impl;
}

static void handle_display_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&management->display_destroy.link);
    wl_global_destroy(management->global);
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    struct ukui_startup_info *info, *tmp;
    wl_list_for_each_safe(info, tmp, &management->ukui_startup_infos, link) {
        wl_list_remove(&info->link);
        free(info);
    }

    wl_list_remove(&management->server_destroy.link);
    free(management);
    management = NULL;
}

bool ukui_startup_management_create(struct view_manager *view_manager)
{
    struct server *server = view_manager->server;
    management = calloc(1, sizeof(*management));
    if (!management) {
        return false;
    }

    management->view_manager = view_manager;
    management->global =
        wl_global_create(server->display, &ukui_startup_management_v1_interface,
                         UKUI_STARTUP_V1_VERSION, management, ukui_startup_management_bind);
    if (!management->global) {
        kywc_log(KYWC_WARN, "ukui startup management create failed");
        free(management);
        return false;
    }

    wl_list_init(&management->ukui_startup_infos);

    management->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(server, &management->server_destroy);
    management->display_destroy.notify = handle_display_destroy;
    wl_display_add_destroy_listener(server->display, &management->display_destroy);

    return false;
}
