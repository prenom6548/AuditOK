// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>

#include <kywc/boxes.h>

#include "effect/scale.h"
#include "effect/snapshot.h"
#include "effect/transform.h"
#include "effect_p.h"
#include "output.h"
#include "render/opengl.h"
#include "scene/surface.h"
#include "util/time.h"

struct scale_entity {
    struct wl_list link;

    struct view *view;
    struct transform *transform;
    struct snapshot *crossfade_snapshot;

    struct kywc_box start_view_box;

    struct wl_listener view_destroy;
    struct wl_listener transform_destroy;
};

struct scale_effect {
    struct transform_effect *effect;
    struct wl_list entities;

    bool support_crossfade;
};

static struct scale_effect *scale_effect = NULL;

/* view box without shadow, no need add padding */
static void scale_calc_view_box(struct kywc_view *view, struct kywc_box *geometry_box,
                                struct kywc_box *box)
{
    box->x = geometry_box->x - view->margin.off_x;
    box->y = geometry_box->y - view->margin.off_y;
    box->width = geometry_box->width + view->margin.off_width;
    box->height = geometry_box->height + view->margin.off_height;
}

static void scale_panel_minimized_geometry_from_view(struct view *view, struct kywc_box *geometry)
{
    while (view) {
        if (kywc_box_empty(&view->minimized_geometry.box)) {
            view = view->parent;
            continue;
        }

        int lx = 0, ly = 0;
        struct scene_buffer *buffer =
            scene_buffer_try_from_surface(view->minimized_geometry.panel_surface);
        if (buffer) {
            scene_node_coords(&buffer->node, &lx, &ly);
        }
        *geometry = view->minimized_geometry.box;
        geometry->x += lx;
        geometry->y += ly;
        return;
    }

    return;
}

static void scale_calc_minimize_start_and_end_geometry(struct view *view, struct kywc_box *start,
                                                       struct kywc_box *end)
{
    *start = view->base.geometry;
    end->width = view->base.geometry.width * 0.4;
    end->height = view->base.geometry.height * 0.4;

    if (view->minimized_when_show_desktop) {
        struct output *output = output_from_kywc_output(view->output);
        end->x = output->geometry.x + (output->geometry.width - end->width) / 2;
        end->y = output->geometry.y + (output->geometry.height - end->height) / 2;

        struct kywc_box *start_geometry = view->base.minimized ? start : end;
        scale_calc_view_box(&view->base, start_geometry, start);
        return;
    }

    struct kywc_box panel_minimized_geometry = { 0 };
    scale_panel_minimized_geometry_from_view(view, &panel_minimized_geometry);
    if (kywc_box_not_empty(&panel_minimized_geometry)) {
        *end = panel_minimized_geometry;
    } else if (kywc_box_not_empty(&view->startup_geometry)) {
        *end = view->startup_geometry;
    } else {
        end->x = view->base.geometry.x + (view->base.geometry.width - end->width) / 2;
        end->y = view->base.geometry.y + (view->base.geometry.height - end->height) / 2;
    }

    struct kywc_box original_start = *start;
    struct kywc_box *start_geometry = view->base.minimized ? start : end;
    scale_calc_view_box(&view->base, start_geometry, start);
    struct kywc_box *end_geometry = view->base.minimized ? end : &original_start;
    scale_calc_view_box(&view->base, end_geometry, end);
}

static void scale_entity_remove(struct scale_entity *scale_entity)
{
    wl_list_remove(&scale_entity->link);
    wl_list_init(&scale_entity->link);
}

static void scale_entity_destroy(struct scale_entity *scale_entity)
{
    wl_list_remove(&scale_entity->transform_destroy.link);
    wl_list_remove(&scale_entity->view_destroy.link);
    scale_entity_remove(scale_entity);
    if (scale_entity->crossfade_snapshot) {
        snapshot_destroy(scale_entity->crossfade_snapshot);
    }

    free(scale_entity);
}

static void scale_handle_transform_destroy(struct wl_listener *listener, void *data)
{
    struct scale_entity *scale_entity = wl_container_of(listener, scale_entity, transform_destroy);
    scale_entity_destroy(scale_entity);
}

