// SPDX-FileCopyrightText: 2023 The wlroots contributors
// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <stdlib.h>

#include <wlr/types/wlr_output.h>

#include "output_p.h"
#include "util/wayland.h"
#include "xdg-output-unstable-v1-protocol.h"
#include "xwayland.h"

#define OUTPUT_MANAGER_VERSION 3
#define OUTPUT_DONE_DEPRECATED_SINCE_VERSION 3
#define OUTPUT_DESCRIPTION_MUTABLE_SINCE_VERSION 3

struct xdg_output_v1 {
    struct wl_list resources;
    struct wl_list link;
    /* resource by xwayland client */
    struct wl_resource *xwayland_resource;

    struct output *output;
    struct wl_listener geometry;
    struct wl_listener disable;
    struct wl_listener description;
};

struct xdg_output_manager_v1 {
    struct wl_global *global;
    struct wl_list outputs;

    struct wl_listener new_enabled_output;
    struct wl_listener max_scale;
    struct wl_listener display_destroy;
    struct wl_listener server_destroy;
};

static void output_handle_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct zxdg_output_v1_interface output_implementation = {
    .destroy = output_handle_destroy,
};

static void output_handle_resource_destroy(struct wl_resource *resource)
{
    struct xdg_output_v1 *output = wl_resource_get_user_data(resource);
    if (output && output->xwayland_resource == resource) {
        output->xwayland_resource = NULL;
    }
    wl_list_remove(wl_resource_get_link(resource));
}

static void output_send_details(struct xdg_output_v1 *xdg_output, struct wl_resource *resource)
{
    struct kywc_box *geo = &xdg_output->output->geometry;
    int32_t x = geo->x, y = geo->y;
    int32_t width = geo->width, height = geo->height;

    if (xdg_output->xwayland_resource == resource) {
        float scale = output_manager_get_max_scale();
        x *= scale, y *= scale;
        width *= scale, height *= scale;
    }

    zxdg_output_v1_send_logical_position(resource, x, y);
    zxdg_output_v1_send_logical_size(resource, width, height);
    if (wl_resource_get_version(resource) < OUTPUT_DONE_DEPRECATED_SINCE_VERSION) {
        zxdg_output_v1_send_done(resource);
    }
}

static void xdg_output_destroy(struct xdg_output_v1 *output)
{
    struct wl_resource *resource, *tmp;
    wl_resource_for_each_safe(resource, tmp, &output->resources) {
        wl_list_remove(wl_resource_get_link(resource));
        wl_list_init(wl_resource_get_link(resource));
        wl_resource_set_user_data(resource, NULL);
    }
    wl_list_remove(&output->geometry.link);
    wl_list_remove(&output->disable.link);
    wl_list_remove(&output->description.link);
    wl_list_remove(&output->link);
    free(output);
}

static const struct zxdg_output_manager_v1_interface output_manager_implementation;

static void output_manager_handle_get_xdg_output(struct wl_client *client,
                                                 struct wl_resource *resource, uint32_t id,
                                                 struct wl_resource *output_resource)
{
    assert(wl_resource_instance_of(resource, &zxdg_output_manager_v1_interface,
                                   &output_manager_implementation));

    struct wl_resource *xdg_output_resource = wl_resource_create(
        client, &zxdg_output_v1_interface, wl_resource_get_version(resource), id);
    if (!xdg_output_resource) {
        wl_client_post_no_memory(client);
        return;
    }

    wl_resource_set_implementation(xdg_output_resource, &output_implementation, NULL,
                                   output_handle_resource_destroy);

    struct wlr_output *wlr_output = wlr_output_from_resource(output_resource);
    if (!wlr_output) {
        wl_list_init(wl_resource_get_link(xdg_output_resource));
        return;
    }

    struct output *output = output_from_wlr_output(wlr_output);
    assert(output);

    struct xdg_output_manager_v1 *manager = wl_resource_get_user_data(resource);
    struct xdg_output_v1 *_xdg_output, *xdg_output = NULL;
    wl_list_for_each(_xdg_output, &manager->outputs, link) {
        if (_xdg_output->output == output) {
            xdg_output = _xdg_output;
            break;
        }
    }
    assert(xdg_output);

    if (xwayland_check_client(client)) {
        xdg_output->xwayland_resource = xdg_output_resource;
    }
    wl_resource_set_user_data(xdg_output_resource, xdg_output);
    wl_list_insert(&xdg_output->resources, wl_resource_get_link(xdg_output_resource));

    // Name and description should only be sent once per output
    uint32_t xdg_version = wl_resource_get_version(xdg_output_resource);
    if (xdg_version >= ZXDG_OUTPUT_V1_NAME_SINCE_VERSION) {
        zxdg_output_v1_send_name(xdg_output_resource, wlr_output->name);
    }
    if (xdg_version >= ZXDG_OUTPUT_V1_DESCRIPTION_SINCE_VERSION &&
        wlr_output->description != NULL) {
        zxdg_output_v1_send_description(xdg_output_resource, wlr_output->description);
    }

    output_send_details(xdg_output, xdg_output_resource);

    uint32_t wl_version = wl_resource_get_version(output_resource);
    if (wl_version >= WL_OUTPUT_DONE_SINCE_VERSION &&
        xdg_version >= OUTPUT_DONE_DEPRECATED_SINCE_VERSION) {
        wl_output_send_done(output_resource);
    }
}

