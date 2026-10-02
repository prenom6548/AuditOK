// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdbool.h>
#include <stdlib.h>

#include <wlr/types/wlr_compositor.h>

#include "effect/action.h"
#include "effect/animator.h"
#include "effect/fade.h"
#include "effect/snapshot.h"
#include "effect/transform.h"
#include "effect_p.h"
#include "scene/surface.h"
#include "util/time.h"

enum fade_center_type {
    FADE_TYPE_CENTER = 0,
    FADE_TYPE_TOPLEFT,
};

struct fade_entity {
    struct transform *transform;
    bool fade_in;
    int x_offset, y_offset;
    float width_factor, height_factor;
    enum fade_center_type center_type;

    struct wl_listener destroy;
};

static struct fade_effect {
    struct transform_effect *effect;
    struct action_effect *action_effect;
} *fade_effect = NULL;

static void fade_entity_calc_scale_geometry(const struct fade_entity *fade,
                                            struct kywc_box *current_geometry,
                                            struct kywc_box *geometry)
{
    int x_offset = fade->x_offset, y_offset = fade->y_offset;
    geometry->width = current_geometry->width * fade->width_factor;
    geometry->height = current_geometry->height * fade->height_factor;
    switch (fade->center_type) {
    case FADE_TYPE_CENTER:
        geometry->x =
            current_geometry->x + (current_geometry->width - geometry->width) / 2 + x_offset;
        geometry->y =
            current_geometry->y + (current_geometry->height - geometry->height) / 2 + y_offset;
        break;
    case FADE_TYPE_TOPLEFT:
        geometry->x = current_geometry->x + x_offset;
        geometry->y = current_geometry->y + y_offset;
        break;
    }
}

static void fade_get_node_geometry(struct ky_scene_node *node, struct kywc_box *geometry)
{
    struct kywc_box box;
    ky_scene_node_get_affected_bounding_box(node, KY_SCENE_BOUNDING_WITHOUT_EFFECT_NODE, &box);
    ky_scene_node_coords(node, &geometry->x, &geometry->y);
    geometry->x += box.x;
    geometry->y += box.y;
    geometry->width = box.width;
    geometry->height = box.height;
}

static void fade_entity_set_transform_options_geometry(struct fade_entity *fade_entity,
                                                       struct ky_scene_node *node,
                                                       struct transform_options *options)
{
    struct kywc_box node_geometry;
    fade_get_node_geometry(node, &node_geometry);
    if (fade_entity->fade_in) {
        fade_entity_calc_scale_geometry(fade_entity, &node_geometry, &options->start.geometry);
        options->end.geometry = node_geometry;
    } else {
        options->start.geometry = node_geometry;
        fade_entity_calc_scale_geometry(fade_entity, &node_geometry, &options->end.geometry);
    }
}

static void fade_handle_transform_destroy(struct wl_listener *listener, void *data)
{
    struct fade_entity *fade_entity = wl_container_of(listener, fade_entity, destroy);
    wl_list_remove(&fade_entity->destroy.link);
    free(fade_entity);
}

static void fade_update_transform_options(struct transform_effect *effect,
                                          struct transform *transform, struct ky_scene_node *node,
                                          struct transform_options *options,
                                          struct animation_data *current, void *data)
{
    struct fade_entity *fade_entity = transform_get_user_data(transform);
    if (!fade_entity || !fade_entity->fade_in) {
        return;
    }

    /**
     * when xdg_popup request fade effect, the current geometry maybe wrong.
     * need to update start geometry.
     */
    fade_entity_set_transform_options_geometry(fade_entity, node, options);
}

static void fade_effect_destroy(struct transform_effect *effect, void *data)
{
    if (!fade_effect) {
        return;
    }

    if (fade_effect->action_effect) {
        action_effect_destroy(fade_effect->action_effect);
    }
    free(fade_effect);
    fade_effect = NULL;
}

static bool fade_entity_create_transform(struct fade_entity *fade_entity,
                                         struct ky_scene_node *node,
                                         struct transform_effect *effect,
                                         struct transform_options *options)
{
    struct effect_render_data render_data = { 0 };
    struct ky_scene_add_effect_event event = {
        .render_data = &render_data,
    };
    struct effect_entity *entity = node_add_custom_transform_effect(node, effect, options, &event);
    if (!entity) {
        return false;
    }

