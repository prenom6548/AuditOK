// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <kywc/log.h>

#include "output.h"
#include "scene/surface.h"
#include "security_p.h"
#include "server.h"

#include "kywc-security-v1-protocol.h"

enum ky_security_type {
    KY_SECURITY_TYPE_SURFACE = 0,
    KY_SECURITY_TYPE_OUTPUT,
};

struct ky_security {
    struct wl_resource *resource;
    struct wl_list link;

    struct security *security;

    union {
        struct {
            struct wlr_surface *surface;
            struct wl_listener map;
            struct wl_listener destroy;
            struct scene_node *node;
        } surface;
        struct {
            struct scene_output *output;
            struct wl_listener destroy;
        } output;
    };

    enum ky_security_type type;
    bool enabled;
};

struct ky_security_manager {
    struct wl_global *global;
    struct wl_list surfaces;

    struct wl_listener display_destroy;
    struct wl_listener server_destroy;
};

static void ky_security_destroy(struct ky_security *security)
{
    wl_resource_set_user_data(security->resource, NULL);
    wl_list_remove(&security->link);

    if (security->type == KY_SECURITY_TYPE_SURFACE) {
        wl_list_remove(&security->surface.map.link);
        wl_list_remove(&security->surface.destroy.link);
    } else if (security->type == KY_SECURITY_TYPE_OUTPUT) {
        wl_list_remove(&security->output.destroy.link);
    }

    if (security->security) {
        // make the security inert
        security_set_user_data(security->security, NULL);
        security_unref(security->security);
    }

    free(security);
}

static void handle_notify(struct security *security, const struct security_action_info *info,
                          void *data)
{
    struct ky_security *ky_security = data;
    if (!ky_security->enabled) {
        return;
    }

    const struct client *client = security_action_client(info);
    if (!client) {
        return;
    }

    char pid[256];
    snprintf(pid, sizeof(pid), "%s(%d)", client->comm, client->pid);
    kywc_security_v1_send_touched(ky_security->resource, pid);
}

static void handle_destroy(struct security *security, void *data)
{
    struct ky_security *ky_security = data;
    ky_security->security = NULL;

    if (ky_security->type == KY_SECURITY_TYPE_SURFACE) {
        ky_security->surface.node = NULL;
    }
}

static const struct security_interface ky_security_impl = {
    .notify = handle_notify,
    .destroy = handle_destroy,
};

static bool security_apply_enabled(struct ky_security *security)
{
    enum security_decision decision =
        security->enabled ? SECURITY_DECISION_DENY : SECURITY_DECISION_ALLOW;

    if (security->security) {
        security_set_decision(security->security, decision);
        return true;
    }

    if (security->type == KY_SECURITY_TYPE_SURFACE) {
        if (!security->surface.node) {
            return true;
        }
        security->security =
            security_create_for_node(security->surface.node, decision, &ky_security_impl, security);
    } else if (security->type == KY_SECURITY_TYPE_OUTPUT) {
        security->security = security_create_for_output(security->output.output, decision,
                                                        &ky_security_impl, security);
    }

    return security->security != NULL;
}

static void security_set_enabled(struct ky_security *security, bool enabled)
{
    if (security->enabled == enabled) {
        return;
    }

    security->enabled = enabled;

    if (!security_apply_enabled(security)) {
        kywc_security_v1_send_failed(security->resource);
        ky_security_destroy(security);
        return;
    }

    kywc_security_v1_send_enabled(security->resource, enabled);
}

static void security_handle_enable(struct wl_client *client, struct wl_resource *resource)
{
    struct ky_security *security = wl_resource_get_user_data(resource);
    if (!security) {
        return;
    }
    security_set_enabled(security, true);
}

static void security_handle_disable(struct wl_client *client, struct wl_resource *resource)
{
    struct ky_security *security = wl_resource_get_user_data(resource);
    if (!security) {
        return;
    }
    security_set_enabled(security, false);
}

static void security_handle_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct kywc_security_v1_interface security_impl = {
    .destroy = security_handle_destroy,
    .enable = security_handle_enable,
    .disable = security_handle_disable,
};

static void security_handle_resource_destroy(struct wl_resource *resource)
{
    struct ky_security *security = wl_resource_get_user_data(resource);
    if (security) {
        ky_security_destroy(security);
    }
}

static struct ky_security *security_surface_get(struct ky_security_manager *manager,
                                                struct wlr_surface *wlr_surface)
{
    struct ky_security *security;
    wl_list_for_each(security, &manager->surfaces, link) {
        if (security->surface.surface == wlr_surface) {
            return security;
        }
    }
    return NULL;
}

static struct scene_node *security_node_get_ancestor(struct scene_node *node)
{
    while (node->parent && node->parent->node.role.type != SCENE_ROLE_WORKSPACE &&
           node->parent->node.role.type != SCENE_ROLE_LAYER &&
           node->parent->node.role.type != SCENE_ROLE_VIEW_PROXY) {
        node = &node->parent->node;
    }
    return node;
}

static void security_handle_surface_map(struct wl_listener *listener, void *data)
{
    struct ky_security *security = wl_container_of(listener, security, surface.map);
    struct scene_buffer *buffer = scene_buffer_try_from_surface(security->surface.surface);
    if (!buffer) {
        return;
    }

    security->surface.node = security_node_get_ancestor(&buffer->node);

    if (!security_apply_enabled(security)) {
        kywc_security_v1_send_failed(security->resource);
        ky_security_destroy(security);
    }
}

