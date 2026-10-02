// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _EFFECT_TRANSFORM_H_
#define _EFFECT_TRANSFORM_H_

#include "effect/animator.h"

struct transform;
struct transform_effect;
struct effect_manager;

/* valid when the snapshot is created from a view */
enum transform_geometry_type {
    /* treat as VIEW_BOUNDING when content is view */
    TRANSFORM_GEOMETRY_NODE_BOUNDING = 0,
    /* include shadow */
    TRANSFORM_GEOMETRY_VIEW_BOUNDING,
    /* include ssd, except shadow */
    TRANSFORM_GEOMETRY_VIEW_BOX,
    /* only surface */
    TRANSFORM_GEOMETRY_SURFACE,
};

enum transform_content_anchor {
    /* content fills geometry box */
    TRANSFORM_CONTENT_ANCHOR_NONE = 0,
    TRANSFORM_CONTENT_ANCHOR_TOP = 1 << 1,
    TRANSFORM_CONTENT_ANCHOR_LEFT = 1 << 2,
    TRANSFORM_CONTENT_ANCHOR_RIGHT = 1 << 3,
    TRANSFORM_CONTENT_ANCHOR_BOTTOM = 1 << 4,
    /* content fills geometry box */
    TRANSFORM_CONTENT_ANCHOR_ALL = (1 << 5) - 1,
};

struct transform_options {
    enum transform_geometry_type geometry_type;
    struct animation_group animations;
    struct animation_data start, end;
    /* if content anchors is none, content fills the geometry */
    uint32_t content_anchors;
    int64_t start_time;
    int duration;
    /* snapshot scale */
    float scale;
    bool below_view;
    /**
     * the buffer is scene_buffer of transform object(tree_node/view),
     * if the content of transform object only one scene_buffer.
     */
    struct ky_scene_buffer *buffer;
    struct ky_scene_tree *new_parent;
    /* using view create snapshot */
    struct view *view;
};

struct transform_effect_interface {
    void (*update_transform_options)(struct transform_effect *effect, struct transform *transform,
                                     struct ky_scene_node *node, struct transform_options *options,
                                     struct animation_data *current, void *data);
    void (*enable)(struct transform_effect *effect, bool enable, void *data);
    void (*destroy)(struct transform_effect *effect, void *data);
};

struct transform_effect *transform_effect_create(struct effect_manager *manager,
                                                 struct transform_effect_interface *impl,
                                                 uint32_t support_actions, const char *name,
                                                 int priority, void *data);

struct effect_entity *node_add_custom_transform_effect(struct ky_scene_node *node,
                                                       struct transform_effect *effect,
                                                       struct transform_options *options,
                                                       struct ky_scene_add_effect_event *event);

struct transform *transform_effect_create_transform(struct transform_effect *effect,
                                                    struct effect_entity *entity,
                                                    struct transform_options *options,
                                                    struct ky_scene_node *node, void *data);

struct transform *transform_effect_get_or_create_transform(struct transform_effect *effect,
                                                           struct transform_options *options,
                                                           struct ky_scene_node *node, void *data,
                                                           struct ky_scene_add_effect_event *event);

void transform_set_user_data(struct transform *transform, void *data);

void *transform_get_user_data(struct transform *transform);

struct transform_options *transform_get_options(struct transform *transform);

struct animation_data *transform_get_current(struct transform *transform);

void transform_block_source_update(struct transform *transform, bool block);

void transform_destroy(struct transform *transform);

void transform_add_destroy_listener(struct transform *transform, struct wl_listener *listener);

#endif /* _EFFECT_TRANSFORM_H_ */
