// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _SECURITY_H_
#define _SECURITY_H_

#include <wayland-server-core.h>

#include "util/client.h"

struct server;
struct security;
struct scene_node;
struct scene_output;

enum security_action {
    SECURITY_ACCESS_NODE = 0,
    SECURITY_ACCESS_OUTPUT,
};

enum security_decision {
    SECURITY_DECISION_ALLOW = 0,
    SECURITY_DECISION_SOFT_ALLOW, // need notify
    SECURITY_DECISION_SOFT_DENY,  // need access
    SECURITY_DECISION_DENY,
};

struct security_interface {
    /* return true when allow */
    bool (*access)(struct security *security, struct client *client, enum security_action action,
                   void *data);
    /* notify the security is touched */
    void (*notify)(struct security *security, struct client *client, enum security_action action,
                   void *data);
    /* the security is destroyed */
    void (*destroy)(struct security *security, void *data);
};

/* the security is inert when data is NULL */
struct security *security_create_for_node(struct scene_node *node, enum security_decision decision,
                                          const struct security_interface *impl, void *data);

struct security *security_create_for_output(struct scene_output *output,
                                            enum security_decision decision,
                                            const struct security_interface *impl, void *data);

void security_set_user_data(struct security *security, void *data);

void *security_get_user_data(struct security *security);

void security_set_decision(struct security *security, enum security_decision decision);

void security_ref(struct security *security);

/* unref the security, destroy callback may called */
void security_unref(struct security *security);

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

enum security_protector_mode {
    PROTECTOR_FILL_NONE = 0,
    PROTECTOR_FILL_COLOR,
    PROTECTOR_FILL_IMAGE,
};

void security_set_protector_mode(enum security_protector_mode mode);

void security_set_protector_color(float r, float g, float b, float a);

bool security_set_protector_image(uint32_t width, uint32_t height, void *data);

#endif /* _SECURITY_H_ */
