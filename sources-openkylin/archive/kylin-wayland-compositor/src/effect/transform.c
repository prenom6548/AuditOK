// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include <kywc/log.h>

#include "effect/animator.h"
#include "effect/node_transform.h"
#include "effect/snapshot.h"
#include "effect/transform.h"
#include "effect_p.h"
#include "render/opengl.h"
#include "render/renderer.h"
#include "scene/scene.h"
#include "scene/thumbnail.h"
#include "util/macros.h"
#include "util/time.h"
#include "xwayland/xwayland.h"

struct transform {
    struct effect_entity *entity;
    struct transform_effect *effect;

    struct animator *animator;
    /* current geometry is dst_box */
    struct animation_data current;
    struct transform_options pending_options;
    struct transform_options options;

    struct scene_tree *old_parent;
    struct wl_listener old_parent_destroy;

    /* reuse transforms */
    int references;

    struct scene_buffer *buffer;

    /**
     * thumbnail textrue comes from three aspects. One, node is tree, it actively generates
     * thumbnails. Two, node is rect, the map stage, directly rendered without using thumbnails, but
     * the destroy stage, need to regenerate thumbnails. Thirdly, node is buffer, the map stage,
     * converted from the buffer to the texture through zero-copy-buffer without generating
     * thumbnails. the destroy stage, the buffer is obtained through thumbnail buffer
     */
    struct snapshot *snapshot;

    struct scene_node *node;
    struct wl_listener add_effect;

    struct {
        struct wl_signal destroy;
    } events;

    void *user_data;
};

struct transform_effect {
    struct effect *effect;
    const struct transform_effect_interface *impl;
    struct effect_manager *manager;

    struct wlr_renderer *renderer;
    bool is_opengl_renderer;

    struct wl_listener enable;
    struct wl_listener disable;
    struct wl_listener destroy;

    void *user_data;
};

static struct transform_effect *node_transform_effect = NULL;

static const struct effect_interface transform_effect_impl;

static bool transform_create_animator(struct transform *transform);

static bool transform_effect_entity_bounding_box(struct effect_entity *entity, struct kywc_box *box)
{
    struct effect_chain *chain = entity->slot.chain;
    struct node_effect_chain *node_chain = wl_container_of(chain, node_chain, base);
    struct transform *transform = entity->user_data;
    if (!transform) {
        node_chain->impl.get_affected_bounding_box(node_chain->node, SCENE_BOUNDING_WITHOUT_EFFECT,
                                                   box);
        return false;
    }

    *box = transform->current.geometry;
    if (box->width <= 0 || box->height <= 0) {
        effect_entity_get_bounding_box(entity, SCENE_BOUNDING_WITHOUT_EFFECT, box);
        return false;
    }

    int lx, ly;
    scene_node_coords(node_chain->node, &lx, &ly);
    box->x -= lx;
    box->y -= ly;

    return false;
}

static float calc_crossfade_blend_alpha(const struct kywc_box *start, const struct kywc_box *end,
                                        const struct kywc_box *current)
{
    float end_area = end->width * end->height;
    float start_area = start->width * start->height;
    float current_area = current->width * current->height;
    if (end_area <= 0.0f) {
        kywc_log(KYWC_DEBUG, "calc crossfade blend alpha: end_area <= 0.0f");
        return 0.0f;
    }

    float blend_progress = 0, end_area_scale = 1.0f;
    float start_area_scale = start_area / end_area;
    float current_area_scale = current_area / end_area;
    /* blend alpha 1.0 -> 0.0 */
    if (!FLOAT_EQUAL(start_area_scale, end_area_scale)) {
        blend_progress =
            (current_area_scale - start_area_scale) / (end_area_scale - start_area_scale);
        blend_progress = blend_progress > 0.5f ? blend_progress + 0.1f : blend_progress;
        return CLAMP(1.0f - pow(blend_progress, 2), 0.0f, 1.0f);
    } else {
        blend_progress = MAX(0, 1.0 - fabs(current_area_scale - start_area_scale));
        blend_progress = blend_progress > 0.5f ? blend_progress + 0.1f : blend_progress;
        return CLAMP(1.0f - pow(blend_progress, 2), 0.0f, 1.0f);
    }
}