static void scale_handle_view_destroy(struct wl_listener *listener, void *data)
{
    struct scale_entity *scale_entity = wl_container_of(listener, scale_entity, view_destroy);
    if (!scale_entity->transform) {
        scale_entity_destroy(scale_entity);
        return;
    }

    transform_destroy(scale_entity->transform);
}

static void scale_update_transform_options(struct transform_effect *effect,
                                           struct transform *transform, struct scene_node *node,
                                           struct transform_options *options,
                                           struct animation_data *current, void *data)
{
    struct view *view = options->view;
    if (!view || view->base.minimized) {
        return;
    }

    if (options->geometry_type == TRANSFORM_GEOMETRY_VIEW_BOX) {
        scale_calc_view_box(&view->base, &view->base.geometry, &options->end.geometry);
    }
}

static void scale_effect_destroy(struct transform_effect *effect, void *data)
{
    struct scale_effect *scale_effect = data;
    struct scale_entity *scale_entity, *tmp;
    wl_list_for_each_safe(scale_entity, tmp, &scale_effect->entities, link) {
        scale_entity_destroy(scale_entity);
    }

    free(scale_effect);
    scale_effect = NULL;
}

struct transform_effect_interface scale_impl = {
    .update_transform_options = scale_update_transform_options,
    .destroy = scale_effect_destroy,
};

bool scale_effect_create(struct effect_manager *manager)
{
    scale_effect = calloc(1, sizeof(*scale_effect));
    if (!scale_effect) {
        return false;
    }

    wl_list_init(&scale_effect->entities);
    scale_effect->support_crossfade = wlr_renderer_is_opengl(manager->server->renderer);
    uint32_t support_actions = EFFECT_ACTION_RESIZE | EFFECT_ACTION_MAXIMIZE |
                               EFFECT_ACTION_MAXIMIZE_RESTORE | EFFECT_ACTION_MINIMIZE |
                               EFFECT_ACTION_MINIMIZE_RESTORE;
    scale_effect->effect =
        transform_effect_create(manager, &scale_impl, support_actions, "scale", 5, scale_effect);
    if (!scale_effect->effect) {
        free(scale_effect);
        return false;
    }

    return true;
}

static struct scale_entity *scale_effect_create_entity(struct view *view)
{
    struct scale_entity *scale_entity = calloc(1, sizeof(*scale_entity));
    if (!scale_entity) {
        return NULL;
    }

    scale_entity->view = view;
    wl_list_init(&scale_entity->transform_destroy.link);
    wl_list_insert(&scale_effect->entities, &scale_entity->link);
    scale_entity->transform_destroy.notify = scale_handle_transform_destroy;

    scale_entity->view_destroy.notify = scale_handle_view_destroy;
    wl_signal_add(&view->base.events.destroy, &scale_entity->view_destroy);
    return scale_entity;
}

static struct scale_entity *scale_effect_find_entity(struct view *view)
{
    struct scale_entity *scale_entity;
    wl_list_for_each(scale_entity, &scale_effect->entities, link) {
        if (scale_entity->view == view) {
            return scale_entity;
        }
    }

    return NULL;
}

bool view_add_scale_effect_pre(struct view *view)
{
    if (!scale_effect || view->base.minimized) {
        return false;
    }

    struct scale_entity *scale_entity = scale_effect_find_entity(view);
    if (scale_entity) {
        return true;
    }

    scale_entity = scale_effect_create_entity(view);
    if (!scale_entity) {
        return false;
    }

    scale_calc_view_box(&view->base,
                        kywc_box_empty(&view->tile_start_geometry) ? &view->base.geometry
                                                                   : &view->tile_start_geometry,
                        &scale_entity->start_view_box);
    if (!scale_effect->support_crossfade) {
        return true;
    }

    scale_entity->crossfade_snapshot = snapshot_create_from_view(
        view, 1.0, SNAPSHOT_RENDER_ENABLE_VIEW_BOX | SNAPSHOT_RENDER_DISABLE_BLUR);
    if (scale_entity->crossfade_snapshot) {
        snapshot_detach(scale_entity->crossfade_snapshot);
    }

    return true;
}

