// SPDX-FileCopyrightText: 2023 The wlroots contributors
// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <stdlib.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_fractional_scale_v1.h>

#include "output.h"
#include "scene/surface.h"
#include "scene_p.h"
#include "util/macros.h"
#include "util/wayland.h"

static double get_surface_preferred_buffer_scale(struct wlr_surface *surface)
{
    double scale = 1;
    struct wlr_surface_output *surface_output;
    wl_list_for_each(surface_output, &surface->current_outputs, link) {
        if (surface_output->output->scale > scale) {
            scale = surface_output->output->scale;
        }
    }
    return scale;
}

static void handle_scene_buffer_outputs_update(struct wl_listener *listener, void *data)
{
    struct scene_surface *surface = wl_container_of(listener, surface, outputs_update);

    double scale = get_surface_preferred_buffer_scale(surface->surface);
    wlr_fractional_scale_v1_notify_scale(surface->surface, scale);
    wlr_surface_set_preferred_buffer_scale(surface->surface, ceil(scale));
}

static void handle_scene_buffer_output_enter(struct wl_listener *listener, void *data)
{
    struct scene_surface *surface = wl_container_of(listener, surface, output_enter);
    struct scene_output *output = data;

    wlr_surface_send_enter(surface->surface, output->output);
}

static void handle_scene_buffer_output_leave(struct wl_listener *listener, void *data)
{
    struct scene_surface *surface = wl_container_of(listener, surface, output_leave);
    struct scene_output *output = data;

    wlr_surface_send_leave(surface->surface, output->output);
}

static void handle_scene_buffer_output_sample(struct wl_listener *listener, void *data)
{
    struct scene_surface *surface = wl_container_of(listener, surface, output_sample);
    const struct scene_output_sample_event *event = data;
    struct scene_output *scene_output = event->output;

    if (surface->buffer->primary_output != scene_output) {
        return;
    }

    scene_surface_presented_on_output(surface->surface, scene_output->output,
                                      event->direct_scanout);
}

static void handle_scene_buffer_frame_done(struct wl_listener *listener, void *data)
{
    struct scene_surface *surface = wl_container_of(listener, surface, frame_done);
    struct timespec *now = data;

    wlr_surface_send_frame_done(surface->surface, now);
}

static void scene_surface_handle_surface_destroy(struct wl_listener *listener, void *data)
{
    struct scene_surface *surface = wl_container_of(listener, surface, surface_destroy);

    scene_node_destroy(&surface->buffer->node);
}

// This is used for wlr_scene where it unconditionally locks buffers preventing
// reuse of the existing texture for shm clients. With the usage pattern of
// wlr_scene surface handling, we can mark its locked buffer as safe
// for mutation.
static void client_buffer_mark_next_can_damage(struct wlr_client_buffer *buffer)
{
    buffer->n_ignore_locks++;
}

static void scene_buffer_unmark_client_buffer(struct scene_buffer *scene_buffer)
{
    if (!scene_buffer->buffer) {
        return;
    }

    struct wlr_client_buffer *buffer = wlr_client_buffer_get(scene_buffer->buffer);
    if (!buffer) {
        return;
    }

    assert(buffer->n_ignore_locks > 0);
    buffer->n_ignore_locks--;
}

static void set_buffer_with_surface_state(struct scene_buffer *scene_buffer,
                                          struct wlr_surface *surface, uint32_t committed)
{
    struct wlr_surface_state *state = &surface->current;

    /* don't use surface->opaque_region */
    if (committed & WLR_SURFACE_STATE_OPAQUE_REGION) {
        scene_buffer_set_opaque_region(scene_buffer, &state->opaque);
    }

    if (committed & (WLR_SURFACE_STATE_VIEWPORT | WLR_SURFACE_STATE_BUFFER |
                     WLR_SURFACE_STATE_SCALE | WLR_SURFACE_STATE_TRANSFORM)) {
        struct wlr_fbox src_box;
        wlr_surface_get_buffer_source_box(surface, &src_box);
        scene_buffer_set_source_box(scene_buffer, &src_box);
        scene_buffer_set_dest_size(scene_buffer, state->width, state->height);
    }

    if (committed & WLR_SURFACE_STATE_TRANSFORM) {
        scene_buffer_set_transform(scene_buffer, state->transform);
    }

    scene_buffer_unmark_client_buffer(scene_buffer);
    if (surface->buffer) {
        client_buffer_mark_next_can_damage(surface->buffer);
    }

