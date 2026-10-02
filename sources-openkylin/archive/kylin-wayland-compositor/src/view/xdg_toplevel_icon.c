// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <wlr/types/wlr_xdg_shell.h>

#include "output.h"
#include "theme.h"
#include "util/macros.h"
#include "view_p.h"
#include "xdg-toplevel-icon-v1-protocol.h"

#define XDG_TOPLEVEL_ICON_MANAGER_VERSION 1

struct xdg_toplevel_icon_manager {
    struct wl_global *global;

    struct wl_listener display_destroy;
    struct wl_listener server_destroy;
};

struct xdg_toplevel_icon_buffer {
    struct wl_list link;

    int scale;
    struct wlr_buffer *wlr_buffer;
};

struct xdg_toplevel_icon {
    char *name;             // may be NULL
    struct wl_list buffers; // xdg_toplevel_icon_buffer.link

    int n_refs;
    bool immutable;
};

struct xdg_icon_toplevel {
    struct xdg_toplevel_icon *icon;
    struct wlr_xdg_toplevel *toplevel;

    struct wl_listener surface_commit;
    struct wl_listener toplevel_destroy;
};

static const struct xdg_toplevel_icon_v1_interface icon_impl;

static struct xdg_toplevel_icon *icon_from_resource(struct wl_resource *resource)
{
    assert(wl_resource_instance_of(resource, &xdg_toplevel_icon_v1_interface, &icon_impl));
    return wl_resource_get_user_data(resource);
}

static void icon_destroy(struct xdg_toplevel_icon *icon)
{
    struct xdg_toplevel_icon_buffer *icon_buffer, *tmp;
    wl_list_for_each_safe(icon_buffer, tmp, &icon->buffers, link) {
        wlr_buffer_unlock(icon_buffer->wlr_buffer);
        wl_list_remove(&icon_buffer->link);
        free(icon_buffer);
    }

    free(icon->name);
    free(icon);
}

static void icon_handle_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static void icon_handle_set_name(struct wl_client *client, struct wl_resource *resource,
                                 const char *name)
{
    struct xdg_toplevel_icon *icon = icon_from_resource(resource);
    if (!icon) {
        return;
    }
    if (icon->immutable) {
        wl_resource_post_error(
            resource, XDG_TOPLEVEL_ICON_V1_ERROR_IMMUTABLE,
            "the icon has already been assigned to a toplevel and must not be changed");
        return;
    }

    char *tmp = strdup(name);
    if (!tmp) {
        wl_resource_post_no_memory(resource);
        return;
    }

    free(icon->name);
    icon->name = tmp;
}

static void icon_handle_add_buffer(struct wl_client *client, struct wl_resource *resource,
                                   struct wl_resource *buffer_resource, int32_t scale)
{
    struct xdg_toplevel_icon *icon = icon_from_resource(resource);
    if (!icon) {
        return;
    }
    if (icon->immutable) {
        wl_resource_post_error(
            resource, XDG_TOPLEVEL_ICON_V1_ERROR_IMMUTABLE,
            "the icon has already been assigned to a toplevel and must not be changed");
        return;
    }

    struct wlr_buffer *wlr_buffer = wlr_buffer_try_from_resource(buffer_resource);
    const char *bad_buffer_msg = NULL;
    struct wlr_shm_attributes shm_attribs;
    if (!wlr_buffer_get_shm(wlr_buffer, &shm_attribs)) {
        bad_buffer_msg = "not backed by wl_shm";
    } else if (wlr_buffer->width != wlr_buffer->height) {
        bad_buffer_msg = "not square";
    }

    if (bad_buffer_msg) {
        wl_resource_post_error(resource, XDG_TOPLEVEL_ICON_V1_ERROR_INVALID_BUFFER,
                               "the provided buffer does not satisfy requirements: %s",
                               bad_buffer_msg);
        wlr_buffer_unlock(wlr_buffer);
        return;
    }

    struct xdg_toplevel_icon_buffer *icon_buffer;
    wl_list_for_each(icon_buffer, &icon->buffers, link) {
        if (icon_buffer->wlr_buffer->width == wlr_buffer->width && icon_buffer->scale == scale) {
            wlr_buffer_unlock(icon_buffer->wlr_buffer);
            icon_buffer->wlr_buffer = wlr_buffer;
            return;
        }
    }