bool view_add_scale_effect(struct view *view, enum scale_action action)
{
    if (!scale_effect) {
        return false;
    }

    /* do not display other scale effect when view minimized */
    if (view->base.minimized && action != SCALE_MINIMIZE) {
        return false;
    }

    struct scale_entity *scale_entity = scale_effect_find_entity(view);
    scale_entity = scale_entity ? scale_entity : scale_effect_create_entity(view);
    if (!scale_entity) {
        return false;
    }

    struct transform_options options = {
        .view = view,
        .start_time = current_time_msec(),
        .geometry_type = TRANSFORM_GEOMETRY_VIEW_BOX,
        .crossfade_snapshot = scale_entity->crossfade_snapshot,
    };
    if (action == SCALE_MAXIMIZE || action == SCALE_FULLSCREEN) {
        options.start.alpha = 1.0;
        options.end.alpha = 1.0;
        options.animations.geometry = animation_manager_get(ANIMATION_TYPE_EASE);
        options.duration = view->base.maximized || view->base.fullscreen ? 300 : 260;

        options.start.geometry = scale_entity->start_view_box;
        scale_calc_view_box(&view->base, &view->base.geometry, &options.end.geometry);
    } else if (action == SCALE_RESIZE) {
        options.start.alpha = 1.0;
        options.end.alpha = 1.0;
        options.duration = 260;
        options.animations.geometry = animation_manager_get(ANIMATION_TYPE_EASE);

        if (kywc_box_not_empty(&scale_entity->start_view_box)) {
            options.start.geometry = scale_entity->start_view_box;
        } else {
            scale_calc_view_box(&view->base,
                                kywc_box_not_empty(&view->tile_start_geometry)
                                    ? &view->tile_start_geometry
                                    : &view->base.geometry,
                                &options.start.geometry);
        }
        scale_calc_view_box(&view->base, &view->base.geometry, &options.end.geometry);
    } else if (action == SCALE_MINIMIZE) {
        if (view->base.minimized) {
            options.duration = 260;
            options.start.alpha = 1.0;
            options.end.alpha = 0;
            options.animations.geometry = view->minimized_when_show_desktop
                                              ? animation_manager_get(ANIMATION_TYPE_33_0_100_75)
                                              : animation_manager_get(ANIMATION_TYPE_0_40_20_100);
            options.animations.alpha = animation_manager_get(ANIMATION_TYPE_33_0_100_75);
        } else {
            options.start.alpha = 0;
            options.end.alpha = 1.0;
            options.duration = 300;
            options.animations.geometry = animation_manager_get(ANIMATION_TYPE_30_15_10_100);
            options.animations.alpha = animation_manager_get(ANIMATION_TYPE_0_40_20_100);
        }
        scale_calc_minimize_start_and_end_geometry(view, &options.start.geometry,
                                                   &options.end.geometry);
    }

    struct effect_render_data render_data = { 0 };
    struct scene_add_effect_event event = {
        .render_data = &render_data,
    };
    struct effect_entity *effect_entity =
        node_add_custom_transform_effect(&view->tree->node, scale_effect->effect, &options, &event);
    if (!effect_entity) {
        scale_entity_destroy(scale_entity);
        return false;
    }

    if (kywc_box_not_empty(&view->tile_start_geometry) && render_data.is_valid) {
        scale_calc_view_box(&view->base, &view->tile_start_geometry, &options.start.geometry);
    }

    /* effect_entity->user_data is transform. if transform is null, don't be scaling */
    if (!effect_entity->user_data) {
        scale_entity->transform = transform_effect_create_transform(
            scale_effect->effect, effect_entity, &options, &view->tree->node, scale_entity);
        if (!scale_entity->transform) {
            effect_entity_destroy(effect_entity);
            scale_entity_destroy(scale_entity);
            return false;
        }
    } else {
        scale_entity->transform = effect_entity->user_data;
        transform_set_user_data(effect_entity->user_data, scale_entity);
    }

    /* take over by transform */
    scale_entity_remove(scale_entity);
    transform_add_destroy_listener(scale_entity->transform, &scale_entity->transform_destroy);
    return true;
}