    if (!entity->user_data) {
        fade_entity->transform =
            transform_effect_create_transform(effect, entity, options, node, fade_entity);
        if (!fade_entity->transform) {
            effect_entity_destroy(entity);
            return false;
        }
    } else {
        fade_entity->transform = entity->user_data;
        transform_set_user_data(entity->user_data, fade_entity);
        struct transform_options *transform_options = transform_get_options(entity->user_data);
        transform_options->start = *transform_get_current(entity->user_data);
    }

    /* prohibit updating thumbnail when fade out */
    transform_block_source_update(fade_entity->transform, !fade_entity->fade_in);

    fade_entity->destroy.notify = fade_handle_transform_destroy;
    transform_add_destroy_listener(fade_entity->transform, &fade_entity->destroy);
    return true;
}

bool view_add_fade_effect(struct view *view, enum fade_action action)
{
    if (!fade_effect) {
        return false;
    }

    struct fade_entity *fade_entity = calloc(1, sizeof(*fade_entity));
    if (!fade_entity) {
        return false;
    }

    struct transform_options options = { 0 };
    struct animation *animation = animation_manager_get(ANIMATION_TYPE_EASE);
    options.animations.alpha = animation;
    options.animations.geometry = animation;

    options.scale = view->surface ? ky_scene_surface_get_scale(view->surface) : 1.0f;
    options.start_time = current_time_msec();
    struct ky_scene_node *node = &view->tree->node;
    if (action == FADE_IN) {
        fade_entity->fade_in = true;
        fade_entity->width_factor = 0.9;
        options.duration = 300;
        options.start.alpha = 0;
        options.end.alpha = 1.0;
    } else {
        fade_entity->fade_in = false;
        fade_entity->width_factor = 0.8;
        options.duration = 260;
        options.start.alpha = 1.0;
        options.end.alpha = 0;
    }

    fade_entity->height_factor = fade_entity->width_factor;
    fade_entity_set_transform_options_geometry(fade_entity, node, &options);

    if (action == FADE_IN) {
        options.buffer = view_try_get_single_buffer(view);
    }

    if (!fade_entity_create_transform(fade_entity, node, fade_effect->effect, &options)) {
        free(fade_entity);
        return false;
    }

    return true;
}

bool popup_add_fade_effect(struct ky_scene_node *node, enum fade_action action, bool topmost,
                           bool seat, float scale)
{
    if (!fade_effect) {
        return false;
    }

    struct fade_entity *fade_entity = calloc(1, sizeof(*fade_entity));
    if (!fade_entity) {
        return false;
    }

    struct transform_options options = { 0 };
    options.scale = scale;
    options.start_time = current_time_msec();
    if (action == FADE_IN) {
        if (topmost && seat) {
            fade_entity->width_factor = 1.0;
            options.duration = 220;
        } else if (topmost && !seat) {
            fade_entity->width_factor = 0.85;
            options.duration = 200;
        } else if (!topmost) {
            fade_entity->width_factor = 1.0;
            fade_entity->y_offset = 4;
            options.duration = 220;
        }
        fade_entity->fade_in = true;
        options.start.alpha = 0;
        options.end.alpha = 1.0;
    } else {
        if (topmost && seat) {
            fade_entity->width_factor = 1.0;
            options.duration = 180;
        } else if (topmost && !seat) {
            fade_entity->width_factor = 0.85;
            options.duration = 150;
        } else if (!topmost) {
            fade_entity->width_factor = 1.0;
            fade_entity->y_offset = -4;
            options.duration = 180;
        }
        fade_entity->fade_in = false;
        options.start.alpha = 1.0;
        options.end.alpha = 0;
    }

    fade_entity->height_factor = fade_entity->width_factor;
    fade_entity_set_transform_options_geometry(fade_entity, node, &options);

    struct animation *animation;
    if (topmost && seat) {
        animation = animation_manager_get(ANIMATION_TYPE_30_2_8_100);
        options.animations.alpha = animation;
        options.animations.geometry = animation;
    } else {
        animation = animation_manager_get(ANIMATION_TYPE_EASE);
        options.animations.alpha = animation;
        options.animations.geometry = animation;
    }

    if (!fade_entity_create_transform(fade_entity, node, fade_effect->effect, &options)) {
        free(fade_entity);
        return false;
    }

    return true;
}

