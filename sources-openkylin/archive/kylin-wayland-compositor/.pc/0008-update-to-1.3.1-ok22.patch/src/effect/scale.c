// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>

#include "effect/scale.h"
#include "effect/transform.h"
#include "effect_p.h"
#include "kywc/boxes.h"
#include "output.h"
#include "scene/surface.h"
#include "util/time.h"

struct scale_entity {
    struct transform *transform;
    struct wl_listener destroy;
    struct wl_listener view_destroy;
};

struct scale_effect {
    struct transform_effect *effect;
};

static struct scale_effect *scale_effect = NULL;

static void scale_calc_view_box(struct kywc_view *view, struct kywc_box *geometry_box,
                                struct kywc_box *box)
{
    box->x = geometry_box->x - view->margin.off_x;
    box->y = geometry_box->y - view->margin.off_y;
    box->width = geometry_box->width + view->margin.off_width;
    box->height = geometry_box->height + view->margin.off_height;
}

static void scale_end_geometry_from_panel_surface(struct view *view, struct kywc_box *geometry)
{
    int lx, ly;
    struct ky_scene_buffer *buffer =
        ky_scene_buffer_try_from_surface(view->minimized_geometry.panel_surface);
    ky_scene_node_coords(&buffer->node, &lx, &ly);
    *geometry = (struct kywc_box){ view->minimized_geometry.geometry.x + lx,
                                   view->minimized_geometry.geometry.y + ly,
                                   view->minimized_geometry.geometry.width,
                                   view->minimized_geometry.geometry.height };
}

static void scale_end_geometry_from_parent_panel(struct view *view, struct kywc_box *geometry)
{
    struct view *parent = view->parent;
    while (parent) {
        if (parent->minimized_geometry.panel_surface) {
            scale_end_geometry_from_panel_surface(parent, geometry);
            break;
        }
        parent = parent->parent;
    }
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

    if (view->minimized_geometry.panel_surface) {
        scale_end_geometry_from_panel_surface(view, end);
    } else if (kywc_box_not_empty(&view->startup_geometry)) {
        *end = view->startup_geometry;
    } else {
        end->x = view->base.geometry.x + view->base.geometry.width / 2;
        end->y = view->base.geometry.y + view->base.geometry.height / 2;
    }

    if (view->parent && !view->minimized_geometry.panel_surface) {
        scale_end_geometry_from_parent_panel(view, end);
    }

    struct kywc_box original_start = *start;
    struct kywc_box *start_geometry = view->base.minimized ? start : end;
    scale_calc_view_box(&view->base, start_geometry, start);
    struct kywc_box *end_geometry = view->base.minimized ? end : &original_start;
    scale_calc_view_box(&view->base, end_geometry, end);
}

static void scale_handle_transform_destroy(struct wl_listener *listener, void *data)
{
    struct scale_entity *scale_entity = wl_container_of(listener, scale_entity, destroy);
    wl_list_remove(&scale_entity->view_destroy.link);
    wl_list_remove(&scale_entity->destroy.link);
    free(scale_entity);
}

static void scale_handle_view_destroy(struct wl_listener *listener, void *data)
{
    struct scale_entity *scale_entity = wl_container_of(listener, scale_entity, view_destroy);
    transform_destroy(scale_entity->transform);
}

static void scale_update_transform_options(struct transform_effect *effect,
                                           struct transform *transform, struct ky_scene_node *node,
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

bool view_add_scale_effect(struct view *view, enum scale_action action)
{
    if (!scale_effect) {
        return false;
    }

    /* do not display maximize effect when view minimized */
    if (view->base.minimized && action == SCALE_MAXIMIZE) {
        return false;
    }

    struct transform_options options = {
        .view = view,
        .start_time = current_time_msec(),
        .geometry_type = TRANSFORM_GEOMETRY_VIEW_BOX,
    };
    if (action == SCALE_MAXIMIZE || action == SCALE_FULLSCREEN) {
        options.start.alpha = 1.0;
        options.end.alpha = 1.0;
        options.animations.geometry = animation_manager_get(ANIMATION_TYPE_EASE);
        options.duration = view->base.maximized || view->base.fullscreen ? 300 : 260;
        if (action == SCALE_MAXIMIZE || (action == SCALE_FULLSCREEN && !view->base.fullscreen)) {
            scale_calc_view_box(&view->base, &view->pending.geometry, &options.start.geometry);
            scale_calc_view_box(&view->base, &view->base.geometry, &options.end.geometry);
        } else {
            /**
             * When in fullscreen mode, no need to calculate ssd and shadows. When exiting
             * fullscreen mode, ssd and shadows need to be calculated
             */
            options.start.geometry = view->pending.geometry;
            options.end.geometry = view->base.geometry;
        }
    } else if (action == SCALE_RESIZE) {
        options.start.alpha = 1.0;
        options.end.alpha = 1.0;
        options.duration = 260;
        options.animations.geometry = animation_manager_get(ANIMATION_TYPE_EASE);
        scale_calc_view_box(&view->base, &view->pending.geometry, &options.end.geometry);
        scale_calc_view_box(&view->base,
                            kywc_box_not_empty(&view->tile_start_geometry)
                                ? &view->tile_start_geometry
                                : &view->base.geometry,
                            &options.start.geometry);
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

    struct scale_entity *scale_entity = calloc(1, sizeof(*scale_entity));
    if (!scale_entity) {
        return false;
    }

    struct effect_render_data render_data = { 0 };
    struct ky_scene_add_effect_event event = {
        .render_data = &render_data,
    };
    struct effect_entity *entity =
        node_add_custom_transform_effect(&view->tree->node, scale_effect->effect, &options, &event);
    if (!entity) {
        free(scale_entity);
        return false;
    }

    if (kywc_box_not_empty(&view->tile_start_geometry) && render_data.is_valid) {
        scale_calc_view_box(&view->base, &view->tile_start_geometry, &options.start.geometry);
    }

    if (!entity->user_data) {
        scale_entity->transform = transform_effect_create_transform(
            scale_effect->effect, entity, &options, &view->tree->node, scale_entity);
        if (!scale_entity->transform) {
            effect_entity_destroy(entity);
            free(scale_entity);
            return false;
        }
    } else {
        scale_entity->transform = entity->user_data;
        transform_set_user_data(entity->user_data, scale_entity);
    }

    scale_entity->view_destroy.notify = scale_handle_view_destroy;
    wl_signal_add(&view->base.events.destroy, &scale_entity->view_destroy);

    scale_entity->destroy.notify = scale_handle_transform_destroy;
    transform_add_destroy_listener(scale_entity->transform, &scale_entity->destroy);

    return true;
}