    if (committed & WLR_SURFACE_STATE_BUFFER) {
        if (surface->buffer) {
            scene_buffer_set_buffer_with_damage(scene_buffer, &surface->buffer->base,
                                                &surface->buffer_damage);
        } else if (surface->current.buffer) {
            scene_buffer_set_buffer_with_damage(scene_buffer, surface->current.buffer,
                                                &surface->buffer_damage);
        } else {
            scene_buffer_set_buffer(scene_buffer, NULL);
        }
    } else if (committed & (WLR_SURFACE_STATE_SURFACE_DAMAGE | WLR_SURFACE_STATE_BUFFER_DAMAGE) &&
               scene_buffer->buffer) {
        scene_buffer_set_buffer_with_damage(scene_buffer, scene_buffer->buffer,
                                            &surface->buffer_damage);
    }
}

static void handle_scene_surface_surface_commit(struct wl_listener *listener, void *data)
{
    struct scene_surface *surface = wl_container_of(listener, surface, surface_commit);
    struct scene_buffer *scene_buffer = surface->buffer;

    set_buffer_with_surface_state(scene_buffer, surface->surface,
                                  surface->surface->current.committed);

    // If the surface has requested a frame done event, honour that. The
    // frame_callback_list will be populated in this case. We should only
    // schedule the frame however if the node is enabled and there is an
    // output intersecting, otherwise the frame done events would never reach
    // the surface anyway.
    bool enabled = scene_node_is_enabled(&scene_buffer->node);

    struct scene_output *primary_output = surface->buffer->primary_output;
    if (!wl_list_empty(&surface->surface->current.frame_callback_list) && primary_output != NULL &&
        enabled) {
        output_schedule_frame(primary_output->output);
    }
}

static void handle_scene_surface_surface_map(struct wl_listener *listener, void *data)
{
    struct scene_surface *surface = wl_container_of(listener, surface, surface_map);

    set_buffer_with_surface_state(surface->buffer, surface->surface,
                                  surface->surface->current.committed);
}

static bool scene_buffer_point_accepts_input(struct scene_buffer *scene_buffer, double *sx,
                                             double *sy)
{
    struct scene_surface *scene_surface = scene_surface_try_from_buffer(scene_buffer);

    return wlr_surface_point_accepts_input(scene_surface->surface, *sx, *sy);
}

static void surface_addon_destroy(struct wlr_addon *addon)
{
    struct scene_surface *surface = wl_container_of(addon, surface, addon);

    scene_buffer_unmark_client_buffer(surface->buffer);

    wlr_addon_finish(&surface->addon);
    wlr_addon_finish(&surface->node_addon);

    wl_list_remove(&surface->outputs_update.link);
    wl_list_remove(&surface->output_enter.link);
    wl_list_remove(&surface->output_leave.link);
    wl_list_remove(&surface->output_sample.link);
    wl_list_remove(&surface->frame_done.link);
    wl_list_remove(&surface->surface_destroy.link);
    wl_list_remove(&surface->surface_commit.link);
    wl_list_remove(&surface->surface_map.link);

    free(surface);
}

static const struct wlr_addon_interface surface_addon_impl = {
    .name = "scene_surface",
    .destroy = surface_addon_destroy,
};

struct scene_surface *scene_surface_try_from_buffer(struct scene_buffer *scene_buffer)
{
    struct wlr_addon *addon =
        wlr_addon_find(&scene_buffer->node.addons, scene_buffer, &surface_addon_impl);
    if (!addon) {
        return NULL;
    }

    struct scene_surface *surface = wl_container_of(addon, surface, addon);
    return surface;
}

static void surface_node_addon_destroy(struct wlr_addon *addon)
{
    /* do nothing, surface destroy signal emitted before surface addon_set finish
     * scene node destroy will call surface_addon_destroy.
     */
}

static const struct wlr_addon_interface surface_node_addon_impl = {
    .name = "scene_surface_node",
    .destroy = surface_node_addon_destroy,
};

static struct scene_surface *scene_surface_try_from_surface(struct wlr_surface *wlr_surface)
{
    struct wlr_addon *node_addon =
        wlr_addon_find(&wlr_surface->addons, wlr_surface, &surface_node_addon_impl);
    if (!node_addon) {
        return NULL;
    }

    struct scene_surface *surface = wl_container_of(node_addon, surface, node_addon);
    return surface;
}

struct scene_surface *scene_surface_create(struct scene_tree *parent,
                                           struct wlr_surface *wlr_surface)
{
    struct scene_surface *surface = calloc(1, sizeof(*surface));
    if (surface == NULL) {
        return NULL;
    }

