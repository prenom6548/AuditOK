// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>

#include <wlr/render/wlr_texture.h>

#include <kywc/log.h>

#include "effect/action.h"
#include "effect/animator.h"
#include "effect/slide.h"
#include "effect/snapshot.h"
#include "effect/transform.h"
#include "effect_p.h"
#include "output.h"
#include "util/time.h"
#include "view/view.h"

enum slide_style {
    SLIDE_ON_OUTPUT = 0,
    SLIDE_ON_NODE,
};

enum slide_location {
    SLIDE_LOCATION_LEFT = 0,
    SLIDE_LOCATION_TOP = 1,
    SLIDE_LOCATION_RIGHT = 2,
    SLIDE_LOCATION_BOTTOM = 3,
};

struct slide_entity {
    struct transform *transform;
    enum slide_style style;
    int location;
    int offset;

    bool slide_in;

    struct wl_listener add_effect;
    struct wl_listener node_destroy;
    /* No need to monitor view destroy signal */
    struct wl_listener destroy;
};

struct slide_effect {
    struct transform_effect *effect;
    struct action_effect *action_effect;
};

static struct slide_effect *slide_effect = NULL;

static void slide_get_node_origin_geometry(struct ky_scene_node *node, struct kywc_box *geometry)
{
    struct kywc_box box;
    ky_scene_node_get_affected_bounding_box(node, KY_SCENE_BOUNDING_WITHOUT_EFFECT_NODE, &box);
    ky_scene_node_coords(node, &geometry->x, &geometry->y);
    geometry->x += box.x;
    geometry->y += box.y;
    geometry->width = box.width;
    geometry->height = box.height;
}

static void slide_calc_start_and_end_geometry(struct slide_entity *slide_entity,
                                              const struct kywc_box *node_geometry, bool slide_in,
                                              struct kywc_box *start_geometry,
                                              struct kywc_box *end_geometry)
{
    struct kywc_box drawer_box = *node_geometry;
    if (slide_entity->style == SLIDE_ON_OUTPUT) {
        double center_x = node_geometry->x + node_geometry->width / 2.0;
        double center_y = node_geometry->y + node_geometry->height / 2.0;
        struct kywc_output *kywc_output = kywc_output_at_point(center_x, center_y);
        struct output *output = output_from_kywc_output(kywc_output);
        drawer_box = output->geometry;
    }

    struct kywc_box slide_geometry = *node_geometry;
    switch (slide_entity->location) {
    case SLIDE_LOCATION_LEFT:
        slide_geometry.x = drawer_box.x + slide_entity->offset;
        slide_geometry.width = 0;
        break;
    case SLIDE_LOCATION_TOP:
        slide_geometry.y = drawer_box.y + slide_entity->offset;
        slide_geometry.height = 0;
        break;
    case SLIDE_LOCATION_BOTTOM:
        slide_geometry.y = drawer_box.y + drawer_box.height - slide_entity->offset;
        slide_geometry.height = 0;
        break;
    case SLIDE_LOCATION_RIGHT:
        slide_geometry.x = drawer_box.x + drawer_box.width - slide_entity->offset;
        slide_geometry.width = 0;
        break;
    }

    if (slide_in) {
        *start_geometry = slide_geometry;
        *end_geometry = *node_geometry;
    } else {
        *start_geometry = *node_geometry;
        *end_geometry = slide_geometry;
    }
}

static void slide_update_transform_options(struct transform_effect *effect,
                                           struct transform *transform, struct ky_scene_node *node,
                                           struct transform_options *options,
                                           struct animation_data *current, void *data)
{
    struct slide_entity *slide_entity = transform_get_user_data(transform);
    if (!slide_entity->slide_in) {
        return;
    }

    struct kywc_box node_geometry;
    slide_get_node_origin_geometry(node, &node_geometry);
    slide_calc_start_and_end_geometry(slide_entity, &node_geometry, slide_entity->slide_in,
                                      &options->start.geometry, &options->end.geometry);
}

