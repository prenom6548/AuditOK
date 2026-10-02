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
struct seat;

enum security_action {
    SECURITY_ACCESS_NODE = 0,
    SECURITY_ACCESS_OUTPUT,
    /* a client writes the selection (copy) */
    SECURITY_WRITE_SELECTION,
    /* a client reads the selection (paste) */
    SECURITY_READ_SELECTION,
    /* a client starts a drag */
    SECURITY_START_DRAG,
    /* a client receives a drop */
    SECURITY_DROP,
};

enum security_decision {
    SECURITY_DECISION_ALLOW = 0,
    SECURITY_DECISION_SOFT_ALLOW, // need notify
    SECURITY_DECISION_SOFT_DENY,  // need access
    SECURITY_DECISION_DENY,
};

/* the operation info for SECURITY_ACCESS_NODE */
struct security_node_info {
    struct scene_node *node; // the accessed node
    struct client *client;   // the accessing client
};

/* the operation info for SECURITY_ACCESS_OUTPUT */
struct security_output_info {
    struct scene_output *output; // the accessed output
    struct client *client;       // the accessing client
};

/* the operation info for SECURITY_*_SELECTION */
struct security_selection_info {
    struct seat *seat;          // the seat holding the selection
    struct client *source;      // owner of the selection source
    struct client *destination; // reader of the selection, NULL for write
    const struct wl_array *mime_types;
    bool primary;
};

/* the operation info for SECURITY_START_DRAG and SECURITY_DROP */
struct security_dnd_info {
    struct seat *seat;          // the seat holding the drag
    struct client *source;      // owner of the drag source
    struct client *destination; // the drop target, NULL for start
    const struct wl_array *mime_types;
};

/* the context of a security access */
struct security_action_info {
    enum security_action action;

    union {
        struct security_node_info node;           // for SECURITY_ACCESS_NODE
        struct security_output_info output;       // for SECURITY_ACCESS_OUTPUT
        struct security_selection_info selection; // for SECURITY_*_SELECTION
        struct security_dnd_info dnd;             // for SECURITY_START_DRAG and SECURITY_DROP
    };
};

struct security_interface {
    /* return true when allow */
    bool (*access)(struct security *security, const struct security_action_info *info, void *data);
    /* notify the security is touched */
    void (*notify)(struct security *security, const struct security_action_info *info, void *data);
    /* the security is destroyed */
    void (*destroy)(struct security *security, void *data);
};

/*
 * the acting client implied by the action: the accessing client for
 * ACCESS_NODE/OUTPUT, the source for WRITE_SELECTION/START_DRAG and the
 * destination for READ_SELECTION/DROP.
 */
static inline const struct client *security_action_client(const struct security_action_info *info)
{
    switch (info->action) {
    case SECURITY_ACCESS_NODE:
        return info->node.client;
    case SECURITY_ACCESS_OUTPUT:
        return info->output.client;
    case SECURITY_WRITE_SELECTION:
        return info->selection.source;
    case SECURITY_READ_SELECTION:
        return info->selection.destination;
    case SECURITY_START_DRAG:
        return info->dnd.source;
    case SECURITY_DROP:
        return info->dnd.destination;
    }
    return NULL;
}

/* the security is inert when data is NULL */
struct security *security_create_for_node(struct scene_node *node, enum security_decision decision,
                                          const struct security_interface *impl, void *data);

struct security *security_create_for_output(struct scene_output *output,
                                            enum security_decision decision,
                                            const struct security_interface *impl, void *data);

struct security *security_create_for_selection(struct seat *seat, enum security_decision decision,
                                               const struct security_interface *impl, void *data);

/* return true if this seat has security for selection and dnd */
bool security_check_selection(struct seat *seat);

/* check the security access, return true when allowed */
bool security_access(const struct security_action_info *info);

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