    struct scene_buffer *scene_buffer = scene_buffer_create(parent, NULL);
    if (!scene_buffer) {
        free(surface);
        return NULL;
    }

    surface->buffer = scene_buffer;
    surface->surface = wlr_surface;
    surface->buffer_default_impl = scene_buffer->node.impl;
    scene_buffer->point_accepts_input = scene_buffer_point_accepts_input;

    scene_node_set_role(&scene_buffer->node, SCENE_ROLE_SURFACE, NULL, wlr_surface);

    surface->outputs_update.notify = handle_scene_buffer_outputs_update;
    wl_signal_add(&scene_buffer->events.outputs_update, &surface->outputs_update);

    surface->output_enter.notify = handle_scene_buffer_output_enter;
    wl_signal_add(&scene_buffer->events.output_enter, &surface->output_enter);

    surface->output_leave.notify = handle_scene_buffer_output_leave;
    wl_signal_add(&scene_buffer->events.output_leave, &surface->output_leave);

    surface->output_sample.notify = handle_scene_buffer_output_sample;
    wl_signal_add(&scene_buffer->events.output_sample, &surface->output_sample);

    surface->frame_done.notify = handle_scene_buffer_frame_done;
    wl_signal_add(&scene_buffer->events.frame_done, &surface->frame_done);

    surface->surface_destroy.notify = scene_surface_handle_surface_destroy;
    wl_signal_add(&wlr_surface->events.destroy, &surface->surface_destroy);

    surface->surface_commit.notify = handle_scene_surface_surface_commit;
    wl_signal_add(&wlr_surface->events.commit, &surface->surface_commit);

    surface->surface_map.notify = handle_scene_surface_surface_map;
    wl_signal_add_next(&wlr_surface->events.map, &surface->surface_map);

    wlr_addon_init(&surface->addon, &scene_buffer->node.addons, scene_buffer, &surface_addon_impl);
    wlr_addon_init(&surface->node_addon, &wlr_surface->addons, wlr_surface,
                   &surface_node_addon_impl);

    /* sync current states from surface */
    set_buffer_with_surface_state(scene_buffer, wlr_surface,
                                  WLR_SURFACE_STATE_OPAQUE_REGION | WLR_SURFACE_STATE_BUFFER |
                                      WLR_SURFACE_STATE_TRANSFORM);

    return surface;
}

struct wlr_surface *wlr_surface_try_from_node(struct scene_node *node)
{
    if (node->type != SCENE_NODE_BUFFER) {
        return NULL;
    }

    struct scene_buffer *scene_buffer = scene_buffer_from_node(node);
    if (!scene_buffer) {
        return NULL;
    }

    struct scene_surface *scene_surface = scene_surface_try_from_buffer(scene_buffer);
    if (!scene_surface) {
        return NULL;
    }

    return scene_surface->surface;
}

struct scene_buffer *scene_buffer_try_from_surface(struct wlr_surface *wlr_surface)
{
    struct scene_surface *scene_surface = scene_surface_try_from_surface(wlr_surface);
    if (!scene_surface) {
        return NULL;
    }

    return scene_surface->buffer;
}

float scene_surface_get_scale(struct wlr_surface *wlr_surface)
{
    struct wlr_surface_state *surface_state = &wlr_surface->current;
    if (surface_state->scale > 1) {
        return surface_state->scale;
    }

    int width = surface_state->width, height = surface_state->height;
    struct scene_buffer *scene_buffer = scene_buffer_try_from_surface(wlr_surface);
    if (scene_buffer) {
        width = scene_buffer->dst_width;
        height = scene_buffer->dst_height;
    }

    int buffer_width = surface_state->buffer_width;
    int buffer_height = surface_state->buffer_height;
    if (surface_state->viewport.has_src) {
        buffer_width = surface_state->viewport.src.width;
        buffer_height = surface_state->viewport.src.height;
    }

    float scale;
    if (width == 0 || height == 0 || width >= buffer_width || height >= buffer_height) {
        scale = 1.0f;
    } else {
        scale = buffer_width > 0 ? (float)buffer_width / (float)width : 1.0f;
    }

    if (!scene_buffer) {
        return scale;
    }

    struct scene *scene = scene_from_node(&scene_buffer->node);
    struct scene_output *scene_output;
    wl_list_for_each(scene_output, &scene->outputs, link) {
        if (scene_buffer->active_outputs & (1ull << scene_output->index)) {
            scale = MAX(scale, scene_output->output->scale);
        }
    }

    return scale;
}