static void slide_get_content_anchors(int location, uint32_t *anchors)
{
    *anchors = TRANSFORM_CONTENT_ANCHOR_ALL;
    switch (location) {
    case SLIDE_LOCATION_LEFT:
        *anchors &= ~TRANSFORM_CONTENT_ANCHOR_LEFT;
        break;
    case SLIDE_LOCATION_RIGHT:
        *anchors &= ~TRANSFORM_CONTENT_ANCHOR_RIGHT;
        break;
    case SLIDE_LOCATION_TOP:
        *anchors &= ~TRANSFORM_CONTENT_ANCHOR_TOP;
        break;
    case SLIDE_LOCATION_BOTTOM:
        *anchors &= ~TRANSFORM_CONTENT_ANCHOR_BOTTOM;
        break;
    }
}

static void slide_effect_destroy(struct transform_effect *effect, void *data)
{
    if (!slide_effect) {
        return;
    }

    if (slide_effect->action_effect) {
        action_effect_destroy(slide_effect->action_effect);
    }
    free(slide_effect);
    slide_effect = NULL;
}

static void slide_handle_transform_destroy(struct wl_listener *listener, void *data)
{
    struct slide_entity *slide_entity = wl_container_of(listener, slide_entity, destroy);
    wl_list_remove(&slide_entity->add_effect.link);
    wl_list_remove(&slide_entity->node_destroy.link);
    wl_list_remove(&slide_entity->destroy.link);
    free(slide_entity);
}

static void slide_handle_add_effect(struct wl_listener *listener, void *data)
{
    struct slide_entity *slide_entity = wl_container_of(listener, slide_entity, add_effect);
    struct ky_scene_add_effect_event *event = data;
    if (!event) {
        return;
    }

    /**
     * allow_add is false will not add any effects, if the same type of effect is found to be
     * reused, it will not trigger add_effect signal. if effect type is differnet, it will trigger
     * signal, so it still supports the interruption of the slide itself.
     */
    event->allow_add = false;
}

static void slide_handle_node_destroy(struct wl_listener *listener, void *data)
{
    struct slide_entity *slide_entity = wl_container_of(listener, slide_entity, node_destroy);
    wl_list_remove(&slide_entity->add_effect.link);
    wl_list_init(&slide_entity->add_effect.link);
    wl_list_remove(&slide_entity->node_destroy.link);
    wl_list_init(&slide_entity->node_destroy.link);
}

static struct slide_entity *create_slide_entity(struct ky_scene_node *node, enum slide_style style,
                                                int location, int offset, bool mapped,
                                                struct transform_options *options)
{
    struct slide_entity *slide_entity = calloc(1, sizeof(*slide_entity));
    if (!slide_entity) {
        return NULL;
    }

    slide_entity->style = style;
    slide_entity->offset = offset;
    slide_entity->location = location;
    slide_entity->slide_in = mapped;

    struct kywc_box node_geometry;
    slide_get_node_origin_geometry(node, &node_geometry);
    slide_calc_start_and_end_geometry(slide_entity, &node_geometry, mapped,
                                      &options->start.geometry, &options->end.geometry);

    struct effect_render_data render_data = { 0 };
    struct ky_scene_add_effect_event event = {
        .render_data = &render_data,
    };
    slide_entity->transform = transform_effect_get_or_create_transform(
        slide_effect->effect, options, node, slide_entity, &event);
    if (!slide_entity->transform) {
        free(slide_entity);
        return false;
    }

    slide_entity->add_effect.notify = slide_handle_add_effect;
    wl_signal_add(&node->events.add_effect, &slide_entity->add_effect);

    slide_entity->node_destroy.notify = slide_handle_node_destroy;
    wl_signal_add(&node->events.destroy, &slide_entity->node_destroy);

    slide_entity->destroy.notify = slide_handle_transform_destroy;
    transform_add_destroy_listener(slide_entity->transform, &slide_entity->destroy);
    return slide_entity;
}