static bool transform_effect_frame_render_pre(struct effect_entity *entity,
                                              struct scene_render_target *target)
{
    struct transform *transform = entity->user_data;
    uint32_t time = current_time_msec();
    if (time > transform->pending_options.start_time + transform->pending_options.duration) {
        effect_entity_destroy(entity);
        return true;
    }

    struct transform_effect *effect = transform->effect;
    struct transform_options *options = &transform->options;
    /* update transform options */
    if (effect->impl && effect->impl->update_transform_options) {
        effect->impl->update_transform_options(effect, transform, transform->node,
                                               &transform->pending_options, &transform->current,
                                               effect->user_data);
    }

    /* compare geometry, alpha, angle, time */
    if (options->start_time != transform->pending_options.start_time) {
        animator_set_time(transform->animator, transform->pending_options.start_time +
                                                   transform->pending_options.duration);
    }
    if ((!kywc_box_equal(&options->start.geometry, &transform->pending_options.start.geometry)) ||
        (options->start.alpha != transform->pending_options.start.alpha) ||
        (options->start.angle != transform->pending_options.start.angle)) {
        if (!transform_create_animator(transform)) {
            effect_entity_destroy(entity);
            return true;
        }
    } else {
        if (options->end.alpha != transform->pending_options.end.alpha) {
            animator_set_alpha(transform->animator, transform->pending_options.end.alpha);
        }
        if (options->end.angle != transform->pending_options.end.angle) {
            animator_set_angle(transform->animator, transform->pending_options.end.angle);
        }
        if (!kywc_box_equal(&options->end.geometry, &transform->pending_options.end.geometry)) {
            animator_set_position(transform->animator, transform->pending_options.end.geometry.x,
                                  transform->pending_options.end.geometry.y);
            animator_set_size(transform->animator, transform->pending_options.end.geometry.width,
                              transform->pending_options.end.geometry.height);
        }
    }

    transform->options = transform->pending_options;
    transform->current = *animator_value(transform->animator, time);

    if (options->crossfade_snapshot && transform->effect->is_opengl_renderer) {
        snapshot_set_dst_box(options->crossfade_snapshot, &transform->current.geometry);
        snapshot_set_mixed_snapshot(transform->snapshot, options->crossfade_snapshot,
                                    calc_crossfade_blend_alpha(&options->start.geometry,
                                                               &options->end.geometry,
                                                               &transform->current.geometry));
    }

    if (transform->snapshot) {
        snapshot_set_opacity(transform->snapshot, transform->current.alpha);
        snapshot_set_dst_box(transform->snapshot, &transform->current.geometry);
        /* fix current geometry(view box) to render box */
        snapshot_get_render_box(transform->snapshot, &transform->current.geometry);
    }

    effect_entity_push_damage(entity, SCENE_DAMAGE_BOTH);
    return true;
}

static void transform_effect_handle_enable(struct wl_listener *listener, void *data)
{
    struct transform_effect *effect = wl_container_of(listener, effect, enable);
    if (effect->impl && effect->impl->enable) {
        effect->impl->enable(effect, true, effect->user_data);
    }
}

static void transform_effect_handle_disable(struct wl_listener *listener, void *data)
{
    struct transform_effect *effect = wl_container_of(listener, effect, disable);
    if (effect->impl && effect->impl->enable) {
        effect->impl->enable(effect, false, effect->user_data);
    }
}

static void transform_effect_handle_destroy(struct wl_listener *listener, void *data)
{
    struct transform_effect *effect = wl_container_of(listener, effect, destroy);
    wl_list_remove(&effect->enable.link);
    wl_list_remove(&effect->disable.link);
    wl_list_remove(&effect->destroy.link);

    if (effect->impl && effect->impl->destroy) {
        effect->impl->destroy(effect, effect->user_data);
    }

    struct effect_entity *entity, *tmp;
    wl_list_for_each_safe(entity, tmp, &effect->effect->entities, effect_link) {
        effect_entity_destroy(entity);
    }
    free(effect);
}

