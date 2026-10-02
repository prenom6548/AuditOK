// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdbool.h>
#include <stdlib.h>
#include <wayland-util.h>

#include "effect/action.h"
#include "server.h"
#include "view/view.h"

static struct action_effect_manager {
    struct wl_list effects;

    struct wl_listener server_destroy;
} *action_effects = NULL;

struct action_effect {
    enum action_effect_type type;
    uint32_t support_stages;
    struct wl_list link;

    const struct action_effect_interface *impl;
    void *data;
};

struct action_effect *action_effect_create(enum action_effect_type type, uint32_t support_stages,
                                           void *data, const struct action_effect_interface *impl)
{
    if (!action_effects) {
        return NULL;
    }

    struct action_effect *effect = calloc(1, sizeof(*effect));
    if (!effect) {
        return NULL;
    }

    effect->impl = impl;
    effect->type = type;
    effect->data = data;
    effect->support_stages = support_stages;

    wl_list_insert(&action_effects->effects, &effect->link);
    return effect;
}

void action_effect_destroy(struct action_effect *effect)
{
    wl_list_remove(&effect->link);
    free(effect);
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&action_effects->server_destroy.link);

    struct action_effect *effect, *tmp;
    wl_list_for_each_safe(effect, tmp, &action_effects->effects, link) {
        wl_list_remove(&effect->link);
        wl_list_init(&effect->link);
    }

    free(action_effects);
    action_effects = NULL;
}

bool action_effect_manager_create(struct server *server)
{
    action_effects = calloc(1, sizeof(*action_effects));
    if (!action_effects) {
        return false;
    }

    wl_list_init(&action_effects->effects);

    action_effects->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(server, &action_effects->server_destroy);
    return true;
}

struct action_effect *action_effect_manager_get_effect(enum action_effect_type type)
{
    struct action_effect *pos;
    wl_list_for_each(pos, &action_effects->effects, link) {
        if (pos->type == type) {
            return pos;
        }
    }

    return NULL;
}

bool action_effect_init_options(struct action_effect *action_effect,
                                struct action_effect_options *options)
{
    if (!action_effect->impl->init_options) {
        return false;
    }

    action_effect->impl->init_options(action_effect, options);
    return true;
}

static void get_action_effect_options(struct action_effect *effect, enum effect_action action,
                                      struct action_effect_options *options,
                                      action_effect_options_adjust_func_t options_adjust,
                                      void *user_data)
{
    action_effect_init_options(effect, options);

    if (!options_adjust) {
        view_manager_adjust_effect_options(action, options, NULL);
        return;
    }

    options_adjust(ACTION_EFFECT_OPTIONS_ADD, action, options, user_data);
    view_manager_adjust_effect_options(action, options, NULL);
    options_adjust(ACTION_EFFECT_OPTIONS_CONFIRM, action, options, user_data);
}

bool node_add_action_effect(struct scene_node *node, enum effect_action action,
                            enum action_effect_type default_type,
                            action_effect_options_adjust_func_t options_adjust, void *user_data)
{
    struct action_effect *effect = action_effect_manager_get_effect(default_type);
    if (!effect || !effect->impl || !effect->impl->add_to_node) {
        return false;
    }

    struct action_effect_options options = { 0 };
    get_action_effect_options(effect, action, &options, options_adjust, user_data);
    struct action_effect *new_effect = action_effect_manager_get_effect(options.effect_type);
    effect = new_effect ? new_effect : effect;

    return effect->impl->add_to_node(effect, node, action, &options);
}

bool view_add_action_effect(struct view *view, enum effect_action action,
                            enum action_effect_type default_type,
                            action_effect_options_adjust_func_t options_adjust, void *user_data)
{
    struct action_effect *effect = action_effect_manager_get_effect(default_type);
    if (!effect || !effect->impl || !effect->impl->add_to_node) {
        return false;
    }

    struct action_effect_options options = { 0 };
    get_action_effect_options(effect, action, &options, options_adjust, user_data);
    struct action_effect *new_effect = action_effect_manager_get_effect(options.effect_type);
    effect = new_effect ? new_effect : effect;

    return effect->impl->add_to_view(effect, view, action, &options);
}
