// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _EFFECT_ACTION_EFFECT_H_
#define _EFFECT_ACTION_EFFECT_H_

#include "animator.h"
#include "effect.h"

struct view;
struct wlr_surface;
struct action_effect;

enum action_effect_type {
    ACTION_EFFECT_NONE = 0,
    ACTION_EFFECT_FADE,
    ACTION_EFFECT_SLIDE,
};

struct action_effect_options {
    enum action_effect_type effect_type;
    /* the surface who can adjust options by protocols */
    struct wlr_surface *surface;

    struct animation_group animations;
    uint32_t duration;
    /* one type of effect maybe have many style */
    uint32_t style;
    float alpha;

    float width_scale, height_scale;
    int x_offset, y_offset;
    uint32_t location;

    /* thumbnail scale */
    float scale;
    /* for snapshot */
    struct ky_scene_buffer *buffer;
    struct ky_scene_tree *new_parent;
};

enum action_effect_options_step {
    ACTION_EFFECT_OPTIONS_ADD = 0,
    ACTION_EFFECT_OPTIONS_SURFACE,
    ACTION_EFFECT_OPTIONS_CONFIRM,
};

struct action_effect_interface {
    void (*init_options)(struct action_effect *effect, struct action_effect_options *options);
    bool (*add_to_node)(struct action_effect *effect, struct ky_scene_node *node,
                        enum effect_action action, struct action_effect_options *options);
    bool (*add_to_view)(struct action_effect *effect, struct view *view, enum effect_action action,
                        struct action_effect_options *options);
};

typedef void (*action_effect_options_adjust_func_t)(enum action_effect_options_step step,
                                                    enum effect_action action,
                                                    struct action_effect_options *options,
                                                    void *user_data);

bool action_effect_manager_create(struct server *server);

struct action_effect *action_effect_manager_get_effect(enum action_effect_type type);

struct action_effect *action_effect_create(enum action_effect_type type, uint32_t support_stages,
                                           void *data, const struct action_effect_interface *impl);

void action_effect_destroy(struct action_effect *effect);

bool action_effect_init_options(struct action_effect *action_effect,
                                struct action_effect_options *options);

bool node_add_action_effect(struct ky_scene_node *node, enum effect_action action,
                            enum action_effect_type default_type,
                            action_effect_options_adjust_func_t options_adjust, void *user_data);

bool view_add_action_effect(struct view *view, enum effect_action action,
                            enum action_effect_type default_type,
                            action_effect_options_adjust_func_t options_adjust, void *user_data);

#endif /* _EFFECT_ACTION_EFFECT_H_ */