struct transform_effect *transform_effect_create(struct effect_manager *manager,
                                                 struct transform_effect_interface *impl,
                                                 uint32_t support_actions, const char *name,
                                                 int priority, void *data)
{
    struct transform_effect *transform_effect = calloc(1, sizeof(*transform_effect));
    if (!transform_effect) {
        return NULL;
    }

    transform_effect->impl = impl;
    transform_effect->user_data = data;
    transform_effect->manager = manager;
    transform_effect->renderer = manager->server->renderer;
    bool enabled = !renderer_is_software(manager->server->renderer);
    transform_effect->is_opengl_renderer = wlr_renderer_is_opengl(transform_effect->renderer);
    struct effect *effect =
        effect_create(name, priority, enabled, &transform_effect_impl, transform_effect);
    if (!effect) {
        free(transform_effect);
        return NULL;
    }

    effect->support_actions = support_actions;
    effect->category = EFFECT_CATEGORY_ACTION;
    transform_effect->effect = effect;
    transform_effect->enable.notify = transform_effect_handle_enable;
    wl_signal_add(&transform_effect->effect->events.enable, &transform_effect->enable);
    transform_effect->disable.notify = transform_effect_handle_disable;
    wl_signal_add(&transform_effect->effect->events.disable, &transform_effect->disable);
    transform_effect->destroy.notify = transform_effect_handle_destroy;
    wl_signal_add(&transform_effect->effect->events.destroy, &transform_effect->destroy);

    return transform_effect;
}

void transform_destroy(struct transform *transform)
{
    if (!transform->entity) {
        return;
    }

    effect_entity_destroy(transform->entity);
}

void transform_set_user_data(struct transform *transform, void *data)
{
    transform->user_data = data;
}

void *transform_get_user_data(struct transform *transform)
{
    return transform->user_data;
}

struct transform_options *transform_get_options(struct transform *transform)
{
    return &transform->pending_options;
}

struct animation_data *transform_get_current(struct transform *transform)
{
    return &transform->current;
}

void transform_block_source_update(struct transform *transform, bool block)
{
    if (!transform->snapshot) {
        return;
    }

    snapshot_mark_wants_update(transform->snapshot, !block);
}

static bool transform_effect_node_push_damage(struct effect_entity *entity,
                                              struct scene_node *damage_node, uint32_t *damage_type,
                                              pixman_region32_t *damage)
{
    struct kywc_box box;
    transform_effect_entity_bounding_box(entity, &box);
    pixman_region32_union_rect(damage, damage, box.x, box.y, box.width, box.height);

    return false;
}

static bool transform_effect_frame_render_post(struct effect_entity *entity,
                                               struct scene_render_target *target)
{
    effect_entity_push_damage(entity, SCENE_DAMAGE_BOTH);
    return true;
}

static bool handle_transform_effect_configure(struct effect *effect,
                                              const struct effect_option *option)
{
    if (effect_option_is_enabled_option(option)) {
        return true;
    }

    return false;
}

static void transform_do_destroy(struct transform *transform)
{
    transform->references--;
    if (transform->references != 0) {
        return;
    }

    wl_signal_emit_mutable(&transform->events.destroy, transform);
    assert(wl_list_empty(&transform->events.destroy.listener_list));

    if (transform->old_parent) {
        wl_list_remove(&transform->old_parent_destroy.link);
        scene_node_reparent(transform->node, transform->old_parent);
    }

    if (transform->buffer) {
        scene_node_destroy(&transform->buffer->node);
    }

    if (transform->snapshot) {
        snapshot_destroy(transform->snapshot);
    }

    if (transform->animator) {
        animator_destroy(transform->animator);
    }

    wl_list_remove(&transform->add_effect.link);
    free(transform);
}

static void transform_effect_entity_destroy(struct effect_entity *entity)
{
    struct transform *transform = entity->user_data;
    if (!transform) {
        return;
    }

    transform_do_destroy(transform);
}

static bool transform_effect_node_render(struct effect_entity *entity, int lx, int ly,
                                         struct scene_render_target *target)
{
    struct transform *transform = entity->user_data;
    if (!target->output) {
        return false;
    }

    if (transform->node->type == SCENE_NODE_RECT) {
        struct scene_node *rect_node = transform->node;
        struct scene_rect *rect = scene_rect_from_node(rect_node);
        float alpha = transform->current.alpha;
        float color[4] = { rect->color[0] * alpha, rect->color[1] * alpha, rect->color[2] * alpha,
                           rect->color[3] * alpha };
        scene_rect_render(rect_node, transform->current.geometry, color, false, target);

        return false;
    }

    if (transform->snapshot) {
        snapshot_render(transform->snapshot, target);
    }

    return false;
}

