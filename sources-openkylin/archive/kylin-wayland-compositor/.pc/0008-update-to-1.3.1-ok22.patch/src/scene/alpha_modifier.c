// SPDX-FileCopyrightText: 2024 The wlroots contributors
// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <stdlib.h>

#include <kywc/log.h>

#include "alpha-modifier-v1-protocol.h"
#include "scene/surface.h"

#define ALPHA_MODIFIER_VERSION 1

struct alpha_modifier_surface_v1_state {
    double multiplier; // between 0 and 1
};

struct alpha_modifier_v1 {
    struct wl_global *global;

    // private state

    struct wl_listener display_destroy;
};

struct alpha_modifier_surface {
    struct wl_resource *resource;
    struct wlr_surface *surface;
    struct wlr_addon addon;
    struct alpha_modifier_surface_v1_state state;
    struct wl_listener surface_commit;
};

static const struct wp_alpha_modifier_surface_v1_interface surface_impl;

// Returns NULL if the wl_surface was destroyed
static struct alpha_modifier_surface *surface_from_resource(struct wl_resource *resource)
{
    assert(
        wl_resource_instance_of(resource, &wp_alpha_modifier_surface_v1_interface, &surface_impl));
    return wl_resource_get_user_data(resource);
}

static void surface_destroy(struct alpha_modifier_surface *surface)
{
    if (surface == NULL) {
        return;
    }
    wl_list_remove(&surface->surface_commit.link);
    wlr_addon_finish(&surface->addon);
    wl_resource_set_user_data(surface->resource, NULL);
    free(surface);
}

static void surface_handle_resource_destroy(struct wl_resource *resource)
{
    struct alpha_modifier_surface *surface = surface_from_resource(resource);
    surface_destroy(surface);
}

static void surface_handle_destroy(struct wl_client *client, struct wl_resource *resource)
{
    struct alpha_modifier_surface *surface = surface_from_resource(resource);
    if (surface) {
        struct ky_scene_buffer *scene_buffer = ky_scene_buffer_try_from_surface(surface->surface);
        if (scene_buffer) {
            ky_scene_buffer_set_opacity(scene_buffer, 1.0);
        }
    }
    wl_resource_destroy(resource);
}

static void alpha_modifier_surface_handle_commit(struct wl_listener *listener, void *data)
{
    struct alpha_modifier_surface *surface = wl_container_of(listener, surface, surface_commit);
    struct ky_scene_buffer *scene_buffer = ky_scene_buffer_try_from_surface(surface->surface);
    if (!scene_buffer) {
        return;
    }
    ky_scene_buffer_set_opacity(scene_buffer, surface->state.multiplier);
    wl_list_remove(&surface->surface_commit.link);
    wl_list_init(&surface->surface_commit.link);
}

static void surface_handle_set_multiplier(struct wl_client *client, struct wl_resource *resource,
                                          uint32_t multiplier)
{
    struct alpha_modifier_surface *surface = surface_from_resource(resource);
    if (surface == NULL) {
        wl_resource_post_error(resource, WP_ALPHA_MODIFIER_SURFACE_V1_ERROR_NO_SURFACE,
                               "The wl_surface object has been destroyed");
        return;
    }
    surface->state.multiplier = (double)multiplier / UINT32_MAX;

    wl_signal_add(&surface->surface->events.commit, &surface->surface_commit);
}

static const struct wp_alpha_modifier_surface_v1_interface surface_impl = {
    .destroy = surface_handle_destroy,
    .set_multiplier = surface_handle_set_multiplier,
};

static void surface_addon_destroy(struct wlr_addon *addon)
{
    struct alpha_modifier_surface *surface = wl_container_of(addon, surface, addon);
    surface_destroy(surface);
}

static const struct wlr_addon_interface surface_addon_impl = {
    .name = "wp_alpha_modifier_surface_v1",
    .destroy = surface_addon_destroy,
};

static struct alpha_modifier_surface *surface_from_wlr_surface(struct wlr_surface *wlr_surface)
{
    struct wlr_addon *addon = wlr_addon_find(&wlr_surface->addons, NULL, &surface_addon_impl);
    if (addon == NULL) {
        return NULL;
    }
    struct alpha_modifier_surface *surface = wl_container_of(addon, surface, addon);
    return surface;
}

static void manager_handle_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static void manager_handle_get_surface(struct wl_client *client,
                                       struct wl_resource *manager_resource, uint32_t id,
                                       struct wl_resource *surface_resource)
{
    struct wlr_surface *wlr_surface = wlr_surface_from_resource(surface_resource);

    if (surface_from_wlr_surface(wlr_surface) != NULL) {
        wl_resource_post_error(
            manager_resource, WP_ALPHA_MODIFIER_V1_ERROR_ALREADY_CONSTRUCTED,
            "The wl_surface object already has a wp_alpha_modifier_surface_v1 object");
        return;
    }

    struct alpha_modifier_surface *surface = calloc(1, sizeof(*surface));
    if (surface == NULL) {
        wl_resource_post_no_memory(manager_resource);
        return;
    }

    uint32_t version = wl_resource_get_version(manager_resource);
    surface->resource =
        wl_resource_create(client, &wp_alpha_modifier_surface_v1_interface, version, id);
    if (surface->resource == NULL) {
        free(surface);
        wl_resource_post_no_memory(manager_resource);
        return;
    }
    wl_resource_set_implementation(surface->resource, &surface_impl, surface,
                                   surface_handle_resource_destroy);

    surface->surface = wlr_surface;
    surface->state.multiplier = 1.0;

    wl_list_init(&surface->surface_commit.link);
    surface->surface_commit.notify = alpha_modifier_surface_handle_commit;
    wlr_addon_init(&surface->addon, &wlr_surface->addons, NULL, &surface_addon_impl);
}

static const struct wp_alpha_modifier_v1_interface manager_impl = {
    .destroy = manager_handle_destroy,
    .get_surface = manager_handle_get_surface,
};

static void manager_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
    struct wl_resource *resource =
        wl_resource_create(client, &wp_alpha_modifier_v1_interface, version, id);
    if (resource == NULL) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &manager_impl, NULL, NULL);
}

static void handle_display_destroy(struct wl_listener *listener, void *data)
{
    struct alpha_modifier_v1 *manager = wl_container_of(listener, manager, display_destroy);
    wl_list_remove(&manager->display_destroy.link);
    wl_global_destroy(manager->global);
    free(manager);
}

bool alpha_modifier_v1_create(struct wl_display *display)
{
    struct alpha_modifier_v1 *manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return false;
    }

    manager->global = wl_global_create(display, &wp_alpha_modifier_v1_interface,
                                       ALPHA_MODIFIER_VERSION, NULL, manager_bind);
    if (!manager->global) {
        kywc_log(KYWC_WARN, "Alpha modifier manager create failed");
        free(manager);
        return false;
    }

    manager->display_destroy.notify = handle_display_destroy;
    wl_display_add_destroy_listener(display, &manager->display_destroy);

    return true;
}
