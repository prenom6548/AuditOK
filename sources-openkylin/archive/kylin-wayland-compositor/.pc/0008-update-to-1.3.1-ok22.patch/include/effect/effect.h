// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _EFFECT_H_
#define _EFFECT_H_

#include <kywc/boxes.h>

#include "scene/render.h"
#include "scene/scene.h"

struct server;
struct effect;
struct effect_entity;

enum effect_option_type {
    EFFECT_OPTION_TYPE_NULL = 0,
    EFFECT_OPTION_TYPE_BOOLEAN,
    EFFECT_OPTION_TYPE_DOUBLE,
    EFFECT_OPTION_TYPE_INT,
    EFFECT_OPTION_TYPE_STRING,
};

struct effect_option {
    const char *key;
    union {
        int32_t num;
        const char *str;
        double realnum;
        bool boolean;
    } value;
};

struct effect_interface {
    void (*entity_create)(struct effect_entity *entity);
    void (*entity_destroy)(struct effect_entity *entity);
    void (*entity_enable)(struct effect_entity *entity);

    bool (*entity_bounding_box)(struct effect_entity *entity, struct kywc_box *box);
    bool (*entity_clip_region)(struct effect_entity *entity, pixman_region32_t *clip_region);
    bool (*entity_opaque_region)(struct effect_entity *entity, pixman_region32_t *opaque_region);

    bool (*node_push_damage)(struct effect_entity *entity, struct ky_scene_node *damage_node,
                             uint32_t *damage_type, pixman_region32_t *damage);

    bool (*node_render)(struct effect_entity *entity, int lx, int ly,
                        struct ky_scene_render_target *target);

    bool (*frame_render_pre)(struct effect_entity *entity, struct ky_scene_render_target *target);
    /**
     * don't render in render begin.
     * when adding damage, if there is fractional scaling, expand by one more pixel.
     */
    bool (*frame_render_begin)(struct effect_entity *entity, struct ky_scene_render_target *target);
    void (*frame_render)(struct effect_entity *entity, struct ky_scene_render_target *target);
    bool (*frame_render_end)(struct effect_entity *entity, struct ky_scene_render_target *target);
    bool (*frame_render_post)(struct effect_entity *entity, struct ky_scene_render_target *target);

    /* option is saved if return true */
    bool (*configure)(struct effect *effect, const struct effect_option *option);
    bool (*allow_direct_scanout)(struct effect *effect, struct ky_scene_render_target *target);
};

enum effect_category {
    EFFECT_CATEGORY_UNKNOWN = 0,
    EFFECT_CATEGORY_STYLE,
    EFFECT_CATEGORY_ACTION,
    EFFECT_CATEGORY_SCENE,
    EFFECT_CATEGORY_UTILS,
};

enum effect_action {
    EFFECT_ACTION_NONE = 0,
    EFFECT_ACTION_MAP = 1 << 0,
    EFFECT_ACTION_UNMAP = 1 << 1,
    EFFECT_ACTION_MOVE = 1 << 2,
    EFFECT_ACTION_RESIZE = 1 << 3,
    EFFECT_ACTION_MAXIMIZE = 1 << 4,
    EFFECT_ACTION_MAXIMIZE_RESTORE = 1 << 5,
    EFFECT_ACTION_MINIMIZE = 1 << 6,
    EFFECT_ACTION_MINIMIZE_RESTORE = 1 << 7,
    EFFECT_ACTION_TRANSFORM = (1 << 8) - 1,
    EFFECT_ACTION_SHAKE = 1 << 8,
    EFFECT_ACTION_ACTIVATE = 1 << 9,
};

struct effect {
    struct wl_list link;
    struct wl_list entities; // effect_entity->link

    const char *name;
    int priority;
    bool enabled, destroying;
    uint32_t types;

    uint32_t support_actions;
    enum effect_category category;
    const struct effect_interface *impl;
    void *user_data;

    struct {
        struct wl_signal enable;
        struct wl_signal disable;
        struct wl_signal destroy;
    } events;

    struct effect_manager *manager;
    struct json_object *options;
};

struct effect_slot {
    struct wl_list link;
    struct effect_chain *chain;
    struct wl_listener chain_destroy;
};

/**
 * the higher the priority, the larger the priority value.
 * the higher the priority, the wider the impact.
 * the highest priority effect is located at the tail of the effect list.
 * however, high priority effects can choose to block the impact of low priority effects.
 * the order of priority of effect in the effects chain is:
 *  0, 1, 2, 3 ...  n, n+1, n+2
 * the order of obtaining the boundary of a certain special effect is:
 *  n+2,  n+1，n
 * when rendering effect, the rendering order is:
 *  n+2,  n+1，n ... 3, 2, 1, 0
 */
struct effect_chain {
    struct wl_list slots;
    struct {
        struct wl_signal destroy;
    } events;
};

struct effect_entity {
    struct effect_slot slot;
    struct effect_slot frame_slot;

    struct effect *effect;
    struct wl_list effect_link;
    struct wl_listener effect_enable;
    struct wl_listener effect_disable;
    struct wl_listener effect_destroy;

    /* the enablement of effect entity is determined by oneself */
    bool enabled;
    void *user_data;
};

/* to be like buffer node */
struct node_effect_chain {
    struct effect_chain base;

    bool last_enabled;
    /* damage type after last collect_damage */
    uint32_t damage_type;

    pixman_region32_t visible_region;

    struct wlr_addon addon;
    struct ky_scene_node *node;
    struct ky_scene_node_interface impl;
};

struct effect_render_data {
    bool is_valid;
    float alpha, angle;
    struct kywc_box geometry;

    struct effect_entity *exist_entity;
};

bool effect_manager_create(struct server *server);

uint32_t effect_manager_scale_time(uint32_t effect_duration);

bool effect_manager_allow_direct_scanout(struct ky_scene_render_target *target);

struct effect *effect_create(const char *name, int priority, bool enabled,
                             const struct effect_interface *impl, void *data);

void effect_destroy(struct effect *effect);

struct effect *effect_by_name(const char *name);

void effect_set_enabled(struct effect *effect, bool enabled);

void effect_set_option(struct effect *effect, enum effect_option_type type,
                       const struct effect_option *option);

void effect_entity_destroy(struct effect_entity *entity);

void effect_entity_push_damage(struct effect_entity *entity, uint32_t damage_type);

void effect_entity_get_bounding_box(struct effect_entity *entity, enum ky_scene_bounding_type type,
                                    struct kywc_box *geometry);

struct ky_scene_node *ky_scene_node_find_upper_effect_node(struct ky_scene_node *node,
                                                           struct ky_scene_node *final_node);

struct effect_entity *ky_scene_node_find_effect_entity(struct ky_scene_node *node,
                                                       struct effect *effect);

struct effect_entity *ky_scene_node_add_effect(struct ky_scene_node *node, struct effect *effect,
                                               struct ky_scene_add_effect_event *event_data);

struct effect_entity *ky_scene_find_effect_entity(struct ky_scene *scene, struct effect *effect);

struct effect_entity *ky_scene_add_effect(struct ky_scene *scene, struct effect *effec);

void ky_scene_output_render_pre(struct ky_scene_render_target *target);

void ky_scene_output_render_begin(struct ky_scene_render_target *target);

bool ky_scene_output_render(struct ky_scene_render_target *target);

void ky_scene_output_render_end(struct ky_scene_render_target *target);

void ky_scene_output_render_post(struct ky_scene_render_target *target);

#endif /* _EFFECT_H_ */
