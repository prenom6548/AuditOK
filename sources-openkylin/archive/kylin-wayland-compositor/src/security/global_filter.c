// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <stdlib.h>

#include "security_p.h"
#include "server.h"

struct global_filter {
    struct wl_list link;
    struct wl_global *global;
    security_global_filter_func_t filter;
    void *filter_data;
};

static struct global_filter_manager {
    struct wl_list filters;
    struct wl_listener server_destroy;
} *manager = NULL;

static struct global_filter *global_filter_from_global(const struct wl_global *global)
{
    struct global_filter *global_filter;
    wl_list_for_each(global_filter, &manager->filters, link) {
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

    struct global_filter *filter = global_filter_from_global(wl_global);
    if (!filter) {
        return true;
    }

    return filter->filter(wl_client, filter->filter_data);
}

bool security_add_global_filter(struct wl_global *global, security_global_filter_func_t filter,
                                void *data)
{
    if (!manager) {
        return false;
    }

    struct global_filter *global_filter = global_filter_from_global(global);
    if (!global_filter) {
        global_filter = calloc(1, sizeof(*global_filter));
        if (!global_filter) {
            return false;
        }
        wl_list_insert(&manager->filters, &global_filter->link);
    }

    global_filter->global = global;
    global_filter->filter = filter;
    global_filter->filter_data = data;

    return true;
}

void security_remove_global_filter(struct wl_global *global)
{
    if (!manager) {
        return;
    }

    struct global_filter *filter = global_filter_from_global(global);
    if (filter) {
        wl_list_remove(&filter->link);
        free(filter);
    }
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    /* global fliter should be removed when global destroy */
    assert(wl_list_empty(&manager->filters));
    wl_list_remove(&manager->server_destroy.link);
    free(manager);
    manager = NULL;
}

bool global_filter_manager_create(struct server *server)
{
    manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return false;
    }

    manager->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(server, &manager->server_destroy);

    wl_list_init(&manager->filters);
    wl_display_set_global_filter(server->display, filter_global, NULL);

    return true;
}
