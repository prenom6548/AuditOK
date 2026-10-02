// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _SECURITY_H_
#define _SECURITY_H_

#include <pixman.h>
#include <wayland-server-core.h>

struct server;
struct output;
struct scene_node;

bool security_manager_create(struct server *server);

/* global filter */
typedef bool (*security_global_filter_func_t)(const struct wl_client *client, void *data);

bool security_add_global_filter(struct wl_global *global, security_global_filter_func_t filter,
                                void *data);

void security_remove_global_filter(struct wl_global *global);

/* security context */
struct security_context_state {
    char *sandbox_engine; // may be NULL
    char *app_id;         // may be NULL
    char *instance_id;    // may be NULL
};

const struct security_context_state *
security_context_manager_lookup_client(const struct wl_client *client);

bool security_check_node(struct scene_node *node);

bool security_check_output(struct output *output, pixman_region32_t *clip);

#endif /* _SECURITY_H_ */