    icon_buffer = calloc(1, sizeof(*icon_buffer));
    if (!icon_buffer) {
        wl_resource_post_no_memory(resource);
    }

    icon_buffer->wlr_buffer = wlr_buffer;
    icon_buffer->scale = scale;
    wl_list_insert(&icon->buffers, &icon_buffer->link);
}

static const struct xdg_toplevel_icon_v1_interface icon_impl = {
    .destroy = icon_handle_destroy,
    .set_name = icon_handle_set_name,
    .add_buffer = icon_handle_add_buffer,
};

static struct xdg_toplevel_icon *xdg_toplevel_icon_ref(struct xdg_toplevel_icon *icon)
{
    ++icon->n_refs;
    return icon;
}

static void xdg_toplevel_icon_unref(struct xdg_toplevel_icon *icon)
{
    if (icon == NULL) {
        return;
    }

    assert(icon->n_refs > 0);
    --icon->n_refs;
    if (icon->n_refs == 0) {
        icon_destroy(icon);
    }
}

static void icon_handle_resource_destroy(struct wl_resource *resource)
{
    xdg_toplevel_icon_unref(icon_from_resource(resource));
}

static void manager_handle_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static void manager_handle_create_icon(struct wl_client *client, struct wl_resource *resource,
                                       uint32_t id)
{
    struct xdg_toplevel_icon *icon = calloc(1, sizeof(*icon));
    if (!icon) {
        wl_client_post_no_memory(client);
        return;
    }

    struct wl_resource *icon_resource = wl_resource_create(client, &xdg_toplevel_icon_v1_interface,
                                                           wl_resource_get_version(resource), id);
    if (!icon_resource) {
        wl_client_post_no_memory(client);
        free(icon);
        return;
    }

    icon->n_refs = 1;
    wl_list_init(&icon->buffers);
    wl_resource_set_implementation(icon_resource, &icon_impl, icon, icon_handle_resource_destroy);
}

static void icon_toplevel_handle_surface_commit(struct wl_listener *listener, void *data)
{
    struct xdg_icon_toplevel *icon_toplevel =
        wl_container_of(listener, icon_toplevel, surface_commit);

    struct xdg_toplevel_icon *icon = icon_toplevel->icon;
    struct wlr_xdg_toplevel *toplevel = icon_toplevel->toplevel;
    // surface commit, the view must have a value
    struct view *view = view_try_from_wlr_surface(toplevel->base->surface);
    if (icon) {
        view_set_icon_name(view, icon->name);
        struct xdg_toplevel_icon_buffer *icon_buffer;
        wl_list_for_each(icon_buffer, &icon->buffers, link) {
            xdg_view_add_icon_buffer(toplevel->base, icon_buffer->scale, icon_buffer->wlr_buffer);
        }
        xdg_toplevel_icon_unref(icon);
    } else {
        view_set_icon_name(view, NULL);
        xdg_view_clear_icon_buffer(toplevel->base);
    }
    view_set_icon(view, true);

    wl_list_remove(&icon_toplevel->surface_commit.link);
    wl_list_remove(&icon_toplevel->toplevel_destroy.link);
    free(icon_toplevel);
}

static void icon_toplevel_handle_toplevel_destroy(struct wl_listener *listener, void *data)
{
    struct xdg_icon_toplevel *icon_toplevel =
        wl_container_of(listener, icon_toplevel, toplevel_destroy);

    if (icon_toplevel->icon) {
        xdg_toplevel_icon_unref(icon_toplevel->icon);
    }
    wl_list_remove(&icon_toplevel->surface_commit.link);
    wl_list_remove(&icon_toplevel->toplevel_destroy.link);
    free(icon_toplevel);
}

static void manager_handle_set_icon(struct wl_client *client, struct wl_resource *resource,
                                    struct wl_resource *toplevel_resource,
                                    struct wl_resource *icon_resource)
{
    struct wlr_xdg_toplevel *toplevel = wlr_xdg_toplevel_from_resource(toplevel_resource);
    if (!toplevel) {
        return;
    }

