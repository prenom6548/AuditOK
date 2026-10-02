// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <kywc/log.h>

#include "security.h"
#include "server.h"

struct security_global_filter {
    struct wl_list link;
    struct wl_global *global;
    security_global_filter_func_t filter;
    void *filter_data;
};

struct security_manager {
    struct wl_listener new_client;
    struct wl_list global_filters;
    struct wl_listener display_destroy;
    struct wl_listener server_destroy;
};

static struct security_manager *manager = NULL;

static char *get_path_from_pid(pid_t pid)
{
    char link_file[32];
    snprintf(link_file, sizeof(link_file), "/proc/%d/exe", (int)pid);

    char link_target[PATH_MAX];
    // PATH_MAX includes the terminating NULL byte
    ssize_t read_len = readlink(link_file, link_target, PATH_MAX - 1);
    if (read_len < 0) {
        kywc_log_errno(KYWC_ERROR, "Failed to readlink() %s", link_file);
        return NULL;
    }

    /* should not assume that the returned contents are null-terminated */
    link_target[read_len] = '\0';

    return strdup(link_target);
}

static void handle_client_destroy(void *data)
{
    struct security_client *client = data;
    kywc_log(KYWC_DEBUG, "Client(pid=%d) %s is disconnect", client->pid, client->path);
    free(client->path);
    free(client);
}

static void handle_new_client(struct wl_listener *listener, void *data)
{
    struct security_client *client = calloc(1, sizeof(*client));
    if (!client) {
        return;
    }

    struct wl_client *wl_client = data;
    client->fd = wl_client_get_fd(wl_client);
    wl_client_get_credentials(wl_client, &client->pid, &client->uid, &client->gid);
    client->path = get_path_from_pid(client->pid);
    kywc_log(KYWC_DEBUG, "Client(pid=%d) %s is connect", client->pid, client->path);

    client->client = wl_client;
    wl_client_set_user_data(wl_client, client, handle_client_destroy);
}

static struct security_global_filter *global_filter_from_global(const struct wl_global *global)
{
    struct security_global_filter *global_filter;
    wl_list_for_each(global_filter, &manager->global_filters, link) {
        if (global_filter->global == global) {
            return global_filter;
        }
    }
    return NULL;
}

static bool filter_global(const struct wl_client *wl_client, const struct wl_global *wl_global,
                          void *data)
{
    // TODO: add some builtin filter rules

    struct security_global_filter *filter = global_filter_from_global(wl_global);
    if (!filter) {
        return true;
    }

    struct security_client *client = wl_client_get_user_data((struct wl_client *)wl_client);
    return filter->filter(client, filter->filter_data);
}

bool security_add_global_filter(struct wl_global *global, security_global_filter_func_t filter,
                                void *data)
{
    struct security_global_filter *global_filter = global_filter_from_global(global);
    if (!global_filter) {
        global_filter = calloc(1, sizeof(*global_filter));
        if (!global_filter) {
            return false;
        }
        wl_list_insert(&manager->global_filters, &global_filter->link);
    }

    global_filter->global = global;
    global_filter->filter = filter;
    global_filter->filter_data = data;

    return true;
}

void security_remove_global_filter(struct wl_global *global)
{
    struct security_global_filter *filter = global_filter_from_global(global);
    if (filter) {
        wl_list_remove(&filter->link);
        free(filter);
    }
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    /* global fliter should be removed when global destroy */
    assert(wl_list_empty(&manager->global_filters));
    wl_list_remove(&manager->server_destroy.link);
    free(manager);
    manager = NULL;
}

static void handle_display_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&manager->display_destroy.link);
    wl_list_remove(&manager->new_client.link);
}

bool security_manager_create(struct server *server)
{
    manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return false;
    }

    manager->new_client.notify = handle_new_client;
    wl_display_add_client_created_listener(server->display, &manager->new_client);

    manager->display_destroy.notify = handle_display_destroy;
    wl_display_add_destroy_listener(server->display, &manager->display_destroy);
    manager->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(server, &manager->server_destroy);

    wl_list_init(&manager->global_filters);
    wl_display_set_global_filter(server->display, filter_global, NULL);

    return true;
}