static void security_handle_surface_destroy(struct wl_listener *listener, void *data)
{
    struct ky_security *security = wl_container_of(listener, security, surface.destroy);
    kywc_security_v1_send_cancelled(security->resource);
    ky_security_destroy(security);
}

static void manager_handle_get_security_for_surface(struct wl_client *client,
                                                    struct wl_resource *resource, uint32_t id,
                                                    struct wl_resource *surface_resource)
{
    struct ky_security *security = calloc(1, sizeof(*security));
    if (!security) {
        wl_client_post_no_memory(client);
        return;
    }

    uint32_t version = wl_resource_get_version(resource);
    security->resource = wl_resource_create(client, &kywc_security_v1_interface, version, id);
    if (!security->resource) {
        wl_client_post_no_memory(client);
        free(security);
        return;
    }

    wl_resource_set_implementation(security->resource, &security_impl, security,
                                   security_handle_resource_destroy);

    /* check if surface already has a security object */
    struct ky_security_manager *manager = wl_resource_get_user_data(resource);
    struct wlr_surface *wlr_surface = wlr_surface_from_resource(surface_resource);
    if (security_surface_get(manager, wlr_surface)) {
        kywc_security_v1_send_failed(security->resource);
        wl_resource_set_user_data(security->resource, NULL);
        free(security);
        return;
    }

    security->type = KY_SECURITY_TYPE_SURFACE;
    wl_list_insert(&manager->surfaces, &security->link);
    security->surface.surface = wlr_surface;
    security->surface.map.notify = security_handle_surface_map;
    wl_signal_add(&wlr_surface->events.map, &security->surface.map);
    security->surface.destroy.notify = security_handle_surface_destroy;
    wl_signal_add(&wlr_surface->events.destroy, &security->surface.destroy);

    /* security is enabled default */
    security_set_enabled(security, true);

    if (wlr_surface->mapped) {
        security_handle_surface_map(&security->surface.map, NULL);
    }
}

static void security_handle_output_destroy(struct wl_listener *listener, void *data)
{
    struct ky_security *security = wl_container_of(listener, security, output.destroy);
    kywc_security_v1_send_cancelled(security->resource);
    ky_security_destroy(security);
}

static void manager_handle_get_security_for_output(struct wl_client *client,
                                                   struct wl_resource *resource, uint32_t id,
                                                   struct wl_resource *output_resource)
{
    struct ky_security *security = calloc(1, sizeof(*security));
    if (!security) {
        wl_client_post_no_memory(client);
        return;
    }

    uint32_t version = wl_resource_get_version(resource);
    security->resource = wl_resource_create(client, &kywc_security_v1_interface, version, id);
    if (!security->resource) {
        wl_client_post_no_memory(client);
        free(security);
        return;
    }

    wl_resource_set_implementation(security->resource, &security_impl, security,
                                   security_handle_resource_destroy);

    struct output *output = output_from_resource(output_resource);
    if (!output || !output->scene_output) {
        kywc_security_v1_send_failed(security->resource);
        wl_resource_set_user_data(security->resource, NULL);
        free(security);
        return;
    }

    security->type = KY_SECURITY_TYPE_OUTPUT;
    security->output.output = output->scene_output;
    security->output.destroy.notify = security_handle_output_destroy;
    wl_signal_add(&output->scene_output->events.destroy, &security->output.destroy);
    wl_list_init(&security->link);

    /* security is enabled default */
    security_set_enabled(security, true);
}

static void manager_handle_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct kywc_security_manager_v1_interface ky_security_manager_impl = {
    .destroy = manager_handle_destroy,
    .get_security_for_surface = manager_handle_get_security_for_surface,
    .get_security_for_output = manager_handle_get_security_for_output,
};

static void ky_security_manager_bind(struct wl_client *client, void *data, uint32_t version,
                                     uint32_t id)
{
    struct wl_resource *resource =
        wl_resource_create(client, &kywc_security_manager_v1_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }

    wl_resource_set_implementation(resource, &ky_security_manager_impl, data, NULL);
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    struct ky_security_manager *manager = wl_container_of(listener, manager, server_destroy);
    wl_list_remove(&manager->server_destroy.link);
    free(manager);
}

static void handle_display_destroy(struct wl_listener *listener, void *data)
{
    struct ky_security_manager *manager = wl_container_of(listener, manager, display_destroy);
    wl_list_remove(&manager->display_destroy.link);
    wl_global_destroy(manager->global);
}

bool ky_security_manager_create(struct server *server)
{
    struct ky_security_manager *manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return false;
    }

    manager->global = wl_global_create(server->display, &kywc_security_manager_v1_interface, 1,
                                       manager, ky_security_manager_bind);
    if (!manager->global) {
        kywc_log(KYWC_WARN, "kywc security manager create failed");
        free(manager);
        return false;
    }

    wl_list_init(&manager->surfaces);

    manager->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(server, &manager->server_destroy);
    manager->display_destroy.notify = handle_display_destroy;
    wl_display_add_destroy_listener(server->display, &manager->display_destroy);

    return true;
}