static void slide_init_action_options(struct action_effect *effect,
                                      struct action_effect_options *options)
{
    *options = (struct action_effect_options){ 0 };
    options->style = SLIDE_ON_NODE;
    options->effect_type = ACTION_EFFECT_SLIDE;
    options->animations.geometry = animation_manager_get(ANIMATION_TYPE_EASE);
    options->animations.alpha = animation_manager_get(ANIMATION_TYPE_EASE);
    options->animations.angle = NULL;
    options->buffer = NULL;
    options->new_parent = NULL;
    options->scale = 1.0f;
    options->alpha = 1.0f;
    options->duration = 300;
    options->y_offset = 0;
    options->location = 0;
}

static bool add_slide_to_node(struct action_effect *action_effect, struct ky_scene_node *node,
                              enum effect_action action,
                              struct action_effect_options *action_options)
{
    struct transform_options options = { 0 };
    options.scale = action_options->scale;
    options.buffer = action_options->buffer;
    options.new_parent = action_options->new_parent;
    options.animations = action_options->animations;
    options.start.alpha = action == EFFECT_ACTION_MAP ? action_options->alpha : 1.0f;
    options.end.alpha = action == EFFECT_ACTION_MAP ? 1.0f : action_options->alpha;
    options.geometry_type = TRANSFORM_GEOMETRY_NODE_BOUNDING;
    options.duration = action_options->duration;
    options.start_time = current_time_msec();

    slide_get_content_anchors(action_options->location, &options.content_anchors);
    return create_slide_entity(node, action_options->style, action_options->location,
                               action_options->y_offset, action == EFFECT_ACTION_MAP, &options);
}

static bool add_slide_to_view(struct action_effect *action_effect, struct view *view,
                              enum effect_action action,
                              struct action_effect_options *action_options)
{
    action_options->buffer = action == EFFECT_ACTION_MAP ? view_try_get_single_buffer(view) : NULL;
    struct view_layer *old_layer = view_manager_get_layer_by_role(view->base.role);
    if (old_layer->layer == LAYER_SYSTEM_WINDOW) {
        struct view_layer *new_layer = view_manager_get_layer(LAYER_NORMAL, false);
        action_options->new_parent = new_layer->tree;
    }

    return add_slide_to_node(action_effect, &view->tree->node, action, action_options);
}

const struct action_effect_interface slide_action_impl = {
    .init_options = slide_init_action_options,
    .add_to_node = add_slide_to_node,
    .add_to_view = add_slide_to_view,
};

struct transform_effect_interface slide_impl = {
    .update_transform_options = slide_update_transform_options,
    .destroy = slide_effect_destroy,
};

bool slide_effect_create(struct effect_manager *manager)
{
    slide_effect = calloc(1, sizeof(*slide_effect));
    if (!slide_effect) {
        return false;
    }

    uint32_t support_actions = EFFECT_ACTION_MAP | EFFECT_ACTION_UNMAP;
    slide_effect->action_effect = action_effect_create(ACTION_EFFECT_SLIDE, support_actions,
                                                       slide_effect, &slide_action_impl);

    slide_effect->effect =
        transform_effect_create(manager, &slide_impl, support_actions, "slide", 5, slide_effect);
    if (!slide_effect->effect) {
        slide_effect_destroy(NULL, NULL);
        return false;
    }

    return true;
}

bool view_add_slide_effect(struct view *view, bool slide_out)
{
    if (!view->use_slide || !slide_effect || !view->surface) {
        return false;
    }

    struct action_effect_options action_options = { 0 };
    slide_init_action_options(NULL, &action_options);

    action_options.style = SLIDE_ON_OUTPUT;
    action_options.y_offset = view->slide.offset;
    action_options.location = view->slide.location;
    return add_slide_to_view(slide_effect->action_effect, view,
                             slide_out ? EFFECT_ACTION_MAP : EFFECT_ACTION_UNMAP, &action_options);
}
