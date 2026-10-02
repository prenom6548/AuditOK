// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <wlr/types/wlr_xdg_shell.h>

#include "view_p.h"
#include "xdg-toplevel-tag-v1-protocol.h"

#define XDG_TOPLEVEL_TAG_MANAGER_VERSION 1

struct xdg_toplevel_tag_manager {
    struct wl_global *global;

    struct wl_listener display_destroy;
    struct wl_listener server_destroy;
};

static void manager_handle_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static void manager_handle_set_tag(struct wl_client *client, struct wl_resource *manager_resource,
                                   struct wl_resource *toplevel_resource, const char *tag)
{
    struct wlr_xdg_toplevel *toplevel = wlr_xdg_toplevel_from_resource(toplevel_resource);
    if (!toplevel) {
        return;
    }

    struct view *view = view_try_from_wlr_surface(toplevel->base->surface);
    if (!view || (view->tag && strcmp(view->tag, tag) == 0)) {
        return;
    }

    free(view->tag);
    view->tag = strdup(tag);
}

static void manager_handle_set_description(struct wl_client *client,
                                           struct wl_resource *manager_resource,
                                           struct wl_resource *toplevel_resource,
                                           const char *description)
{
    struct wlr_xdg_toplevel *toplevel = wlr_xdg_toplevel_from_resource(toplevel_resource);
    if (!toplevel) {
        return;
    }

    struct view *view = view_try_from_wlr_surface(toplevel->base->surface);
    if (!view || (view->description && strcmp(view->description, description) == 0)) {
        return;
    }

    free(view->description);
    view->description = strdup(description);
}

static const struct xdg_toplevel_tag_manager_v1_interface xdg_toplevel_tag_manager_impl = {
    .destroy = manager_handle_destroy,
    .set_toplevel_tag = manager_handle_set_tag,
    .set_toplevel_description = manager_handle_set_description,
};

static void xdg_toplevel_tag_manager_bind(struct wl_client *client, void *data, uint32_t version,
                                          uint32_t id)
{
    struct xdg_toplevel_tag_manager *manager = data;

    struct wl_resource *resource =
        wl_resource_create(client, &xdg_toplevel_tag_manager_v1_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }

    wl_resource_set_implementation(resource, &xdg_toplevel_tag_manager_impl, manager, NULL);
}

static void handle_display_destroy(struct wl_listener *listener, void *data)
{
    struct xdg_toplevel_tag_manager *manager = wl_container_of(listener, manager, display_destroy);
    wl_list_remove(&manager->display_destroy.link);
    wl_global_destroy(manager->global);
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    struct xdg_toplevel_tag_manager *manager = wl_container_of(listener, manager, server_destroy);
    wl_list_remove(&manager->server_destroy.link);
    free(manager);
}

bool xdg_toplevel_tag_manager_create(struct server *server)
{
    struct xdg_toplevel_tag_manager *manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return false;
    }

    manager->global =
        wl_global_create(server->display, &xdg_toplevel_tag_manager_v1_interface,
                         XDG_TOPLEVEL_TAG_MANAGER_VERSION, manager, xdg_toplevel_tag_manager_bind);
    if (!manager->global) {
        kywc_log(KYWC_WARN, "Xdg toplevel tag manager create failed");
        free(manager);
        return false;
    }

    manager->display_destroy.notify = handle_display_destroy;
    wl_display_add_destroy_listener(server->display, &manager->display_destroy);
    manager->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(server, &manager->server_destroy);

    return true;
}
