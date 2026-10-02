// SPDX-FileCopyrightText: 2024 The wlroots contributors
// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <stdlib.h>

#include <kywc/log.h>

#include "scene_p.h"
#include "tearing-control-v1-protocol.h"

#define TEARING_CONTROL_MANAGER_VERSION 1

struct tearing_control_v1 {
    struct wl_list link;
    struct wl_resource *resource;

    struct wlr_surface *surface;
    struct wl_listener surface_commit;
    struct wlr_addon addon;

    enum wp_tearing_control_v1_presentation_hint current, pending;
};

static struct tearing_control_manager_v1 {
    struct wl_global *global;
    struct wl_listener display_destroy;

    struct wl_list surface_hints;
} *manager = NULL;

static const struct wp_tearing_control_v1_interface tearing_control_impl;

static struct tearing_control_v1 *tearing_surface_hint_from_resource(struct wl_resource *resource)
{
    assert(
        wl_resource_instance_of(resource, &wp_tearing_control_v1_interface, &tearing_control_impl));
    return wl_resource_get_user_data(resource);
}

static void tearing_control_v1_destroy(struct tearing_control_v1 *hint)
{
    if (!hint) {
        return;
    }

    wl_resource_set_user_data(hint->resource, NULL);
    wl_resource_set_destructor(hint->resource, NULL);
    wl_list_remove(&hint->link);

    wlr_addon_finish(&hint->addon);
    wl_list_remove(&hint->surface_commit.link);

    free(hint);
}

static void surface_addon_destroy(struct wlr_addon *addon)
{
    struct tearing_control_v1 *hint = wl_container_of(addon, hint, addon);
    tearing_control_v1_destroy(hint);
}

static const struct wlr_addon_interface surface_addon_impl = {
    .name = "wp_tearing_control_v1",
    .destroy = surface_addon_destroy,
};

static void resource_handle_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static void tearing_control_handle_resource_destroy(struct wl_resource *resource)
{
    struct tearing_control_v1 *hint = tearing_surface_hint_from_resource(resource);
    tearing_control_v1_destroy(hint);
}

static void tearing_control_handle_set_presentation_hint(struct wl_client *client,
                                                         struct wl_resource *resource,
                                                         uint32_t hint)
{
    struct tearing_control_v1 *surface_hint = tearing_surface_hint_from_resource(resource);
    if (surface_hint) {
        surface_hint->pending = hint;
    }
}

static const struct wp_tearing_control_v1_interface tearing_control_impl = {
    .destroy = resource_handle_destroy,
    .set_presentation_hint = tearing_control_handle_set_presentation_hint,
};

static void hint_handle_surface_commit(struct wl_listener *listener, void *data)
{
    struct tearing_control_v1 *hint = wl_container_of(listener, hint, surface_commit);
    hint->current = hint->pending;
}

static void tearing_handle_get_tearing_control(struct wl_client *client,
                                               struct wl_resource *resource, uint32_t id,
                                               struct wl_resource *surface_resource)
{
    struct wlr_surface *surface = wlr_surface_from_resource(surface_resource);
    if (wlr_addon_find(&surface->addons, manager, &surface_addon_impl)) {
        wl_resource_post_error(resource, WP_TEARING_CONTROL_MANAGER_V1_ERROR_TEARING_CONTROL_EXISTS,
                               "Tearing control object already exists!");
        return;
    }

    struct tearing_control_v1 *hint = calloc(1, sizeof(*hint));
    if (!hint) {
        wl_client_post_no_memory(client);
        return;
    }

    hint->resource = wl_resource_create(client, &wp_tearing_control_v1_interface,
                                        wl_resource_get_version(resource), id);
    if (!hint->resource) {
        wl_resource_post_no_memory(resource);
        free(hint);
        return;
    }

    wl_list_insert(&manager->surface_hints, &hint->link);
    wl_resource_set_implementation(hint->resource, &tearing_control_impl, hint,
                                   tearing_control_handle_resource_destroy);

    hint->surface = surface;
    wlr_addon_init(&hint->addon, &hint->surface->addons, manager, &surface_addon_impl);
    hint->surface_commit.notify = hint_handle_surface_commit;
    wl_signal_add(&surface->events.commit, &hint->surface_commit);
}

static const struct wp_tearing_control_manager_v1_interface tearing_impl = {
    .destroy = resource_handle_destroy,
    .get_tearing_control = tearing_handle_get_tearing_control,
};

static void tearing_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
    struct wl_resource *resource =
        wl_resource_create(client, &wp_tearing_control_manager_v1_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &tearing_impl, manager, NULL);
}

static void handle_display_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&manager->display_destroy.link);

    struct tearing_control_v1 *hint, *tmp;
    wl_list_for_each_safe(hint, tmp, &manager->surface_hints, link) {
        tearing_control_v1_destroy(hint);
    }

    wl_global_destroy(manager->global);
    free(manager);
    manager = NULL;
}

bool tearing_control_manager_v1_create(struct wl_display *display)
{
    manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return false;
    }

    manager->global = wl_global_create(display, &wp_tearing_control_manager_v1_interface,
                                       TEARING_CONTROL_MANAGER_VERSION, manager, tearing_bind);
    if (!manager->global) {
        kywc_log(KYWC_WARN, "WP tearing control manager create failed");
        free(manager);
        manager = NULL;
        return false;
    }

    wl_list_init(&manager->surface_hints);
    manager->display_destroy.notify = handle_display_destroy;
    wl_display_add_destroy_listener(display, &manager->display_destroy);

    return true;
}

bool scene_surface_is_tearing_allowed(struct scene *scene, struct wlr_surface *surface)
{
    if (!manager || !surface) {
        return false;
    }

    struct wlr_addon *addon = wlr_addon_find(&surface->addons, manager, &surface_addon_impl);
    if (!addon) {
        return false;
    }

    struct tearing_control_v1 *hint = wl_container_of(addon, hint, addon);
    return hint->current == WP_TEARING_CONTROL_V1_PRESENTATION_HINT_ASYNC;
}

bool scene_is_tearing_needed(struct scene *scene)
{
    if (!manager) {
        return false;
    }

    struct tearing_control_v1 *hint;
    wl_list_for_each(hint, &manager->surface_hints, link) {
        if (hint->current == WP_TEARING_CONTROL_V1_PRESENTATION_HINT_ASYNC) {
            return true;
        }
    }

    return false;
}