static void transform_effect_node_destroy(struct effect_entity *entity,
                                          struct scene_node **data);

static bool handle_transform_effect_allow_direct_scanout(struct effect *effect,
                                                         struct scene_render_target *target)
{
    struct effect_entity *entity;
    wl_list_for_each(entity, &effect->entities, effect_link) {
        struct transform *transform = entity->user_data;
        if (!transform) {
            continue;
        }

        if (kywc_box_intersects(&target->logical, &transform->current.geometry)) {
            return false;
        }
    }

    return true;
}

static const struct effect_interface transform_effect_impl = {
    .entity_destroy = transform_effect_entity_destroy,
    .entity_bounding_box = transform_effect_entity_bounding_box,
    .node_push_damage = transform_effect_node_push_damage,
    .node_render = transform_effect_node_render,
    .node_destroy = transform_effect_node_destroy,
    .frame_render_pre = transform_effect_frame_render_pre,
    .frame_render_post = transform_effect_frame_render_post,
    .configure = handle_transform_effect_configure,
    .allow_direct_scanout = handle_transform_effect_allow_direct_scanout,
};

static void transform_handle_old_parent_destroy(struct wl_listener *listener, void *data)
{
    struct transform *transform = wl_container_of(listener, transform, old_parent_destroy);

    wl_list_remove(&transform->old_parent_destroy.link);
    wl_list_init(&transform->old_parent_destroy.link);
    transform->old_parent = NULL;

    /* old_parent destroy, transform->node should be destroy */
    scene_node_destroy(transform->node);
}

static bool transform_create_animator(struct transform *transform)
{
    struct transform_options *options = &transform->pending_options;
    struct animator *animator =
        animator_create(&options->start, &options->animations, options->start_time,
                        options->start_time + options->duration);
    if (!animator) {
        return false;
    }

    if (transform->animator) {
        animator_destroy(transform->animator);
    }
    transform->animator = animator;

    animator_set_position(transform->animator, options->end.geometry.x, options->end.geometry.y);
    animator_set_size(transform->animator, options->end.geometry.width,
                      options->end.geometry.height);
    animator_set_alpha(transform->animator, options->end.alpha);
    animator_set_angle(transform->animator, options->end.angle);
    return true;
}

static struct transform *transform_create(struct transform_options *options,
                                          struct transform_effect *effect, struct scene_node *node,
                                          void *data)
{
    struct transform *transform = calloc(1, sizeof(*transform));
    if (!transform) {
        return NULL;
    }

    transform->buffer = NULL;
    transform->effect = effect;
    transform->node = node;
    transform->pending_options = *options;
    transform->options = *options;
    transform->user_data = data;
    transform->references = 1;

    wl_signal_init(&transform->events.destroy);
    wl_list_init(&transform->add_effect.link);
    wl_list_init(&transform->old_parent_destroy.link);

    if (!transform_create_animator(transform)) {
        transform_do_destroy(transform);
        return NULL;
    }

    if (transform->options.new_parent) {
        transform->old_parent = node->parent;
        scene_node_reparent(transform->node, transform->options.new_parent);
        transform->old_parent_destroy.notify = transform_handle_old_parent_destroy;
        wl_signal_add(&transform->old_parent->node.events.destroy, &transform->old_parent_destroy);
    }

    /* node is rect, don't generate thumbnails */
    if (transform->node->type == SCENE_NODE_RECT) {
        return transform;
    }

    float scale = options->scale <= 0 ? 1.0f : options->scale;
    uint32_t snapshot_options = effect->is_opengl_renderer ? 0 : SNAPSHOT_RENDER_DISABLE_BLUR;
    if (options->view) {
        if (options->geometry_type == TRANSFORM_GEOMETRY_VIEW_BOX) {
            snapshot_options |= SNAPSHOT_RENDER_ENABLE_VIEW_BOX;
        }
        transform->snapshot = snapshot_create_from_view(options->view, scale, snapshot_options);
    } else {
        assert(options->geometry_type == TRANSFORM_GEOMETRY_NODE_BOUNDING);
        transform->snapshot = snapshot_create_from_node(
            options->buffer ? &options->buffer->node : node, scale, snapshot_options);
    }

    if (!transform->snapshot) {
        transform_do_destroy(transform);
        return NULL;
    }

    uint32_t snapshot_anchors = (options->content_anchors & SNAPSHOT_ANCHOR_TOP) |
                                (options->content_anchors & SNAPSHOT_ANCHOR_LEFT) |
                                (options->content_anchors & SNAPSHOT_ANCHOR_RIGHT) |
                                (options->content_anchors & SNAPSHOT_ANCHOR_BOTTOM);
    snapshot_set_anchors(transform->snapshot, snapshot_anchors);
    return transform;
}