    struct xdg_icon_toplevel *icon_toplevel = calloc(1, sizeof(*icon_toplevel));
    if (!icon_toplevel) {
        wl_client_post_no_memory(client);
        return;
    }

    struct xdg_toplevel_icon *icon = icon_resource ? icon_from_resource(icon_resource) : NULL;
    if (icon) {
        icon->immutable = true;
        if (!icon->name && wl_list_empty(&icon->buffers)) {
            // Same as supplying null icon
            icon = NULL;
        }
    }

    if (icon) {
        icon_toplevel->icon = xdg_toplevel_icon_ref(icon);
    }
    icon_toplevel->toplevel = toplevel;
    icon_toplevel->surface_commit.notify = icon_toplevel_handle_surface_commit;
    wl_signal_add(&toplevel->base->surface->events.commit, &icon_toplevel->surface_commit);
    icon_toplevel->toplevel_destroy.notify = icon_toplevel_handle_toplevel_destroy;
    wl_signal_add(&toplevel->events.destroy, &icon_toplevel->toplevel_destroy);
}

static const struct xdg_toplevel_icon_manager_v1_interface xdg_toplevel_icon_manager_impl = {
    .destroy = manager_handle_destroy,
    .create_icon = manager_handle_create_icon,
    .set_icon = manager_handle_set_icon,
};

static bool get_output_scale_icon_size(struct kywc_output *output, int index, void *data)
{
    struct wl_array *sizes = data;
    struct theme *theme = theme_manager_get_theme();
    float theme_icon_size = theme->icon_size * output->state.scale;

    uint32_t *size;
    wl_array_for_each(size, sizes) {
        if (FLOAT_EQUAL(*size, theme_icon_size)) {
            return false;
        }
    }

    uint32_t *icon_size = wl_array_add(sizes, sizeof(*icon_size));
    if (icon_size) {
        *icon_size = theme_icon_size;
    }
    return false;
}

static void xdg_toplevel_icon_manager_bind(struct wl_client *client, void *data, uint32_t version,
                                           uint32_t id)
{
    struct xdg_toplevel_icon_manager *manager = data;

    struct wl_resource *resource =
        wl_resource_create(client, &xdg_toplevel_icon_manager_v1_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }

    wl_resource_set_implementation(resource, &xdg_toplevel_icon_manager_impl, manager, NULL);

    struct wl_array sizes;
    wl_array_init(&sizes);
    output_manager_for_each_output(get_output_scale_icon_size, true, &sizes);
    uint32_t *size;
    wl_array_for_each(size, &sizes) {
        xdg_toplevel_icon_manager_v1_send_icon_size(resource, *size);
    }
    wl_array_release(&sizes);
    xdg_toplevel_icon_manager_v1_send_done(resource);
}

static void manager_handle_display_destroy(struct wl_listener *listener, void *data)
{
    struct xdg_toplevel_icon_manager *manager = wl_container_of(listener, manager, display_destroy);
    wl_list_remove(&manager->display_destroy.link);
    wl_global_destroy(manager->global);
}

static void manager_handle_server_destroy(struct wl_listener *listener, void *data)
{
    struct xdg_toplevel_icon_manager *manager = wl_container_of(listener, manager, server_destroy);
    wl_list_remove(&manager->server_destroy.link);
    free(manager);
}

bool xdg_toplevel_icon_manager_create(struct server *server)
{
    struct xdg_toplevel_icon_manager *manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return false;
    }

    manager->global = wl_global_create(server->display, &xdg_toplevel_icon_manager_v1_interface,
                                       XDG_TOPLEVEL_ICON_MANAGER_VERSION, manager,
                                       xdg_toplevel_icon_manager_bind);
    if (!manager->global) {
        kywc_log(KYWC_WARN, "Xdg toplevel icon manager create failed");
        free(manager);
        return false;
    }

    manager->display_destroy.notify = manager_handle_display_destroy;
    wl_display_add_destroy_listener(server->display, &manager->display_destroy);
    manager->server_destroy.notify = manager_handle_server_destroy;
    server_add_destroy_listener(server, &manager->server_destroy);

    return true;
}
