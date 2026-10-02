// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>

#include <kywc/log.h>

#include "security_p.h"
#include "server.h"

static struct security_manager {
    struct wl_listener server_destroy;
} *manager = NULL;

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&manager->server_destroy.link);
    free(manager);
    manager = NULL;
}

bool security_manager_create(struct server *server)
{
    manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return false;
    }

    manager->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(server, &manager->server_destroy);

    global_filter_manager_create(server);
    ky_security_manager_create(server);
    security_context_manager_create(server->display);

    return true;
}
