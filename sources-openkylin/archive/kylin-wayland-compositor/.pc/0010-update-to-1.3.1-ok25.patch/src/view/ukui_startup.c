// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <strings.h>

#include "util/client.h"
#include "util/time.h"
#include "view_p.h"

#include "ukui-startup-v2-protocol.h"

#define UKUI_STARTUP_V2_VERSION 1
#define DEFAULT_RETIRE_TIME 5000

struct ukui_startup_management {
    struct wl_global *global;
    struct wl_list ukui_startup_infos;

    struct wl_listener display_destroy;
    struct wl_listener server_destroy;
};

struct ukui_startup_info {
    struct wl_list link;
    struct wl_resource *resource;

    const char *app_id;
    pid_t pid;
    struct kywc_box box;

    uint32_t time_msec;
};

static struct ukui_startup_management *management = NULL;

static void ukui_startup_info_destroy(struct ukui_startup_info *info)
{
    /* resource may be destroyed in destroy request */
    if (info->resource) {
        wl_resource_set_user_data(info->resource, NULL);
    }

    wl_list_remove(&info->link);
    free((void *)info->app_id);
    free(info);
}

static void ukui_get_startup_geometry_recursive(pid_t pid, struct view *view)
{
    struct ukui_startup_info *info, *tmp;
    wl_list_for_each_reverse_safe(info, tmp, &management->ukui_startup_infos, link) {
        if (info->pid == pid) {
            view->startup_geometry = info->box;
            ukui_startup_info_destroy(info);
            return;
        }
    }

    pid_t parent_pid = client_get_parent(pid);
    /* init/systemd (PID=1) */
    if (parent_pid > 1) {
        ukui_get_startup_geometry_recursive(parent_pid, view);
        return;
    }

    if (!view->base.app_id) {
        return;
    }

    wl_list_for_each_safe(info, tmp, &management->ukui_startup_infos, link) {
        if (info->app_id && strcasecmp(info->app_id, view->base.app_id) == 0) {
            kywc_log(KYWC_DEBUG, "Get startup_geometry from app_id %s", info->app_id);
            view->startup_geometry = info->box;
            ukui_startup_info_destroy(info);
            return;
        }
    }

    kywc_log(KYWC_INFO, "Failed to get startup_geometry for PID: %d, app_id: %s", pid,
             view->base.app_id);
}

static void ukui_get_startup_geometry(struct view *view, void *data)
{
    if (wl_list_empty(&management->ukui_startup_infos)) {
        return;
    }

    /* remove retired info */
    uint32_t time_now = current_time_msec();
    struct ukui_startup_info *info, *tmp;
    wl_list_for_each_safe(info, tmp, &management->ukui_startup_infos, link) {
        if (time_now - info->time_msec > DEFAULT_RETIRE_TIME) {
            ukui_startup_info_destroy(info);
        }
    }

    if (wl_list_empty(&management->ukui_startup_infos)) {
        return;
    }

    ukui_get_startup_geometry_recursive(view->pid, view);
}

static void handle_set_startup_geometry(struct wl_client *client, struct wl_resource *resource,
                                        int32_t x, int32_t y, uint32_t width, uint32_t height)
{
    struct ukui_startup_info *info = wl_resource_get_user_data(resource);
    if (!info) {
        return;
    }

    info->box = (struct kywc_box){ x, y, width, height };
}

static void handle_set_pid(struct wl_client *client, struct wl_resource *resource, uint32_t pid)
{
    struct ukui_startup_info *info = wl_resource_get_user_data(resource);
    if (!info) {
        return;
    }

    info->pid = pid;
}

static void handle_set_appid(struct wl_client *client, struct wl_resource *resource,
                             const char *app_id)
{
    struct ukui_startup_info *info = wl_resource_get_user_data(resource);
    if (!info) {
        return;
    }

    free((void *)info->app_id);
    info->app_id = strdup(app_id);
}

static void handle_info_destroy(struct wl_client *client, struct wl_resource *resource)
{
    struct ukui_startup_info *info = wl_resource_get_user_data(resource);
    if (info) {
        info->resource = NULL;
    }
    wl_resource_destroy(resource);
}

static const struct ukui_startup_info_v2_interface ukui_startup_info_v2_ipml = {
    .set_startup_geometry = handle_set_startup_geometry,
    .set_pid = handle_set_pid,
    .set_appid = handle_set_appid,
    .destroy = handle_info_destroy,
};

static void handle_create_startup_info(struct wl_client *client, struct wl_resource *resource,
                                       uint32_t id)
{
    struct ukui_startup_info *info = calloc(1, sizeof(*info));
    if (!info) {
        wl_client_post_no_memory(client);
        return;
    }

    int version = wl_resource_get_version(resource);
    struct wl_resource *info_resource =
        wl_resource_create(client, &ukui_startup_info_v2_interface, version, id);
    if (!resource) {
        free(info);
        wl_client_post_no_memory(client);
        return;
    }

    info->resource = info_resource;
    info->time_msec = current_time_msec();
    wl_resource_set_implementation(info_resource, &ukui_startup_info_v2_ipml, info, NULL);
    wl_list_insert(&management->ukui_startup_infos, &info->link);
}

static void handle_management_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct ukui_startup_management_v2_interface ukui_startup_management_v2_ipml = {
    .destroy = handle_management_destroy,
    .create_startup_info = handle_create_startup_info,
};

static void ukui_startup_management_bind(struct wl_client *client, void *data, uint32_t version,
                                         uint32_t id)
{
    struct wl_resource *resource =
        wl_resource_create(client, &ukui_startup_management_v2_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }

    wl_resource_set_implementation(resource, &ukui_startup_management_v2_ipml, management, NULL);
}

static void handle_display_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&management->display_destroy.link);
    struct ukui_startup_info *info, *tmp;
    wl_list_for_each_safe(info, tmp, &management->ukui_startup_infos, link) {
        ukui_startup_info_destroy(info);
    }
    wl_global_destroy(management->global);
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
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

    management->global =
        wl_global_create(server->display, &ukui_startup_management_v2_interface,
                         UKUI_STARTUP_V2_VERSION, management, ukui_startup_management_bind);
    if (!management->global) {
        kywc_log(KYWC_WARN, "UKUI startup management create failed");
        free(management);
        management = NULL;
        return false;
    }

    wl_list_init(&management->ukui_startup_infos);
    view_manager->impl.get_startup_geometry = ukui_get_startup_geometry;

    management->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(server, &management->server_destroy);
    management->display_destroy.notify = handle_display_destroy;
    wl_display_add_destroy_listener(server->display, &management->display_destroy);

    return true;
}