static void transform_handle_add_effect(struct wl_listener *listener, void *data)
{
    struct transform *transform = wl_container_of(listener, transform, add_effect);
    struct scene_add_effect_event *event = data;
    if (!event) {
        return;
    }

    if (event->is_subeffect) {
        event->allow_add = true;
        return;
    }

    struct effect_render_data *render_data = event->render_data;
    if (render_data) {
        render_data->is_valid = true;
        render_data->exist_entity = transform->entity;
        render_data->geometry = transform->current.geometry;
        render_data->alpha = transform->current.alpha;
        render_data->angle = transform->current.angle;
    }

    if (event->effect == transform->effect->effect) {
        event->allow_add = false;
        return;
    }

    event->allow_add = true;
    if (transform->effect->impl->add_new_effect) {
        transform->effect->impl->add_new_effect(transform->effect, transform, event);
        return;
    }

    /* remove exist effect entities */
    effect_entity_destroy(transform->entity);
}

static void transform_effect_node_destroy(struct effect_entity *entity,
                                          struct scene_node **proxy_node)
{
    /* node destroy before effect entity destroy */
    struct transform *transform = entity->user_data;
    /* cleanup listeners for the destroying node */
    wl_list_remove(&transform->add_effect.link);
    wl_list_init(&transform->add_effect.link);
    /**
     * transform on the proxy node,
     * entity will destroy in the node addons finish.
     */
    if (transform->buffer) {
        transform->buffer = NULL;
        return;
    }

    /**
     * if the node is destroyed, it does not need to be placed back under the old parent node.
     * the newly created buffer node will not be placed under the old parent node.
     */
    wl_list_remove(&transform->old_parent_destroy.link);
    wl_list_init(&transform->old_parent_destroy.link);
    transform->old_parent = NULL;

    /* skip if proxy_node already created by higher priority effect */
    if (*proxy_node) {
        /* TODO: add effect on proxy_ndoe */
        return;
    }

    /* create snapshot if not provided */
    if (transform->node->type == SCENE_NODE_RECT) {
        int snapshot_options =
            (transform->effect->is_opengl_renderer ? 0 : SNAPSHOT_RENDER_DISABLE_BLUR);
        transform->snapshot = snapshot_create_from_node(
            transform->node, transform->options.scale <= 0 ? 1.0f : transform->options.scale,
            snapshot_options);
    }

    if (!transform->snapshot) {
        return;
    }

    snapshot_detach(transform->snapshot);

    /**
     * when node destroy, if visible is empty, there is no need to render, not add effect.
     * when node was just created, it was never rendered, visible is also empty, but it needs to
     * render and add effect.
     */
    struct effect_entity *_entity = transform->entity;
    struct node_effect_chain *node_chain = wl_container_of(_entity->slot.chain, node_chain, base);
    if (!pixman_region32_not_empty(&node_chain->visible_region)) {
        return;
    }

    struct view_layer *layer = view_manager_get_layer_by_node(transform->node, true);
    if (!layer) {
        kywc_log(KYWC_ERROR, "Node is not in layer");
        return;
    }

    int node_x = transform->node->x, node_y = transform->node->y;
    struct scene_node *sibing = transform->node;
    struct scene_tree *parent = sibing->parent;
    while (parent != layer->tree) {
        sibing = &parent->node;
        node_x += sibing->x;
        node_y += sibing->y;
        parent = parent->node.parent;
    }

    struct scene_buffer *buffer = snapshot_create_scene_buffer(parent, transform->snapshot);
    if (!buffer) {
        return;
    }

    transform->buffer = buffer;
    scene_node_set_position(&buffer->node, node_x, node_y);
    transform->options.below_view ? scene_node_place_below(&buffer->node, sibing)
                                  : scene_node_place_above(&buffer->node, sibing);
    scene_node_set_input_bypassed(&buffer->node, true);
    snapshot_set_scene_buffer_radius(transform->snapshot, buffer);

