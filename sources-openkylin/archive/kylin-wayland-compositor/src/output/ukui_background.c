// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>

#include <wayland-server-core.h>
#include <wayland-util.h>

#include <kywc/log.h>

#include "output_p.h"
#include "scene/scene.h"
#include "server.h"
#include "ukui-background-v1-protocol.h"
#include "util/macros.h"

#define UKUI_BACKGROUND_MANAGER_VERSION 1

struct ukui_background {
    struct wl_list link;
    struct wl_resource *resource;

    struct wlr_output *wlr_output;
    struct wlr_buffer *buffer;

    struct wl_listener output_destroy;
    struct wl_listener output_commit;

    enum ukui_background_manager_v1_type type;
    bool buffer_updated, buffer_used;
};

struct ukui_background_manager {
    struct wl_global *global;
    struct wl_list backgrounds;

    struct wl_listener display_destroy;
    struct wl_listener server_destroy;
} *manager = NULL;

struct wlr_buffer *output_get_background_buffer(struct wlr_output *output, uint32_t background_type,
                                                bool *updated)
{
    struct ukui_background *background;
    wl_list_for_each(background, &manager->backgrounds, link) {
        if (background->wlr_output == output && background->type == background_type) {
            if (updated) {
                *updated = background->buffer_updated;
                background->buffer_updated = false;
            }
            background->buffer_used = true;
            return background->buffer;
        }
    }

    if (updated) {
        *updated = false;
    }
    return NULL;
}

static void ukui_background_destroy(struct ukui_background *background)
{
    wl_list_remove(&background->output_destroy.link);
    wl_list_remove(&background->output_commit.link);
    wl_list_remove(&background->link);

    struct output *output = output_from_wlr_output(background->wlr_output);
    if (output && output->scene_output) {
        scene_output_damage_whole(output->scene_output);
    }

    if (background->buffer) {
        wlr_buffer_unlock(background->buffer);
    }
    free(background);
}

static void ukui_background_handle_output_commit(struct wl_listener *listener, void *data)
{
    struct ukui_background *background = wl_container_of(listener, background, output_commit);
    background->buffer_updated = background->buffer_used ? false : background->buffer_updated;
}

static void ukui_background_handle_output_destroy(struct wl_listener *listener, void *data)
{
    struct ukui_background *background = wl_container_of(listener, background, output_destroy);

    wl_resource_set_user_data(background->resource, NULL);
    wl_resource_set_destructor(background->resource, NULL);

    ukui_background_v1_send_failed(background->resource);
    ukui_background_destroy(background);
}

static bool check_buffer_valid(struct wlr_buffer *wlr_buffer, struct wlr_output *wlr_output)
{
    float width, height;
    if (wlr_output->transform % 2 == 0) {
        width = wlr_output->width;
        height = wlr_output->height;
    } else {
        width = wlr_output->height;
        height = wlr_output->width;
    }

    return FLOAT_EQUAL(wlr_buffer->width / width, wlr_buffer->height / height);
}

static void handle_ukui_background_set_buffer(struct wl_client *client,
                                              struct wl_resource *resource,
                                              struct wl_resource *buffer)
{
    struct ukui_background *background = wl_resource_get_user_data(resource);
    if (!background) {
        return;
    }

    struct wlr_buffer *wlr_buffer = wlr_buffer_try_from_resource(buffer);
    if (!wlr_buffer) {
        return;
    }

    if (!check_buffer_valid(wlr_buffer, background->wlr_output)) {
        kywc_log(KYWC_DEBUG, "buffer size is not match output size");
    }

    if (background->buffer) {
        wlr_buffer_unlock(background->buffer);
    }

    background->buffer = wlr_buffer;
    background->buffer_updated = true;
    background->buffer_used = false;

    struct output *output = output_from_wlr_output(background->wlr_output);
    scene_output_damage_whole(output->scene_output);
}

static void handle_ukui_background_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct ukui_background_v1_interface ukui_background_v1_impl = {
    .destroy = handle_ukui_background_destroy,
    .set_buffer = handle_ukui_background_set_buffer,
};

static void ukui_background_resource_destroy(struct wl_resource *resource)
{
    struct ukui_background *background = wl_resource_get_user_data(resource);
    if (background) {
        ukui_background_destroy(background);
    }
}

static void handle_background_manager_create_background(struct wl_client *client,
                                                        struct wl_resource *manager_resource,
                                                        uint32_t id, uint32_t background_type,
                                                        struct wl_resource *output)
{
    int version = wl_resource_get_version(manager_resource);
    struct wl_resource *resource =
        wl_resource_create(client, &ukui_background_v1_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }

    wl_resource_set_implementation(resource, &ukui_background_v1_impl, NULL,
                                   ukui_background_resource_destroy);

    if (background_type != UKUI_BACKGROUND_MANAGER_V1_TYPE_WALLPAPER &&
        background_type != UKUI_BACKGROUND_MANAGER_V1_TYPE_SCREENLOCK) {
        ukui_background_v1_send_failed(resource);
        return;
    }

    struct wlr_output *wlr_output = wlr_output_from_resource(output);
    if (!wlr_output) {
        ukui_background_v1_send_failed(resource);
        return;
    }

    /* check if this type background already exists for this output */
    struct ukui_background *existing;
    wl_list_for_each(existing, &manager->backgrounds, link) {
        if (existing->wlr_output == wlr_output && existing->type == background_type) {
            wl_resource_post_error(manager_resource,
                                   UKUI_BACKGROUND_MANAGER_V1_ERROR_BACKGROUND_EXISTS,
                                   "This type background already exists for this output");
            return;
        }
    }

    struct ukui_background *background = calloc(1, sizeof(*background));
    if (!background) {
        wl_client_post_no_memory(client);
        return;
    }

    background->type = background_type;
    background->wlr_output = wlr_output;
    background->resource = resource;
    wl_resource_set_user_data(resource, background);

    wl_list_insert(&manager->backgrounds, &background->link);

    background->output_destroy.notify = ukui_background_handle_output_destroy;
    wl_signal_add(&wlr_output->events.destroy, &background->output_destroy);
    background->output_commit.notify = ukui_background_handle_output_commit;
    wl_signal_add(&wlr_output->events.commit, &background->output_commit);
}

static void handle_background_manager_destroy(struct wl_client *client,
                                              struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct ukui_background_manager_v1_interface ukui_background_manager_v1_impl = {
    .destroy = handle_background_manager_destroy,
    .create_background = handle_background_manager_create_background,
};

static void ukui_background_manager_bind(struct wl_client *client, void *data, uint32_t version,
                                         uint32_t id)
{
    struct wl_resource *resource =
        wl_resource_create(client, &ukui_background_manager_v1_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }

    wl_resource_set_implementation(resource, &ukui_background_manager_v1_impl, manager, NULL);
}

static void handle_display_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&manager->display_destroy.link);
    wl_global_destroy(manager->global);
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&manager->server_destroy.link);
    free(manager);
    manager = NULL;
}

bool ukui_background_manager_create(struct server *server)
{
    manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return false;
    }

    manager->global =
        wl_global_create(server->display, &ukui_background_manager_v1_interface,
                         UKUI_BACKGROUND_MANAGER_VERSION, manager, ukui_background_manager_bind);
    if (!manager->global) {
        kywc_log(KYWC_WARN, "UKUI background manager create failed");
        free(manager);
        manager = NULL;
        return false;
    }

    wl_list_init(&manager->backgrounds);
    manager->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(server, &manager->server_destroy);
    manager->display_destroy.notify = handle_display_destroy;
    wl_display_add_destroy_listener(server->display, &manager->display_destroy);

    return true;
}