static void output_manager_handle_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct zxdg_output_manager_v1_interface output_manager_implementation = {
    .destroy = output_manager_handle_destroy,
    .get_xdg_output = output_manager_handle_get_xdg_output,
};

static void output_manager_bind(struct wl_client *wl_client, void *data, uint32_t version,
                                uint32_t id)
{
    struct wl_resource *resource =
        wl_resource_create(wl_client, &zxdg_output_manager_v1_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(wl_client);
        return;
    }

    struct xdg_output_manager_v1 *manager = data;
    wl_resource_set_implementation(resource, &output_manager_implementation, manager, NULL);
}

static void handle_output_geometry(struct wl_listener *listener, void *data)
{
    struct xdg_output_v1 *output = wl_container_of(listener, output, geometry);

    struct wl_resource *resource;
    wl_resource_for_each(resource, &output->resources) {
        output_send_details(output, resource);
    }

    wlr_output_schedule_done(output->output->wlr_output);
}

static void handle_output_disable(struct wl_listener *listener, void *data)
{
    struct xdg_output_v1 *output = wl_container_of(listener, output, disable);
    xdg_output_destroy(output);
}

static void handle_output_description(struct wl_listener *listener, void *data)
{
    struct xdg_output_v1 *xdg_output = wl_container_of(listener, xdg_output, description);
    struct wlr_output *output = xdg_output->output->wlr_output;

    if (!output->description) {
        return;
    }

    struct wl_resource *resource;
    wl_resource_for_each(resource, &xdg_output->resources) {
        if (wl_resource_get_version(resource) >= OUTPUT_DESCRIPTION_MUTABLE_SINCE_VERSION) {
            zxdg_output_v1_send_description(resource, output->description);
        }
    }
}

static void handle_new_enabled_output(struct wl_listener *listener, void *data)
{
    struct xdg_output_manager_v1 *manager = wl_container_of(listener, manager, new_enabled_output);
    struct kywc_output *kywc_output = data;
    struct output *output = output_from_kywc_output(kywc_output);

    struct xdg_output_v1 *xdg_output = calloc(1, sizeof(*xdg_output));
    if (!xdg_output) {
        return;
    }

    wl_list_init(&xdg_output->resources);
    wl_list_insert(&manager->outputs, &xdg_output->link);

    xdg_output->output = output;
    xdg_output->geometry.notify = handle_output_geometry;
    wl_signal_add(&output->events.geometry, &xdg_output->geometry);
    xdg_output->disable.notify = handle_output_disable;
    wl_signal_add(&output->events.disable, &xdg_output->disable);
    xdg_output->description.notify = handle_output_description;
    wl_signal_add(&output->wlr_output->events.description, &xdg_output->description);
}

static void handle_max_scale(struct wl_listener *listener, void *data)
{
    struct xdg_output_manager_v1 *manager = wl_container_of(listener, manager, max_scale);

    /* update xdg_output pos and size to xwayland */
    struct xdg_output_v1 *output;
    wl_list_for_each(output, &manager->outputs, link) {
        struct kywc_output *kywc_output = &output->output->base;
        if (kywc_output->destroying || !kywc_output->state.enabled) {
            continue;
        }
        if (!output->xwayland_resource) {
            continue;
        }
        output_send_details(output, output->xwayland_resource);
        /* send wl_output.done to apply the geometry in xwayland */
        struct wl_resource *resource =
            wl_resource_find_for_client(&output->output->wlr_output->resources,
                                        wl_resource_get_client(output->xwayland_resource));
        if (resource && wl_resource_get_version(resource) >= WL_OUTPUT_DONE_SINCE_VERSION) {
            wl_output_send_done(resource);
        }
    }
}

static void handle_display_destroy(struct wl_listener *listener, void *data)
{
    struct xdg_output_manager_v1 *manager = wl_container_of(listener, manager, display_destroy);
    struct xdg_output_v1 *output, *tmp;
    wl_list_for_each_safe(output, tmp, &manager->outputs, link) {
        xdg_output_destroy(output);
    }
    wl_list_remove(&manager->display_destroy.link);
    wl_list_remove(&manager->new_enabled_output.link);
    wl_list_remove(&manager->max_scale.link);
    wl_global_destroy_safe(manager->global);
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    struct xdg_output_manager_v1 *manager = wl_container_of(listener, manager, server_destroy);
    wl_list_remove(&manager->server_destroy.link);
    free(manager);
}

bool xdg_output_manager_v1_create(struct server *server)
{
    struct xdg_output_manager_v1 *manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return false;
    }

    manager->global = wl_global_create(server->display, &zxdg_output_manager_v1_interface,
                                       OUTPUT_MANAGER_VERSION, manager, output_manager_bind);
    if (!manager->global) {
        kywc_log(KYWC_WARN, "Failed to create %s global", zxdg_output_manager_v1_interface.name);
        free(manager);
        return false;
    }

    wl_list_init(&manager->outputs);

    manager->new_enabled_output.notify = handle_new_enabled_output;
    output_manager_add_new_enabled_listener(&manager->new_enabled_output);
    manager->max_scale.notify = handle_max_scale;
#if HAVE_XWAYLAND
    output_manager_add_max_scale_listener(&manager->max_scale);
#else
    wl_list_init(&manager->max_scale.link);
#endif
    manager->display_destroy.notify = handle_display_destroy;
    wl_display_add_destroy_listener(server->display, &manager->display_destroy);
    manager->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(server, &manager->server_destroy);

    return true;
}