/**
 * The default parameters of fade can be very simple, without distinguishing between multiple
 * situations. But when switching from other effects to fade effects through the
 * protocol(ukui-effect-v1), more fade parameters need to be set.
 */
static void fade_init_options(struct action_effect *action_effect,
                              struct action_effect_options *options)
{
    options->alpha = 0.0f;
    options->y_offset = 0;
    options->x_offset = 0;
    options->width_scale = 0.85f;
    options->height_scale = 0.85f;
    options->duration = 200;
    options->effect_type = ACTION_EFFECT_FADE;
    options->style = FADE_TYPE_CENTER;
    struct animation *animation = animation_manager_get(ANIMATION_TYPE_EASE);
    options->animations.geometry = animation;
    options->animations.alpha = animation;
    options->animations.angle = NULL;

    options->buffer = NULL;
    options->new_parent = NULL;
    options->scale = 1.0f;
}

static bool add_fade_to_node(struct action_effect *action_effect, struct ky_scene_node *node,
                             enum effect_action action, struct action_effect_options *options)
{
    if (!action_effect || !node->enabled) {
        return false;
    }

    struct fade_entity *fade_entity = calloc(1, sizeof(*fade_entity));
    if (!fade_entity) {
        return false;
    }

    fade_entity->y_offset = options->y_offset;
    fade_entity->x_offset = options->x_offset;
    fade_entity->fade_in = action == EFFECT_ACTION_MAP;
    fade_entity->center_type = options->style;
    fade_entity->width_factor = options->width_scale;
    fade_entity->height_factor = options->height_scale;

    struct transform_options fade_options = { 0 };
    fade_options.buffer = options->buffer;
    fade_options.scale = options->scale;
    fade_options.start_time = current_time_msec();
    fade_options.geometry_type = TRANSFORM_GEOMETRY_NODE_BOUNDING;

    fade_options.duration = options->duration;
    fade_options.start.alpha = fade_entity->fade_in ? options->alpha : 1.0f;
    fade_options.end.alpha = fade_entity->fade_in ? 1.0f : options->alpha;
    fade_options.animations = options->animations;

    fade_entity_set_transform_options_geometry(fade_entity, node, &fade_options);
    if (!fade_entity_create_transform(fade_entity, node, fade_effect->effect, &fade_options)) {
        free(fade_entity);
        return false;
    }

    return true;
}

static bool add_fade_to_view(struct action_effect *effect, struct view *view,
                             enum effect_action action, struct action_effect_options *options)
{
    options->scale = view->surface ? view->surface->current.scale : 1.0f;
    if (action == EFFECT_ACTION_MAP) {
        options->buffer = view_try_get_single_buffer(view);
    }

    return add_fade_to_node(effect, &view->tree->node, action, options);
}

const struct action_effect_interface fade_action_impl = {
    .init_options = fade_init_options,
    .add_to_node = add_fade_to_node,
    .add_to_view = add_fade_to_view,
};

struct transform_effect_interface fade_impl = {
    .update_transform_options = fade_update_transform_options,
    .destroy = fade_effect_destroy,
};

bool fade_effect_create(struct effect_manager *manager)
{
    fade_effect = calloc(1, sizeof(*fade_effect));
    if (!fade_effect) {
        return false;
    }

    uint32_t support_actions = EFFECT_ACTION_MAP | EFFECT_ACTION_UNMAP;
    fade_effect->action_effect =
        action_effect_create(ACTION_EFFECT_FADE, support_actions, fade_effect, &fade_action_impl);

    fade_effect->effect =
        transform_effect_create(manager, &fade_impl, support_actions, "fade", 10, NULL);
    if (!fade_effect->effect) {
        fade_effect_destroy(NULL, NULL);
        return false;
    }

    return true;
}