    struct effect_entity *new_entity =
        scene_node_add_effect(&buffer->node, transform->effect->effect, NULL);
    if (!new_entity) {
        scene_node_destroy(&buffer->node);
        transform->buffer = NULL;
        return;
    }

    *proxy_node = &buffer->node;
    /**
     * in general, transform is used to modify node. When destroy signal is triggered, the proxy
     * node method is used. transform is used to modify buffer node
     */
    transform->node = &buffer->node;

    transform->entity = new_entity;
    new_entity->user_data = transform;
    transform->references++;
}

void transform_add_destroy_listener(struct transform *transform, struct wl_listener *listener)
{
    wl_signal_add(&transform->events.destroy, listener);
}

struct effect_entity *node_add_custom_transform_effect(struct scene_node *node,
                                                       struct transform_effect *effect,
                                                       struct transform_options *options,
                                                       struct scene_add_effect_event *event)
{
    if (!effect->effect->enabled) {
        return NULL;
    }

    struct effect_entity *entity = scene_node_find_effect_entity(node, effect->effect);
    /* current effect interrupt previous effect, reuse same entity and transform */
    if (entity) {
        struct transform *transform = entity->user_data;
        /* prevent timeout remove effect entity before update function */
        transform->pending_options.start_time = current_time_msec();
        transform->pending_options.end = options->end;
        return entity;
    }

    entity = scene_node_add_effect(node, effect->effect, event);
    if (!entity) {
        return NULL;
    }

    if (event && event->render_data) {
        struct effect_render_data *render_data = event->render_data;
        if (render_data->is_valid) {
            options->start.geometry = render_data->geometry;
            options->start.alpha = render_data->alpha;
            options->start.angle = render_data->angle;
        }
        /**
         * TODO: start geometry is dst_box, not the view_box.
         * some effect should convert it to view_box.
         */
        if ((!kywc_box_equal(&options->start.geometry, &options->end.geometry) &&
             options->animations.geometry == NULL)) {
            options->animations.alpha = animation_manager_get(ANIMATION_TYPE_EASE);
        }
        if ((options->start.alpha != options->end.alpha) && options->animations.alpha == NULL) {
            options->animations.alpha = animation_manager_get(ANIMATION_TYPE_EASE);
        }
        if ((options->start.angle != options->end.angle) && options->animations.angle == NULL) {
            options->animations.angle = animation_manager_get(ANIMATION_TYPE_EASE);
        }
    }

    return entity;
}

struct transform *transform_effect_create_transform(struct transform_effect *effect,
                                                    struct effect_entity *entity,
                                                    struct transform_options *options,
                                                    struct scene_node *node, void *data)
{
    options->duration = effect_manager_scale_time(options->duration);
    struct transform *transform = transform_create(options, effect, node, data);
    if (!transform) {
        return NULL;
    }

    transform->add_effect.notify = transform_handle_add_effect;
    wl_signal_add(&node->events.add_effect, &transform->add_effect);
    transform->entity = entity;
    entity->user_data = transform;

    return transform;
}

struct transform *transform_effect_get_or_create_transform(struct transform_effect *effect,
                                                           struct transform_options *options,
                                                           struct scene_node *node, void *data,
                                                           struct scene_add_effect_event *event)
{
    struct effect_entity *entity = node_add_custom_transform_effect(node, effect, options, event);
    if (!entity) {
        return NULL;
    }

    if (entity->user_data) {
        transform_set_user_data(entity->user_data, data);
        return entity->user_data;
    }

    struct transform *transform =
        transform_effect_create_transform(effect, entity, options, node, data);
    if (!transform) {
        effect_entity_destroy(entity);
        return NULL;
    }

    return transform;
}

bool node_transform_effect_create(struct effect_manager *manager)
{
    uint32_t support_actions = EFFECT_ACTION_TRANSFORM;
    node_transform_effect =
        transform_effect_create(manager, NULL, support_actions, "transform_effect", 98, NULL);
    if (!node_transform_effect) {
        return false;
    }

    return true;
}

bool node_add_transform_effect(struct scene_node *node, struct transform_options *options)
{
    if (!node_transform_effect) {
        return false;
    }

    return transform_effect_get_or_create_transform(node_transform_effect, options, node, NULL,
                                                    NULL);
}
